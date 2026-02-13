#pragma once

#pragma pack(push, 1) // 패킷 정렬 최적화 (데이터가 중간에 비지 않게 함)

// 1. 패킷 타입 정의
#define TYPE_SIMULATION 2000
#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 9000

// 객체의 상태 정의 (패킷 정의)
#define IDLE      0
#define MOVE      1
#define ATTACK    2
#define DIE       3

// 2. 메인 패킷 구조체
typedef struct {
    int type;            // 메시지 타입 (TYPE_SIMULATION)
    int sequence;        // 패킷 순서 (1, 2, 3...) -> 힙 정렬 후 유실 확인용
    long long timestamp; // 서버 발송 시각 (고정밀 타이머 값)
} SIM_PACKET;

#pragma pack(pop)