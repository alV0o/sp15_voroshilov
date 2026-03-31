// sp15_voroshilov_client.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//


#include <iostream>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

CRITICAL_SECTION cs;

DWORD WINAPI RecvThread(LPVOID lpParam) {
	
	SOCKET ConnectSocket = (SOCKET)lpParam;

	int iResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	while (true) {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		EnterCriticalSection(&cs);
		if (iResult > 0) {
			recvbuf[iResult] = '\0';
			std::cout << "\r" << recvbuf << "> ";
		}
		else if (iResult == 0) {
			std::cout << "[SERVER] Connection closed" << std::endl;
			LeaveCriticalSection(&cs);
			break;
		}
		else {
			std::cout << "[SERVER] recv failed: " << WSAGetLastError() << std::endl;
			LeaveCriticalSection(&cs);
			break;
		}
		LeaveCriticalSection(&cs);
	}
	return 0;
}


int InitializeWinSock(WSADATA *wsaData, int iResult, SOCKET *ConnectSocket) {
	iResult = WSAStartup(MAKEWORD(2, 2), wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed " << iResult << std::endl;
		return 1;
	}

	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	char ipaddress[256];
	std::cout << "give me ip: ";
	std::cin >> ipaddress;
	std::cin.ignore();

	iResult = getaddrinfo(ipaddress, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		std::cout << "getaddrinfo failed: " << iResult << std::endl;
		WSACleanup();
		return 1;
	}

	*ConnectSocket = INVALID_SOCKET;

	ptr = result;
	*ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if (*ConnectSocket == INVALID_SOCKET) {
		std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = connect(*ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(*ConnectSocket);
		*ConnectSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if (*ConnectSocket == INVALID_SOCKET) {
		std::cout << "Unable to connect to server!" << std::endl;
		WSACleanup();
		return 1;
	}

	return 0;
}


int main()
{
	WSADATA wsaData;
	int iResult = 0;
	SOCKET ConnectSocket;
	
	InitializeCriticalSection(&cs);

	int res = InitializeWinSock(&wsaData, iResult, &ConnectSocket);
	if (res != 0) return 1;

	char nickname[256];
	std::cout << "Enter your nickname: ";
	std::cin >> nickname;
	std::cin.ignore();
	iResult = 0;

	iResult = send(ConnectSocket, nickname, (int)strlen(nickname), 0);
	if (iResult == SOCKET_ERROR) {
		std::cout << "send failed :" << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		return 1;
	}

	HANDLE hThread;
	DWORD IDThread;

	hThread = CreateThread(NULL, NULL, RecvThread, (LPVOID)ConnectSocket, NULL, &IDThread);
	if (hThread == NULL)
		return GetLastError();

	while (true) {

		char sendbuf[256];
		std::cout << "> ";
		std::cin.getline(sendbuf, 256);
		
		iResult = 0;

		iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
		if (iResult == SOCKET_ERROR) {
			std::cout << "send failed :" << WSAGetLastError() << std::endl;
			closesocket(ConnectSocket);
			break;
		}

		if (strcmp(sendbuf, "/exit") == 0) break;
	}


	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cout << "shutdown failed: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	
	DeleteCriticalSection(&cs);

	CloseHandle(hThread);

	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
