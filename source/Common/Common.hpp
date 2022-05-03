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





using FASInteger = uint64_t;
using FASFloat = long double;
struct FASNone
{
	template<typename X>
	inline
	FASNone & operator=(X &) {return *this};

	template<typename X>
	inline
	FASNone & operator=(X &&) {return *this};

	FASNone(void) {};

	template<typename X>
	FASNone(X &) {};
};





template<typename X>
concept fas_numeric = std::same_as<X, FASInteger> || std::same_as<X, FASFloat>;

template<typename X>
concept fas_numeric_or_none = fas_numeric<X> || std::same_as<X, FASNone>;





template<typename ValueType>
using UnaryMap = std::map<std::string, ValueType>;

template<typename ValueType>
using BinaryMap = std::map<std::string, UnaryMap<ValueType>>;










// Operators for FASNone










//





#endif // __FASOLVER_COMMON_HPP__
