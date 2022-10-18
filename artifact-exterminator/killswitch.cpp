#include <stdio.h>
#include "killswitch.h"

/*
 * A function that blocks until the specified socket sends a "1"
 *
 * @param wchar_t* socketIP      IPv4 address of remote socket
 * @param int      port          Port number of remote socket
 * @param int      pollInterval  Interval in seconds to retry connection to socket
 */
void pollKillSwitch(wchar_t* wstrSocketIP, int port, int pollInterval)
{
    int socketIPBufLen = wcslen(wstrSocketIP) + 1;
    char* socketIP = (char*)malloc(socketIPBufLen);
    WideCharToMultiByte(CP_UTF8, NULL, wstrSocketIP, -1, socketIP, socketIPBufLen, NULL, NULL);
    printf("Kill switch IP: %s, port: %d, pollInterval: %d\n", socketIP, port, pollInterval);

    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"WSAStartup function failed with error: %d\n", iResult);
    }
    // Create a SOCKET for connecting to server
    SOCKET ConnectSocket;
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        wprintf(L"socket function failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
    }
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port of the server to be connected to.
    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(socketIP);
    clientService.sin_port = htons(port);

    //----------------------
    // Connect to server. Keep trying until connected.
    do
    {
		iResult = connect(ConnectSocket, (SOCKADDR *) &clientService, sizeof (clientService));
		if (iResult == SOCKET_ERROR) {
			wprintf(L"connect function failed with error: %ld\n", WSAGetLastError());
		}
        Sleep(pollInterval * 1000);

    } while (iResult != 0);

    wprintf(L"Connected to server.\n");

    // Keep receiving data until the server socket sends "1"
    char recvBuf[2] = "0";
    do
    {
        recvBuf[0] = '\0';
        printf("Waiting for data from socket.\n");
        iResult = recv(ConnectSocket, recvBuf, 1, NULL);

        // Auto-reconnect in case kill switch socket closes
        if (iResult <= 0)
        {
            printf("Connection closed, retrying connection.\n");

		    // Create new socket instance
            closesocket(ConnectSocket);
		    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	        if (ConnectSocket == INVALID_SOCKET) {
                // Shouldn't happen
				wprintf(L"socket function failed with error: %ld\n", WSAGetLastError());
			}
            // Retry connection
			iResult = connect(ConnectSocket, (SOCKADDR *) &clientService, sizeof (clientService));
			if (iResult == SOCKET_ERROR) {
				wprintf(L"connect function failed with error: %ld\n", WSAGetLastError());
				Sleep(pollInterval * 1000);
			}
			continue;
        }
        recvBuf[1] = '\0';
        printf("Received: %s\n", recvBuf);

    } while (recvBuf[0] != '1');

    iResult = closesocket(ConnectSocket);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
        free(socketIP);
        WSACleanup();
        return;
    }

    free(socketIP);
    WSACleanup();
}
