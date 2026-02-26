#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// 헤더 순서 주의
#include <winsock2.h> 
#include <ws2tcpip.h>
#include <windows.h>  


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

    // 콘솔 출력 코드 페이지를 UTF-8(65001)로 설정
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    SetUILayout();                              //메뉴 라인 계산
    EnableVTMode();                             //가상 콘솔 모드 활성화
    SetScrollRegion();                          //로그 영역 지정(콘솔 크기를 변경하면 함수를 재사용하여 영역 재정의 필요)
    HideCursor();                               //커서 숨키기
    

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

    SafeLog("--- 패킷 정렬 시뮬레이션 서버 가동 ---");
    SafeLog("포트 번호: %d (UDP IPv4)", SERVERPORT);

    while (1) {
        sockaddr_in clientaddr;
        int addrlen = sizeof(clientaddr);
        SIM_PACKET trigger;

        int retval = recvfrom(sock, (char*)&trigger, sizeof(SIM_PACKET), 0,
            (struct sockaddr*)&clientaddr, &addrlen);

        //에로 코드 출력
        if (retval == SOCKET_ERROR) {
            printf("recvfrom error: %d\n", WSAGetLastError());
            continue;
        }

        printf("연결 요청 받음, size=%d\n", retval);

        // 이미 관리 중인 클라이언트인지 확인
        std::string key = GetClientKey(clientaddr);
        if (g_clientList.find(key) != g_clientList.end()) {
            SafeLog("[Info] %s는 이미 스트리밍 중입니다.", key.c_str());
            continue;
        }

        // 새로운 클라이언트라면 리스트에 등록하고 스레드 생성
        g_clientList[key] = true;

        THREAD_PARAM* param = new THREAD_PARAM;
        param->sock = sock;
        param->clientaddr = clientaddr;
        _beginthreadex(NULL, 0, StreamThread, param, 0, NULL);

        Sleep(100);
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
    SafeLog("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// [전담 스레드] 특정 클라이언트에게 무한히 패킷을 쏘는 역할
unsigned int WINAPI StreamThread(LPVOID arg) {
    THREAD_PARAM* pParam = (THREAD_PARAM*)arg;
    sockaddr_in clientAddr = pParam->clientaddr;
    SOCKET privateSock = socket(AF_INET, SOCK_DGRAM, 0);        //새 소캣 생성

    std::string clientKey = GetClientKey(pParam->clientaddr);   //클라이언트 키 기록

    delete pParam; // 파라미터 메모리 해제

    // clientAddr의 주소를 가진 클라이언트에만 보내겠다는 선언(binding + connetc)
    if (connect(privateSock, (sockaddr*)&clientAddr, sizeof(clientAddr)) == SOCKET_ERROR) {
        SafeLog("Connect Error!");
        closesocket(privateSock);
        return 0;
    }

    // [답장 패킷 전송] 
    SIM_PACKET helloPkt;
    memset(&helloPkt, 0, sizeof(SIM_PACKET));
    helloPkt.type = HELLOW; 
    if (send(privateSock, (char*)&helloPkt, sizeof(SIM_PACKET), 0) == SOCKET_ERROR) {
        SafeLog("[Error] 답장 패킷 전송 실패");
        closesocket(privateSock);
        return 0;
    }
    // [답장 패킷 전송] 

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
    long long lastEcho = 0;

    // 클라이언트 전용 힙 생성 및 초기화
    PacketHeap* pClientHeap = new PacketHeap();
    InitHeap(pClientHeap);

    int myIdx = -1; // 내가 등록된 인덱스 저장용

    for (int i = 0; i < MAX_CLIENT; i++) {
        if (!g_Clients[i].bActive) {
            myIdx = i; // 빈 자리 발견!
            g_Clients[myIdx].key = clientKey;
            g_Clients[myIdx].bActive = true;
            g_Clients[myIdx].heapSize = 0; // 초기화
            break;
        }
    }

    if (myIdx == -1) goto THREAD_EXIT;

    SafeLog("[%s:%d] 전용 스트림 스레드 시작", IPAddr, port);

    while (1) {
        // [수신] recv 사용 (넌블로킹)
        SIM_PACKET recvPkt;
        while (recv(privateSock, (char*)&recvPkt, sizeof(SIM_PACKET), 0) > 0) {
            PushHeap(pClientHeap, recvPkt);
        }

        g_Clients[myIdx].heapSize = pClientHeap->size;
        //[판정 및 상태 우선순위] 힙에서 꺼내어 처리
        SIM_PACKET sortedPkt;
        while (PopHeap(pClientHeap, &sortedPkt)) {
            if (sortedPkt.type == -1) { // 클라이언트의 종료 신호 (나 뒤짐)
                goto THREAD_EXIT;
            }
            lastEcho = sortedPkt.timestamp; // 가장 최근 패킷의 시간을 저장

            if (sortedPkt.type == ATTACK || sortedPkt.type == PARRY) { // 클라이언트가 나(서버)를 때렸을 때
                if (sortedPkt.curFrame >= 1 && sortedPkt.curFrame <= 3) {

                    if (currentState == GUARD) {
                        //[패링 판정] 가드를 올리는 찰나(4번)에 맞음
                        currentState = PARRY;
                        currentFrame = 0;
                        SafeLog("[%s:%d] 패링 성공! (Server State: PARRY)", IPAddr, port);
                    }
                    else if (currentState == IDLE2) {
                        //[가드 판정] 이미 가드 중(6번)일 때 맞음
                        // 상태 변화 없음 (방패 이펙트는 클라이언트가 알아서 함)
                        SafeLog("[%s:%d] 가드로 방어함. (Server State 유지)", IPAddr, port);
                    }
                    else if (currentState != HIT && currentState != PARRY) {
                        //[피격 판정] 무방비 상태(IDLE, MOVE 등)에서 맞음
                        currentState = HIT;
                        currentFrame = 0;
                        SafeLog("[%s:%d] 적중! (Server State: HIT)", IPAddr, port);
                    }
                }
            }
        }

        // [송신 패킷 생성] GenerateNextPacket 활용
        if (!g_SimulationMode) {
            // [Clean 모드] 
            SIM_PACKET p;
            GenerateNextPacket(p, currentState, currentFrame, nextSeq, lastEcho);
            send(privateSock, (const char*)&p, sizeof(SIM_PACKET), 0);
            Sleep(50); // 50FPS 정도 유지
        }
        else {
            // [Sim 모드] 5개 묶음 셔플 전송
            std::vector<SIM_PACKET> batch;
            for (int i = 0; i < 5; i++) {
                SIM_PACKET p;
                GenerateNextPacket(p, currentState, currentFrame, nextSeq, lastEcho);
                batch.push_back(p);
            }
            std::random_shuffle(batch.begin(), batch.end());

            for (auto& p : batch) {
                // 전역 변수 g_JitterRange를 사용하여 지연폭 조절 (ping == 0에 대한 조건문 추가)
                int jitter = (g_JitterRange > 0) ? (rand() % g_JitterRange) : 0;
                Sleep(jitter);
                send(privateSock, (const char*)&p, sizeof(SIM_PACKET), 0);
            }
        }
    }

THREAD_EXIT:
    if (myIdx != -1) {
        g_Clients[myIdx].bActive = false;
        g_Clients[myIdx].key = "";
    }
    SafeLog("[%s:%d] 클라이언트가 스스로 종료를 알렸습니다.", IPAddr, port);

    DestroyHeap(pClientHeap);
    delete pClientHeap;
    closesocket(privateSock);
    return 0;
}

// 클라이언트 식별을 위한 키 생성 함수 (IP:Port)
std::string GetClientKey(sockaddr_in& addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
    return std::string(ip) + ":" + std::to_string(ntohs(addr.sin_port));
}

void GenerateNextPacket(SIM_PACKET& p, int& state, int& frame, int& seq, long long lastEcho) {
    p.type = state;
    p.curFrame = frame;
    p.sequence = seq++;
    p.timestamp = lastEcho;

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
    // 현재 커서 위치 저장
    printf("\x1b[s");

    // 1. 왼쪽 하단: 서버 설정 메뉴 출력
    printf("\x1b[%d;1H", g_menuStartLine);
    printf("==========================================\n");
    printf(" [MODE] %-15s | [JITTER] %3d ms\n",
        g_SimulationMode ? "SIMULATION (LAG)" : "CLEAN (NORMAL)", g_JitterRange);
    printf(" 명령어: 1(일반), 2(렉), +(증가), -(감소) \n");
    printf("==========================================");

    // 원래 커서 위치로 복구
    printf("\x1b[u");
}

// 키보드 입력 및 화면 갱신 전용 스레드
unsigned int WINAPI ControlThread(LPVOID arg) {
    while (1) {
        if (_kbhit()) {
            char ch = _getch();
            if (ch == '1') g_SimulationMode = false;
            else if (ch == '2') g_SimulationMode = true;
            else if (ch == '+') g_JitterRange += 10;
            else if (ch == '-') g_JitterRange = (g_JitterRange > 50) ? g_JitterRange - 10 : 50;

            // 키를 누를 때마다 상단 상태창 갱신
            UpdateStatus();
        }
        UpdateHeapStatus();
        Sleep(500);
    }
    return 0;
}

void SafeLog(const char* fmt, ...) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);

    // 로그 영역의 맨 마지막 줄(메뉴 바로 윗줄) 좌표 계산
    int windowHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    SHORT logBottomLine = (SHORT)(windowHeight - MENU_HEIGHT);

    // 1. 커서를 로그 영역의 끝으로 이동
    printf("\x1b[%d;1H", logBottomLine);

    // 2. 실제 로그 출력
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n"); // 여기서 한 줄 내려가면서 스크롤 발생!

    // 하단 메뉴가 깨지지 않게 다시 그려줌
    UpdateStatus();
}
// ANSI 모드(가상 터미널) 활성화 
void EnableVTMode() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

