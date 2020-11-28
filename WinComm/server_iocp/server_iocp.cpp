// IOCP_TCPIP_Socket_Server.cpp
// Reference：https://blog.csdn.net/neicole/article/details/7549497

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>
#include <vector>
#include <iostream>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")		// Socket lib
#pragma comment(lib, "Kernel32.lib")	// IOCP lib


const int DataBuffSize = 2 * 1024;
typedef struct
{
	OVERLAPPED overlapped;
	WSABUF databuff;
	char buffer[DataBuffSize];
	int BufferLen;
	int operationType;
}PER_IO_OPERATEION_DATA, * LPPER_IO_OPERATION_DATA, * LPPER_IO_DATA, PER_IO_DATA;

typedef struct
{
	SOCKET socket;
	SOCKADDR_STORAGE ClientAddr;
}PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

vector < PER_HANDLE_DATA* > clientGroup;
DWORD WINAPI ServerWorkThread(LPVOID CompletionPortID);


int main()
{
	WORD wVersionRequested = MAKEWORD(2, 2); 
	WSADATA wsaData;
	DWORD err = WSAStartup(wVersionRequested, &wsaData);
	if (0 != err) {
		cerr << "Request Windows Socket Library Error!\n";
		system("pause");
		return -1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		cerr << "Request Windows Socket Version 2.2 Error!\n";
		system("pause");
		return -1;
	}

	// Create IOCP object
	HANDLE completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == completionPort) {
		cerr << "CreateIoCompletionPort failed. Error:" << GetLastError() << endl;
		system("pause");
		return -1;
	}

	// Start threadpool to handle iocp message
	{
		SYSTEM_INFO mySysInfo;
		GetSystemInfo(&mySysInfo);

		// create threads based the cpu core count
		for (DWORD i = 0; i < (mySysInfo.dwNumberOfProcessors * 2); ++i) {
			HANDLE ThreadHandle = CreateThread(NULL, 0, ServerWorkThread, completionPort, 0, NULL);
			if (NULL == ThreadHandle) {
				cerr << "Create Thread Handle failed. Error:" << GetLastError() << endl;
				system("pause");
				return -1;
			}
			CloseHandle(ThreadHandle);
		}
	}
	
	SOCKET srvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN srvAddr;
	srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(6000);
	int bindResult = bind(srvSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
	if (SOCKET_ERROR == bindResult) {
		cerr << "Bind failed. Error:" << GetLastError() << endl;
		system("pause");
		return -1;
	}

	int listenResult = listen(srvSocket, 10);
	if (SOCKET_ERROR == listenResult) {
		cerr << "Listen failed. Error: " << GetLastError() << endl;
		system("pause");
		return -1;
	}

	while (true) {
		PER_HANDLE_DATA* PerHandleData = NULL;
		SOCKADDR_IN saRemote;
		int RemoteLen;
		SOCKET acceptSocket;

		RemoteLen = sizeof(saRemote);
		acceptSocket = accept(srvSocket, (SOCKADDR*)&saRemote, &RemoteLen);
		if (SOCKET_ERROR == acceptSocket) {
			cerr << "Accept Socket Error: " << GetLastError() << endl;
			system("pause");
			return -1;
		}

		
		PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));	
		PerHandleData->socket = acceptSocket;
		memcpy(&PerHandleData->ClientAddr, &saRemote, RemoteLen);
		clientGroup.push_back(PerHandleData);


		CreateIoCompletionPort((HANDLE)(PerHandleData->socket), completionPort, (DWORD)PerHandleData, 0);

		// Trigger recv
		LPPER_IO_OPERATION_DATA PerIoData = NULL;
		PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;	// read
		DWORD RecvBytes;
		DWORD Flags = 0;
		WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}

	system("pause");
	return 0;
}


DWORD WINAPI ServerWorkThread(LPVOID IpParam)
{
	HANDLE CompletionPort = (HANDLE)IpParam;
	DWORD BytesTransferred;
	LPOVERLAPPED IpOverlapped;
	LPPER_HANDLE_DATA PerHandleData = NULL;
	LPPER_IO_DATA PerIoData = NULL;
	DWORD RecvBytes;
	DWORD Flags = 0;
	BOOL bRet = false;

	while (true) {
		bRet = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&IpOverlapped, INFINITE);
		if (bRet == 0) {
			cerr << "GetQueuedCompletionStatus Error: " << GetLastError() << endl;
			return -1;
		}
		PerIoData = (LPPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, overlapped);
		if (0 == BytesTransferred) {
			closesocket(PerHandleData->socket);
			GlobalFree(PerHandleData);
			GlobalFree(PerIoData);
			continue;
		}

		// Handle received data
		cout << "A Client says: " << PerIoData->databuff.buf << endl;

		// Continue to recv
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;	// read
		WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}

	return 0;
}