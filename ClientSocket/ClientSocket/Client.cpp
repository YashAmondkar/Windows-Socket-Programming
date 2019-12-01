#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define DEFAULT_PORT_NUMBER "27015"
#define DEFAULT_BUFFER_LENGTH 512

#pragma comment(lib,"WS2_32.lib")

using namespace std;

int main(int argc , char *argv[]) {

	WSADATA wsaData;
	int iresult;

	// Initialize Winsock
	iresult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iresult != 0) {
		cout << "\n WSAStartup initialization failed : " << iresult;
		return 1;
	}

	struct addrinfo *result = nullptr, *ptr = nullptr, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// localhost
	iresult = getaddrinfo("127.0.0.1", DEFAULT_PORT_NUMBER, &hints, &result);
	if (iresult != 0) {
		cout << "\n getaddrinfo returned : " << iresult;
		WSACleanup();
		return 1;
	}
	
	SOCKET connectSocket = INVALID_SOCKET;

	ptr = result;
	connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if (connectSocket == INVALID_SOCKET) {
		cout << "\n Error at socket() : " << WSAGetLastError();
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iresult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iresult == SOCKET_ERROR) {
		closesocket(connectSocket);
		connectSocket = INVALID_SOCKET;
	}
	freeaddrinfo(result);
	if (connectSocket == INVALID_SOCKET) {
		cout << " \n Unable to connect to server";
		WSACleanup();
		return 1;
	}

	int recvbuflen = DEFAULT_BUFFER_LENGTH;

	const char *sendbuf = "this is a test";
	char recvbuf[DEFAULT_BUFFER_LENGTH];

	// Send an initial buffer
	iresult = send(connectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iresult == SOCKET_ERROR) {
		cout << "\n send failed: " << WSAGetLastError();
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}
	cout << "\n Bytes Sent:" << iresult;

	// shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	iresult = shutdown(connectSocket, SD_SEND);
	if (iresult == SOCKET_ERROR) {
		cout << "\n shutdown failed:"<< WSAGetLastError();
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	// Receive data until the server closes the connection
	do {
		iresult = recv(connectSocket, recvbuf, recvbuflen, 0);
		if (iresult > 0)
			cout << "\n Bytes received: " << iresult;
		else if (iresult == 0)
			cout << "\n Connection closed ";
		else
			cout << "\n recv failed : " << WSAGetLastError();
	} while (iresult > 0);
	
	// shutdown the send half of the connection since no more data will be sent
	iresult = shutdown(connectSocket, SD_SEND);
	if (iresult == SOCKET_ERROR) {
		cout << "\n shutdown failed: "<< WSAGetLastError();
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(connectSocket);
	WSACleanup();
	getchar();
	return 0;
}