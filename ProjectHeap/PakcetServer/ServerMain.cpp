#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm> // std::shuffle용
#include "../Protocol.h"

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

    printf("--- [아방가르드] 패킷 정렬 시뮬레이션 서버 가동 ---\n");
    printf("포트 번호: %d (UDP IPv4)\n", SERVERPORT);

    struct sockaddr_in clientaddr;
    int addrlen;
    char buf[BUFSIZE];
    int nextSeq = 0;

    while (1) {
        addrlen = sizeof(clientaddr);
        // 클라이언트로부터 신호 대기 (예: "공격 패킷 줘!")
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr*)&clientaddr, &addrlen);
        if (retval == SOCKET_ERROR) {
            err_display("recvfrom()");
            continue;
        }
        
        SIM_PACKET* recvPkt = (SIM_PACKET*)buf;
        char IPAddr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientaddr.sin_addr, IPAddr, sizeof(IPAddr));
        printf("\n[클라이언트 요청 수신] %s:%d\n", IPAddr, ntohs(clientaddr.sin_port));

        // --- 패킷 셔플링 시뮬레이션 로직 ---
        // 공격 애니메이션 10개 프레임을 생성하여 뒤섞어 보냅니다.
        std::vector<SIM_PACKET> packetList;
        for (int i = 0; i < 10; i++) {
            SIM_PACKET p;
            p.type = 2; // ATTACK 상태
            p.curFrame = i;
            p.sequence = nextSeq++;
            p.timestamp = GetTickCount64();
            packetList.push_back(p);
        }

        // 의도적으로 순서 뒤섞기 (Shuffle)
        std::random_shuffle(packetList.begin(), packetList.end());

        printf("[발송] 뒤섞인 프레임 패킷 10개를 전송합니다...\n");
        for (auto& p : packetList) {
            sendto(sock, (const char*)&p, sizeof(SIM_PACKET), 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
            printf("  -> 발송 완료: Frame[%d] | Seq[%d]\n", p.curFrame, p.sequence);
            Sleep(10); // 너무 빠르면 UDP 드랍 발생 가능성이 있으니 살짝 여유
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}