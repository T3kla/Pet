#pragma once

#include <iostream>
#include <string.h>

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
LogData<Pair<Begin, const Value &>> operator<<(LogData<Begin> begin, const Value &value)
{
    return {{begin.list, value}};
}

template <typename Begin, size_t n>
LogData<Pair<Begin, const char *>> operator<<(LogData<Begin> begin, const char (&value)[n])
{
    return {{begin.list, value}};
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
#endif
