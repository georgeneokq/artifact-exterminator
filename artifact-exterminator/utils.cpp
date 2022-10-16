#include "utils.h"
#include <Windows.h>
#include <stdio.h>


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
