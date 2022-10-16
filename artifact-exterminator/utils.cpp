#include "utils.h"
#include <Windows.h>
#include <stdio.h>

BOOL SetPrivilege(
    HANDLE hToken,          // access token handle
    LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
    BOOL bEnablePrivilege   // to enable or disable privilege
)
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(
        NULL,            // lookup privilege on local system
        lpszPrivilege,   // privilege to lookup 
        &luid))        // receives LUID of privilege
    {
        wprintf(L"LookupPrivilegeValue error: %u\n", GetLastError());
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // Enable the privilege or disable all privileges.

    if (!AdjustTokenPrivileges(
        hToken,
        FALSE,
        &tp,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES)NULL,
        (PDWORD)NULL))
    {
        printf("AdjustTokenPrivileges error: %u\n", GetLastError());
        return FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        printf("The token does not have the specified privilege. \n");
        return FALSE;
    }

    return TRUE;
}

/*
 * @param int    argc          The argc parameter passed into the main function
 * @param char** argv          The argv parameter passed into the main function
 * @param char*  option        The option to look for
 * @param char*  ptrRecvValue  A buffer that receives the value of the option
 * @param size_t bufSize       Size of buffer
 * 
 * @return BOOL  Returns false if specified option was not found in argument array
*/
BOOL getCommandLineValue(int argc, char* argv[], const char* option, char* buf, size_t bufSize)
{
    for (int i = 0; i < argc - 1; i++)
    {
        char* arg = argv[i];
        // If the option has been found, get the value, which is the next item in the argument array
        if (strcmp(arg, option) == 0)
        {
            strncpy_s(buf, bufSize, argv[i + 1], bufSize - 1);
            return TRUE;
        }
    }
    return FALSE;
}
