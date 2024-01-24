/*
**	Author: zerico2005 (2024)
**	Project: 
**	License: MIT License
**	A copy of the MIT License should be included with
**	this project. If not, see https://opensource.org/license/MIT
*/

#include "Common_Def.h"

uint64_t getNanoTime() { // Returns the time in nanoseconds
	using nanoseconds = std::chrono::duration<std::uint64_t, std::nano>;
	auto now = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<nanoseconds>(now.time_since_epoch()).count();
}

fp64 getDecimalTime() { // Returns the time in seconds
	return (fp64)getNanoTime() / 1.0e9;
}