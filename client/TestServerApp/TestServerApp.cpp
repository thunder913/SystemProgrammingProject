#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <map>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "54000"

using namespace std;
struct cmpByBiggestValue {
    bool operator()(const std::string& a, const std::string& b) const {
        return stof(a) > stof(b);
    }
};
static map<string, int, cmpByBiggestValue> bills;

void SetDefaultBillsToReturn() {
    bills = {
    { "0.01", 0 },
    { "0.02", 0 },
    { "0.05", 0 },
    { "0.10", 0 },
    { "0.20", 0 },
    { "0.50", 0 },
    { "1.00", 0 },
    { "2.00", 0 },
    { "5.00", 0 },
    { "10.00", 0 },
    { "20.00", 0 },
    { "50.00", 0 },
    { "100.00", 0 },
    };
}

int __cdecl main(int argc, char** argv)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    const char* sendbuf = "this is a test";
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL;ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection

    //TODO add an algorithms to send the money payment in bills
    srand((unsigned int)time(NULL));
    int day = 1;
    do {
        cout << "\n\n" << "Beginning day " << day << "\n\n\n";
        string dayMessage = "Day: " + to_string(day++);
        iResult = send(ConnectSocket, dayMessage.c_str(), (int)strlen(dayMessage.c_str()), 0);
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

        int clients = rand() % 10 + 1;
        for (size_t i = 0; i < clients; i++)
        {
            SetDefaultBillsToReturn();
            int billLeva = rand() % 1000;
            int billCoins = rand() % 100;

            int paidLeva = billLeva / 100;
            int paidCoins = rand() % 100;
            string paidString = string();
            for (auto& kv : bills) {
                if (kv.first != "100.00") {
                    int count = rand() % 10;
                    bills[kv.first] = count;
                }
                else {
                    bills[kv.first] = paidLeva > 2 ? paidLeva-2 : 0;
                }

                paidString += kv.first + "-" + to_string(kv.second) + ",";
            }
            string toSend = to_string(billLeva) + "." + to_string(billCoins) + "," +  paidString;
            iResult = send(ConnectSocket, toSend.c_str(), (int)strlen(toSend.c_str()), 0);

            iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
            string received(recvbuf, iResult);
            cout << received << "\n";
            Sleep((1000 + rand() % 3000));
        }
    } while (true);

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}