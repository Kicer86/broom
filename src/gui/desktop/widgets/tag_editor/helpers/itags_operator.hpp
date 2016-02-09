

#ifndef ITAGS_OPEARTOR_HPP
#define ITAGS_OPEARTOR_HPP

#include <core/tag.hpp>
#include <database/iphoto_info.hpp>

struct ITagsOperator
{
    virtual ~ITagsOperator();

    virtual void operateOn(const std::vector<IPhotoInfo::Ptr> &) = 0;

    virtual Tag::TagsList getTags() const = 0;

    virtual void setTag(const TagNameInfo &, const TagValue &) = 0;
    virtual void setTag(const TagNameInfo &, const QVariant &) = 0;
    virtual void setTags(const Tag::TagsList &) = 0;
    virtual void updateTag(const QString& name, const QString& rawList) = 0;
    virtual void updateTag(const QString& name, const QVariant& rawList) = 0;
};


#endif // ITAGS_OPEARTOR_HPP
