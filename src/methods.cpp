/*
**	Author: zerico2005 (2024)
**	Project: 
**	License: MIT License
**	A copy of the MIT License should be included with
**	this project. If not, see https://opensource.org/license/MIT
*/

#include "Common_Def.h"
#include "methods.h"

/*
**	Assume that BufferBox buf and uint8_t* vram are set and NOT nullptr
**	The first bytes of the array are set to the pattern, but you may also copy from the pattern array
** -1 <= Error, 0 == Success, 1 >= Warning
*/

/*
**	uint8_t* buf is used to write the pattern to
**	The function can use the Raw data or the Image data to preform the Pattern Fill
**	Raw data:
**		size_t bufSize is the total size of the buffer
**		uint8_t* PatternData holds the source Pattern
**		size_t PatternSuze is the size/length of PatternData
**	Image data:
**		BufferBox box contains ResX, ResY, Channels, Pitch etc.
*/

int method_Memset(uint8_t* buf, size_t bufSize, const uint8_t* PatternData, size_t PatternSize, BufferBox box) {
	if (buf == nullptr) { return -1; }
	if (bufSize == 0) { return -1; }
	memset(buf,0,bufSize);
	return 0;
}

// Copies Pattern Data one byte at a time
int method_Simple(uint8_t* buf, size_t bufSize, const uint8_t* PatternData, size_t PatternSize, BufferBox box) {
	if (buf == nullptr || PatternData == nullptr) { return -1; }
	if (bufSize == 0 || PatternSize == 0) { return -1; }
	size_t p = 0;
	for (size_t i = 0; i < (bufSize); i++) {
		buf[i] = PatternData[p];
		p++;
		p = (p == PatternSize) ? 0 : p;
	}
	return 0;
}

// Copies a from a few indexes backwards to repeat the pattern
int method_Backwards(uint8_t* buf, size_t bufSize, const uint8_t* PatternData, size_t PatternSize, BufferBox box) {
	if (buf == nullptr || PatternData == nullptr) { return -1; }
	if (bufSize == 0 || PatternSize == 0) { return -1; }
	if (bufSize <= PatternSize) {
		memcpy(buf,PatternData,bufSize);
		return 0;
	}
	memcpy(buf,PatternData,PatternSize); // Initial Copy
	for (size_t i = PatternSize; i < (bufSize); i++) {
		buf[i] = buf[i - PatternSize];
	}
	return 0;
}

// Copies Pattern Data one memcpy at a time
int method_SimpleMemcpy(uint8_t* buf, size_t bufSize, const uint8_t* PatternData, size_t PatternSize, BufferBox box) {
	if (buf == nullptr || PatternData == nullptr) { return -1; }
	if (bufSize == 0 || PatternSize == 0) { return -1; }
	uint8_t* bufPtr = buf;
	size_t i = 0;
	for (; i < (bufSize); i += PatternSize) {
		memcpy(bufPtr,PatternData,PatternSize);
		bufPtr += PatternSize;
	}
	memcpy(buf,PatternData,i - bufSize);
	return 0;
}

// Doubles the size it copies each iteration
int method_BinaryMemcpy(uint8_t* buf, size_t bufSize, const uint8_t* PatternData, size_t PatternSize, BufferBox box) {
	if (buf == nullptr || PatternData == nullptr) { return -1; }
	if (bufSize == 0 || PatternSize == 0) { return -1; }
	if (bufSize <= PatternSize) {
		memcpy(buf,PatternData,bufSize);
		return 0;
	}
	memcpy(buf,PatternData,PatternSize); // Initial Copy
	size_t len = PatternSize;
	size_t pos = PatternSize;
	
	while (pos + len <= bufSize) {
		memcpy(buf + pos,buf,len); 
		pos += len;
		len *= 2; // Doubles copy size each iteration
	}
	memcpy(buf + pos,buf,bufSize - len); // Copies the remaining portion
	return 0;
}

// Mimics the LDIR eZ80 Assembly Instruction
int method_LDIR(uint8_t* buf, size_t bufSize, const uint8_t* PatternData, size_t PatternSize, BufferBox box) {
	if (buf == nullptr || PatternData == nullptr) { return -1; }
	if (bufSize == 0 || PatternSize == 0) { return -1; }
	if (bufSize <= PatternSize) {
		memcpy(buf,PatternData,bufSize);
		return 0;
	}
	memcpy(buf,PatternData,PatternSize); // Initial Copy
	uint8_t* HL = buf;
	uint8_t* DE = buf + PatternSize;
	size_t BC = (bufSize) - PatternSize;
	// LDIR
	while (BC != 0) {
		*DE = *HL;
		HL++;
		DE++;
		BC--;
	}
	return 0;
}

// Add the methods you want to test to this list
const Method_Test method_test_list[] = {
	//{method_Memset,"Memset"},
	{method_Simple,"Simple"},
	{method_Backwards,"Backwards"},
	{method_SimpleMemcpy,"Simple Memcpy"},
	{method_BinaryMemcpy,"Binary Memcpy"},
	{method_LDIR,"eZ80 LDIR"},
	{NULL,NULL} // End of List
};

// Used to verify the output of each method
const Method_Ptr Method_Truth = method_Simple;