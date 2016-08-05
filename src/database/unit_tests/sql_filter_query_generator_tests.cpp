
#include <gtest/gtest.h>

#include "sql_filter_query_generator.hpp"

TEST(SqlFilterQueryGeneratorTest, HandlesEmptyList)
{
    Database::SqlFilterQueryGenerator generator;

    std::deque<Database::IFilter::Ptr> filters;
    const QString query = generator.generate(filters);

    EXPECT_EQ("SELECT photos.id AS photos_id FROM photos", query);
}


TEST(SqlFilterQueryGeneratorTest, HandlesFlagsFilter)
{
    Database::SqlFilterQueryGenerator generator;
    std::deque<Database::IFilter::Ptr> filters;

    std::shared_ptr<Database::FilterPhotosWithFlags> filter = std::make_shared<Database::FilterPhotosWithFlags>();
    filters.push_back(filter);

    filter->flags[Photo::FlagsE::ExifLoaded] = 1;
    QString query = generator.generate(filters);
    EXPECT_EQ("SELECT photos.id AS photos_id FROM photos JOIN (flags) ON (flags.photo_id = photos.id) WHERE flags.tags_loaded = '1'", query);

    filter->flags.clear();
    filter->flags[Photo::FlagsE::Sha256Loaded] = 2;
    query = generator.generate(filters);
    EXPECT_EQ("SELECT photos.id AS photos_id FROM photos JOIN (flags) ON (flags.photo_id = photos.id) WHERE flags.sha256_loaded = '2'", query);

    filter->flags.clear();
    filter->flags[Photo::FlagsE::StagingArea] = 3;
    query = generator.generate(filters);
    EXPECT_EQ("SELECT photos.id AS photos_id FROM photos JOIN (flags) ON (flags.photo_id = photos.id) WHERE flags.staging_area = '3'", query);

    filter->flags.clear();
    filter->flags[Photo::FlagsE::ThumbnailLoaded] = 4;
    query = generator.generate(filters);
    EXPECT_EQ("SELECT photos.id AS photos_id FROM photos JOIN (flags) ON (flags.photo_id = photos.id) WHERE flags.thumbnail_loaded = '4'", query);
}


TEST(SqlFilterQueryGeneratorTest, HandlesTagsFilter)
{
    Database::SqlFilterQueryGenerator generator;
    std::deque<Database::IFilter::Ptr> filters;

    std::shared_ptr<Database::FilterPhotosWithTag> filter = std::make_shared<Database::FilterPhotosWithTag>();
    filters.push_back(filter);

    filter->tagName = TagNameInfo(BaseTagsList::Date);
    filter->tagValue = QString("test_value");

    const QString query = generator.generate(filters);

    EXPECT_EQ("SELECT photos.id AS photos_id FROM photos "
              "JOIN (tags) "
              "ON (tags.photo_id = photos.id) "
              "WHERE tags.name = '3' AND tags.value = 'test_value'", query);
}


TEST(SqlFilterQueryGeneratorTest, HandlesFilterNotMatchingFilter)
{
    Database::SqlFilterQueryGenerator generator;
    std::deque<Database::IFilter::Ptr> filters;

    std::shared_ptr<Database::FilterNotMatchingFilter> filter = std::make_shared<Database::FilterNotMatchingFilter>();
    filters.push_back(filter);

    std::shared_ptr<Database::FilterPhotosWithTag> sub_filter1 = std::make_shared<Database::FilterPhotosWithTag>();
    sub_filter1->tagName = TagNameInfo(BaseTagsList::Time);
    filter->filter = sub_filter1;

    const QString query = generator.generate(filters);

    EXPECT_EQ("SELECT photos.id AS photos_id FROM photos "
              "WHERE photos.id NOT IN "
              "(SELECT photos.id AS photos_id FROM photos "
              "JOIN (tags) ON (tags.photo_id = photos.id) "
              "WHERE tags.name = '4')", query);
}


TEST(SqlFilterQueryGeneratorTest, HandlesSha256Filter)
{
    Database::SqlFilterQueryGenerator generator;
    std::deque<Database::IFilter::Ptr> filters;

    std::shared_ptr<Database::FilterPhotosWithSha256> filter = std::make_shared<Database::FilterPhotosWithSha256>();
    filters.push_back(filter);

    filter->sha256 = "1234567890";

    const QString query = generator.generate(filters);

    EXPECT_EQ("SELECT photos.id AS photos_id FROM photos "
              "JOIN (sha256sums) ON (sha256sums.photo_id = photos.id) "
              "WHERE sha256sums.sha256 = '1234567890'", query);
}


TEST(SqlFilterQueryGeneratorTest, HandlesIdFilter)
{
    Database::SqlFilterQueryGenerator generator;
    std::deque<Database::IFilter::Ptr> filters;

    std::shared_ptr<Database::FilterPhotosWithId> filter = std::make_shared<Database::FilterPhotosWithId>();
    filters.push_back(filter);

    filter->filter = Photo::Id(1234567890);

    const QString query = generator.generate(filters);

    EXPECT_EQ("SELECT photos.id AS photos_id FROM photos WHERE photos.id = '1234567890'", query);
}


