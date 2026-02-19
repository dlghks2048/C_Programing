#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm> // std::shuffle용
#include <thread>
#include <process.h>
#include "../Protocol.h"
#include "ServerMain.h"

#pragma comment(lib, "ws2_32")

#define SERVERPORT 9000
#define BUFSIZE 256



// 소켓 함수 오류 출력
void err_display(const char* msg) {
    LPVOID lpMsgBuf;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s\n", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// [전담 스레드] 특정 클라이언트에게 무한히 패킷을 쏘는 역할
unsigned int WINAPI StreamThread(LPVOID arg) {
    THREAD_PARAM* param = (THREAD_PARAM*)arg;
    SOCKET sock = param->sock;
    sockaddr_in clientaddr = param->clientaddr;
    int addrlen = sizeof(clientaddr);

    int nextSeq = 0;

    char IPAddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientaddr.sin_addr, IPAddr, sizeof(IPAddr));
    printf("[%s:%d] 전용 스트리밍 스레드 시작!\n", IPAddr, ntohs(clientaddr.sin_port));

    delete param; // 동적 할당된 메모리 해제

    while (1) {
        // 5개씩 묶어서 셔플 전송
        std::vector<SIM_PACKET> batch;
        for (int i = 0; i < 5; i++) {
            SIM_PACKET p;
            p.type = ATTACK;
            p.curFrame = i;
            p.sequence = nextSeq++;
            p.timestamp = GetTickCount64();
            batch.push_back(p);
        }

        std::random_shuffle(batch.begin(), batch.end());

        // 다이어트된 사이즈로 발송
        for (auto& p : batch) {
            int retval = sendto(sock, (const char*)&p, sizeof(SIM_PACKET), 0,
                (struct sockaddr*)&clientaddr, addrlen);

            if (retval == SOCKET_ERROR) {
                printf("[%s] 클라이언트 연결 끊김으로 판단, 스레드 종료.\n", IPAddr);
                return 0; // 클라이언트가 소켓을 닫거나 문제가 생기면 스레드 자폭
            }
        }

        // 스트리밍 속도 조절 (초당 약 100개 패킷)
        Sleep(50);
    }
    return 0;
}

// 클라이언트 식별을 위한 키 생성 함수 (IP:Port)
std::string GetClientKey(sockaddr_in& addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
    return std::string(ip) + ":" + std::to_string(ntohs(addr.sin_port));
}

int main() {
    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

    // UDP 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) err_display("socket()");

    // 주소 설정 및 바인드
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);

    retval = bind(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_display("bind()");

    printf("--- 패킷 정렬 시뮬레이션 서버 가동 ---\n");
    printf("포트 번호: %d (UDP IPv4)\n", SERVERPORT);

    while (1) {
        sockaddr_in clientaddr;
        int addrlen = sizeof(clientaddr);
        SIM_PACKET trigger;

        int retval = recvfrom(sock, (char*)&trigger, sizeof(SIM_PACKET), 0,
            (struct sockaddr*)&clientaddr, &addrlen);

        // 이미 관리 중인 클라이언트인지 확인
        std::string key = GetClientKey(clientaddr);
        if (g_clientList.find(key) != g_clientList.end()) {
            printf("[Info] %s는 이미 스트리밍 중입니다.\n", key.c_str());
            continue;
        }

        // 새로운 클라이언트라면 리스트에 등록하고 스레드 생성
        g_clientList[key] = true;

        THREAD_PARAM* param = new THREAD_PARAM;
        param->sock = sock;
        param->clientaddr = clientaddr;
        _beginthreadex(NULL, 0, StreamThread, param, 0, NULL);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}