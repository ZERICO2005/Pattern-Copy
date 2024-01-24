/*
**	Author: zerico2005 (2024)
**	Project: 
**	License: MIT License
**	A copy of the MIT License should be included with
**	this project. If not, see https://opensource.org/license/MIT
*/

#ifndef METHODS_H
#define METHODS_H

#include "Common_Def.h"

/*
**	Assume that BufferBox buf and uint8_t* vram are set and NOT nullptr
**	The first bytes of the array are set to the pattern, but you may also copy from the pattern array
** -1 <= Error, 0 == Success, 1 >= Warning
*/

/*
**	uint8_t* buf is used to write the pattern to
**	size_t Size is the total size of the buffer
**	uint8_t* PatternData holds the source Pattern
**	size_t PatternSuze is the size/length of PatternData
**	BufferBox box contains ResX, ResY, Channels, Pitch etc.
*/

struct BufferBox {
	size_t resX;
	size_t resY;
	size_t channels;
	size_t pitch;
}; typedef struct BufferBox BufferBox;

// If it helps you
#define Method_Parameters uint8_t* buf, size_t bufSize, const uint8_t* PatternData, size_t PatternSize, BufferBox box

typedef int (*Method_Ptr)(Method_Parameters);

struct Method_Test {
	Method_Ptr method_ptr;
	const char* name;
}; typedef struct Method_Test Method_Test;

extern const Method_Test method_test_list[];

extern const Method_Ptr Method_Truth;


#endif /* METHODS_H */