TEST(SqlFilterQueryGeneratorTest, HandlesSimpleMergesWell)
{
    Database::SqlFilterQueryGenerator generator;
    std::deque<Database::IFilter::Ptr> filters;

    // sha256
    std::shared_ptr<Database::FilterPhotosWithSha256> sha_filter = std::make_shared<Database::FilterPhotosWithSha256>();
    filters.push_back(sha_filter);
    sha_filter->sha256 = "1234567890";

    //tag
    std::shared_ptr<Database::FilterPhotosWithTag> tag_filter = std::make_shared<Database::FilterPhotosWithTag>();
    filters.push_back(tag_filter);
    tag_filter->tagName = TagNameInfo(BaseTagsList::People);
    tag_filter->tagValue = QString("test_value");

    //flags
    std::shared_ptr<Database::FilterPhotosWithFlags> flags_filter = std::make_shared<Database::FilterPhotosWithFlags>();
    filters.push_back(flags_filter);
    flags_filter->flags[Photo::FlagsE::ExifLoaded] = 1;

    const QString query = generator.generate(filters);

    EXPECT_EQ("SELECT photos.id AS photos_id FROM photos "
              "JOIN (tags, flags, sha256sums) "
              "ON (tags.photo_id = photos.id AND flags.photo_id = photos.id AND sha256sums.photo_id = photos.id) "
              "WHERE sha256sums.sha256 = '1234567890' AND tags.name = '5' AND tags.value = 'test_value' AND flags.tags_loaded = '1'", query);
}


TEST(SqlFilterQueryGeneratorTest, HandlesTagFiltersMergingWell)
{
    Database::SqlFilterQueryGenerator generator;
    std::deque<Database::IFilter::Ptr> filters;

    // #1 tag
    std::shared_ptr<Database::FilterPhotosWithTag> tag1_filter = std::make_shared<Database::FilterPhotosWithTag>();
    filters.push_back(tag1_filter);
    tag1_filter->tagName = TagNameInfo(BaseTagsList::Place);
    tag1_filter->tagValue = QString("test_value");

    // #2 tag
    std::shared_ptr<Database::FilterPhotosWithTag> tag2_filter = std::make_shared<Database::FilterPhotosWithTag>();
    filters.push_back(tag2_filter);
    tag2_filter->tagName = TagNameInfo(BaseTagsList::Event);
    tag2_filter->tagValue = QString("test_value2");

    const QString query = generator.generate(filters);

    EXPECT_EQ("SELECT photos_id FROM "
              "( SELECT photos.id AS photos_id FROM photos "
                "JOIN (tags) ON (tags.photo_id = photos.id) "
                "WHERE tags.name = '2' AND tags.value = 'test_value') AS level_1_query "
              "JOIN (tags) ON (tags.photo_id = photos_id) "
              "WHERE tags.name = '1' AND tags.value = 'test_value2'", query);
}


TEST(SqlFilterQueryGeneratorTest, HandlesSimpleOrFilters)
{
    Database::SqlFilterQueryGenerator generator;
    std::deque<Database::IFilter::Ptr> filters;

    std::shared_ptr<Database::FilterPhotosWithFlags> flags = std::make_shared<Database::FilterPhotosWithFlags>();
    flags->flags[Photo::FlagsE::ExifLoaded] = 100;
    flags->flags[Photo::FlagsE::StagingArea] = 200;
    flags->mode = Database::FilterPhotosWithFlags::Mode::Or;

    filters.push_back(flags);

    const QString query = generator.generate(filters);

    EXPECT_EQ("SELECT photos.id AS photos_id FROM photos "
              "JOIN (flags) ON (flags.photo_id = photos.id) "
              "WHERE ( flags.staging_area = '200' OR flags.tags_loaded = '100' )", query);
}


TEST(SqlFilterQueryGeneratorTest, HandlesMergeOfIdFilterWithFlagsOne)
{
    Database::SqlFilterQueryGenerator generator;
    std::deque<Database::IFilter::Ptr> filters;

    std::shared_ptr<Database::FilterPhotosWithFlags> flags = std::make_shared<Database::FilterPhotosWithFlags>();
    flags->flags[Photo::FlagsE::ExifLoaded] = 100;
    flags->flags[Photo::FlagsE::StagingArea] = 200;
    flags->mode = Database::FilterPhotosWithFlags::Mode::Or;

    auto id = std::make_shared<Database::FilterPhotosWithId>();
    id->filter = Photo::Id(1234567890);

    filters.push_back(flags);
    filters.push_back(id);

    const QString query = generator.generate(filters);

    EXPECT_EQ("SELECT photos.id AS photos_id FROM photos "
              "JOIN (flags) ON (flags.photo_id = photos.id) "
              "WHERE ( flags.staging_area = '200' OR flags.tags_loaded = '100' ) AND photos.id = '1234567890'", query);
}
