#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <fstream>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Server.h"
#include "Helpers.h"
#include <mswsock.h>

#pragma comment(lib, "Ws2_32.lib")
//#pragma comment (lib, "mswsock.lib")

using namespace std;

constexpr auto DEFAULT_BUFLEN = 512;
const string filename = "fileToProcess.txt";

SOCKET ListenSocket = INVALID_SOCKET;
SOCKET AcceptSocket = INVALID_SOCKET;
WSABUF DataBuf;
char buffer[DEFAULT_BUFLEN];

constexpr auto TIMEOUT_SEC = 150;
constexpr auto TIMEOUT_MSEC = 0;

#define WM_SOCKET (WM_USER + 1)

void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
DWORD WINAPI WorkerThread(LPVOID lpParameter);

typedef struct _SOCKET_INFORMATION{
    CHAR Buffer[DEFAULT_BUFLEN];
    WSABUF DataBuf;
    SOCKET Socket;
    WSAOVERLAPPED Overlapped;
    DWORD BytesSEND;
    DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
DWORD EventTotal = 0;

int fileSize = 0;

void OutputClientState(SOCKET* socket, bool isConnected)
{
    int foreground_color, background_color;
    string state;

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(*socket, (struct sockaddr*)&sin, &len);

    if (isConnected)
    {
        foreground_color = FOREGROUND_GREEN;
        background_color = BACKGROUND_GREEN;
        state = "connected";
    }
    else
    {
        foreground_color = FOREGROUND_RED;
        background_color = BACKGROUND_RED;
        state = "disconnected";
    }

    HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsoleOutput, foreground_color);
    cout << "The client with an IP address ";
    SetConsoleTextAttribute(hConsoleOutput, background_color);
    cout << inet_ntoa(sin.sin_addr);
    SetConsoleTextAttribute(hConsoleOutput, foreground_color);
    cout << " was " << state << " via " << htons(sin.sin_port) << " port." << endl;
    SetConsoleTextAttribute(hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

Server::Server(string& serverPort)
{
    WSADATA wsaData;
    SOCKADDR_IN InternetAddr;
    INT Ret;
    HANDLE ThreadHandle;
    DWORD ThreadId;
    WSAEVENT AcceptEvent;

    if ((Ret = WSAStartup((2, 2), &wsaData)) != 0)
    {
        printf("WSAStartup() failed with error %d\n", Ret);
        WSACleanup();
        return;
    }
    else
        printf("WSAStartup() is OK!\n");

    struct addrinfo* result = NULL, * ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    // Resolve the local address and port to be used by the server
    int iResult = getaddrinfo(NULL, serverPort.c_str(), &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return;
    }

    if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        printf("Failed to get a socket %d\n", WSAGetLastError());
        return;
    }
    else
        printf("WSASocket() is pretty fine!\n");

    auto CtrlHandler = [](DWORD fdwCtrlType)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        closesocket(ListenSocket);
        WSACleanup();
        return FALSE;
    };

    iResult = ::bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }

    if (listen(ListenSocket, 5))
    {
        printf("listen() failed with error %d\n", WSAGetLastError());
        return;
    }
    else
        printf("listen() is OK!\n");

    if ((AcceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
        return;
    }
    else
        printf("WSACreateEvent() is OK!\n");

    // Create a worker thread to service completed I/O requests
    if ((ThreadHandle = CreateThread(NULL, 0,   WorkerThread, (LPVOID)AcceptEvent, 0, &ThreadId)) == NULL)
    {
        printf("CreateThread() failed with error %d\n", GetLastError());
        return;
    }
    else
        printf("CreateThread() should be fine!\n");

    while (TRUE)
    {
        AcceptSocket = accept(ListenSocket, NULL, NULL);
        if (WSASetEvent(AcceptEvent) == FALSE)
        {
            printf("WSASetEvent() failed with error %d\n", WSAGetLastError());
            return;
        }
        else
            printf("\nWSASetEvent() should be working!\n");
    }
}
        
void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    DWORD SendBytes, RecvBytes;
    DWORD Flags;

    if (Error != 0 || BytesTransferred == 0)
    {
        // Either a bad error occurred on the socket or the socket was closed by a peer
        closesocket(AcceptSocket);
        return;
    }
    // At this point, an overlapped WSARecv() request
    // completed successfully. Now we can retrieve the
    // received data that is contained in the variable
    // DataBuf. After processing the received data, we
    // need to post another overlapped WSARecv() or
    // WSASend() request. For simplicity, we will post another WSARecv() request.
    Flags = 0;
    ZeroMemory(&Overlapped, sizeof(WSAOVERLAPPED));
    DataBuf.len = DEFAULT_BUFLEN;
    DataBuf.buf = buffer;
    if (WSARecv(AcceptSocket, &DataBuf, 1, &RecvBytes, &Flags, Overlapped, WorkerRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            printf("WSARecv() failed with error %d\n", WSAGetLastError());
            return;
        }
    }
}

