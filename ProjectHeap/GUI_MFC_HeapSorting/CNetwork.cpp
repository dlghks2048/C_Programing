#include "CNetwork.h"
#include <process.h>
#include <stdio.h>

CNetwork::CNetwork()
    : m_sock(INVALID_SOCKET), m_hThread(NULL), m_bIsRunning(false), m_nPing(0) {
}

CNetwork::~CNetwork() {
    Disconnect();
}

bool CNetwork::InitAndConnect(const char* szIP, int nPort) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // UDP 소켓 생성 (connect 하지 않음!)
    m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // 서버 메인 주소 설정
    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_addr.s_addr = inet_addr(szIP);
    m_serverAddr.sin_port = htons(nPort);

    // [인사 패킷 전송]
    SIM_PACKET helloPkt;
    helloPkt.type = HELLOW;

    sendto(m_sock, (char*)&helloPkt, sizeof(SIM_PACKET), 0,
        (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));

    printf("서버(%s:%d)에 연결 패킷 전송.\n", szIP, nPort);

    // 수신 스레드 가동
    m_bIsRunning = true;
    m_hThread = (HANDLE)_beginthreadex(NULL, 0, ReceiveThread, this, 0, NULL);

    return true;
}

void CNetwork::Disconnect() {
    m_bIsRunning = false;

    // UDP는 Disconnect 시 "나 이제 간다"라고 서버에 알려주는 게 매너입니다.
    int nBye = 0;
    SendPacket(nBye);

    if (m_sock != INVALID_SOCKET) {
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
    }

    if (m_hThread != NULL) {
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    WSACleanup();
}

void CNetwork::SendPacket(int nType) {
    if (m_sock == INVALID_SOCKET) return;

    // UDP 전송 (connect를 했으므로 send를 써도 무방합니다)
    send(m_sock, (char*)&nType, sizeof(int), 0);
}

bool CNetwork::GetNextFrame(SIM_PACKET* pOutPkt, bool bIgnoreBuffer) {
    // 힙에 쌓인 패킷이 Threshold(5개) 미만이면 리턴 (지터 버퍼링)
    if (!bIgnoreBuffer && (int)m_heap.size < m_nBufferThreshold) {
        return false;
    }

    // m_heap.pop()이나 m_heap.Extract() 같은 함수가 필요합니다.
    // return m_heap.Pop(pOutPkt);
    return false;
}

UINT WINAPI CNetwork::ReceiveThread(LPVOID pParam) {
    CNetwork* pThis = (CNetwork*)pParam;

    while (pThis->m_bIsRunning) {
        SIM_PACKET pkt;

        int nRev = recv(pThis->m_sock, (char*)&pkt, sizeof(SIM_PACKET), 0);

        if (nRev > 0) {

        }
    }
    return 0;
}