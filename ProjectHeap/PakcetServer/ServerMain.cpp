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
    // 1. 파라미터 복사 및 메모리 해제 (누수 방지)
    THREAD_PARAM* param = (THREAD_PARAM*)arg;
    sockaddr_in clientaddr = param->clientaddr;
    SOCKET mainSock = param->sock; // 사실 이건 이제 안 써도 됨 (전용 소켓 쓸 거니까)
    delete param;

    int nextSeq = 0; // 시퀀스 초기화 (이 클라이언트만의 번호)
    int addrlen = sizeof(clientaddr);
    char IPAddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientaddr.sin_addr, IPAddr, sizeof(IPAddr));

    // 2.  전용 소켓 생성 및 고정
    SOCKET privateSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (connect(privateSock, (struct sockaddr*)&clientaddr, addrlen) == SOCKET_ERROR) {
        closesocket(privateSock);
        return 0;
    }

    printf("[%s:%d] 전용 채널 생성 완료. 스트리밍 시작!\n", IPAddr, ntohs(clientaddr.sin_port));

    // 3. 무한 루프 시작
    while (1) {
        // [중요] 기존의 패킷 생성 및 셔플링 로직 복구
        std::vector<SIM_PACKET> batch;
        for (int i = 0; i < 5; i++) {
            SIM_PACKET p;
            p.type = ATTACK;
            p.curFrame = i;           // 프레임 번호 (애니메이션용)
            p.sequence = nextSeq++;    // 시퀀스 번호 (정렬용 - 계속 증가!)
            p.timestamp = GetTickCount64();
            batch.push_back(p);
        }

        // 의도적으로 순서 뒤섞기 (클라이언트의 힙 정렬 테스트용)
        std::random_shuffle(batch.begin(), batch.end());

        // 4. 전송
        for (auto& p : batch) {
            // connect를 했으므로 sendto 대신 send를 써도 무방함 (더 깔끔!)
            int retval = send(privateSock, (const char*)&p, sizeof(SIM_PACKET), 0);

            if (retval == SOCKET_ERROR) {
                printf("[%s] 통신 두절. 전용 채널을 폐쇄합니다.\n", IPAddr);
                goto THREAD_EXIT; // 루프 탈출 후 정리 로직으로
            }
        }

        Sleep(50); // 전송 속도 조절
    }

THREAD_EXIT:
    // 5. [핵심] 자원 해제
    // 여기서 안 닫아주면 소켓 핸들 누수가 발생함!
    closesocket(privateSock);
    return 0;
}
// 클라이언트 식별을 위한 키 생성 함수 (IP:Port)
std::string GetClientKey(sockaddr_in& addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
    return std::string(ip) + ":" + std::to_string(ntohs(addr.sin_port));
}

void GenerateNextPacket(SIM_PACKET& p, int& state, int& frame, int& seq) {
    p.type = state;
    p.curFrame = frame;
    p.sequence = seq++;
    p.timestamp = GetTickCount64();

    frame++;

    // 해당 스레드의 상태 전이 로직
    if (frame >= g_stateMaxFrame[state]) {
        frame = 0;
        if (state == ATTACK || state == HIT || state == PARRY) {
            state = IDLE;
        }
        else {
            int r = rand() % 10;
            if (r < 5) state = IDLE;
            else if (r < 8) state = MOVE;
            else state = ATTACK;
        }
    }
}