
#include <gmock/gmock.h>

#include <QImage>

#include "unit_tests_utils/mock_athumbnail_generator.hpp"
#include "thumbnail_manager.hpp"


using testing::_;

struct MockResponse
{
    MOCK_METHOD2(result, void(int, QImage));

    void operator()(int h, const QImage& img) { result(h, img); }
};


TEST(ThumbnailManagerTest, constructs)
{
    EXPECT_NO_THROW(
    {
        ThumbnailManager(nullptr);
    });
}


TEST(ThumbnailManagerTest, askGeneratorForThumbnailWhenNoCache)
{
    const QString path = "/some/example/path";
    const int height = 100;

    MockResponse response;
    EXPECT_CALL(response, result(height, _)).Times(1);

    MockThumbnailGenerator generator;
    EXPECT_CALL(generator, run(path, height, _)).Times(1).WillOnce([](const QString &, int h, std::unique_ptr<MockThumbnailGenerator::ICallback> callback)
    {
        QImage img(h, h, QImage::Format_RGB32);
        callback->result(img);
    });

    ThumbnailManager tm(&generator);
    tm.fetch(path, height, response);
}