
#include"Cashier.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <string>
#include <WS2tcpip.h>
#include <Windows.h>
#pragma comment (lib, "ws2_32.lib")

using namespace std;

// use threads for each client
int main()
{
	// Initialize winsock
	WSAData wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0) {
		cerr << "Can't initialize winsock. Quiting!" << endl;
		return 0;
	}

	// Create a socket

	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		cerr << "Can't create a socket! Quitting" << endl;
		return 0;
	}

	// Bind the socket to the ip and port
	
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;
	
	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// Tell winsock the socket is for listeing

	listen(listening, SOMAXCONN);

	// Wait for conenction

	sockaddr_in client;
	int clientSize = sizeof(client);
	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

	char host[NI_MAXHOST];
	char service[NI_MAXHOST];

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXHOST);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		cout << host << " connected on port " << service << endl;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connected on port " << ntohs(client.sin_port) << endl;
	}


	// Close listening socket

	closesocket(listening);

	// While loop: accept and echo message back to client

	char buf[4096];
	Cashier cashier = Cashier();

	// TODO receive the money send in bills and ADD them to the current bills, why wasnt I doing it before....
	string received;
	while (true)
	{
		ZeroMemory(buf, 4096);

		// Wait for client to send data
		int bytesReceived = recv(clientSocket, buf, 4096, 0);
		received = string(buf, bytesReceived);

		if (received.find("Day") != std::string::npos) {
			thread([&cashier, &clientSocket, &received]
				{
					int index = received.find(" ");
					int dayNumber = stoi(received.substr(index + 1));

					cashier.PrintDailyMoney(dayNumber);
					send(clientSocket, "ok", string("ok").length(), 0);
				}).detach();
		}
		else if(received.length()){
			thread([&cashier, &clientSocket, &received, &bytesReceived]
				{
					int index = received.find(",");

					string bill = received.substr(0, index);
					string paid = received.substr(index + 1, bytesReceived - index);
					received = received.substr(index + 1);

					string toReturn = cashier.Pay(Money(bill), received);
					send(clientSocket, toReturn.c_str(), toReturn.length(), 0);
				}).detach();

		}
	}

	// Close the socket

	closesocket(clientSocket);

	// Cleanup winsock
	WSACleanup();

}