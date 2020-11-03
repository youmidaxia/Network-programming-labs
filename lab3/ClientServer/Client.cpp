#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <fstream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <thread>
#include "Client.h"
#include "Helpers.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;

constexpr auto DEFAULT_BUFLEN = 512;

Client::Client(string& PORT, string& IP_ADDRESS)
{
    WSADATA wsaData;
    int iResult;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
                   * ptr = NULL,
                     hints;

    // Initializes Winsock.
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolves a server address and port.
    iResult = getaddrinfo(IP_ADDRESS.c_str(), PORT.c_str(), &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {

        // Creates a SOCKET for connecting to a server.
        ConnectSocket = socket(ptr->ai_family,
                               ptr->ai_socktype,
                               ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return;
        }

        // Connects to a server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) 
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return;
    }

    string input;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    string path;

    cout << "Type the path to a file." << endl;
    cin >> path;

    // Reads contents of the file.
    ifstream in(path);
    string contents((istreambuf_iterator<char>(in)),
        istreambuf_iterator<char>());

    // Sends the size of the file contents and then the contents itself.
    string contentsSize = to_string(contents.length());
    send(ConnectSocket, contentsSize.c_str(), contentsSize.length(), 0);
    iResult = send(ConnectSocket, contents.c_str(), contents.length(), 0);
    if (iResult == SOCKET_ERROR)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        printf("The server was shutdown.");
        CorrectClosing(ConnectSocket);
        return;
    }
    else
    {
        // Receives the answer from a server.
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        string actualData(recvbuf, recvbuf + iResult);

        // Nicely displays the answer.
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE);
        cout << "The num of words 5 letters or more in the file: ";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_BLUE);
        cout << actualData << endl << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
}