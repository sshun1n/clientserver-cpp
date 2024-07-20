#include <iostream>
#include <string>
#include <WinSock2.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

SOCKET Connection;

enum Packet
{
	P_ChatMessage,
	P_Test
};

bool ProcessPacket(Packet packet_type)
{
	switch (packet_type) {
	case P_ChatMessage:
	{
		int msg_size;
		recv(Connection, (char*)&msg_size, sizeof(int), NULL);
		char* message = new char[msg_size + 1];
		message[msg_size] = '\0';
		recv(Connection, message, msg_size, NULL);
		std::cout << message << std::endl;
		delete[] message;
		break;
	}

	case P_Test:
		std::cout << "Test packet" << std::endl;
		break;

	default:
		std::cerr << "Unrecognized packet: " << packet_type << std::endl;
		break;
	}
	return true;
}

void ClientHandler()
{
	Packet packet_type;
	while (true)
	{
		recv(Connection, (char*)&packet_type, sizeof(Packet), NULL);

		if (!ProcessPacket(packet_type)) break;
	}

	closesocket(Connection);
}

int main(int argc, char* argv[]) {
	//WSAStartup

	WSADATA wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0)
	{
		std::cerr << "Error: failed to load the lib" << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(8888);
	addr.sin_family = AF_INET;

	Connection = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) 
	{
		std::cerr << "Error: failed to connect to the server" << std::endl;
		return 1;
	}

	std::cout << "Connected to the server!" << std::endl;

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

	std::string message_input;
	while (true)
	{
		std::getline(std::cin, message_input);
		int msg_size = message_input.size();
		Packet packet_type = P_ChatMessage;
		send(Connection, (char*)&packet_type, sizeof(Packet), NULL);
		send(Connection, (char*)&msg_size, sizeof(msg_size), NULL);
		send(Connection, message_input.c_str(), msg_size, NULL);
		Sleep(10);
	}

	system("pause");
	return 0;
}