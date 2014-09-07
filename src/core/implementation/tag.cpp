
#include "tag.hpp"

#include <iostream>
#include <algorithm>

#include "algo.hpp"

ITagData::~ITagData()
{

}


std::ostream& operator<<(std::ostream& stream, const ITagData &tagData)
{
    for (std::pair<TagNameInfo, Tag::ValuesSet> tags: tagData.getTags())
    {
        stream << tags.first.getName().toStdString() << ": ";

        Tag::ValuesSet::const_iterator valuesIt = tags.second.cbegin();
        Tag::ValuesSet::const_iterator valuesEnd = tags.second.cend();

        while (valuesIt != valuesEnd)
        {
            stream << valuesIt->value().toStdString();

            ++valuesIt;

            if (valuesIt != valuesEnd)
                stream << tags.first.getSeparator() << " ";
        }

        stream << std::endl;
    }

    return stream;
}

/*****************************************************************************/


TagDataBase::TagDataBase(): ITagData()
{
}


TagDataBase::~TagDataBase()
{
}


void TagDataBase::setTag(const TagNameInfo& name, const TagValueInfo& value)
{
    Tag::ValuesSet values;
    values.insert(value);

    this->setTag(name, values);
}


void TagDataBase::setTags(const Tag::TagsList& tags)
{
    clear();
    for(auto tag: tags)
        setTag(tag.first, tag.second);
}


TagDataBase& TagDataBase::operator=(const TagDataBase& other)
{
    setTags(other.getTags());

	return *this;
}


/*****************************************************************************/


TagData::TagData(): TagDataBase(), m_tags()
{
}


TagData::TagData(const TagData& other): TagDataBase(), m_tags(other.getTags())
{
}


TagData::~TagData()
{

}


Tag::TagsList TagData::getTags() const
{
    return m_tags;
}


void TagData::setTag(const TagNameInfo& name, const Tag::ValuesSet& values)
{
    m_tags[name] = values;
}


void TagData::clear()
{
    m_tags.clear();
}


bool TagData::isValid() const
{
    return true;
}
