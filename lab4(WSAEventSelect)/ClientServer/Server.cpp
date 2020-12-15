#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <thread>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include "Server.h"
#include "Helpers.h"

#pragma comment(lib, "Ws2_32.lib")

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
    DWORD BytesSEND;
    DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
BOOL CreateSocketInformation(SOCKET s);
void FreeSocketInformation(DWORD Event);
DWORD EventTotal = 0;

int bytes_read;
char buf[1024];
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
    MSG msg;
    DWORD Ret;
    SOCKADDR _INInternetAddr;
    HWND Window;
    WSADATA wsaData;

    int iResult;

    // Initialize Winsock
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
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol); 
    if (ListenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return;
    }

    if (CreateSocketInformation(ListenSocket) == FALSE)
        printf("CreateSocketInformation() failed!\n");
    else
        printf("CreateSocketInformation() is OK lol!\n");

    if (WSAEventSelect(ListenSocket, EventArray[EventTotal - 1], FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
    {
        printf("WSAEventSelect() failed with error %d\n", WSAGetLastError());
        return; 
    }
    else
        printf("WSAEventSelect() is pretty fine!\n");

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

    DWORD Event;
    WSANETWORKEVENTS NetworkEvents;
    SOCKET Accept;
    LPSOCKET_INFORMATION SocketInfo;
    DWORD Flags;
    DWORD RecvBytes;
    DWORD SendBytes;
    while (TRUE)
    {
        cout << "\n";
        // Wait for one of the sockets to receive I/O notification and
        if ((Event = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
        {
            printf("WSAWaitForMultipleEvents() failed with error %d\n", WSAGetLastError());
            return;
        }
        else
            printf("WSAWaitForMultipleEvents() is pretty damn OK!\n");
        if (WSAEnumNetworkEvents(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket,
            EventArray[Event - WSA_WAIT_EVENT_0], &NetworkEvents) == SOCKET_ERROR)
        {
            printf("WSAEnumNetworkEvents() failed with error %d\n", WSAGetLastError());

            return;
        }
        else
            printf("WSAEnumNetworkEvents() should be fine!\n");
        if (NetworkEvents.lNetworkEvents & FD_ACCEPT)
        {
            if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
            {
                printf("FD_ACCEPT failed with error %d\n",

                    NetworkEvents.iErrorCode[FD_ACCEPT_BIT]);
                break;
            }
            if ((Accept = accept(SocketArray[Event - WSA_WAIT_EVENT_0] -> Socket, NULL, NULL)) == INVALID_SOCKET)
            {
                printf("accept() failed with error %d\n", WSAGetLastError());
                break;
            }
            else
                printf("accept() should be OK!\n");
            if (EventTotal > WSA_MAXIMUM_WAIT_EVENTS)
            {
                printf("Too many connections - closing socket...\n");
                closesocket(Accept);
                break;
            }
            CreateSocketInformation(Accept);

            if (WSAEventSelect(Accept, EventArray[EventTotal - 1], FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
            {
                printf("WSAEventSelect() failed with error %d\n", WSAGetLastError());
                return;
            }
            else
                printf("WSAEventSelect() is OK!\n");
            printf("Socket %d got connected...\n", (int)Accept);
        }
        // Try to read and write data to and from the data buffer if read and write events occur

        if (NetworkEvents.lNetworkEvents & FD_READ || NetworkEvents.lNetworkEvents & FD_WRITE)
        {
            if (NetworkEvents.lNetworkEvents & FD_READ && NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
            {
                printf("FD_READ failed with error %d\n", NetworkEvents.iErrorCode[FD_READ_BIT]);
                break;
            }
            else
                printf("FD_READ is OK!\n");
            if (NetworkEvents.lNetworkEvents & FD_WRITE && NetworkEvents.iErrorCode[FD_WRITE_BIT] != 0)
            {
                printf("FD_WRITE failed with error %d\n",

                    NetworkEvents.iErrorCode[FD_WRITE_BIT]);
                break;
            }
            else
                printf("FD_WRITE is OK!\n");
            SocketInfo = SocketArray[Event - WSA_WAIT_EVENT_0];
            // Read data only if the receive buffer is empty
            if (SocketInfo->BytesRECV == 0)
            {
                SocketInfo->DataBuf.buf = SocketInfo->Buffer;
                SocketInfo->DataBuf.len = DEFAULT_BUFLEN;
                Flags = 0;
                if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        printf("WSARecv() failed with error %d\n", WSAGetLastError());

                        FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
                        return;
                    }
                }
                else
                {
                    printf("WSARecv() is working!\n");
                    cout << "bytes_read = " << RecvBytes << endl;
                    SocketInfo->BytesRECV = RecvBytes;  
                    // Отправляем данные обратно клиенту
                    // Receive until the peer shuts down the connection.

                    string actualData(SocketInfo->DataBuf.buf, SocketInfo->DataBuf.buf + SocketInfo->BytesRECV);

                    if (fileSize == 0)
                    {
                        fileSize = stoi(actualData);
                        SocketInfo->BytesRECV = 0;
                        continue;
                    }

                    string fileData;
                    fileData.append(actualData);

                    fileSize -= RecvBytes;
                    if (fileSize == 0)
                    {
                        // A file receiving is complete.

                        // Writes file contents in binary to a new file.
                        fstream myfile(filename, ios::binary | ios::out | ios::trunc);
                        myfile.write(fileData.c_str(), fileData.length());
                        myfile.close();
                        myfile.open(filename, ios::in);

                        string word;
                        int numOfWordsGTEQFiveLetters = 0;
                        // Reads the file word by word.
                        while (myfile >> word)
                        {
                            if (word.length() >= 5)
                            {
                                numOfWordsGTEQFiveLetters++;
                            }
                        }
                        string valueToSend = to_string(numOfWordsGTEQFiveLetters);

                        SocketInfo->DataBuf.buf = (char*)valueToSend.c_str();
                        SocketInfo->DataBuf.len = valueToSend.length();
                        SendBytes = valueToSend.length();
                        if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
                        {
                            if (WSAGetLastError() != WSAEWOULDBLOCK)
                            {
                                printf("WSASend() failed with error %d\n", WSAGetLastError());

                                FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
                                return;
                            }
                            // A WSAEWOULDBLOCK error has occurred. An FD_WRITE event will be posted
                            // when more buffer space becomes available
                        }
                        else
                        {
                            printf("WSASend() is fine! Thank you...\n");
                            SocketInfo->BytesRECV = 0;
                        }
                        myfile.close();
                        fileData.clear();
                    }
                }
            }
        }
        if (NetworkEvents.lNetworkEvents & FD_CLOSE)
        {
            if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
            {
                printf("FD_CLOSE failed with error %d\n",

                    NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
                break;
            }
            else
                printf("FD_CLOSE is OK!\n");
            printf("Closing socket information %d\n", SocketArray[Event - WSA_WAIT_EVENT_0]->Socket);

            FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
        }
    }
    return;
}

BOOL CreateSocketInformation(SOCKET s)
{
    LPSOCKET_INFORMATION SI;   
    if ((EventArray[EventTotal] = WSACreateEvent()) == WSA_INVALID_EVENT)

    {
        printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
        return FALSE;
    }
    else
        printf("WSACreateEvent() is OK!\n");
    if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION)))
        == NULL)
    {
        printf("GlobalAlloc() failed with error %d\n", GetLastError());
        return FALSE;
    }
    else
        printf("GlobalAlloc() for LPSOCKET_INFORMATION is OK!\n");
    // PrepareSocketInfo structure for use
    SI->Socket = s;
    SI->BytesSEND = 0;
    SI->BytesRECV = 0;
    SocketArray[EventTotal] = SI;
    EventTotal++;
    return(TRUE);
}

void FreeSocketInformation(DWORD Event)
{
    LPSOCKET_INFORMATION SI = SocketArray[Event];
    DWORD i;
    closesocket(SI->Socket);
    GlobalFree(SI);
    if (WSACloseEvent(EventArray[Event]) == TRUE)
        printf("WSACloseEvent() is OK!\n\n");
    else
        printf("WSACloseEvent() failed miserably!\n\n");
    // Squash the socket and event arrays
    for (i = Event; i < EventTotal; i++)
    {
        EventArray[i] = EventArray[i + 1];
        SocketArray[i] = SocketArray[i + 1];
    }
    EventTotal--;
}