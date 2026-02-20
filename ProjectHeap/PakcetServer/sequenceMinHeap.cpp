#include "sequenceMinHeap.h"

void InitHeap(PacketHeap* pHp) {
    pHp->size = 0;
    InitializeCriticalSection(&pHp->cs);
}

void PushHeap(PacketHeap* pHp, SIM_PACKET pkt) {
    EnterCriticalSection(&pHp->cs);

    if (pHp->size >= MAX_HEAP_SIZE) {
        LeaveCriticalSection(&pHp->cs);
        return;
    }

    int curr = pHp->size++;
    pHp->heapArray[curr] = pkt;

    // Up-Heap: sequence 번호가 작을수록 위로 (Min-Heap)
    while (curr > 0) {
        int parent = (curr - 1) / 2;
        // 내 시퀀스가 부모보다 작으면 위로 올라감
        if (pHp->heapArray[curr].sequence < pHp->heapArray[parent].sequence) {
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
        return 0;
    }

    // 루트(시퀀스 번호가 가장 작은 패킷) 추출
    *pOutPkt = pHp->heapArray[0];
    pHp->heapArray[0] = pHp->heapArray[--pHp->size];

    int curr = 0;
    // Down-Heap: 자식들 중 더 작은 시퀀스를 가진 쪽과 교체
    while (1) {
        int left = curr * 2 + 1;
        int right = curr * 2 + 2;
        int smallest = curr;

        if (left < pHp->size && pHp->heapArray[left].sequence < pHp->heapArray[smallest].sequence)
            smallest = left;
        if (right < pHp->size && pHp->heapArray[right].sequence < pHp->heapArray[smallest].sequence)
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