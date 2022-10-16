#pragma once
#include <Windows.h>
#include "utils.h"

BOOL SetPrivilege(
    HANDLE hToken,
    LPCTSTR lpszPrivilege,
    BOOL bEnablePrivilege
);

BOOL getCommandLineValue(int argc, char* argv[], const char* option, char* buf, size_t bufSize);
