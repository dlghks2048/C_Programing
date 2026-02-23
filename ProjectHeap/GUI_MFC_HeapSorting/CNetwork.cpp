#include "CNetwork.h"
#include <process.h>
#include <stdio.h>
#include <string.h>
#include <chrono>

CNetwork::CNetwork()
    : m_sock(INVALID_SOCKET), m_hThread(NULL), m_bIsRunning(false),
    m_nPing(0), m_nNextSequence(0), m_serverAddr{ 0 } 
{
    InitHeap(&m_heap);
}

CNetwork::~CNetwork() {
    Disconnect();
}

bool CNetwork::InitAndConnect(const char* szIP, int nPort) {
    // 1. 소켓 생성 및 주소 설정 (기존 코드)
    m_sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(nPort);
    if (inet_pton(AF_INET, szIP, &servAddr.sin_addr) <= 0) {
        closesocket(m_sock);
        return false;
    }

    // 2. 서버에 트리거 패킷 전송 (먼저 찌르기)
    SIM_PACKET trigger;
    memset(&trigger, 0, sizeof(trigger));
    trigger.type = HELLOW;
    sendto(m_sock, (char*)&trigger, sizeof(SIM_PACKET), 0, (sockaddr*)&servAddr, sizeof(servAddr));

    // 3. 서버 응답 대기 (타임아웃 설정)
    struct timeval tv;
    tv.tv_sec = 5; 
    tv.tv_usec = 0;
    setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    SIM_PACKET recvPkt;
    sockaddr_in fromAddr;
    int addrLen = sizeof(fromAddr);

    // HELLOW 패킷이 오는지 직접 확인
    int nRev = recvfrom(m_sock, (char*)&recvPkt, sizeof(SIM_PACKET), 0, (sockaddr*)&fromAddr, &addrLen);

    if (nRev > 0 && recvPkt.type == HELLOW) {
        if (connect(m_sock, (sockaddr*)&fromAddr, sizeof(fromAddr)) == SOCKET_ERROR) {
            return false;
        }
        // 타임아웃 해제
        tv.tv_sec = 0;
        setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

        m_bIsRunning = true;
        m_hThread = (HANDLE)_beginthreadex(NULL, 0, ReceiveThread, this, 0, NULL);
        return true;
    }

    // [실패] 응답 없으면 소켓 닫고 종료
    closesocket(m_sock);
    m_sock = INVALID_SOCKET;
    return false;
}

void CNetwork::Disconnect() {
    if (!m_bIsRunning) return;

    // 종료 신호 (-1) 전송
    SendPacket(-1, 0);

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

void CNetwork::SendPacket(int state, int frame) {
    if (m_sock == INVALID_SOCKET) return;

    SIM_PACKET pkt;
    memset(&pkt, 0, sizeof(SIM_PACKET));

    pkt.type = state;
    pkt.curFrame = frame;
    pkt.sequence = m_nNextSequence++;
    pkt.timestamp = GetTickCount64();

    // connect 이전에 보내면 에러 가능성 존재
    int nResult = send(m_sock, (const char*)&pkt, sizeof(SIM_PACKET), 0);

    // 가능성은 낮지만 에러일 경우 sendto로 전송.
    if (nResult == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAENOTCONN) {
            sendto(m_sock, (const char*)&pkt, sizeof(SIM_PACKET), 0,
                (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
        }
    }
}

bool CNetwork::GetNextFrame(SIM_PACKET* pOutPkt, bool bIgnoreBuffer) {
    if (!bIgnoreBuffer && (int)m_heap.size < m_nBufferThreshold) {
        return false;
    }
    return PopHeap(&m_heap, pOutPkt);
}

UINT WINAPI CNetwork::ReceiveThread(LPVOID pParam) {
    CNetwork* pThis = (CNetwork*)pParam;

    while (pThis->m_bIsRunning) {
        SIM_PACKET recvPkt;
        // 이미 connect가 되어 있으므로 recv만 쓰면 됨
        int nRev = recv(pThis->m_sock, (char*)&recvPkt, sizeof(SIM_PACKET), 0);

        if (nRev > 0) {
            if (recvPkt.timestamp > 0 && recvPkt.type != HELLOW) {
                long long now = GetTickCount64();
                pThis->m_nPing = (int)(now - recvPkt.timestamp);
            }
            PushHeap(&pThis->m_heap, recvPkt);
        }
        else if (nRev == SOCKET_ERROR) {
            // 소켓이 닫히거나 에러 나면 탈출
            break;
        }
    }
    return 0;
}