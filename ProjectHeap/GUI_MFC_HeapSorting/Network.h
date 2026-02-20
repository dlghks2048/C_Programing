#pragma once
#include <winsock2.h>
#include "CPacketHeap.h"
#include "../Protocol.h"

class CNetwork {
public:
    CNetwork();
    ~CNetwork();

    // 1. 초기화 및 서버 접속 (나 접속할거야! 신호 포함)
    bool InitAndConnect(const char* szIP, int nPort);

    // 2. 다잉 메시지 발송 및 종료
    void Disconnect();

    // 3. 서버에 내 액션(공격 등) 전송
    void SendPacket(int nType);

    // 4. 지터 버퍼링: 힙에서 적절한 패킷 하나 꺼내오기
    // bIgnoreBuffer가 false면 버퍼가 일정 수치 찰 때까지 기다림
    bool GetNextFrame(SIM_PACKET* pOutPkt, bool bIgnoreBuffer = false);

    // 5. 통계 정보 (GUI에서 핑 그래프 그릴 때 사용)
    int GetCurrentPing() { return m_nPing; }
    int GetHeapSize() { return m_heap.size; }

private:
    // 6. 통신 전담 스레드 (내부용)
    static UINT WINAPI ReceiveThread(LPVOID pParam);

    SOCKET       m_sock;
    sockaddr_in  m_serverAddr;
    PacketHeap   m_heap;        // 패킷 저장용 Min-Heap
    bool         m_bIsRunning;  // 스레드 제어 플래그
    int          m_nPing;       // 실시간 지연 시간(ms)

    // 버퍼링 임계값 (예: 5개 쌓이기 전엔 멈춰있기)
    const int    m_nBufferThreshold = 5;
};