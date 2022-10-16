#pragma once
#include <Windows.h>

struct ShimcacheEntry
{
	int startIndex;
	int endIndex;
};

void removeShimcache(LPCWSTR executableName);
BOOL valueInRange(int value, size_t arr_size, ShimcacheEntry* arr, int* arrIndexFound);
