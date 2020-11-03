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

typedef struct _SOCKET_INFORMATION {
    CHAR Buffer[DEFAULT_BUFLEN];
    WSABUF DataBuf;
    SOCKET Socket;
    OVERLAPPED Overlapped;
    DWORD BytesSEND;
    DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

DWORD TotalSockets = 0;
LPSOCKET_INFORMATION SocketArray[FD_SETSIZE];

SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;

constexpr auto TIMEOUT_SEC = 150;
constexpr auto TIMEOUT_MSEC = 0;

timeval timeout = { TIMEOUT_SEC, TIMEOUT_MSEC };

void OutputClientState(SocketData* socketData, bool isConnected)
{
    int foreground_color, background_color;
    string state;

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
    cout << socketData->ip;
    SetConsoleTextAttribute(hConsoleOutput, foreground_color);
    cout << " was " << state << " via " << socketData->port << " port." << endl;
    SetConsoleTextAttribute(hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

BOOL __stdcall CtrlHandler(DWORD fdwCtrlType)
{
    CorrectClosing(ListenSocket);
    return FALSE;
}

void ClientSession(SocketData socketData, fd_set readset)
{
    OutputClientState(&socketData, true);
    ClientSocket = socketData.socket;

    FD_ZERO(&readset);
    FD_SET(ClientSocket, &readset);

    if (select(1, &readset, NULL, NULL, &timeout) <= 0)
    {
        cout << "Error with SELECT function" << endl;
        return;
    }
    if (!FD_ISSET(ClientSocket, &readset))
    {
        printf("Error. FD_ISSET is false");
        return;
    }

    // ѕоступили данные от клиента, читаем их

    char recvbuf[DEFAULT_BUFLEN];
    string fileData;
    int iResult,
        fileSize = 0,
        recvbuflen = DEFAULT_BUFLEN;

    // Receive until the peer shuts down the connection.
    while (iResult = recv(ClientSocket, recvbuf, recvbuflen, 0))
    {
        if (iResult > 0)
        {
            string actualData(recvbuf, recvbuf + iResult);

            if (fileSize == 0)
            {
                fileSize = stoi(actualData);
                continue;
            }

            fileData.append(actualData);

            fileSize -= iResult;
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
                send(ClientSocket, valueToSend.c_str(), valueToSend.length(), 0);

                myfile.close();
                fileData.clear();
            }
        }
        else
        {
            OutputClientState(&socketData, false);

            shutdown(ClientSocket, SD_BOTH);
            closesocket(ClientSocket);
            return;
        }
    }
}

Server::Server(string& serverPort)
{
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

    // Change the socket mode on the listening socket from blocking to
    // non-block so the application will not block waiting for requests
    u_long NonBlock = 1;
    if (ioctlsocket(ListenSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
    {
        printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
        return;
    }

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
    freeaddrinfo(result);

    // Not necessary. 
    SetConsoleCtrlHandler(CtrlHandler, true);

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }    

    while (1)
    {
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(ListenSocket, &readset);
        
        int SelectTiming = select(1, &readset, NULL, NULL, &timeout);
        switch (SelectTiming)
        {
        case 0:
            // Timed out, do whatever you want to handle this situation
            printf("\nServer: Timeout lor while waiting you retard client!...\n");
            return;
        case -1:
            // Error occurred, more tweaking here and the recvTimeOutTCP()...
            printf("\nServer: Some error encountered with code number : % ld\n", WSAGetLastError());
            return;
        default:
            printf("Select is OK\n");
            break;
        }

        struct sockaddr socketInfo = {};
        socklen_t sockLen = sizeof(socketInfo);
        if (FD_ISSET(ListenSocket, &readset))
        {
            cout << "FD_ISSET(listener, &readset) succesfull" << endl;
            // ѕоступил новый запрос на соединение, используем accept
            ClientSocket = accept(ListenSocket, &socketInfo, &sockLen);
            if (ClientSocket < 0)
            {
                cout << "Error with ACCEPT function" << endl;
            }
            else {
                cout << "ACCEPT succesfull" << endl;
            }
            u_long ul1 = 1;
            int nRet1;
            nRet1 = ioctlsocket(ClientSocket, FIONBIO, &ul1);
        }

        // Gets an IP address.
        char ip[INET_ADDRSTRLEN];
        getIP(socketInfo, ip);
        string sip(ip);

        SocketData socketData;
        socketData.socket = ClientSocket;
        socketData.ip = sip;
        socketData.port = serverPort;

        ClientSession(socketData, readset);
    }
}