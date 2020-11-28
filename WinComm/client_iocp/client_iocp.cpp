#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")
#define DEFAULT_BUFLEN 512

int main()
{
    // Initialize winsocket dll
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (0 != WSAStartup(wVersionRequested, &wsaData))
    {
        std::cout << "WSAStartup failed" << std::endl;
        return 1;
    }
    if (wVersionRequested != wsaData.wVersion)
    {
        std::cout << "Counld not find a usable version of Winsock" << std::endl;
        WSACleanup();
        return 1;
    }

    // Create socket
    SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == connectSocket)
    {
        std::cout << "Socket creation failed" << std::endl;
        WSACleanup();
        return 1;
    }

    // connect server socket
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
    service.sin_port = htons(6000);
    if (SOCKET_ERROR == connect(connectSocket, (const sockaddr*)&service, sizeof service))
    {
        std::cout << "Connect failed" << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    while (true)
    {
        std::string input;
        std::cin >> input;
        if (SOCKET_ERROR == send(connectSocket, input.c_str(), input.size() + 1, 0))
        {
            std::cout << "Send failed" << std::endl;
            closesocket(connectSocket);
            WSACleanup();
            return 1;
        }
    }

    if (SOCKET_ERROR == shutdown(connectSocket, SD_SEND))
    {
        std::cout << "Send failed" << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    closesocket(connectSocket);
    WSACleanup();
    return 0;
}
