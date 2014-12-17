
#include <gtest/gtest.h>

#include <QPixmap>

#include <core/base_tags.hpp>
#include <Qt5/model_view/model_helpers/idx_data_manager.hpp>
#include <Qt5/components/photos_data_model.hpp>

#include "test_helpers/mock_database.hpp"
#include "test_helpers/internal_task_executor.hpp"
#include "test_helpers/mock_photo_info.hpp"


// TODO: tests for IdxDataManager are requiring changes in IdxDataManager (remove dependency to DBDataModel)

namespace
{
    struct DatabaseNotifier: Database::ADatabaseSignals
    {
    };

    struct PhotoInfo: IPhotoInfo
    {

    };
}


TEST(IdxDataManagerShould, BeConstructable)
{
    EXPECT_NO_THROW({
        PhotosDataModel model;
        IdxDataManager manager(&model);
    });
}


/*
TEST(IdxDataManagerShould, AddUniversalNodeOnTopWhenPhotoDoesntMatchOtherTopNodes)
{
    using ::testing::Return;
    using ::testing::ReturnRef;

    //construct objects
    DatabaseNotifier notifier;
    MockDatabase database;
    InternalTaskExecutor executor;
    Hierarchy hierarchy;
    PhotosDataModel model;
    MockPhotoInfo* photoInfo = new MockPhotoInfo;
    MockPhotoInfo::Ptr photoInfoPtr(photoInfo);
    IPhotoInfo::Flags photoFlags;
    IPhotoInfo::Id photoId(1);
    Tag::TagsList photoTags;
    IdxDataManager manager(&model);

    //define expectations
    EXPECT_CALL(database, notifier()).WillRepeatedly(Return(&notifier));
    EXPECT_CALL(*photoInfo, getFlags()).WillRepeatedly(Return(photoFlags));
    EXPECT_CALL(*photoInfo, getID()).WillRepeatedly(Return(photoId));
    EXPECT_CALL(*photoInfo, getTags()).WillRepeatedly(ReturnRef(photoTags));

    //setup data
    hierarchy.levels = { { BaseTags::get(BaseTagsList::Date), Hierarchy::Level::Order::ascending } };
    model.setDatabase(&database);
    model.setHierarchy(hierarchy);
    model.set(&executor);

    //do test
    emit notifier.photoAdded(photoInfoPtr);

    //verification
    IdxData* root = manager.getRoot();

    EXPECT_EQ(1, root->m_children.size());          //one child

    IdxData* node_child = root->m_children[0];      //node child
    EXPECT_EQ(true, node_child->isNode());
    EXPECT_EQ(1, node_child->m_children.size());

    IdxData* photo_child = node_child->m_children[0];
    EXPECT_EQ(true, photo_child->isPhoto());
    EXPECT_EQ(true, photo_child->m_children.empty());
}
*/
