// Reference:https://segmentfault.com/a/1190000019207061

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

    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == serverSock)
    {
        std::cout << "socket failed" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_port = htons(54321);
    service.sin_addr.s_addr = inet_addr("0.0.0.0");
    if (SOCKET_ERROR == bind(serverSock, (const sockaddr*)&service, sizeof service))
    {
        std::cout << "bind failed" << std::endl;
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    if (SOCKET_ERROR == listen(serverSock, SOMAXCONN))
    {
        std::cout << "listen failed" << std::endl;
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    fd_set allsocks;
    FD_ZERO(&allsocks);
    FD_SET(serverSock, &allsocks);
    while (true)
    {
        fd_set tmpsocks = allsocks;
        int nRes = select(0, &tmpsocks, NULL, NULL, NULL);
        if (nRes <= 0 || tmpsocks.fd_count <= 0)
        {
            closesocket(serverSock);
            WSACleanup();
            return 1;
        }
        else
        {
            for (UINT i = 0; i < tmpsocks.fd_count; ++i)
            {
                if (serverSock == tmpsocks.fd_array[i]) // server accept
                {
                    // Accept the connect
                    sockaddr_in clientSockaddr;
                    int addrLen = sizeof sockaddr;
                    SOCKET clientSock = accept(serverSock, (sockaddr*)&clientSockaddr, &addrLen);
                    if (INVALID_SOCKET == clientSock)
                    {
                        std::cout << "Accept failed" << std::endl;
                        closesocket(serverSock);
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
                        FD_SET(clientSock, &allsocks);            // add client socket
                    }
                }
                else // client listen
                {
                    SOCKET clientSock = tmpsocks.fd_array[i];
                    char recvbuf[DEFAULT_BUFLEN];
                    if (0 >= recv(clientSock, recvbuf, DEFAULT_BUFLEN, 0))
                    {
                        closesocket(clientSock);
                        FD_CLR(clientSock, &allsocks);
                        break;
                    }
                    std::cout << recvbuf << std::endl;
                }
            }
        }
    }

    closesocket(serverSock);
    WSACleanup();
    return 0;
}