DWORD WINAPI WorkerThread(LPVOID lpParameter)
{
    DWORD Flags;
    LPSOCKET_INFORMATION SocketInfo;
    WSAEVENT EventArray[1];
    DWORD Index;
    DWORD RecvBytes;
    // Save the accept event in the event array
    EventArray[0] = (WSAEVENT)lpParameter;
    while (TRUE)
    {
        // Wait for accept() to signal an event and also process WorkerRoutine() returns
        while (TRUE)
        {
            Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);
            if (Index == WSA_WAIT_FAILED)
            {
                printf("WSAWaitForMultipleEvents() failed with error %d\n", WSAGetLastError());
                return FALSE;
            }
            else
                printf("WSAWaitForMultipleEvents() should be OK!\n");
            if (Index != WAIT_IO_COMPLETION)
            {
                // An accept() call event is ready - break the wait loop    
                break;
            }
        }

        WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
        // Create a socket information structure to associate with the accepted socket
        if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
        {
            printf("GlobalAlloc() failed with error %d\n", GetLastError());
            return FALSE;
        }
        else
            printf("GlobalAlloc() for SOCKET_INFORMATION is OK!\n");
        // Fill in the details of our accepted socket
        SocketInfo->Socket = AcceptSocket;
        ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
        SocketInfo->BytesSEND = 0;
        SocketInfo->BytesRECV = 0;
        SocketInfo->DataBuf.len = DEFAULT_BUFLEN;
        SocketInfo->DataBuf.buf = SocketInfo->Buffer;
        Flags = 0;
        if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
            &(SocketInfo->Overlapped), WorkerRoutine) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                printf("WSARecv() failed with error %d\n", WSAGetLastError());
                return FALSE;
            }
        }
        else
        {
            printf("WSARecv() is OK!\n");

            //cout << "bytes_read = " << RecvBytes << endl;
            //
            //string actualData(SocketArray[EventTotal]->DataBuf.buf, SocketArray[EventTotal]->DataBuf.buf + SocketArray[EventTotal]->BytesRECV);

            //if (fileSize == 0)
            //{
            //    fileSize = stoi(actualData);
            //    SocketArray[EventTotal]->BytesRECV = 0;
            //    continue;
            //}

            //string fileData;
            //fileData.append(actualData);

            //fileSize -= RecvBytes;
            //if (fileSize == 0)
            //{
            //    // A file receiving is complete.

            //    // Writes file contents in binary to a new file.
            //    fstream myfile(filename, ios::binary | ios::out | ios::trunc);
            //    myfile.write(fileData.c_str(), fileData.length());
            //    myfile.close();
            //    myfile.open(filename, ios::in);

            //    string word;
            //    int numOfWordsGTEQFiveLetters = 0;
            //    // Reads the file word by word.
            //    while (myfile >> word)
            //    {
            //        if (word.length() >= 5)
            //        {
            //            numOfWordsGTEQFiveLetters++;
            //        }
            //    }   
            //    string valueToSend = to_string(numOfWordsGTEQFiveLetters);

            //    SocketArray[EventTotal]->DataBuf.buf = (char*)valueToSend.c_str();
            //    SocketArray[EventTotal]->DataBuf.len = valueToSend.length();
            //    SendBytes = valueToSend.length();
            //    if (WSASend(SocketArray[EventTotal]->Socket, &(SocketArray[EventTotal]->DataBuf), 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
            //    {
            //        if (WSAGetLastError() != WSAEWOULDBLOCK)
            //        {
            //            printf("WSASend() failed with error %d\n", WSAGetLastError());
            //            return;
            //        }
            //        // A WSAEWOULDBLOCK error has occurred. An FD_WRITE event will be posted
            //        // when more buffer space becomes available
            //    }
            //    else
            //    {
            //        printf("WSASend() is fine! Thank you...\n");
            //        SocketArray[EventTotal]->BytesRECV = 0;
            //    }
            //    myfile.close();
            //    fileData.clear();
            //} 
        }
        printf("Socket %d got connected...\n", (int)AcceptSocket);
    }
    return TRUE;
}