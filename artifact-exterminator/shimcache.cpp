#include <Windows.h>
#include <stdio.h>
#include "shimcache.h"

// Magic value used by Windows 8.1
#define WIN81_MAGIC "10ts"

/*
 * Given an array of ShimcacheEntry indexes, check if the given value is within range of any of the entries.
 * @param int value             The value to check for
 * @param size_t arr_size       The size of the array of ShimcacheEntry structs
 * @param ShimcacheEntry* arr   Array of ShimcacheEntry structs
 * @param int* arrIndexFound    Pointer to variable that receives the index at which shimcache entry the value falls under.
 *                              If function returns FALSE, ptrIndexFound will have its value set to -1
*/
BOOL valueInRange(int value, size_t arr_size, ShimcacheEntry* arr, int* ptrIndexFound)
{
	if(ptrIndexFound != NULL)
		*ptrIndexFound = -1;
	for (int i = 0; i < arr_size; i++)
	{
		if (value >= arr[i].startIndex && value <= arr[i].endIndex)
		{
			if (ptrIndexFound != NULL)
				*ptrIndexFound = i;
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * Remove shimcache entries that contain the specified executable name.
 */
void removeShimcache(LPCWSTR executableName)
{
	// Get shimcache value
	HKEY hKey;
	RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\AppCompatCache", &hKey);

	// Get size of data
	DWORD cbData = 0;
	RegQueryValueExW(hKey, L"AppCompatCache", NULL, NULL, NULL, &cbData);

	// Allocate buffer to hold shimcache data
	BYTE* shimcacheData = (BYTE*)malloc(cbData);

	RegGetValueW(hKey, NULL, L"AppCompatCache", RRF_RT_REG_BINARY, NULL, shimcacheData, &cbData);

	// Prepare array of ShimcacheEntry structs to keep track of entries to be removed.
	// Max records stored in shimcache is 1024.
	unsigned int numMatches = 0;
	ShimcacheEntry shimcacheEntryIndexes[1024];

	size_t lenMagic = strlen(WIN81_MAGIC);
	size_t lenExecutableName = wcslen(executableName);

	// Declare an index outside of the loop so that it is safely changeable,
	// as we may not always need to go in sequence.
	unsigned int loopIndex = 0;

	ShimcacheEntry currentEntryIndexes = { -1, -1 };
	BOOL executableNameFound = FALSE;
	while (TRUE)
	{
		// Check for magic bytes for new shimcache entry.
		// Assume at the start that we found a sequence of magic bytes,
		// and disprove it later.
		BOOL foundMagic = TRUE;
		if (shimcacheData[loopIndex] == WIN81_MAGIC[0])
		{
			for (int i = 1; i < lenMagic; i++)
			{
				if (shimcacheData[loopIndex + i] != WIN81_MAGIC[i])
				{
					foundMagic = FALSE;
					break;
				}
			}
		}
		else
		{
			foundMagic = FALSE;
		}

		// If we found the start of a new shimcache entry, update the currentEntryIndexes variable.
		// If the indexes are not both -1 (means that we were previously processing another shimcache entry),
		// and executableNameFound has been set to TRUE, copy the current currentEntryIndexes into shimcacheEntryIndexes array.
		if (foundMagic)
		{
			// If executable name has been found the previously processed record, add to shimcacheEntryIndexes array.
			if (executableNameFound)
			{
				currentEntryIndexes.endIndex = loopIndex - 1;
				shimcacheEntryIndexes[numMatches] = currentEntryIndexes;
				numMatches++;
			}

			// Reset search variables
			currentEntryIndexes = { 0 };
			currentEntryIndexes.startIndex = loopIndex;
			loopIndex += lenMagic;
			executableNameFound = FALSE;
			continue;
		}

		// If we reached the end of the shimcache data, check if the executable name was found.
		// Similar to what would happen if magic bytes were found.
		if (loopIndex == cbData - 1)
		{
			if (executableNameFound)
			{
				currentEntryIndexes.endIndex = loopIndex;
				shimcacheEntryIndexes[numMatches] = currentEntryIndexes;
				numMatches++;
			}
			// Break out of main loop
			break;
		}

		// If current loop index + length of magic bytes / length of executable name * 2 > num of bytes in shimcache
		// (*2 because stored in unicode format)
		// stop processing
		if (loopIndex + lenMagic >= cbData || loopIndex + lenExecutableName * 2 >= cbData)
			break;
		
		// If magic bytes was not found, there are 2 possible situations:
		// 1. No shimcache entry has been found, so we should just increment the loopIndex
		//    to find a new entry.
		// 2. We are currently processing a shimcache entry, so we should continue finding the specified executable name.
		
		// Only if a shimcache entry has been found (currently processing), and executable name has not been found
		if (currentEntryIndexes.startIndex != -1 && !executableNameFound)
		{
			// Convert specified executable name to bytes. Do a one-to-one comparison
			BYTE* byteArray = (BYTE*)executableName;
			for (int i = 0; i < lenExecutableName * 2; i++)
			{
				if (byteArray[i] != shimcacheData[loopIndex + i])
					break;
				if (i == lenExecutableName * 2 - 1)
					executableNameFound = TRUE;
			}
		}

		loopIndex++;
	}

	// Print indexes to purge
	for (int i = 0; i < (int) numMatches; i++)
	{
		wprintf(L"[DEBUG] Match %d: (%d, %d)\n", i + 1, shimcacheEntryIndexes[i].startIndex, shimcacheEntryIndexes[i].endIndex);
	}

	// Copy byte-by-byte, skipping indexes where executable name was found
	loopIndex = 0;
	int bytesCopied = 0;
	int indexFound = -1;
	BYTE* copyBuf = (BYTE*)malloc(cbData);
	while (loopIndex < cbData)
	{
		// Skip indexes where executable name was found
		if (valueInRange(loopIndex, numMatches, shimcacheEntryIndexes, &indexFound))
		{
			loopIndex = shimcacheEntryIndexes[indexFound].endIndex + 1;
			continue;
		}

		// Copy bytes
		copyBuf[bytesCopied] = shimcacheData[loopIndex];
		bytesCopied++;
		loopIndex++;
	}

	// Set the new buffer as the new data for AppCompatCache value
	RegSetKeyValueW(hKey, NULL, L"AppCompatCache", REG_BINARY, copyBuf, bytesCopied);
	RegCloseKey(hKey);

	free(copyBuf);
	free(shimcacheData);
}