/*
**	Author: zerico2005 (2024)
**	Project: 
**	License: MIT License
**	A copy of the MIT License should be included with
**	this project. If not, see https://opensource.org/license/MIT
*/

#include "Common_Def.h"
#include "methods.h"

constexpr size_t maxPatternPreview = 25;
constexpr size_t maximumBufSize = 0x7FFFFF00; // INT32_MAX rounded down
constexpr size_t maximumTestCount = 16777216;
constexpr size_t maximumPatternSize = 16777216;

// Copy pasted since I don't need to reinvent the wheel
void insertionSort(uint64_t arr[], size_t n) {
	if (arr == nullptr) {
		printError("arr is nullptr in insertionSort");
		return;
	}
    size_t i, j;
    uint64_t key;
    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;
        while (j != ~((size_t)0) && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// Assumes list is sorted
fp64 getStandardDeviation(const uint64_t* timeList, size_t testCount) {
	if (timeList == nullptr || testCount <= 1) {
		return 0.0;
	}
	uint64_t sum_squared_diff = 0;
	//uint64_t median = timeList[(uint64_t)((fp64)(testCount - 1) * 0.5)];
	uint64_t mean = 0;
	for (size_t i = 0; i < testCount; i++) {
		mean += timeList[i];
	}
	mean = (uint64_t)((fp64)mean / (fp64)testCount);
    for (size_t i = 0; i < testCount; i++) {
        uint64_t diff_val = (timeList[i] < mean) ? (mean - timeList[i]) : (timeList[i] - mean);
        sum_squared_diff += diff_val * diff_val;
    }
	return (sqrt(sum_squared_diff / (fp64)(testCount - 1))) / 1.0e6;
}

void printTestStatistics(uint64_t totalTime, const uint64_t* testTime, size_t testCount) {
	if (testTime == nullptr) {
		printError("testTime is nullptr");
		return;
	}
	if (testCount == 0) {
		printError("testCount is 0");
		return;
	}
	if (testCount == 1) {
		printf("\n\tTime: %.4fms",(fp64)testTime[0] / 1.0e6);
		fflush(stdout);
		return;
	}
	uint64_t* timeList = (uint64_t*)calloc(testCount,sizeof(uint64_t));
	if (timeList == nullptr) {
		printError("Failed to calloc timeList");
		return;
	}
	memcpy(timeList,testTime,testCount * sizeof(uint64_t));
	insertionSort(timeList,testCount);
	printf("\n\tTotal Time:         %.3lfs",(fp64)totalTime / 1.0e9);
	printf("\n\tAverage Time:       %.4lfms",((fp64)totalTime / (fp64)testCount) / 1.0e6);
	fp64 time_StandardDeviation = getStandardDeviation(timeList,testCount);
	printf("\n\tStandard Deviation: %.4lfms",time_StandardDeviation);
	printf("\n\tStandard Error:     %.4lEms",(time_StandardDeviation / sqrt((fp64)testCount)));
	printf("\n\t     Min :       1%% |      10%% |      50%% |      90%% |      99%% :      Max");
	printf("\n\t%6.3lfms : %6.3lfms | %6.3lfms | %6.3lfms | %6.3lfms | %6.3lfms : %6.3lfms",
		(fp64)timeList[(uint64_t)((fp64)(testCount - 1) * 0.00)] / 1.0e6,
		(fp64)timeList[(uint64_t)((fp64)(testCount - 1) * 0.01)] / 1.0e6,
		(fp64)timeList[(uint64_t)((fp64)(testCount - 1) * 0.10)] / 1.0e6,
		(fp64)timeList[(uint64_t)((fp64)(testCount - 1) * 0.50)] / 1.0e6,
		(fp64)timeList[(uint64_t)((fp64)(testCount - 1) * 0.90)] / 1.0e6,
		(fp64)timeList[(uint64_t)((fp64)(testCount - 1) * 0.99)] / 1.0e6,
		(fp64)timeList[(uint64_t)((fp64)(testCount - 1) * 1.0)] / 1.0e6
	);

	fflush(stdout);
	FREE(timeList);
}

int testMethod(
	const Method_Test* method,
	uint8_t* buf, size_t bufSize, const uint8_t* testData,
	BufferBox box, FILE *file, size_t testCount = 1, uint32_t seed = 123456
) {
	if (method == nullptr) {
		printError("method is nullptr");
		return -1;
	}
	printf("\nMethod %s:",((method->name == nullptr) ? "<nullptr>" : method->name));
	fprintf(file,"\n,%s",((method->name == nullptr) ? "<nullptr>" : method->name));
	if (method->method_ptr == nullptr) {
		printError("method->method_ptr is nullptr");
		return -1;
	}
	size_t patternSize = box.channels;
	uint8_t* patternData = (uint8_t*)calloc(patternSize,sizeof(uint8_t));
	if (patternData == nullptr) {
		printError("Failed to calloc patternData");
		return -1;
	}
	uint64_t* testTime = (uint64_t*)calloc(testCount,sizeof(uint64_t));
	if (testTime == nullptr) {
		FREE(patternData);
		printError("Failed to calloc testTime");
		return -1;
	}
	int retVal = 0;
	uint64_t totalTime = 0;
	for (size_t t = 0; t < testCount; t++) {
		memset(buf,0,bufSize);
		memset(patternData,0,patternSize);
		srand(seed);
		for (size_t i = 0; i < patternSize; i++) {
			patternData[i] = rand() & 0xFF;
		}
		// srand(seed);
		// for (size_t i = 0; (i < patternSize) && (i < bufSize); i++) {
		// 	buf[i] = rand() & 0xFF;
		// }
		{
			uint64_t startTime = getNanoTime();
			retVal = method->method_ptr(buf, bufSize, patternData, patternSize, box);
			uint64_t finishTime = getNanoTime();
			testTime[t] = finishTime - startTime;
			totalTime += testTime[t];
		}
		fprintf(file,",%.6lf",(fp64)testTime[t] / 1.0e6);
		if (retVal < 0) {
			printError("Method returned an error");
			FREE(testTime);
			FREE(patternData);
			return -1;
		}
	}
	if (memcmp(buf,testData,bufSize) != 0) {
		size_t errorCount = 0;
		size_t firstError = ~((size_t)0);
		for (size_t i = 0; i < bufSize; i++) {
			errorCount += (buf[i] != testData[i]) ? 1 : 0;
			if (firstError == ~((size_t)0) && errorCount == 1) {
				firstError = i;
			}
		}
		if ((errorCount <= patternSize * 2) && (patternSize * 2 < bufSize)) {
			printf("\n\tWARNING: Errors detected (%zu) (%.3lf%%) Starting at [%zu] Check for off by 1 errors",errorCount,((fp64)errorCount / (fp64)bufSize) * 100.0,firstError);
		} else {
			printf("\n\tWARNING: Errors detected (%zu) (%.3lf%%) Starting at [%zu]",errorCount,((fp64)errorCount / (fp64)bufSize) * 100.0,firstError);
		}
	}
	printTestStatistics(totalTime,testTime,testCount);
	printf("\n\t");
	size_t testPrintLen = (bufSize < (maxPatternPreview)) ? (bufSize) : (maxPatternPreview);
	for (size_t i = 0; i < testPrintLen; i++) {
		if (i != 0) {
			printf(" ");
		}
		printf("%02X",buf[i]);
	}
	printf("\n");
	fflush(stdout);
	FREE(testTime);
	FREE(patternData);
	return retVal;
}

int main(int argc, char* argv[]) {
	//printf("\nArgument Count: %d",argc);
	if (argc <= 1) {
		printError("\nEmpty Arguments");
		return -1;
	}
	if (argc < 5) {
		printError("\nMissing Arguments");
		return -1;
	}
	BufferBox box;
	size_t testCount = 1;
	uint32_t seed = (uint32_t)(getNanoTime());
    // Use sscanf to extract values
	for (int i = 1; i < argc; i++) {
		const char* input = argv[i];
		sscanf(input, "x%zu", &box.resX);
		sscanf(input, "y%zu", &box.resY);
		sscanf(input, "c%zu", &box.channels);
		sscanf(input, "t%zu", &testCount);
		sscanf(input, "s%u", &seed);
	}

	box.pitch = box.channels * box.resY;

	if (Method_Truth == nullptr) {
		printError("Method_Truth is nullptr");
		return -1;
	}

	if (box.resX == 0 || box.resY == 0 || box.channels == 0 || box.pitch == 0) {
		printError("box has invalid parameters");
		return -1;
	}

	if (testCount < 1) {
		testCount = 1;
	} else if (testCount > maximumTestCount) {
		printf("\nWarning: testCount %zu is extremely large and will be set to %zu",testCount, maximumTestCount);
		testCount = maximumTestCount;
	}
	if (box.channels > maximumPatternSize) {
		printError("channels/pattern size is too large %zu > %zu",box.channels,maximumPatternSize);
		return -1;
	}

	// Generate Data
	size_t bufSize = box.pitch * box.resY;
	if (bufSize >= maximumBufSize) { 
		printError("bufSize is too large %zuMiB > %zuMiB",bufSize / 1048576, maximumBufSize / 1048576);
		return -1;
	}

	uint8_t* buf = (uint8_t*)calloc(bufSize,sizeof(uint8_t));
	if (buf == nullptr) {
		printError("Failed to calloc buf");
		return -1;
	}
	uint8_t* testData = (uint8_t*)calloc(bufSize,sizeof(uint8_t));
	if (testData == nullptr) {
		FREE(buf);
		printError("Failed to calloc testData");
		return -1;
	}
	size_t testPatternSize = box.channels;
	uint8_t* testPatternData = (uint8_t*)calloc(testPatternSize,sizeof(uint8_t));
	if (testPatternData == nullptr) {
		FREE(testData);
		FREE(buf);
		printError("Failed to calloc patternData");
		return -1;
	}

	srand(seed);
	printf("\nSource Pattern:\n\tSeed: 0x%08X\n\t",seed);
	for (size_t i = 0; (i < testPatternSize); i++) {
		testPatternData[i] = rand() & 0xFF;
		if (i < maxPatternPreview) {
			if (i != 0) {
				printf(" ");
			}
			printf("%02X",testPatternData[i]);
		}
	}
	if (testPatternSize > maxPatternPreview) {
		printf(" ...");
	}
	printf("\n");
	fflush(stdout);
	Method_Truth(testData,bufSize,testPatternData,testPatternSize,box);

	#define fileName "./results.csv"
    FILE* file = fopen(fileName, "w");
    if (file == nullptr) {
        printError("Unable to write results to \"%s\". Ensure that other programs are not currently using \"%s\"",fileName,fileName);
		FREE(testPatternData);
		FREE(testData);
		FREE(buf);
        return -1; // Exit with an error code
    }
	fprintf(file,"\n,v%s,%s\n",PROGRAM_NAME,PROGRAM_VERSION);

	size_t method_index = 0;
	while (method_test_list[method_index].method_ptr != NULL) {
	 	int retVal = testMethod(&method_test_list[method_index],buf,bufSize,testData,box,file,testCount,seed);
		if (retVal < 0) {
			fprintf(file,"Pattern-Copy encountered an error");
			fclose(file);
			return retVal;
		}
		method_index++;
	}
	fprintf(file, "\n");
	fclose(file);
	FREE(testPatternData);
	FREE(testData);
	FREE(buf);
	return 0;
}