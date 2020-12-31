#pragma once

#include <cstdint>

constexpr std::size_t operator""_B(unsigned long long __nbytes)
{
    return __nbytes;
}

constexpr std::size_t operator"" _KB(unsigned long long __nkbytes)
{
    return __nkbytes * 1024;
}

constexpr std::size_t operator"" _KB(long double __nkbytes)
{
    return static_cast<std::size_t>(__nkbytes * 1024);
}

constexpr std::size_t operator"" _MB(unsigned long long __nmbytes)
{
    return __nmbytes * 1024 * 1024;
}

constexpr std::size_t operator"" _MB(long double __nmbytes)
{
    return static_cast<std::size_t>(__nmbytes * 1_KB * 1_KB);
}

constexpr std::size_t operator"" _GB(unsigned long long __ngbytes)
{
    return __ngbytes * 1024 * 1024 * 1024;
}

constexpr std::size_t operator"" _GB(long double __ngbytes)
{
    return static_cast<std::size_t>(__ngbytes * 1_KB * 1_KB * 1_KB);
}