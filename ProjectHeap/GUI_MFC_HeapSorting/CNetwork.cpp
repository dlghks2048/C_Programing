#include "CNetwork.h"
#include <process.h>
#include <stdio.h>
#include <string.h>

CNetwork::CNetwork()
    : m_sock(INVALID_SOCKET), m_hThread(NULL), m_bIsRunning(false), m_nPing(0) {
}

CNetwork::~CNetwork() {
    Disconnect();
}

bool CNetwork::InitAndConnect(const char* szIP, int nPort) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;

    m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_sock == INVALID_SOCKET) return false;

    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_addr.s_addr = inet_addr(szIP);
    m_serverAddr.sin_port = htons(nPort);

    // [규약 준수] 변수명은 무조건 type
    SIM_PACKET helloPkt;
    memset(&helloPkt, 0, sizeof(SIM_PACKET));
    helloPkt.type = HELLOW; // 100
    helloPkt.curFrame = 0;
    helloPkt.sequence = 0;
    helloPkt.timestamp = 0;

    sendto(m_sock, (const char*)&helloPkt, sizeof(SIM_PACKET), 0,
        (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));

    m_bIsRunning = true;
    m_hThread = (HANDLE)_beginthreadex(NULL, 0, ReceiveThread, this, 0, NULL);

    return true;
}

void CNetwork::Disconnect() {
    if (!m_bIsRunning) return;

    // 종료 신호 (-1) 전송
    SendPacket(-1);

    m_bIsRunning = false;
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

void CNetwork::SendPacket(int nType) { // 여기서 인자로 받는 nType은 그냥 숫자임
    if (m_sock == INVALID_SOCKET) return;

    SIM_PACKET pkt;
    memset(&pkt, 0, sizeof(SIM_PACKET));

    // 구조체 멤버 변수명은 무조건 type!
    pkt.type = nType;
    pkt.curFrame = 0;
    pkt.sequence = 0;
    pkt.timestamp = 0;

    send(m_sock, (const char*)&pkt, sizeof(SIM_PACKET), 0);
}

bool CNetwork::GetNextFrame(SIM_PACKET* pOutPkt, bool bIgnoreBuffer) {
    if (!bIgnoreBuffer && (int)m_heap.size < m_nBufferThreshold) {
        return false;
    }
    return PopHeap(&m_heap, pOutPkt);
}

UINT WINAPI CNetwork::ReceiveThread(LPVOID pParam) {
    CNetwork* pThis = (CNetwork*)pParam;
    sockaddr_in fromAddr;
    int addrLen = sizeof(fromAddr);
    bool bConnected = false;

    while (pThis->m_bIsRunning) {
        SIM_PACKET recvPkt;
        int nRev = 0;

        if (!bConnected) {
            nRev = recvfrom(pThis->m_sock, (char*)&recvPkt, sizeof(SIM_PACKET), 0,
                (sockaddr*)&fromAddr, &addrLen);

            // 구조체 변수명 type 체크
            if (nRev > 0 && recvPkt.type == HELLOW) {
                connect(pThis->m_sock, (sockaddr*)&fromAddr, sizeof(fromAddr));
                bConnected = true;
                continue;
            }
        }
        else {
            nRev = recv(pThis->m_sock, (char*)&recvPkt, sizeof(SIM_PACKET), 0);
        }

        if (nRev > 0) {
            PushHeap(&pThis->m_heap, recvPkt);
        }
    }
    return 0;
}