// 스크롤 영역 설정 
void SetScrollRegion() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);

    int windowHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    int logBottom = windowHeight - MENU_HEIGHT;

    // ANSI 코드: [1;로그바닥r -> 1번 줄부터 logBottom 줄까지만 스크롤 영역으로 지정
    SafeLog("\x1b[1;%dr", logBottom);
}

// 커서 숨기기
void HideCursor() { 
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    // 현재 커서 정보를 가져옴
    GetConsoleCursorInfo(hOut, &cursorInfo);

    // bVisible 멤버를 FALSE로 설정
    cursorInfo.bVisible = FALSE;

    // 변경된 정보를 적용
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

void UpdateHeapStatus() {
    printf("\x1b[s"); // 커서 위치 저장

    int baseCol = 45;      // 메뉴가 42열 정도까지 쓰니까 45열부터 시작하면 안전
    int colWidth = 25;     // 한 단의 너비
    int rowsPerCol = 5;    // 한 단에 몇 명씩 세로로 배치할지

    for (int i = 0; i < MAX_CLIENT; i++) {
        int colIdx = i / rowsPerCol; // 몇 번째 칸(단)인지
        int rowIdx = i % rowsPerCol; // 해당 단에서 몇 번째 줄인지

        int targetRow = g_menuStartLine + rowIdx;
        int targetCol = baseCol + (colIdx * colWidth);

        // 해당 칸으로 이동
        printf("\x1b[%d;%dH", targetRow, targetCol);

        if (g_Clients[i].bActive) {
            char buf[64];
            sprintf(buf, "ID:%-8s|H:%3d", g_Clients[i].key.c_str(), g_Clients[i].heapSize);
            printf("%-24s", buf);
        }
        else {
            // 비활성 상태: 해당 칸만 공백으로 확실히 지우기
            printf("%-24s", " ");
        }
    }

    printf("\x1b[u"); // 커서 위치 복구
}

// 메뉴 라인 계산, 시작시에, 화면 크기 변경시에 호출
void SetUILayout() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    int windowHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    g_menuStartLine = (SHORT)(windowHeight - MENU_HEIGHT + 1);
}