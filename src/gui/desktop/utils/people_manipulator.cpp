/*
 * Copyright (C) 2020  Michał Walenciak <Kicer86@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "people_manipulator.hpp"

#include <QFileInfo>

#include <core/icore_factory_accessor.hpp>
#include <core/iexif_reader.hpp>
#include <core/task_executor_utils.hpp>
#include <database/ibackend.hpp>
#include <face_recognition/face_recognition.hpp>


template<typename T>
struct ExecutorTraits<Database::IDatabase, T>
{
    static void exec(Database::IDatabase* db, T&& t)
    {
        db->exec(std::forward<T>(t));
    }
};


namespace
{
    const QString faces_recognized_flag = QStringLiteral("faces_recognized");

    Person::Fingerprint average_fingerprint(const std::vector<PersonFingerprint>& faces)
    {
        if (faces.empty())
            return {};

        if (faces.size() == 1)
            return faces.front().fingerprint();

        const std::size_t items = faces.front().fingerprint().size();
        const std::size_t faces_count = faces.size();

        Person::Fingerprint avg_face(items, 0.0);

        for(std::size_t i = 0; i < items; i++)
        {
            const double sum = std::accumulate(faces.cbegin(), faces.cend(), 0.0, [i](double s, const auto& v) { return s + v.fingerprint()[i]; });
            const double avg = sum / faces_count;
            avg_face[i] = avg;
        }

        return avg_face;
    }

    auto fetchPeopleAndFingerprints(Database::IDatabase& db)
    {
        typedef std::tuple<std::vector<Person::Fingerprint>, std::vector<Person::Id>> Result;

        return evaluate<Result(Database::IBackend *)>(&db, [](Database::IBackend* backend)
        {
            std::vector<Person::Fingerprint> people_fingerprints;
            std::vector<Person::Id> people;

            const auto all_people = backend->listPeople();
            for(const auto& person: all_people)
            {
                const auto fingerprints = backend->peopleInformationAccessor().fingerprintsFor(person.id());

                if (fingerprints.empty() == false)
                {
                    people_fingerprints.push_back(average_fingerprint(fingerprints));
                    people.push_back(person.id());
                }
            }

            return std::tuple(people_fingerprints, people);
        });
    }

    auto fetchFingerprints(Database::IDatabase& db, const std::vector<PersonInfo::Id>& ids)
    {
        typedef std::map<PersonInfo::Id, PersonFingerprint> Result;

        return evaluate<Result(Database::IBackend *)>
                        (&db, [ids](Database::IBackend* backend)
        {
            const Result result = backend->peopleInformationAccessor().fingerprintsFor(ids);

            return result;
        });
    }

    std::vector<PersonName> fetchPeople(Database::IDatabase& db)
    {
        return evaluate<std::vector<PersonName>(Database::IBackend *)>(&db, [](Database::IBackend* backend)
        {
            auto people = backend->listPeople();

            return people;
        });
    }

    PersonName personData(Database::IDatabase& db, const Person::Id& id)
    {
        const PersonName person = evaluate<PersonName (Database::IBackend *)>
            (&db, [id](Database::IBackend* backend)
        {
            const auto people = backend->person(id);

            return people;
        });

        return person;
    }

    PersonName storeNewPerson(Database::IDatabase& db, const QString& name)
    {
        const PersonName person = evaluate<PersonName (Database::IBackend *)>
                (&db, [name](Database::IBackend* backend)
        {
            const PersonName d(Person::Id(), name);
            const auto id = backend->store(d);
            return PersonName(id, name);
        });

        return person;
    }

    QString pathFor(Database::IDatabase* db, const Photo::Id& id)
    {
        return evaluate<QString(Database::IBackend *)>(db, [id, db](Database::IBackend *)
        {
            Database::IUtils* db_utils = db->utils();
            auto photo = db_utils->getPhotoFor(id);

            return photo->getPath();
        });
    }
}



PeopleManipulator::PeopleManipulator(const Photo::Id& pid, Database::IDatabase& db, ICoreFactoryAccessor& core)
    : m_pid(pid)
    , m_core(core)
    , m_db(db)
{
    qRegisterMetaType<QVector<QRect>>("QVector<QRect>");

    findFaces();
}


PeopleManipulator::~PeopleManipulator()
{
    m_callback_ctrl.invalidate();
}


std::size_t PeopleManipulator::facesCount() const
{
    return m_faces.size();
}


const QString& PeopleManipulator::name(std::size_t n) const
{
    return m_faces[n].person.name();
}


const QRect& PeopleManipulator::position(std::size_t n) const
{
    return m_faces[n].face.rect;
}


void PeopleManipulator::setName(std::size_t n, const QString& name)
{
    const QString trimmed_name = name.trimmed();

    if (n < m_faces.size() && m_faces[n].person.name() != trimmed_name)
    {
        PersonName new_name(trimmed_name);
        m_faces[n].person = new_name;
    }
}


void PeopleManipulator::store()
{
    store_people_names();
    store_fingerprints();

    // update names assigned to face locations
    for (auto& face: m_faces)
        face.face.p_id = face.person.id();

    // update fingerprints assigned to face locations
    for (auto& face: m_faces)
        if (face.face.f_id.valid() == false)
            face.face.f_id = face.fingerprint.id();

    store_people_information();
}


void PeopleManipulator::runOnThread(void (PeopleManipulator::*method)())
{
    auto task = std::bind(method, this);
    auto safe_task = m_callback_ctrl.make_safe_callback<void()>(task);
    auto executor = m_core.getTaskExecutor();

    runOn(executor, safe_task);
}


void PeopleManipulator::findFaces()
{
    runOnThread(&PeopleManipulator::findFaces_thrd);
}


void PeopleManipulator::findFaces_thrd()
{
    QVector<QRect> result;

    const QString path = pathFor(&m_db, m_pid);
    const QFileInfo pathInfo(path);
    const QString full_path = pathInfo.absoluteFilePath();
    m_image = OrientedImage(m_core.getExifReaderFactory()->get(), full_path);

    const std::vector<QRect> list_of_faces = fetchFacesFromDb();

    if (list_of_faces.empty())
    {
        FaceRecognition face_recognition(&m_core);
        const auto faces = face_recognition.fetchFaces(full_path);

        for(const QRect& face: faces)
            result.append(face);
    }
    else
    {
        result.reserve(static_cast<int>(list_of_faces.size()));

        std::copy(list_of_faces.cbegin(), list_of_faces.cend(), std::back_inserter(result));
    }

    invokeMethod(this, &PeopleManipulator::findFaces_result, result);
}


void PeopleManipulator::findFaces_result(const QVector<QRect>& faces)
{
    m_faces.reserve(faces.size());

    std::copy(faces.cbegin(), faces.cend(), std::back_inserter(m_faces));

    for (auto& face: m_faces)
        face.face.ph_id = m_pid;

    recognizeFaces();
}


void PeopleManipulator::recognizeFaces()
{
    runOnThread(&PeopleManipulator::recognizeFaces_thrd);
}


void PeopleManipulator::recognizeFaces_thrd_fetch_from_db()
{
    const std::vector<PersonInfo> peopleData = fetchPeopleFromDb();

    for (FaceInfo& faceInfo: m_faces)
    {
        // check if we have data for given face rect in db
        auto person_it = std::find_if(peopleData.cbegin(), peopleData.cend(), [faceInfo](const PersonInfo& pi)
        {
            return pi.rect == faceInfo.face.rect;
        });

        if (person_it != peopleData.cend())   // rect matches
        {
            if (person_it->p_id.valid())
                faceInfo.person = personData(m_db, person_it->p_id);     // fill name

            faceInfo.face = *person_it;
        }
    }

    // collect faces and try to access theirs fingerprints
    std::vector<PersonInfo::Id> faces_ids;

    for (FaceInfo& faceInfo: m_faces)
        if (faceInfo.face.id.valid())
            faces_ids.push_back(faceInfo.face.id);

    const auto fingerprints = fetchFingerprints(m_db, faces_ids);

    for (FaceInfo& faceInfo: m_faces)
        if (faceInfo.face.id.valid())
        {
            auto it = fingerprints.find(faceInfo.face.id);

            if (it != fingerprints.end())
                faceInfo.fingerprint = it->second;
        }
}


void PeopleManipulator::recognizeFaces_thrd_calculate_missing_fingerprints()
{
    for (FaceInfo& faceInfo: m_faces)
        if (faceInfo.fingerprint.id().valid() == false)
        {
            FaceRecognition face_recognition(&m_core);

            const auto fingerprint = face_recognition.getFingerprint(m_image, faceInfo.face.rect);

            faceInfo.fingerprint = fingerprint;
        }
}


void PeopleManipulator::recognizeFaces_thrd_recognize_people()
{
    FaceRecognition face_recognition(&m_core);
    const auto people_fingerprints = fetchPeopleAndFingerprints(m_db);
    const std::vector<Person::Fingerprint>& known_fingerprints = std::get<0>(people_fingerprints);

    for (FaceInfo& faceInfo: m_faces)
        if (faceInfo.person.name().isEmpty())
        {
            const int pos = face_recognition.recognize(faceInfo.fingerprint.fingerprint(), known_fingerprints);

            if (pos >=0)
            {
                const std::vector<Person::Id>& known_people = std::get<1>(people_fingerprints);
                const Person::Id found_person = known_people[pos];
                faceInfo.person = personData(m_db, found_person);
            }
        }
}


void PeopleManipulator::recognizeFaces_thrd()
{
    recognizeFaces_thrd_fetch_from_db();

    const bool missing_fingerprints = std::any_of(m_faces.cbegin(), m_faces.cend(), [](const auto& face){ return face.fingerprint.id().valid() == false; });

    if (missing_fingerprints)
    {
        recognizeFaces_thrd_calculate_missing_fingerprints();
        recognizeFaces_thrd_recognize_people();
    }

    invokeMethod(this, &PeopleManipulator::recognizeFaces_result);
}


void PeopleManipulator::recognizeFaces_result()
{
    emit facesAnalyzed();
}


void PeopleManipulator::store_people_names()
{
    const std::vector<PersonName> people = fetchPeople(m_db);

    // make sure each name is known (exists in db)
    for (auto& face: m_faces)
        if (face.person.id().valid() == false && face.person.name().isEmpty() == false)  // no id + name set
        {
            const QString& name = face.person.name();

            auto it = std::find_if(people.cbegin(), people.cend(), [name](const PersonName& d)
            {
                return d.name() == name;
            });

            if (it == people.cend())        // new name, store it in db
                face.person = storeNewPerson(m_db, name);
            else
                face.person = *it;
        }
}


void PeopleManipulator::store_fingerprints()
{
    for (auto& face: m_faces)
        if (face.fingerprint.id().valid() == false)
        {
            const PersonFingerprint::Id fid =
                evaluate<PersonFingerprint::Id(Database::IBackend*)>(&m_db, [fingerprint = face.fingerprint](Database::IBackend* backend)
            {
                return backend->peopleInformationAccessor().store(fingerprint);
            });

            const PersonFingerprint fingerprint(fid, face.fingerprint.fingerprint());
            face.fingerprint = fingerprint;
        }
}


void PeopleManipulator::store_people_information()
{
    for (const auto& face: m_faces)
    {
        const PersonInfo& faceInfo = face.face;
        const PersonFingerprint& fingerprint = face.fingerprint;

        m_db.exec([faceInfo, fingerprint](Database::IBackend* backend)
        {
            backend->store(faceInfo);
        });
    }
}


std::vector<QRect> PeopleManipulator::fetchFacesFromDb() const
{
    return evaluate<std::vector<QRect>(Database::IBackend*)>
        (&m_db, [id = m_pid](Database::IBackend* backend)
    {
        std::vector<QRect> faces;

        const auto people = backend->listPeople(id);
        for(const auto& person: people)
            if (person.rect.isValid())
                faces.push_back(person.rect);

        return faces;
    });
}


std::vector<PersonInfo> PeopleManipulator::fetchPeopleFromDb() const
{
    return evaluate<std::vector<PersonInfo>(Database::IBackend *)>
        (&m_db, [id = m_pid](Database::IBackend* backend)
    {
        auto people = backend->listPeople(id);

        return people;
    });
}