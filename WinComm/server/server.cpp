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
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == serverSocket)
    {
        std::cout << "Socket creation failed" << std::endl;
        WSACleanup();
        return 1;
    }

    // Bind the socket
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("0.0.0.0");
    service.sin_port = htons(12345);
    if (SOCKET_ERROR == bind(serverSocket, (const sockaddr*)&service, sizeof service))
    {
        std::cout << "Bind failed" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen the socket
    if (SOCKET_ERROR == listen(serverSocket, SOMAXCONN))
    {
        std::cout << "Listen failed" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    while (true)
    {
        // Accept the connect
        sockaddr_in clientSockaddr;
        int addrLen = sizeof sockaddr;
        SOCKET clientSock = accept(serverSocket, (sockaddr*)&clientSockaddr, &addrLen);
        if (INVALID_SOCKET == clientSock)
        {
            std::cout << "Accept failed" << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }
        else
        {
            std::cout << "Get connection: " 
                << inet_ntoa(clientSockaddr.sin_addr) // IP
                << ": "
                << ntohs(clientSockaddr.sin_port)     // Port
                << std::endl;
        }

        // Start thread to receive data
        new std::thread([clientSock]()-> void {
            while (true)
            {
                char recvbuf[DEFAULT_BUFLEN];
                if (0 >= recv(clientSock, recvbuf, DEFAULT_BUFLEN, 0))
                {
                    closesocket(clientSock);
                    break;
                }
                std::cout << recvbuf << std::endl;
            }
        });
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
