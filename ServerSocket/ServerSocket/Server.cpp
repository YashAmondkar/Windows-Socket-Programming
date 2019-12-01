#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mutex>

#define DEFAULT_PORT_NUMBER "27015"
#define DAFAULT_BUFFER_SIZE 512
#define MAXCLEINT 50

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

std::mutex mLock;
static int threadCount;
thread t[MAXCLEINT];

int ServerThread(SOCKET *clientSocket,SOCKET *listenSocket);

int main() {

	WSAData wsaData;

	int iresult;

	iresult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iresult != 0) {
		cout << " \n WSAStartup initialization failed : " << iresult;
		return 1;
	}

	struct addrinfo * result = nullptr, *ptr = nullptr, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iresult = getaddrinfo(nullptr, DEFAULT_PORT_NUMBER, &hints, &result);
	if (iresult != 0) {
		cout << "\n getaddrinfo failed : " << iresult;
		WSACleanup();
		return 1;
	}

	SOCKET listenSocket = INVALID_SOCKET;

	listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (listenSocket == INVALID_SOCKET)
	{
		cout << "\n Error at Socket() : " << WSAGetLastError();
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iresult = ::bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iresult == SOCKET_ERROR)
	{
		cout << "\n bind failed with error: " << WSAGetLastError();
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	if (listen(listenSocket, MAXCLEINT) == SOCKET_ERROR)
	{
		cout << "\n Listen failed with error: " << WSAGetLastError();
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	SOCKET clientSocket = INVALID_SOCKET;

	while (true) 
	{		
		clientSocket = accept(listenSocket, nullptr, nullptr);

		if (clientSocket == INVALID_SOCKET) 
		{
				cout << "\n Accept Failed : " << WSAGetLastError();
				closesocket(listenSocket);
				WSACleanup();
				return 1;
		}

		std::lock_guard<std::mutex> guard(mLock);
		{
			t[threadCount++] = thread(ServerThread, &clientSocket, &listenSocket);
			
			if (threadCount > MAXCLEINT) 
			{
				int i = 0;
				while (i < MAXCLEINT)
				{
					t[i].join();
				}
				i = 0;
			}
		}
	}
	
	closesocket(clientSocket);
	WSACleanup();
	getchar();
	return 0;
}

int ServerThread(SOCKET *clientSocket,SOCKET *listenSocket)
{
		char recvbuf[DAFAULT_BUFFER_SIZE];
		int recvBufferLen = DAFAULT_BUFFER_SIZE;
		int isentResult = 0;
		int iresult;

		do {
			std::lock_guard<std::mutex> guard(mLock);
			{
				iresult = recv(*clientSocket, recvbuf, recvBufferLen, 0);
			}
			if (iresult > 0) 
			{
				cout << "\n Byte Received : " << iresult;

				isentResult = send(*clientSocket, recvbuf, iresult, 0);

				if (isentResult == SOCKET_ERROR) {
					cout << "\n send Failed : " << WSAGetLastError();
					closesocket(*clientSocket);
					WSACleanup();
					return 1;
				}
				cout << "\n Byte Sent : " << isentResult;
			}
			else if (iresult == 0)
			{
				cout << "\n connection closing";
			}
			else
			{
				cout << "\n recv failed :" << WSAGetLastError();
				closesocket(*listenSocket);
				WSACleanup();
				return 1;
			}

		} while (iresult > 0);

		
		iresult = shutdown(*clientSocket, SD_SEND);
		if (iresult == SOCKET_ERROR) 
		{
			cout << " \n shutdown failed:" << WSAGetLastError();
			closesocket(*clientSocket);
			WSACleanup();
			return 1;
		}	
}
