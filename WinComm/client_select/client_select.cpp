#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")
#define DEFAULT_BUFLEN 512

int main()
{
    WORD wVersionRequired = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (0 != WSAStartup(wVersionRequired, &wsaData))
    {
        std::cout << "Initialization failed" << std::endl;
        return 1;
    }
    if (wVersionRequired != wsaData.wVersion)
    {
        std::cout << "Counld not find a usable version of Winsock" << std::endl;
        WSACleanup();
        return 1;
    }

    SOCKET clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == clientSock)
    {
        std::cout << "Socket creation failed" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_port = htons(54321);
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (SOCKET_ERROR == connect(clientSock, (const sockaddr*)&service, sizeof service))
    {
        std::cout << "Connect failed" << std::endl;
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    while (true)
    {
        std::string input;
        std::cin >> input;
        if (SOCKET_ERROR == send(clientSock, input.c_str(), input.size() + 1, 0))
        {
            std::cout << "Send failed" << std::endl;
            closesocket(clientSock);
            WSACleanup();
            return 1;
        }
    }

    if (SOCKET_ERROR == shutdown(clientSock, SD_SEND))
    {
        std::cout << "Shutdown failed" << std::endl;
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    closesocket(clientSock);
    WSACleanup();
    return 0;
}
