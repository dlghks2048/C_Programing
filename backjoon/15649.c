#include <stdio.h>

// POSIX 표준인 getchar_unlocked를 사용 (Windows 환경이면 _getchar_nolock 사용)
#ifdef _WIN32
#define GETCHAR _getchar_nolock
#else
#define GETCHAR getchar_unlocked
#endif

int N, M;
int res[9];
int isUsed[9];
char out_buf[1000000]; // 출력 내용을 한 번에 담을 거대 버퍼
int out_ptr = 0;       // 버퍼의 현재 위치

// 정수를 문자열로 바꿔서 버퍼에 채우는 함수
void append_to_buf(int n) {
    if (n >= 10) out_buf[out_ptr++] = (n / 10) + '0';
    out_buf[out_ptr++] = (n % 10) + '0';
    out_buf[out_ptr++] = ' ';
}

void dfs(int depth) {
    if (depth == M) {
        for (int i = 0; i < M; i++) {
            append_to_buf(res[i]);
        }
        out_buf[out_ptr - 1] = '\n'; // 마지막 공백을 줄바꿈으로 교체
        return;
    }

    for (int i = 1; i <= N; i++) {
        if (!isUsed[i]) {
            res[depth] = i;
            isUsed[i] = 1;
            dfs(depth + 1);
            isUsed[i] = 0;
        }
    }
}

int main() {
    int n = 0, m = 0;
    char c = GETCHAR();
    while (c < '0' || c > '9') c = GETCHAR();
    while (c >= '0' && c <= '9') { n = n * 10 + (c - '0'); c = GETCHAR(); }
    while (c < '0' || c > '9') c = GETCHAR();
    while (c >= '0' && c <= '9') { m = m * 10 + (c - '0'); c = GETCHAR(); }
    
    N = n; M = m;
    
    dfs(0);

    fwrite(out_buf, 1, out_ptr, stdout);

    return 0;
}