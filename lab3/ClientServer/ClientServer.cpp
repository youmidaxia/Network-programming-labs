#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <iterator>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <boost/program_options.hpp>

#include "Client.h"
#include "Server.h"
#include "ClientServer.h"
#include "Helpers.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    string PORT, IP_ADDRESS;

    try {
        po::options_description desc("Options");
        desc.add_options()
            ("help,h", "Produce a help message.")
            ("server,S", "The program will start as a server.")
            ("client,C", "The program will start as a client.")
            ("ip,I", po::value< string >(&IP_ADDRESS), "Provides an IP address.")
            ("port,P", po::value< string >(&PORT), "Provides a port.")
            ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << "\n";
            return 0;
        }

        cout << "IP: " << IP_ADDRESS << endl
             << "Port: " << PORT << endl << endl;

        //SetConsoleCtrlHandler(CtrlHandler, true);

        if (vm.count("server")) {
            cout << "The program was started as a server." << "\n";
            new Server(PORT);
        }
        else if (vm.count("client"))
        {
            cout << "The program was started as a client." << "\n";
            new Client(PORT, IP_ADDRESS);
        }
    }
    catch (std::exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch (...) {
        cerr << "Exception of unknown type!\n";
    }

    return 0;
}