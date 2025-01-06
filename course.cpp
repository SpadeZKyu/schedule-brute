#include "course.h"

using namespace std;

bool Class::check_conflict(const Class& c) const
{
    if (term != c.term)
        return false;
    if (!(week & c.week))
        return false;
    for (unsigned i = 0; i < 7; i++)
        if (time[i] & c.time[i])
            return true;
    return false;
}

bool Class::add_depend(unsigned d)
{
    depends_count += !--depend_count[d];
    return depends_count;
}

bool Class::del_depend(unsigned d)
{
    depends_count -= !depend_count[d]++;
    return !depends_count;
}