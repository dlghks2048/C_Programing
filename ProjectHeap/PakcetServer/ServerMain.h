#pragma once
#include <map>
#include <string>

// 클라이언트 정보를 스레드에 넘기기 위한 구조체
typedef struct {
    SOCKET sock;
    sockaddr_in clientaddr;
} THREAD_PARAM;

std::map<std::string, bool> g_clientList; //클라이언트를 체크하기 위한 맵

void err_display(const char* msg);              //소켓 오류 함수 출력
unsigned int WINAPI StreamThread(LPVOID arg);   // 패킷 전송 스레드
std::string GetClientKey(sockaddr_in& addr);    //클라이언트 식별을 위한 키 식별 함수