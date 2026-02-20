#pragma once

#ifndef __CPACKET_HEAP_H__
#define __CPACKET_HEAP_H__

#include "../Protocol.h"
#include <windows.h> // CRITICAL_SECTION 사용을 위해 필요(쓰래드)

#define MAX_HEAP_SIZE 1024

// C 컴파일러라고 선언.
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    SIM_PACKET heapArray[MAX_HEAP_SIZE];
    int size;
    CRITICAL_SECTION cs; // 스레드 동기화용
} PacketHeap;

// 함수 인터페이스
void InitHeap(PacketHeap* pHp);
void PushHeap(PacketHeap* pHp, SIM_PACKET pkt);
int PopHeap(PacketHeap* pHp, SIM_PACKET* pOutPkt); // 성공 시 1, 실패 시 0 반환
void DestroyHeap(PacketHeap* pHp);

#ifdef __cplusplus
}
#endif

#endif