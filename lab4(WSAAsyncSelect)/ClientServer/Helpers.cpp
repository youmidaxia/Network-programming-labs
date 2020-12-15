#include <vector>
#include <iterator>
#include <string>
#include <ws2tcpip.h>
#include "Helpers.h"

using namespace std;

// A helper function to simplify the main part.
template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
    copy(v.begin(), v.end(), ostream_iterator<T>(os, " "));
    return os;
}

void CorrectClosing(SOCKET socket)
{
    // Sets the console attributes to default values.
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    closesocket(socket);
    // Should be called at the end of the program and be paired with WSAStartup.
    WSACleanup();
}

void getIP(sockaddr socketInfo, char* ip)
{
    sockaddr_in* sin = reinterpret_cast<sockaddr_in*>(&socketInfo);
    inet_ntop(AF_INET, &sin->sin_addr, ip, INET_ADDRSTRLEN);
}