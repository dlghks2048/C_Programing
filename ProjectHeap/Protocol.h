#pragma once

#pragma pack(push, 1) // 패킷 정렬 최적화 (데이터가 중간에 비지 않게 함)

// 1. 패킷 타입 정의
#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 9000

// 객체의 상태 정의 (패킷 정의)
#define IDLE        0   //평소 상태
#define MOVE        1   //움직이는 상태(하지만 idle과 큰 차이는 없음)
#define ATTACK      2   //공격(상대에게 주는 신호)
#define HIT         3   //공격에 대한 반응이 없다면 적용되는 상태(스스로 재생하지 못하는 특수한 상태)
#define GUARD       4   //방어(공격에만 상호작용이 일으키는 신호)
#define PARRY       5   //특수한 경우에만 보내는 패킷, 가드 신호와 공격 신호의 오차가 굉장히 적을때(0.2초 이내?)
#define IDLE2       6   // 방패를 든 채 대기하는 상태

// 2. 메인 패킷 구조체
typedef struct {
    int type;            // 메시지 타입(객채의 상태)
    int curFrame;        // 몆 번째 프레임인지
    int sequence;        // 패킷 순서 (1, 2, 3...) -> 힙 정렬 후 유실 확인용
    long long timestamp; // 서버 발송 시각 (고정밀 타이머 값)
} SIM_PACKET;

#pragma pack(pop)