#include "CPacketHeap.h"

void InitHeap(PacketHeap* pHp) {
    pHp->size = 0;
    InitializeCriticalSection(&pHp->cs);
}

void PushHeap(PacketHeap* pHp, SIM_PACKET pkt) {
    EnterCriticalSection(&pHp->cs);

    if (pHp->size >= MAX_HEAP_SIZE) {
        LeaveCriticalSection(&pHp->cs);
        return; // 꽉 참
    }

    int curr = pHp->size++;
    pHp->heapArray[curr] = pkt;

    // Up-Heap: 부모와 비교하여 위로
    while (curr > 0) {
        int parent = (curr - 1) / 2;
        if (pHp->heapArray[curr].timestamp < pHp->heapArray[parent].timestamp) {
            // Swap
            SIM_PACKET temp = pHp->heapArray[curr];
            pHp->heapArray[curr] = pHp->heapArray[parent];
            pHp->heapArray[parent] = temp;
            curr = parent;
        }
        else break;
    }

    LeaveCriticalSection(&pHp->cs);
}

int PopHeap(PacketHeap* pHp, SIM_PACKET* pOutPkt) {
    EnterCriticalSection(&pHp->cs);

    if (pHp->size <= 0) {
        LeaveCriticalSection(&pHp->cs);
        return 0; // 비어있음
    }

    *pOutPkt = pHp->heapArray[0]; // 루트(최소값) 추출
    pHp->heapArray[0] = pHp->heapArray[--pHp->size]; // 맨 뒤 요소를 루트로

    int curr = 0;
    // Down-Heap: 자식과 비교하여 아래로
    while (1) {
        int left = curr * 2 + 1;
        int right = curr * 2 + 2;
        int smallest = curr;

        if (left < pHp->size && pHp->heapArray[left].timestamp < pHp->heapArray[smallest].timestamp)
            smallest = left;
        if (right < pHp->size && pHp->heapArray[right].timestamp < pHp->heapArray[smallest].timestamp)
            smallest = right;

        if (smallest != curr) {
            SIM_PACKET temp = pHp->heapArray[curr];
            pHp->heapArray[curr] = pHp->heapArray[smallest];
            pHp->heapArray[smallest] = temp;
            curr = smallest;
        }
        else break;
    }

    LeaveCriticalSection(&pHp->cs);
    return 1;
}

void DestroyHeap(PacketHeap* pHp) {
    DeleteCriticalSection(&pHp->cs);
}