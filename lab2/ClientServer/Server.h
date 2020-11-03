#pragma once

#include <string>
#include <WinSock2.h>
#include <ws2tcpip.h>

using namespace std;

class Server
{
public:
    Server(string& serverPort);
};