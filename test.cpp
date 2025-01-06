#include "schedule.h"

#include <bits/stdc++.h>

int main()
{
    std::ifstream ifs("sample.txt");
    std::string str;
    std::getline(ifs, str, char(EOF));

    unsigned cnt = 0;
    Schedule schedule(str);
    auto gen = schedule.solve();
    while (gen.next())
    {
        std::cout << cnt++ << std::endl;
        std::ostringstream oss;
        for (auto& c : gen.value())
        {
            oss << c.size() << '\n';
            for (auto& [s, t] : c)
                oss << s << ' ' << t << '\n';
        }
        std::cout << oss.str() << std::endl;
    }
    return 0;
}