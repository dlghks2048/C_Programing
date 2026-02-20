#pragma once
#include <map>
#include <string>
#include "../Protocol.h"

// 각 상태별 애니메이션 총 프레임 수 정의
int g_stateMaxFrame[MAX_STATE] = {4, 7, 6, 4, 7, 5, 4};
bool g_SimulationMode = false;
int g_JitterRange = 100;        // 지연폭 (0~100ms)
std::map<std::string, bool> g_clientList; //클라이언트를 체크하기 위한 맵

// 클라이언트 정보를 스레드에 넘기기 위한 구조체
typedef struct THREAD_PARAM {
    SOCKET sock;
    sockaddr_in clientaddr;
} THREAD_PARAM;

void err_display(const char* msg);                                          //소켓 오류 함수 출력
unsigned int WINAPI StreamThread(LPVOID arg);                               // 패킷 전송 스레드
std::string GetClientKey(sockaddr_in& addr);                                //클라이언트 식별을 위한 키 식별 함수
void GenerateNextPacket(SIM_PACKET& p, int& state, int& frame, int& seq);   //랜덤 패킷 생성 함수
void UpdateStatus();                                                        //콘솔창 출력(간단한 명령어)
unsigned int WINAPI ControlThread(LPVOID arg);                              // 키보드 입력 및 화면 갱신 전용 스레드