#pragma once

#include <bits/stdc++.h>

#define MAXN (1 << 11)

struct Course
{
    std::string str;
    unsigned term, credit, priority;
    std::vector<unsigned> depend, antidepend, extradepend, classes_id;
};

struct Class
{
    std::string str;
    std::array<unsigned, 7> time;
    unsigned week, term, course_id;

    unsigned priority;
    std::bitset<MAXN> conflict;

    unsigned depends_count;
    std::array<unsigned, MAXN> depend_count;

    bool check_conflict(const Class& c) const;
    bool add_depend(unsigned d);
    bool del_depend(unsigned d);
};