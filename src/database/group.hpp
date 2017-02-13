
#ifndef GROUP_HPP
#define GROUP_HPP

#include <core/id.hpp>

namespace Group
{
    typedef Id<int> Id;
}

struct GroupInfo
{
    enum Role
    {
        None,
        Representative,
        Member,
    };

    Group::Id group_id;      // valid if photo is part of group (if valid, role will be != None)
    Role      role;          // photo's role in group (if None, then group_id is expected to be invalid.)

    GroupInfo(): group_id(), role(None) {}
    GroupInfo(const Group::Id& id, Role r): group_id(id), role(r)
    {
        assert( (role == None) ^ group_id.valid() );
    }

    GroupInfo(const GroupInfo &) = default;

    GroupInfo& operator=(const GroupInfo &) = default;
};

#endif