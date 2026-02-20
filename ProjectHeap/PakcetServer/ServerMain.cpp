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
#include <conio.h>
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

    _beginthreadex(NULL, 0, ControlThread, NULL, 0, NULL);  // 컨트롤 스레드 실행
    UpdateStatus();                                         // 초기 UI 출력

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
            if (sortedPkt.type == -1) { // 클라이언트의 종료 신호 (나 뒤짐)
                printf("[%s:%d] 클라이언트가 스스로 종료를 알렸습니다.\n", IPAddr, port);
                goto THREAD_EXIT;
            }

            if (sortedPkt.type == ATTACK) { // 클라이언트가 나(서버)를 때렸을 때

                if (currentState == GUARD) {
                    //[패링 판정] 가드를 올리는 찰나(4번)에 맞음
                    currentState = PARRY;
                    currentFrame = 0;
                    printf("[JUDGE] 패링 성공! (Server State: PARRY)\n");
                }
                else if (currentState == IDLE2) {
                    //[가드 판정] 이미 가드 중(6번)일 때 맞음
                    // 상태 변화 없음 (방패 이펙트는 클라이언트가 알아서 함)
                    printf("[JUDGE] 가드로 방어함. (Server State 유지)\n");
                }
                else if (currentState != HIT && currentState != PARRY) {
                    //[피격 판정] 무방비 상태(IDLE, MOVE 등)에서 맞음
                    currentState = HIT;
                    currentFrame = 0;
                    printf("[JUDGE] 적중! (Server State: HIT)\n");
                }
            }
        }

        // [송신 패킷 생성] GenerateNextPacket 활용
        if (!g_SimulationMode) {
            // [Clean 모드] 
            SIM_PACKET p;
            GenerateNextPacket(p, currentState, currentFrame, nextSeq);
            send(privateSock, (const char*)&p, sizeof(SIM_PACKET), 0);
            Sleep(50); // 50FPS 정도 유지
        }
        else {
            // [Sim 모드] 5개 묶음 셔플 전송
            std::vector<SIM_PACKET> batch;
            for (int i = 0; i < 5; i++) {
                SIM_PACKET p;
                GenerateNextPacket(p, currentState, currentFrame, nextSeq);
                batch.push_back(p);
            }
            std::random_shuffle(batch.begin(), batch.end());

            for (auto& p : batch) {
                // 전역 변수 g_JitterRange를 사용하여 지연폭 조절
                int jitter = rand() % g_JitterRange;
                Sleep(jitter);
                send(privateSock, (const char*)&p, sizeof(SIM_PACKET), 0);
            }
        }
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

    // 현재 상태의 애니메이션이 끝났는지 확인
    if (frame >= g_stateMaxFrame[state]) {
        frame = 0;

        int r1 = rand() % 10;
        int r2 = rand() % 10;
        // [세부 상태 전이 로직]
        switch (state) {
        case HIT:
        case ATTACK:
        case PARRY:
            // 큰 동작이 끝나면 일단 IDLE로 복귀
            state = IDLE;
            break;

        case GUARD:
            if (r1 < 7) state = IDLE2;  // 70% 확률로 가드 계속 유지
            else if (r1 < 9) state = IDLE;
            else state = ATTACK;
            break;

        case IDLE2:
            // 6번(가드 유지) 중에도 일정 확률로 가드를 풀거나 다시 공격
            if (r2 < 3) state = IDLE2;      // 30% 유지
            else if (r2 < 8) state = IDLE;  // 50% 가드 해제
            else state = ATTACK;            // 20% 기습 공격
            break;

        case IDLE:
        case MOVE:
        default:
            // 일반 상태에서의 랜덤 행동
            int r = rand() % 10;
            if (r < 4) state = IDLE;
            else if (r < 6) state = MOVE;
            else if (r < 8) state = GUARD; // 가드 올리기 시도
            else state = ATTACK;
            break;
        }
    }
}

void UpdateStatus() {
    // 커서를 맨 위(0,0)로 이동 (Windows API 사용)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(hOut, coord);

    printf("==========================================\n");
    printf(" [MODE] %-15s | [JITTER] %3d ms\n",
        g_SimulationMode ? "SIMULATION (LAG)" : "CLEAN (NORMAL)", g_JitterRange);
    printf(" 명령어: 1(일반), 2(렉), +(증가), -(감소) \n");
    printf("==========================================\n\n");
}

// 키보드 입력 및 화면 갱신 전용 스레드
unsigned int WINAPI ControlThread(LPVOID arg) {
    while (1) {
        if (_kbhit()) {
            char ch = _getch();
            if (ch == '1') g_SimulationMode = false;
            else if (ch == '2') g_SimulationMode = true;
            else if (ch == '+') g_JitterRange += 50;
            else if (ch == '-') g_JitterRange = (g_JitterRange > 50) ? g_JitterRange - 50 : 0;

            // 키를 누를 때마다 상단 상태창 갱신
            UpdateStatus();
        }
        Sleep(100);
    }
    return 0;
}