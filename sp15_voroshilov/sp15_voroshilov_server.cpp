// sp15_voroshilov.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
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



DWORD WINAPI ClientSocketThread(LPVOID lpParam) {
	SOCKET ClientSocket = (SOCKET)lpParam;

	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;

	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {

			recvbuf[iResult] = '\0';

			std::cout << "Bytes received: " << recvbuf << std::endl;

			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				std::cout << "send failed: " << WSAGetLastError() << std::endl;
				closesocket(ClientSocket);
				break;
			}
			std::cout << "Bytes sent: " << recvbuf << std::endl;
		}
		else if (iResult == 0) std::cout << "Connection closing..." << std::endl;
		else {
			std::cout << "recv failed:" << WSAGetLastError() << std::endl;
			closesocket(ClientSocket);
			break;
		}

	} while (iResult > 0);

	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cout << "shutdown failed: " << WSAGetLastError() << std::endl;
	}
	closesocket(ClientSocket);
}


int main()
{
	WSADATA wsaData;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed " << iResult<<std::endl;
		return 1;
	}

	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		std::cout << "getaddrinfo failed " << iResult << std::endl;
		WSACleanup();
		return 1;
	}

	SOCKET ListenSocket = INVALID_SOCKET;

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		std::cout << "Error at socket(): " << WSAGetLastError()<<std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "Listen failed with error: " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	std::vector<HANDLE*> hThreads;

	while (true) {
		SOCKET ClientSocket;
		//!!!!
		//Обратите внимание, что этот базовый пример очень прост и не использует несколько потоков. В примере также прослушивается и принимается только одно подключение.
		//!!!!
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			std::cout << "accept failed: " << WSAGetLastError() << std::endl;
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		HANDLE hThread;
		DWORD IDThread;

		hThread = CreateThread(NULL, NULL, ClientSocketThread, (LPVOID)ClientSocket, NULL, &IDThread);
		if (hThread == NULL)
			return GetLastError();

		hThreads.push_back(&hThread);
	}

	WSACleanup();

	closesocket(ListenSocket);

	for (int i = 0; i < hThreads.size(); i++) {
		CloseHandle(hThreads[i]);
	}

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
