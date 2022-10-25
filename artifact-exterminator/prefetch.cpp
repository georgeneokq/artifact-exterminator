#include "prefetch.h"

/*
 * Removes prefetch file of specified executable name
 * 
 * @param wchar_t* executableName  Executable name to remove prefetch file for
 *
 * @return BOOL  TRUE if prefetch file was found, else FALSE
 */
BOOL clearPrefetch(wchar_t* executableName)
{
    // Capitalize the executable name
    int len = wcslen(executableName);
    wchar_t* capitalizedExecutableName = (wchar_t*)malloc(sizeof(wchar_t) * (len + 1));
    wcsncpy_s(capitalizedExecutableName, len + 1, executableName, len);
    toUpperString(capitalizedExecutableName);

    // Form the path to search for prefetch files in
    wchar_t prefetchFilesPath[MAX_PATH * 2 + 1];
    wsprintf(prefetchFilesPath, L"C:\\Windows\\Prefetch\\%s*", capitalizedExecutableName);

    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFileW(prefetchFilesPath, &data);

    // For some reason, .exe extension of a file is not included in prefetch file name.
    // Remove the extension and try again
    if (hFind == INVALID_HANDLE_VALUE)
    {
        wchar_t* separatorPtr = wcsrchr(capitalizedExecutableName, L'.');
        int separatorPos = separatorPtr - capitalizedExecutableName;
        capitalizedExecutableName[separatorPos] = '\0';
        wsprintf(prefetchFilesPath, L"C:\\Windows\\Prefetch\\%s*", capitalizedExecutableName);
        hFind = FindFirstFileW(prefetchFilesPath, &data);
    }
    
    if ( hFind != INVALID_HANDLE_VALUE )
    {
        if (wcswcs(data.cFileName, capitalizedExecutableName) != NULL)
        {
            WCHAR deleteFilePath[MAX_PATH * 2 + 1];
            wsprintf(deleteFilePath, L"C:\\Windows\\Prefetch\\%s", data.cFileName);
            wprintf(L"[DEBUG] Found prefetch file to delete: %s\n", deleteFilePath);
            if (!DeleteFileW(deleteFilePath))
                wprintf(L"[DEBUG] DeleteFileW on prefetch file failed. Error: %d", GetLastError());
            free(capitalizedExecutableName);
            return TRUE;
        }
        FindClose(hFind);
    }
    free(capitalizedExecutableName);
    return FALSE;
}