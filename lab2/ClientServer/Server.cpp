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

void ClientSession(SocketData socketData)
{
    OutputClientState(&socketData, true);

    SOCKET ClientSocket = socketData.socket;
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

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }

    // Not necessary. 
    SetConsoleCtrlHandler(CtrlHandler, true);

    // Accept a client socket
    struct sockaddr socketInfo = {};
    socklen_t sockLen = sizeof(socketInfo);

    SOCKET client_socket = INVALID_SOCKET;
    while ((client_socket = accept(ListenSocket, &socketInfo, &sockLen)))
    {
        // Gets an IP address.
        char ip[INET_ADDRSTRLEN];
        getIP(socketInfo, ip);
        string sip(ip);

        // Creates a new thread for the accepted client (also pass the accepted client socket).
        unsigned threadID;
        SocketData socketData;
        socketData.socket = client_socket;
        socketData.ip = sip;
        socketData.port = serverPort;

        // Creates a new thread and awaits for it.
        thread thread_object(&ClientSession, socketData);
        thread_object.detach();
    }
}