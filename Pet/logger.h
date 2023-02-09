#pragma once

#include <iostream>

namespace Logger
{

struct None
{
};

template <typename First, typename Second> struct Pair
{
    First first;
    Second second;
};

template <typename List> struct LogData
{
    List list;
};

template <typename Begin, typename Value>
LogData<Pair<Begin, const Value &>> operator<<(LogData<Begin> lhs, const Value &rhs)
{
    return {{lhs.list, rhs}};
}

template <typename Begin, size_t n>
LogData<Pair<Begin, const char *>> operator<<(LogData<Begin> lhs, const char (&rhs)[n])
{
    return {{lhs.list, rhs}};
}

inline void PrintList(std::ostream &os, None)
{
}

template <typename Begin, typename Last> void PrintList(std::ostream &os, const Pair<Begin, Last> &data)
{
    PrintList(os, data.first);
    os << data.second;
}

template <typename List> void Log(const char *file, int line, const LogData<List> &data)
{
    std::cout << file << " [" << line << "]: ";
    PrintList(std::cout, data.list);
    std::cout << "\n";
}

} // namespace Logger

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#ifdef _DEBUG
#define LOG(x) (Logger::Log(__FILENAME__, __LINE__, Logger::LogData<Logger::None>() << x))
#else
#define LOG(x)
#endif
