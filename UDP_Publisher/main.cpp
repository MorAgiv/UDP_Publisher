#include <iostream>
#include <WS2tcpip.h>
#include <chrono>
#include <thread>
#include "Circle.h"
#include "Square.h"
#include <unordered_map>
#pragma comment (lib, "ws2_32.lib") // Includes the Winsock library
using namespace std;

void publishCircle(unordered_map<int, sockaddr_in>& map, SOCKET out);
void publishSquare(unordered_map<int, sockaddr_in>& map1, unordered_map<int, sockaddr_in>& map2, SOCKET out);
string PublishError(sockaddr_in server, const string& msg);
sockaddr_in createSocket(int portNum, string ipAddress);
int initializeWinsock();
const int ListeningPortNum = 65535;

int main(int argc, char* argv[]) // We can pass in a command line option
{
	cout << "Server starts " << endl;
	int initWin = initializeWinsock();
	if (initWin) { return 1; }


	//Circle subscribtions Listening Socket:
	sockaddr_in ListeningSocket = createSocket(ListeningPortNum, "127.0.0.1");

	// Socket creation, socket type is datagram
	SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);//same
	DWORD timeout = 1000;
	setsockopt(out, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));


	if (bind(out, (sockaddr*)&ListeningSocket, sizeof(ListeningSocket)) == SOCKET_ERROR)
	{
		cout << "Can't bind socket to port " << ListeningPortNum << " ! " << WSAGetLastError() << endl;
		return 1;
	}
	sockaddr_in client;
	int clientLength = sizeof(client);
	char buf[1024];
	unordered_map<int, sockaddr_in> CirclesSquareSubscribers;
	unordered_map<int, sockaddr_in> SquareSubscribers;

	// Write out to that socket
	while (1) {
		//check for new connections:
		cout << "checks new connections" << endl;

		ZeroMemory(&client, clientLength);
		ZeroMemory(buf, 1024);
		int bytesIn56000 = recvfrom(out, buf, 1024, 0, (sockaddr*)&client, &clientLength);
		if (bytesIn56000 != SOCKET_ERROR)
		{
			char clientIp[256];
			ZeroMemory(clientIp, 256);
			inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);
			cout << "recieved msg is: " << buf << endl;

			if (strncmp(buf, "REGISTER circles", 16) == 0) //squares and circles
			{
				int subscriberPort;
				sscanf_s(buf, "REGISTER  circles%d", &subscriberPort);//takes the portNum
				cout << "subscriberPort recieved is: " << subscriberPort << endl;
				// check if port is already taken:
				if (CirclesSquareSubscribers.find(subscriberPort) != CirclesSquareSubscribers.end()) {//port already exists
					cout << " port: " << subscriberPort << " is already taken" << endl;
				}
				else {
					cout << " port: " << subscriberPort << " is added" << endl;
					CirclesSquareSubscribers[subscriberPort] = createSocket(subscriberPort, "127.0.0.1");
				}
			}
			else if (strncmp(buf, "REGISTER squares", 16) == 0)//squares only:
			{
				int subscriberPort;
				sscanf_s(buf, "REGISTER  squares%d", &subscriberPort);//takes the portNum
				cout << "subscriberPort recieved is: " << subscriberPort << endl;
				// check if port is already taken:
				if (SquareSubscribers.find(subscriberPort) != SquareSubscribers.end()) {//port already exists
					cout << " port: " << subscriberPort << " is already taken" << endl;
				}
				else {
					cout << " port: " << subscriberPort << " is added" << endl;
					SquareSubscribers[subscriberPort] = createSocket(subscriberPort, "127.0.0.1");
				}
			}
		}

		else {
			//cout << "recvfrom port " << CirclePortNum << " returned SOCKET_ERROR, error code: "<< WSAGetLastError() << endl;
		}


		cout << "publishes shapes" << endl;
		//######################################

		//publish shapes:
		try {
			//wait 0.5 seconds
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			//create and publish circle:
			publishCircle(CirclesSquareSubscribers, out);

			//wait another 0.5 seconds
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			//create and publish circle + square
			publishCircle(CirclesSquareSubscribers, out);
			publishSquare(CirclesSquareSubscribers, SquareSubscribers, out);
		}
		catch (string str) {
			cout << str << ", re-publishing sequence from the beginning" << endl;
		}

	}

	// Close the socket
	closesocket(out);

	// Close down Winsock
	WSACleanup();
	return 0;
}


//functions:
void publishCircle(unordered_map<int, sockaddr_in>& map, SOCKET out) {
	Circle circle;
	string circleStr = circle.shapeInfo();
	for (auto it = map.begin(); it != map.end(); it++) {
		int sendOk = sendto(out, circleStr.c_str(), circleStr.size() + 1, 0, (sockaddr*)&(it->second), sizeof(it->second));
		if (sendOk == SOCKET_ERROR) {
			throw(PublishError(it->second, circleStr));
		}
	}
}


void publishSquare(unordered_map<int, sockaddr_in>& map1, unordered_map<int, sockaddr_in>& map2, SOCKET out) {
	Square square;
	//square- p55000
	string squareStr = square.shapeInfo();
	for (auto it = map1.begin(); it != map1.end(); it++) {
		int sendOk = sendto(out, squareStr.c_str(), squareStr.size() + 1, 0, (sockaddr*)&(it->second), sizeof((it->second)));
		if (sendOk == SOCKET_ERROR) {
			throw(PublishError(it->second, squareStr));
		}
	}
	for (auto it = map2.begin(); it != map2.end(); it++) {
		int sendOk = sendto(out, squareStr.c_str(), squareStr.size() + 1, 0, (sockaddr*)&(it->second), sizeof((it->second)));
		if (sendOk == SOCKET_ERROR) {
			throw(PublishError(it->second, squareStr));
		}
	}

}

string PublishError(sockaddr_in server, const string& msg) {
	string s = "Error occured: " + to_string(WSAGetLastError()) + "while sending: " + msg + " to port: " +
		to_string(server.sin_port);
	return s;
}


sockaddr_in createSocket(int portNum, string ipAddress) {
	sockaddr_in server;
	server.sin_family = AF_INET; // AF_INET = IPv4 addresses
	server.sin_port = htons(portNum); // Little to big endian conversion
	inet_pton(AF_INET, ipAddress.c_str(), &server.sin_addr); // Convert from string to byte array
	return server;
}

int initializeWinsock() {

	WSADATA data;
	WORD version = MAKEWORD(2, 2);

	// Start WinSock
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0)
	{
		cout << "Can't start Winsock! " << wsOk;
		return 1;
	}
	return 0;
}