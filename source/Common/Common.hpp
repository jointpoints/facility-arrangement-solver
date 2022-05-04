/**
 * @file Common.hpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_COMMON_HPP__
#define __FASOLVER_COMMON_HPP__





#include <concepts>
#include <cstdint>
#include <memory>
#include <string>
#include <map>





#define FASFLOAT_ULPS 0b100





using FASInteger = uint64_t;
struct FASFloat
{
	long double core;

	FASFloat(void) : core(0) {};
	FASFloat(FASFloat const &value) : core(value.core) {};
	
	template<typename X>
		requires std::floating_point<X> || std::integral<X>
	FASFloat(X const value) : core(value) {};
};
struct FASNone
{
	FASNone(void) {};

	template<typename X>
	FASNone(X &) {};

	template<typename X>
	inline
	FASNone & operator=(X &) {return *this};

	template<typename X>
	inline
	FASNone & operator=(X &&) {return *this};
};





template<typename X>
concept fas_numeric = std::same_as<X, FASInteger> || std::same_as<X, FASFloat>;

template<typename X>
concept fas_numeric_or_none = fas_numeric<X> || std::same_as<X, FASNone>;





template<typename ValueType>
using UnaryMap = std::map<std::string, ValueType>;

template<typename ValueType>
using BinaryMap = std::map<std::string, UnaryMap<ValueType>>;










// Operators for FASFloat










template<typename X>
	requires std::floating_point<X> || std::integral<X> || std::same_as<X, FASFloat>
inline
bool const operator==(FASFloat const &fas_float, X const &other)
{
	return std::abs(fas_float.core - other) <= std::numeric_limits<X>::epsilon * std::abs(fas_float.core + other) * FASFLOAT_ULPS || std::abs(fas_float.core - other) < std::numeric_limits<X>::min();
}

template<>
inline
bool const operator==(FASFloat const &fas_float, FASFloat const &other)
{
	return fas_float == other.core;
}

template<typename X>
	requires std::floating_point<X> || std::integral<X>
inline
bool const operator==(X const &other, FASFloat const &fas_float)
{
	return fas_float == other;
}



template<typename X>
	requires std::floating_point<X> || std::integral<X> || std::same_as<X, FASFloat>
inline
bool const operator<(FASFloat const &fas_float, X const &other)
{
	return fas_float != other && fas_float.core < other;
}

template<>
inline
bool const operator<(FASFloat const& fas_float, FASFloat const &other)
{
	return fas_float < other.core;
}

template<typename X>
	requires std::floating_point<X> || std::integral<X>
inline
bool const operator<(X const &other, FASFloat const &fas_float)
{
	return fas_float != other && other < fas_float.core;
}



template<typename X>
	requires std::floating_point<X> || std::integral<X> || std::same_as<X, FASFloat>
inline
bool const operator>(FASFloat const &fas_float, X const &other)
{
	return fas_float != other && fas_float.core > other;
}

template<>
inline
bool const operator>(FASFloat const& fas_float, FASFloat const &other)
{
	return fas_float > other.core;
}

template<typename X>
	requires std::floating_point<X> || std::integral<X>
inline
bool const operator>(X const &other, FASFloat const &fas_float)
{
	return fas_float != other && other > fas_float.core;
}



#define OP(s)                                                                        \
template<typename X>                                                                 \
    requires std::floating_point<X> || std::integral<X> || std::same_as<X, FASFloat> \
inline                                                                               \
FASFloat & operator s (FASFloat const &fas_float, X const &other)                    \
{                                                                                    \
    return FASFloat(fas_float s other);                                              \
}                                                                                    \
                                                                                     \
template<>                                                                           \
inline                                                                               \
FASFloat & operator s (FASFloat const &fas_float, FASFloat const &other)             \
{                                                                                    \
    return fas_float s other.core;                                                   \
}                                                                                    \
                                                                                     \
template<typename X>                                                                 \
    requires std::floating_point<X> || std::integral<X>                              \
inline                                                                               \
FASFloat & operator s (X const &other, FASFloat const &fas_float)                    \
{                                                                                    \
    return FASFloat(other s fas_float);                                              \
}

OP(+)
OP(-)
OP(*)
OP(/)



#define OP(s)                                                                        \
template<typename X>                                                                 \
    requires std::floating_point<X> || std::integral<X> || std::same_as<X, FASFloat> \
inline                                                                               \
FASFloat & operator s (FASFloat &fas_float, X const &other)                          \
{                                                                                    \
    fas_float.core s other;                                                          \
    return fas_float;                                                                \
}                                                                                    \
                                                                                     \
template<>                                                                           \
inline                                                                               \
FASFloat & operator s (FASFloat &fas_float, FASFloat const &other)                   \
{                                                                                    \
    fas_float s other.core;                                                          \
    return fas_float;                                                                \
}                                                                                    \
                                                                                     \
template<typename X>                                                                 \
    requires std::floating_point<X> || std::integral<X>                              \
inline                                                                               \
X & operator s (X &other, FASFloat const &fas_float)                                 \
{                                                                                    \
    other s fas_float.core;                                                          \
    return other;                                                                    \
}

OP(+=)
OP(-=)
OP(*=)
OP(/=)

#undef OP



template<typename X>
inline
std::basic_ostream<X> & operator<<(std::basic_ostream<X> &stream, FASFloat const &fas_float)
{
	return stream << fas_float.core;
}





#endif // __FASOLVER_COMMON_HPP__
