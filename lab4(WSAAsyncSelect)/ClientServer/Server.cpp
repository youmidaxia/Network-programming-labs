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
HWND MakeWorkerWindow(void);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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

void ClientSession(SOCKET ClientSocket)
{
    // Поступили данные от клиента, читаем их

    char recvbuf[DEFAULT_BUFLEN];
    string fileData;
    int iResult,
        recvbuflen = DEFAULT_BUFLEN;

    // Receive until the peer shuts down the connection.
    iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);


    string actualData(recvbuf, recvbuf + iResult);

    if (fileSize == 0)
    {
        fileSize = stoi(actualData);
        return;
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


Server::Server(string& serverPort)
{
    MSG msg;
    DWORD Ret;
    SOCKET Listen;
    SOCKADDR _INInternetAddr;
    HWND Window;
    WSADATA wsaData;

    if ((Window = MakeWorkerWindow()) == NULL)
    {
        printf("MakeWorkerWindow() failed!\n");
        return;
    }
    else
        printf("MakeWorkerWindow() is OK!\n");

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

    if (WSAAsyncSelect(ListenSocket, Window, WM_SOCKET, FD_ACCEPT | FD_CLOSE) == 0)
        printf("WSAAsyncSelect() is OK lol!\n");
    else
        printf("WSAAsyncSelect() failed with error code %d\n", WSAGetLastError());

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

    while (Ret = GetMessage(&msg, NULL, ListenSocket, 0))
    {
        if (Ret == -1)
        {
            printf("\nGetMessage() failed with error %d\n", GetLastError());
            return;
        }
        else
            printf("\nGet a message...\n");

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    char recvbuf[DEFAULT_BUFLEN];
    string fileData;
    int iResult,
        fileSize = 0,
        recvbuflen = DEFAULT_BUFLEN;

    if (uMsg == WM_SOCKET)
    {
        if (WSAGETSELECTERROR(lParam))
        {
            printf("Socket failed with error %d\n", WSAGETSELECTERROR(lParam));
        }
        else
        {
            //printf("Socket looks fine!\n");
            switch (WSAGETSELECTEVENT(lParam))
            {
            case FD_ACCEPT:
                if ((ClientSocket = accept(wParam, NULL, NULL)) == INVALID_SOCKET)
                {
                    printf("accept() failed with error %d\n", WSAGetLastError());
                    break;
                }
                else
                    printf("accept() is OK!\n");

                printf("Socket number %d connected\n", (int)ClientSocket);
                WSAAsyncSelect(ClientSocket, hwnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
                OutputClientState(&ClientSocket, true);
                break;
            case FD_READ:
                printf("FD_READ");
                ClientSession(ClientSocket);
                break;
            case FD_WRITE:
                printf("FD_WRITE");
                break;
            case FD_CLOSE:
                printf("Closing socket %d\n", (int)wParam);
                OutputClientState(&ClientSocket, false);
                /*shutdown(ClientSocket, SD_BOTH);
                closesocket(ClientSocket);*/
                break;
            }
        }

        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND MakeWorkerWindow(void)
{
    WNDCLASS wndclass;
    CHAR* ProviderClass = (CHAR*)"AsyncSelect";
    HWND Window;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = (WNDPROC)WindowProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = NULL;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = (LPCWSTR)ProviderClass;

    if (RegisterClass(&wndclass) == 0)
    {
        printf("RegisterClass() failed with error %d\n", GetLastError());
        return NULL;
    }
    else
        printf("RegisterClass() is OK!\n");
    // Create a window
    if ((Window = CreateWindow(
        (LPCWSTR)ProviderClass,
        L"",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        NULL,
        NULL)) == NULL)
    {
        printf("CreateWindow() failed with error %d\n", GetLastError());
        return NULL;
    }
    else
        printf("CreateWindow() is OK!\n");
    return Window;
}