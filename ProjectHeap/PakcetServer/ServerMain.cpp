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
#include "ServerMain.h"
#include "sequenceMinHeap.h"
#include "../Protocol.h"

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
    THREAD_PARAM* pParam = (THREAD_PARAM*)arg;
    SOCKET privateSock = pParam->sock;
    sockaddr_in clientAddr = pParam->clientaddr;
    delete pParam; // 파라미터 메모리 해제

    char IPAddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, IPAddr, sizeof(IPAddr));
    int port = ntohs(clientAddr.sin_port);

    //[넌블로킹 설정] recv가 패킷 없어도 바로 리턴하게 함
    u_long mode = 1;
    ioctlsocket(privateSock, FIONBIO, &mode);

    // 스레드별 상태 관리 변수
    int currentState = IDLE;
    int currentFrame = 0;
    int nextSeq = 0;

    // 클라이언트 전용 힙 생성 및 초기화
    PacketHeap clientHeap;
    InitHeap(&clientHeap);

    printf("[%s:%d] 전용 스트림 스레드 시작\n", IPAddr, port);

    while (1) {
        // [수신] recv 사용 (넌블로킹)
        SIM_PACKET recvPkt;
        while (recv(privateSock, (char*)&recvPkt, sizeof(SIM_PACKET), 0) > 0) {
            PushHeap(&clientHeap, recvPkt);
        }

        //[판정 및 상태 우선순위] 힙에서 꺼내어 처리
        SIM_PACKET sortedPkt;
        while (PopHeap(&clientHeap, &sortedPkt)) {
            // [HIT 우선순위] 현재 HIT 상태가 아닐 때만 공격을 받음 (무적 판정)
            if (currentState != HIT && sortedPkt.type == ATTACK) {
                currentState = HIT;
                currentFrame = 0;
                printf("[%s:%d] Hit 판정! 상태 전환: HIT\n", IPAddr, port);
            }
        }

        // [송신 패킷 생성] GenerateNextPacket 활용
        std::vector<SIM_PACKET> batch;
        for (int i = 0; i < 5; i++) {
            SIM_PACKET p;
            GenerateNextPacket(p, currentState, currentFrame, nextSeq);
            batch.push_back(p);
        }

        // 지터 시뮬레이션을 위한 셔플
        std::random_shuffle(batch.begin(), batch.end());

        // ⑤ [전송]
        for (auto& p : batch) {
            // 네트워크 지연 시뮬레이션
            Sleep(10 + rand() % 20);
            int retval = send(privateSock, (const char*)&p, sizeof(SIM_PACKET), 0);

            if (retval == SOCKET_ERROR) {
                if (WSAGetLastError() != WSAEWOULDBLOCK) {
                    printf("[%s:%d] 연결 종료됨\n", IPAddr, port);
                    goto THREAD_EXIT;
                }
            }
        }

        Sleep(30); // CPU 과부하 방지
    }

THREAD_EXIT:
    DestroyHeap(&clientHeap);
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

    // 현재 상태의 애니메이션이 끝났는지 확인 (g_stateMaxFrame 배열 사용)
    if (frame >= g_stateMaxFrame[state]) {
        frame = 0; // 프레임 초기화

        // [상태 전이 로직]
        if (state == HIT || state == ATTACK || state == PARRY) {
            // 피격, 공격, 패리 동작이 끝나면 기본 대기(IDLE)로 복귀
            state = IDLE;
        }
        else {
            // IDLE이나 MOVE 상태일 때는 다음 행동을 랜덤 결정
            int r = rand() % 10;
            if (r < 4) state = IDLE;       // 60% 확률 대기
            else if (r < 6) state = MOVE;  // 30% 확률 이동
            else if (r < 8) state = GUARD;
            else state = ATTACK;           // 10% 확률 공격
        }
    }
}