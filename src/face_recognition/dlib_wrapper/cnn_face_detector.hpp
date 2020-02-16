/*
 * Photo Broom - photos management tool.
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

#ifndef CNN_FACE_DETECTION_MODEL_V1_HPP
#define CNN_FACE_DETECTION_MODEL_V1_HPP

#include <string>
#include <vector>
#include <dlib/image_processing/full_object_detection.h>

class QImage;

namespace dlib_api
{
    /**
    * @brief face detection tool
    * Based on https://github.com/davisking/dlib/blob/6b581d91f6b9b847a8163420630ef947e7cc88db/tools/python/src/cnn_face_detector.cpp
    */
    class cnn_face_detection_model_v1
    {

    public:

        explicit cnn_face_detection_model_v1(const std::string& model_filename);
        ~cnn_face_detection_model_v1();

        std::vector<dlib::mmod_rect> detect (
            const QImage& qimage,
            const int upsample_num_times
        ) const;

    private:

        struct data;
        std::unique_ptr<data> m_data;
    };
}

#endif // CNN_FACE_DETECTION_MODEL_V1_H
