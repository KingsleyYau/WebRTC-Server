/*
 * CompileFuncHeader.h
 *
 *  Created on: 2023年6月20日
 *      Author: max
 */

#ifndef INCLUDE_COMPILEFUNCHEADER_H_
#define INCLUDE_COMPILEFUNCHEADER_H_

#include <stdint.h>
#include <string>
#include <sstream>
using namespace std;

constexpr uint64_t prime = 0x100000001B3ull;
constexpr uint64_t basis = 0xCBF29CE484222325ull;

#define HASH_INIT \
constexpr uint64_t hash_compile_time(const char* str, uint64_t last = basis) { \
	return *str?hash_compile_time(str+1, (*str ^ last) * prime):last; \
} \
constexpr uint64_t operator "" _hash_compile_time(const char* str, size_t size) { \
	return hash_compile_time(str, basis); \
} \
uint64_t hash_run_time(const string& str) { \
	uint64_t ret{basis}; \
	string::const_iterator it = str.begin(); \
	while (it != str.end()) { \
		ret ^= *it; \
		ret *= prime; \
		it++; \
	} \
	return ret; \
}

#endif /* INCLUDE_COMPILEFUNCHEADER_H_ */
