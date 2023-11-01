#include<WinSock2.h>
#include<windows.h>
#include<WS2tcpip.h>
#include<stdio.h>
#pragma comment(lib, "Ws2_32.lib")
#define BUF_SIZE 1024
char szMsg[BUF_SIZE];
unsigned SendMsg(void* arg)
{
    SOCKET sock = *((SOCKET*)arg);
    while (1)
    {
        scanf("%s", szMsg);
        if (!strcmp(szMsg,"QUIT\n")||!strcmp(szMsg,"quit\n"))
        {
            closesocket(sock);
            exit(0);
        }
        send(sock, szMsg, strlen(szMsg), 0);

    }
    return 0;

}
unsigned RecvMsg(void* arg)
{
    SOCKET sock = *((SOCKET*)arg);
    char msg[BUF_SIZE];
    while (1)
    {
        int len=recv(sock, msg, sizeof(msg) - 1, 0);
        if (len == -1)
        {
            printf("发送 error");
            return -1;
        }
        msg[len] = '\0';
        printf("%s\n", msg);
    }
    return 0;
}
int main()
{
	//初始化环境

    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("初始化 error");
        return -1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2) {
        WSACleanup();
        printf("初始化 error");
        return -1;
    }
    //创建socket
    SOCKET cSock;
    SOCKADDR_IN serverAdr;
    cSock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serverAdr, 0, sizeof(serverAdr));
    serverAdr.sin_family = AF_INET;
    serverAdr.sin_port= htons(9999);
    inet_pton(AF_INET, "175.178.254.28", &serverAdr.sin_addr);
    
    //connect
    if (connect(cSock, (SOCKADDR*)&serverAdr, sizeof(serverAdr)) == SOCKET_ERROR)
    {
        printf("connect error:%d", GetLastError());
        return -1;
    }
    else
    {
        printf("welcome:");
    }
    //循环接发
    HANDLE hsendhand=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendMsg, (void*)&cSock, 0, NULL);
    HANDLE hrecvhand=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvMsg, (void*)&cSock, 0, NULL);
    WaitForSingleObject(hsendhand, INFINITE);
    WaitForSingleObject(hrecvhand, INFINITE);
    closesocket(cSock);
    WSACleanup();
}