#pragma once
#include <string>
#include <sstream>
#include <type_traits>
#include <stdexcept>

template<typename To, typename Str>
To string_to_number(const Str& s) {
	if constexpr (std::is_floating_point_v<To>)
        return static_cast<To>(std::stod(s));

    else 
        return static_cast<To>(std::stoll(s));
}

template<typename To, typename From>
To castx(const From& v) {
    if constexpr (std::is_same_v<From, To>)
        return v;

    else if constexpr (std::is_arithmetic_v<From> && std::is_arithmetic_v<To>)
        return static_cast<To>(v);

    else if constexpr (std::is_same_v<From, std::string> && std::is_arithmetic_v<To>)
    	return string_to_number<To>(v);

    else if constexpr (std::is_arithmetic_v<From> && std::is_same_v<To, std::string>)
        return std::to_string(v);

    else if constexpr (std::is_convertible_v<From, const char*> && std::is_arithmetic_v<To>)
    	return string_to_number<To>(std::string(v));

    else if constexpr (std::is_convertible_v<From, const char*> && std::is_same_v<To, std::string>)
        return std::string(v);

    else
        static_assert(!sizeof(From*), "castx: unsupported conversion");
}