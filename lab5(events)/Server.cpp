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
SOCKET ClientSocket = INVALID_SOCKET;

constexpr auto TIMEOUT_SEC = 150;
constexpr auto TIMEOUT_MSEC = 0;

#define WM_SOCKET (WM_USER + 1)

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
    CHAR AcceptBuffer[2 * (sizeof(SOCKADDR_IN) + 16)];
    DWORD Ret;
    SOCKADDR _INInternetAddr;
    HWND Window;
    WSADATA wsaData;
    WSAOVERLAPPED ListenOverlapped;
    DWORD RecvBytes, SendBytes;
    DWORD dwBytes;
    DWORD Index;
    DWORD Flags;
    DWORD Bytes;
    DWORD BytesTransferred;
    LPSOCKET_INFORMATION SI;

    // Initialize Winsock
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return;
    }

    SOCKET ClientSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    iResult = getaddrinfo(NULL, serverPort.c_str(), &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return;
    }

    // Create a SOCKET for the server to listen for client connections
    if ((ListenSocket = WSASocket(result->ai_family, result->ai_socktype, result->ai_protocol, NULL, 0,
        WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        printf("Failed to get a socket %d\n", WSAGetLastError());
        return;
    }
    else
        printf("WSASocket() is OK!\n");

    /*InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(PORT);*/

    // Setup the TCP listening socket
    // Without a double colon behind the bind function it won't work. It means that it will use the bind func from the global namespace.
    iResult = ::bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }
    //freeaddrinfo(result); 

    auto CtrlHandler = [](DWORD fdwCtrlType)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        closesocket(ListenSocket);
        WSACleanup();
        return FALSE;
    };
    SetConsoleCtrlHandler(CtrlHandler, true);
        
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }

    // Setup the listening socket for connections
    SOCKET AcceptSocket;
    if ((AcceptSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
        WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        printf("Failed to get a socket %d\n", WSAGetLastError());
        return;
    }
    else
        printf("WSASocket() is OK!\n");

    ZeroMemory(&ListenOverlapped, sizeof(OVERLAPPED));  
    if ((EventArray[0] = ListenOverlapped.hEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
        return;
    }
    else
        printf("WSACreateEvent() is OK!\n");
    EventTotal = 1;

    if (AcceptEx(ListenSocket, AcceptSocket, (PVOID)AcceptBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &Bytes, &ListenOverlapped) == FALSE)
    {
        if (WSAGetLastError() != ERROR_IO_PENDING)
        {
            printf("AcceptEx() failed with error %d\n", WSAGetLastError());
            return;
        }
        else
            printf("AcceptEx() is OK!\n");
    }

    // Process asynchronous AcceptEx, WSASend, WSARecv requests
    while (TRUE)
    {
        if ((Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
        {
            printf("WSAWaitForMultipleEvents()failed %d\n", WSAGetLastError());
            return;
        }
        else
            printf("\nWSAWaitForMultipleEvents() is OK!\n");
        // If the event triggered was zero then a connection attempt was made
        // on our listening socket
        if ((Index - WSA_WAIT_EVENT_0) == 0)
        {
            // Check the returns from the overlapped I/O operation on the listening socket
            if (WSAGetOverlappedResult(ListenSocket, &(ListenOverlapped), &BytesTransferred, FALSE, &Flags) == FALSE)
            {
                printf("WSAGetOverlappedResult() failed with error %d\n", WSAGetLastError());
                return;
            }
            else
                printf("WSAGetOverlappedResult() is OK!\n");
            printf("Socket %d got connected...\n", (int)AcceptSocket);
            if (EventTotal > WSA_MAXIMUM_WAIT_EVENTS)
            {
                printf("Too many connections - closing socket.\n");
                closesocket(AcceptSocket);
                continue;
            }
            else
            {   
                // Create a socket information structure to associate with the accepted socket
                if ((SocketArray[EventTotal] = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
                {
                         printf("GlobalAlloc() failed with error %d\n", GetLastError());
                    return;
                }
                else
                    printf("GlobalAlloc() for LPSOCKET_INFORMATION is OK!\n");

                // Fill in the details of our accepted socket
                SocketArray[EventTotal]->Socket = AcceptSocket;
                ZeroMemory(&(SocketArray[EventTotal]->Overlapped), sizeof(OVERLAPPED));
                SocketArray[EventTotal]->BytesSEND = 0;
                SocketArray[EventTotal]->BytesRECV = 0;
                SocketArray[EventTotal]->DataBuf.len = DEFAULT_BUFLEN;
                SocketArray[EventTotal]->DataBuf.buf = SocketArray[EventTotal]->Buffer;
                if ((SocketArray[EventTotal]->Overlapped.hEvent = EventArray[EventTotal] = WSACreateEvent()) == WSA_INVALID_EVENT)
                {
                    printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
                    return;
                }
                else
                    printf("WSACreateEvent() is OK!\n");
                // Post a WSARecv request to to begin receiving data on the socket
                if (WSARecv(SocketArray[EventTotal]->Socket, &(SocketArray[EventTotal]->DataBuf), 1, &RecvBytes, &Flags, &(SocketArray[EventTotal]->Overlapped), NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != ERROR_IO_PENDING)
                    {
                        printf("WSARecv() failed with error %d\n", WSAGetLastError());
                        return;
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
                    EventTotal++;
                }
                // Make a new socket for accepting future connections and post another AcceptEx call
                if ((AcceptSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
                {
                    printf("Failed to get a socket %d\n", WSAGetLastError());
                    return;
                }
                else
                    printf("WSASocket() is OK!\n");
                WSAResetEvent(EventArray[0]);
                ZeroMemory(&ListenOverlapped, sizeof(OVERLAPPED));
                ListenOverlapped.hEvent = EventArray[0];
                if (AcceptEx(ListenSocket, AcceptSocket, (PVOID)AcceptBuffer, 0,
                    sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &Bytes, &ListenOverlapped) == FALSE)
                {
                    if (WSAGetLastError() != ERROR_IO_PENDING)
                    {
                        printf("AcceptEx() failed with error %d\n", WSAGetLastError());
                        return;
                    }
                }
                else
                    printf("AcceptEx() is OK!\n");
                continue;
            }
            
            SI = SocketArray[Index - WSA_WAIT_EVENT_0];
            WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
            if (WSAGetOverlappedResult(SI->Socket, &(SI->Overlapped), &BytesTransferred, FALSE, &Flags) == FALSE)
            {
                printf("WSAGetOverlappedResult() failed with error %d\n", WSAGetLastError());
                return;
            }
            else
                printf("WSAGetOverlappedResult() is OK!\n");
            // First check to see if the peer has closed the connection and if so
            // then close the socket and cleanup the SOCKET_INFORMATION structure
            // associated with the socket
            if (BytesTransferred == 0)
            {
                printf("Closing socket %d\n", (int)SI->Socket);
                if (closesocket(SI->Socket) == SOCKET_ERROR)
                {
                    printf("closesocket() failed with error %d\n", WSAGetLastError());
                }
                else
                    printf("closesocket() is OK!\n");
                GlobalFree(SI);
                WSACloseEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
                // Cleanup SocketArray and EventArray by removing the socket event handle
                // and socket information structure if they are not at the end of the arrays
                if ((Index - WSA_WAIT_EVENT_0) + 1 != EventTotal)
                    for (int i = Index - WSA_WAIT_EVENT_0; i < EventTotal; i++)
                    {
                        EventArray[i] = EventArray[i + 1];
                        SocketArray[i] = SocketArray[i + 1];
                    }
                EventTotal--;
                continue;
            }
            // Check to see if the BytesRECV field equals zero. If this is so, then
            // this means a WSARecv call just completed so update the BytesRECV field
            // with the BytesTransferred value from the completed WSARecv() call

            if (SI->BytesRECV == 0)
            {
                SI->BytesRECV = BytesTransferred;
                SI->BytesSEND = 0;
            }
            else
            {
                SI->BytesSEND += BytesTransferred;
            }

            if (SI->BytesRECV > SI->BytesSEND)
            {
                // Post another WSASend() request
                // Since WSASend() is not guaranteed to send all of the bytes requested,
                // continue posting WSASend() calls until all received bytes are sent
                ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
                SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];
                SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
                SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;

                if (WSASend(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0, &(SI->Overlapped), NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != ERROR_IO_PENDING)
                    {
                        printf("WSASend() failed with error %d\n", WSAGetLastError());
                        return;
                    }
                }
                else
                    printf("WSASend() is OK!\n");
            }
            else
            {
                SI->BytesRECV = 0;
                // Now that there are no more bytes to send post another WSARecv() request
                Flags = 0;
                ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
                SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];
                SI->DataBuf.len = DEFAULT_BUFLEN;
                SI->DataBuf.buf = SI->Buffer;
                if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags, &(SI->Overlapped), NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != ERROR_IO_PENDING)
                    {
                        printf("WSARecv() failed with error %d\n", WSAGetLastError());
                        return;
                    }
                }
                else
                    printf("WSARecv() is OK!\n");
            }
        }
    }
}