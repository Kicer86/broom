/*
 * Photo Broom - photos management tool.
 * Copyright (C) 2019  Michał Walenciak <Kicer86@gmail.com>
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

#include "../series_detector.hpp"

#include <unordered_set>
#include <QDateTime>

#include <core/iexif_reader.hpp>
#include <ibackend.hpp>
#include <iphoto_operator.hpp>


namespace
{
    std::chrono::milliseconds timestamp(const Photo::Data& data)
    {
        qint64 timestamp = -1;

        const auto dateIt = data.tags.find(TagTypes::Date);

        if (dateIt != data.tags.end())
        {
            const QDate date = dateIt->second.getDate();
            const auto timeIt = data.tags.find(TagTypes::Time);
            const QTime time = timeIt != data.tags.end()? timeIt->second.getTime(): QTime();
            const QDateTime dateTime(date, time);

            timestamp = dateTime.toMSecsSinceEpoch();
        }

        return std::chrono::milliseconds(timestamp);
    }

    class BaseGroupValidator
    {
    public:
        void setCurrentPhoto(const Photo::Data& d)
        {
            m_data = d;
        }

        Photo::Data m_data;
    };

    template<Group::Type>
    class GroupValidator;

    template<>
    class GroupValidator<Group::Type::Animation>: BaseGroupValidator
    {
    public:
        GroupValidator(IExifReader& exif, const SeriesDetector::Rules &)
            : m_exifReader(exif)
        {

        }

        void setCurrentPhoto(const Photo::Data& d)
        {
            BaseGroupValidator::setCurrentPhoto(d);
            m_sequence = m_exifReader.get(m_data.path, IExifReader::TagType::SequenceNumber);
        }

        bool canBePartOfGroup()
        {
            const bool has_exif_data = m_sequence.has_value();

            if (has_exif_data)
            {
                const int s = std::any_cast<int>(m_sequence.value());
                auto s_it = m_sequence_numbers.find(s);

                return s_it == m_sequence_numbers.end();
            }
            else
                return false;
        }

        void accept()
        {
            assert(m_sequence);

            const int s = std::any_cast<int>(m_sequence.value());

            m_sequence_numbers.insert(s);
        }

        std::optional<std::any> m_sequence;

        std::unordered_set<int> m_sequence_numbers;
        IExifReader& m_exifReader;
    };

    template<>
    class GroupValidator<Group::Type::HDR>: GroupValidator<Group::Type::Animation>
    {
        typedef GroupValidator<Group::Type::Animation> Base;

    public:
        GroupValidator(IExifReader& exif, const SeriesDetector::Rules& r)
            : Base(exif, r)
        {

        }

        void setCurrentPhoto(const Photo::Data& d)
        {
            Base::setCurrentPhoto(d);
            m_exposure = m_exifReader.get(d.path, IExifReader::TagType::Exposure);
        }

        bool canBePartOfGroup()
        {
            const bool has_exif_data = Base::canBePartOfGroup() && m_exposure;

            if (has_exif_data)
            {
                const int e = std::any_cast<float>(m_exposure.value());

                auto e_it = m_exposures.find(e);

                return e_it == m_exposures.end();
            }
            else
                return false;
        }

        void accept()
        {
            assert(m_exposure);

            Base::accept();

            const int e = std::any_cast<float>(m_exposure.value());
            m_exposures.insert(e);
        }

        std::optional<std::any> m_exposure;
        std::unordered_set<float> m_exposures;
    };

    template<>
    class GroupValidator<Group::Type::Generic>: BaseGroupValidator
    {
    public:
        GroupValidator(IExifReader &, const SeriesDetector::Rules& r)
            : m_prev_stamp(0)
            , m_rules(r)
        {

        }

        void setCurrentPhoto(const Photo::Data& d)
        {
            BaseGroupValidator::setCurrentPhoto(d);
            m_current_stamp = timestamp(d);
        }

        bool canBePartOfGroup()
        {
            return m_prev_stamp.count() == 0 || m_current_stamp - m_prev_stamp <= m_rules.manualSeriesMaxGap;
        }

        void accept()
        {
            m_prev_stamp = m_current_stamp;
        }

        std::chrono::milliseconds m_prev_stamp,
                                  m_current_stamp;
        const SeriesDetector::Rules& m_rules;
    };

    class SeriesTaker
    {
    public:
        SeriesTaker(Database::IBackend& backend,
                    IExifReader& exifReader,
                    const SeriesDetector::Rules& r)
            : m_backend(backend)
            , m_exifReader(exifReader)
            , m_rules(r)
        {

        }

        template<Group::Type type>
        std::vector<SeriesDetector::GroupCandidate> take(std::deque<Photo::Id>& photos) const
        {
            std::vector<SeriesDetector::GroupCandidate> results;

            for (auto it = photos.begin(); it != photos.end();)
            {
                SeriesDetector::GroupCandidate group;
                group.type = type;

                GroupValidator<type> validator(m_exifReader, m_rules);

                for (auto it2 = it; it2 != photos.end(); ++it2)
                {
                    const auto id = *it2;
                    const Photo::Data data = m_backend.getPhoto(id);

                    validator.setCurrentPhoto(data);

                    if (validator.canBePartOfGroup())
                    {
                        group.members.push_back(data);
                        validator.accept();
                    }
                    else
                        break;
                }

                const auto members = group.members.size();

                // each photo should have different exposure
                if (members > 1)
                {
                    results.push_back(group);

                    auto first = it;
                    auto last = first + members;

                    it = photos.erase(first, last);
                }
                else
                    ++it;
            }

            return results;
        }

    private:
        Database::IBackend& m_backend;
        IExifReader& m_exifReader;
        const SeriesDetector::Rules& m_rules;
    };
}


SeriesDetector::Rules::Rules(std::chrono::milliseconds manualSeriesMaxGap)
    : manualSeriesMaxGap(manualSeriesMaxGap)
{

}


SeriesDetector::SeriesDetector(Database::IBackend& backend, IExifReader* exif):
    m_backend(backend), m_exifReader(exif)
{

}


std::vector<SeriesDetector::GroupCandidate> SeriesDetector::listCandidates(const Rules& rules) const
{
    std::vector<GroupCandidate> result;

    // find photos which are not part of any group
    Database::IFilter::Ptr group_filter = std::make_unique<Database::FilterPhotosWithRole>(Database::FilterPhotosWithRole::Role::Regular);
    const auto photos = m_backend.photoOperator().onPhotos( {group_filter}, Database::Actions::SortByTimestamp() );

    // find groups
    auto sequence_groups = analyze_photos(photos, rules);

    return sequence_groups;
}


std::vector<SeriesDetector::GroupCandidate> SeriesDetector::analyze_photos(const std::vector<Photo::Id>& photos,
                                                                           const Rules& rules) const
{
    std::deque<Photo::Id> photos_deq(photos.begin(), photos.end());

    const SeriesTaker taker(m_backend, *m_exifReader, rules);

    auto hdrs = taker.take<Group::Type::HDR>(photos_deq);
    auto animations = taker.take<Group::Type::Animation>(photos_deq);
    auto generics = taker.take<Group::Type::Generic>(photos_deq);

    std::vector<SeriesDetector::GroupCandidate> sequences;
    std::copy(hdrs.begin(), hdrs.end(), std::back_inserter(sequences));
    std::copy(animations.begin(), animations.end(), std::back_inserter(sequences));
    std::copy(generics.begin(), generics.end(), std::back_inserter(sequences));

    return sequences;
}
