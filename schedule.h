#pragma once

#include "course.h"
#include "generator.hpp"

#include <bits/stdc++.h>

class Schedule
{
  public:
    Schedule(const std::string&);
    Generator<
        std::vector<std::vector<std::pair<std::string, std::string>>>>
    solve();

  private:
    Course& class2course(unsigned);
    void init_classes(const std::string&);
    void copy_classes();
    bool check_score();
    Generator<
        std::vector<std::vector<std::pair<std::string, std::string>>>>
        BronKerbosch(std::bitset<MAXN>, std::bitset<MAXN>);

    unsigned course_count, class_count, prev_count, succ_count,
        credit_limit, term_limit;
    std::map<std::string, unsigned> course_id, class_id;
    std::vector<Course> courses;
    std::vector<Class> classes;
    std::vector<unsigned> credits;

    unsigned score;
    std::vector<std::vector<std::pair<std::string, std::string>>>
        result;
    std::bitset<MAXN> r, p, x, a; // r is always feasible
    decltype(std::chrono::steady_clock::now()) start;
};