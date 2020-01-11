/*
 * Interface for face recognition.
 * Copyright (C) 2018  Michał Walenciak <Kicer86@gmail.com>
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


#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "face_recognition.hpp"

#include <cassert>
#include <future>
#include <memory>
#include <string>

#include <QByteArray>
#include <QDirIterator>
#include <QImage>
#include <QFileInfo>
#include <QString>
#include <QRect>
#include <QSaveFile>
#include <QTemporaryFile>

#include <core/icore_factory_accessor.hpp>
#include <core/iexif_reader.hpp>
#include <core/image_tools.hpp>
#include <core/ipython_thread.hpp>
#include <database/filter.hpp>
#include <system/filesystem.hpp>
#include <system/system.hpp>

#include "dlib_wrapper/dlib_face_recognition_api.hpp"


namespace py = pybind11;
using namespace std::placeholders;

namespace
{
    std::mutex g_dlibMutex;   // global mutex for dlib usage.

    QRect tupleToRect(const py::tuple& tuple)
    {
        QRect result;

        std::vector<long> rect;
        const std::size_t coordinates = tuple.size();

        if (coordinates == 4)
        {
            for(std::size_t i = 0; i < coordinates; i++)
            {
                auto part = tuple[i];

                const long n = part.cast<long>();
                rect.push_back(n);
            }

            const int top = rect[0];
            const int right = rect[1];
            const int bottom = rect[2];
            const int left = rect[3];
            const int width = right - left;
            const int height = bottom - top;

            result = QRect(left, top, width, height);
        }

        return result;
    }

    QStringList missingModules()
    {
        QStringList result;

        try
        {
            py::module system_test = py::module::import("system_test");
            py::object missing = system_test.attr("detect_required_modules")();

            auto missing_list = missing.cast<py::list>();

            const std::size_t count = missing_list.size();
            for (std::size_t i = 0; i < count; i++)
            {
                auto item = missing_list[i];

                const std::string missing_module = item.cast<std::string>();
                result.append(missing_module.c_str());
            }
        }
        catch (const std::exception& ex)
        {
            std::cout << ex.what() << std::endl;
        }

        return result;
    }
}


FaceRecognition::FaceRecognition(ICoreFactoryAccessor* coreAccessor):
    m_tmpDir(System::createTmpDir("FaceRecognition", System::Confidential)),
    m_pythonThread(coreAccessor->getPythonThread()),
    m_exif(coreAccessor->getExifReaderFactory()->get())
{

}


FaceRecognition::~FaceRecognition()
{

}


QVector<QRect> FaceRecognition::fetchFaces(const QString& path) const
{
    std::lock_guard lock(g_dlibMutex);
    QVector<QRect> result;

    const QString normalizedPhotoPath = System::getTmpFile(m_tmpDir->path(), "jpeg");
    const bool s = Image::normalize(path, normalizedPhotoPath, m_exif);

    if (s)
    {
        QImage image(normalizedPhotoPath);
        result = dlib_api::face_locations(image, 0, dlib_api::hog);
    }

    return result;
}


QString FaceRecognition::recognize(const QString& path, const QRect& face, const QString& storage) const
{
    std::lock_guard lock(g_dlibMutex);

    QDirIterator di(storage, { "*.jpg" });

    std::vector<dlib_api::FaceEncodings> known_faces;
    std::vector<QString> known_faces_names;

    while(di.hasNext())
    {
        const QString filePath = di.next();
        const QFileInfo fileInfo(filePath);
        const QString fileName = fileInfo.baseName();

        const dlib_api::FaceEncodings faceEncodings = encodingForFace(filePath);

        known_faces.push_back(faceEncodings);
        known_faces_names.push_back(fileInfo.fileName());
    }

    const QString normalizedPhotoPath = System::getTmpFile(m_tmpDir->path(), "jpeg");
    Image::normalize(path, normalizedPhotoPath, m_exif);

    const QImage photo(normalizedPhotoPath);
    const QImage face_photo = photo.copy(face);

    const dlib_api::FaceEncodings unknown_face_encodings = dlib_api::face_encodings(face_photo);
    const std::vector<double> distance = dlib_api::face_distance(known_faces, unknown_face_encodings);

    assert(distance.size() == known_faces_names.size());

    const auto closest_distance = std::min_element(distance.cbegin(), distance.cend());
    const auto pos = std::distance(distance.cbegin(), closest_distance);

    const QString best_face_file = distance[pos] <= 0.6? known_faces_names[pos]: QString();

    return best_face_file;
}


QString FaceRecognition::best(const QStringList& faces)
{
    if (faces.size() < 3)           // we need at least 3 faces to do any serious job
        return {};

    std::map<QString, dlib_api::FaceEncodings> encoded_faces;
    for (const QString& face_path: faces)
    {
        const auto encoded_face = encodingForFace(face_path);
        encoded_faces[face_path] = encoded_face;
    }

    std::map<QString, double> average_distances;
    for(const auto& [face_path, face_encoding]: encoded_faces)
    {
        double total_distance = 0.0;
        int count = 0;

        for(const auto& [face_path2, face_encoding2]: encoded_faces)
        {
            if (face_path == face_path2)
                continue;

            const auto distance = dlib_api::face_distance({face_encoding}, face_encoding2);

            total_distance += distance.front();
            count++;
        }

        if (count > 0)
        {
            const double avg_distance = total_distance / count;
            average_distances[face_path] = avg_distance;
        }
    }

    QString best_photo;
    if (average_distances.empty() == false)
    {
        const auto pos_of_best = std::min_element(average_distances.cbegin(), average_distances.cend(), [](const auto& lhs, const auto& rhs)
        {
            return lhs.second < rhs.second;
        });

        best_photo = pos_of_best->first;
    }

    return best_photo;
}


dlib_api::FaceEncodings FaceRecognition::encodingForFace(const QString& face_image_path) const
{
    dlib_api::FaceEncodings faceEncodings;

    const QFileInfo fileInfo(face_image_path);
    const QString fileName = fileInfo.baseName();
    const QString storage = fileInfo.absolutePath();
    const QString encodingsFilePath = storage + "/" + fileName + ".enc";

    if (QFile::exists(encodingsFilePath))
    {
        QFile encodingsFile(encodingsFilePath);
        encodingsFile.open(QFile::ReadOnly);

        while(encodingsFile.atEnd() == false)
        {
            const QByteArray line = encodingsFile.readLine();
            const double value = line.toDouble();

            faceEncodings.push_back(value);
        }
    }
    else
    {
        const QImage faceImage(face_image_path);
        faceEncodings = dlib_api::face_encodings(faceImage);

        QSaveFile encodingsFile(encodingsFilePath);
        encodingsFile.open(QFile::WriteOnly);

        for(double v: faceEncodings)
        {
            const QByteArray line = QByteArray::number(v) + '\n';
            encodingsFile.write(line);
        }

        encodingsFile.commit();
    }

    return faceEncodings;
}
