#include <iostream>
#include <string>
#include <WinSock2.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

SOCKET Connections[100];
int connections_amount = 0;

std::time_t t;
std::tm* now;

enum Packet
{
	P_ChatMessage,
	P_Test
};

bool ProcessPacket(int index, Packet packet_type) 
{
	switch (packet_type)
	{
	case P_ChatMessage:
	{
		int msg_size;
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char* message = new char[msg_size + 1];
		message[msg_size] = '\0';
		recv(Connections[index], message, msg_size, NULL);
		t = std::time(nullptr);
		now = std::localtime(&t);
		std::cout << "[" << index + 1 << "][" << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << "]: " << message << std::endl;

		for (int i = 0; i < connections_amount; i++)
		{
			if (i == index) continue;
			Packet msg_type = P_ChatMessage;
			send(Connections[i], (char*)&msg_type, sizeof(Packet), NULL);
			send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
			send(Connections[i], message, msg_size, NULL);
		}

		delete[] message;

		break;
	}
	default:
		std::cerr << "Unrecognized packet: " << packet_type << std::endl;
		break;
	}

	return true;
}

void ClientHandler(int index)
{
	Packet packet_type;

	while (true)
	{
		recv(Connections[index], (char*)&packet_type, sizeof(Packet), NULL);

		if (!ProcessPacket(index, packet_type)) break;
	}

	closesocket(Connections[index]);
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

	SOCKET slisten = socket(AF_INET, SOCK_STREAM, NULL);
	bind(slisten, (SOCKADDR*)&addr, sizeofaddr);
	listen(slisten, SOMAXCONN);

	SOCKET newConnection;
	for (int i = 0; i < 100; i++)
	{
		newConnection = accept(slisten, (SOCKADDR*)&addr, &sizeofaddr);

		if (!newConnection)
		{
			std::cerr << "Error: falied to connect the client" << std::endl;
		}
		else
		{
			std::cout << "Client " << connections_amount + 1 << " connected sucsessfully" << std::endl;
			std::string message = "Welcome to network chat!";
			int msg_size = message.size();

			Packet msg_type = P_ChatMessage;
			send(newConnection, (char*)&msg_type, sizeof(Packet), NULL);
			send(newConnection, (char*)&msg_size, sizeof(int), NULL);
			send(newConnection, message.c_str(), msg_size, NULL);

			Connections[i] = newConnection;
			connections_amount++;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);

			Packet testpacket = P_Test;
			send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);
		}
	}
	system("pause");
	return 0;
}