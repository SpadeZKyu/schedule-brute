#include "schedule.h"

using namespace std;

Schedule::Schedule(const string& data)
    : course_id{}, class_id{}, courses{}, classes{}, credits{},
      score{}, result{}, r{}, p{}, x{}, a{},
      start{chrono::steady_clock::now()}
{
    init_classes(data), copy_classes();
}

Generator<vector<vector<pair<string, string>>>> Schedule::solve()
{
    auto gen = BronKerbosch(p, x);
    while (gen.next())
        co_yield gen.value();
    co_return;
}

Course& Schedule::class2course(unsigned class_id)
{
    return courses[classes[class_id].course_id];
}

void Schedule::init_classes(const string& data)
{
    istringstream iss(data);
    iss >> course_count >> class_count >> prev_count >> succ_count >>
        credit_limit >> term_limit;
    credits.resize(term_limit);
    result.resize(term_limit);

    for (int i = 0; i < course_count; i++)
    {
        string str;
        unsigned term, credit, priority;
        iss >> str >> term >> credit >> priority;
        if (!priority)
            priority = MAXN * MAXN;

        course_id[str] = i;
        courses.push_back({
            str,
            term,
            credit,
            priority,
            {},
            {},
            {},
        });
    }
    for (int i = 0; i < class_count; i++)
    {
        string course_str, class_str;
        array<unsigned, 7> time;
        unsigned week;
        iss >> course_str >> class_str;
        for (unsigned& t : time)
            iss >> t;
        iss >> week;

        class_id[course_str + class_str] = i;
        classes.push_back({
            class_str,
            time,
            week,
            courses[course_id[course_str]].term,
            course_id[course_str],
            courses[course_id[course_str]].priority,
            {},
            {},
            {},
        });
        courses[course_id[course_str]].classes_id.emplace_back(i);
    }
    for (int i = 0; i < prev_count; i++)
    {
        string str, prev_str;
        iss >> str >> prev_str;
        courses[course_id[str]].depend.emplace_back(
            course_id[prev_str]);
        courses[course_id[prev_str]].antidepend.emplace_back(
            course_id[str]);
    }
    for (int i = 0; i < succ_count; i++)
    {
        string str, succ_str;
        iss >> str >> succ_str;
        courses[course_id[str]].extradepend.emplace_back(
            course_id[succ_str]);
    }
    return;
}

void Schedule::copy_classes()
{
    vector<Class> copy_classes = classes;

    for (unsigned i = 0; i < class_count; i++)
        for (unsigned j = 1; j < term_limit; j++)
            if (class2course(i).term + (j << 1) < term_limit)
            {
                courses[classes[i].course_id].classes_id.emplace_back(
                    copy_classes.size());
                copy_classes.emplace_back(classes[i]);
                copy_classes.back().term += j << 1;
            }

    classes.swap(copy_classes);
    class_count = classes.size();

    for (unsigned i = 0; i < class_count; i++)
        for (unsigned j = 0; j < class_count; j++)
            if (class2course(i).str == class2course(j).str ||
                classes[i].check_conflict(classes[j]))
                classes[i].conflict.set(j);

    queue<unsigned> que;
    array<unsigned, MAXN> degree = {};
    array<bitset<MAXN>, MAXN> depend = {};

    for (unsigned i = 0; i < class_count; i++)
        for (unsigned d : class2course(i).depend)
            for (unsigned j : courses[d].classes_id)
                if (classes[j].term < classes[i].term)
                    degree[j]++;

    for (unsigned i = 0; i < class_count; i++)
        classes[i].depends_count = class2course(i).depend.size();

    for (unsigned i = 0; i < class_count; i++)
    {
        if (!degree[i])
            que.emplace(i);
        if (class2course(i).depend.empty())
            a.set(i);
        p.set(i);
    }

    while (!que.empty())
    {
        unsigned u = que.front();
        que.pop(), depend[u].set(u);
        classes[u].priority += depend[u].count();
        for (unsigned d : class2course(u).depend)
            for (unsigned v : courses[d].classes_id)
                if (classes[v].term < classes[u].term)
                {
                    depend[v] |= depend[u];
                    if (!--degree[v])
                        que.emplace(v);
                }
    }

    vector<unsigned> index;
    for (int i = 0; i < class_count; i++)
        index.push_back(i);
    sort(index.begin(), index.end(),
         [this](unsigned i, unsigned j)
         { return classes[i].priority > classes[j].priority; });
    return;
}

bool Schedule::check_score()
{
    unsigned s = 0;
    for (auto i = r._Find_first(); i != MAXN; i = r._Find_next(i))
    {
        s += class2course(i).priority;
        for (auto d : class2course(i).extradepend)
            for (int j : courses[d].classes_id)
                if (r.test(j))
                    s += MAXN;
    }
    if (score < s)
    {
        score = s;
        for (auto& t : result)
            t.clear();
        for (auto i = r._Find_first(); i != MAXN; i = r._Find_next(i))
            result[classes[i].term].emplace_back(class2course(i).str,
                                                 classes[i].str);
        return true;
    }
    return false;
}

Generator<vector<vector<pair<string, string>>>>
Schedule::BronKerbosch(bitset<MAXN> p, bitset<MAXN> x)
{
    if (check_score())
        co_yield result;
    if (p.none() || (chrono::steady_clock::now() - start) > 1s)
        co_return;

    unsigned pivot = MAXN, pvsum = 0;
    bitset<MAXN> avail = p | x;
    for (auto i = avail._Find_first(); i != MAXN;
         i = avail._Find_next(i))
    {
        unsigned sum = 0;
        bitset<MAXN> avail = p & classes[i].conflict & a;
        for (auto i = avail._Find_first(); i != MAXN;
             i = avail._Find_next(i))
            sum += classes[i].priority;
        if (pvsum < sum)
            pivot = i, pvsum = sum;
    }

    avail = p & a;
    if (pivot != MAXN && (avail & classes[pivot].conflict).any())
        avail &= classes[pivot].conflict;
    vector<unsigned> index;
    for (auto i = avail._Find_first(); i != MAXN;
         i = avail._Find_next(i))
        index.emplace_back(i);
    sort(index.begin(), index.end(),
         [this](unsigned i, unsigned j)
         { return classes[i].priority > classes[j].priority; });

    for (unsigned i : index)
    {
        if (credits[classes[i].term] + class2course(i).credit >
            credit_limit)
            continue;

        credits[classes[i].term] += class2course(i).credit;
        for (unsigned d : class2course(i).antidepend)
            for (int j : courses[d].classes_id)
                if (classes[j].term > classes[i].term &&
                    classes[j].del_depend(classes[i].course_id))
                    a.set(j);
        for (unsigned d : class2course(i).extradepend)
            for (int j : courses[d].classes_id)
                if (classes[j].term == classes[i].term + 1)
                    classes[j].priority += MAXN;
        r.set(i);

        auto gen = BronKerbosch(p ^ p & classes[i].conflict,
                                x ^ x & classes[i].conflict);
        while (gen.next())
            co_yield gen.value();

        r.reset(i);
        for (unsigned d : class2course(i).extradepend)
            for (int j : courses[d].classes_id)
                if (classes[j].term == classes[i].term + 1)
                    classes[j].priority -= MAXN;
        for (unsigned d : class2course(i).antidepend)
            for (int j : courses[d].classes_id)
                if (classes[j].term > classes[i].term &&
                    classes[j].add_depend(classes[i].course_id))
                    a.reset(j);
        p.reset(i), x.set(i);
        credits[classes[i].term] -= class2course(i).credit;
    }

    co_return;
}