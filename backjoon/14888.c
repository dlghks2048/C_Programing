#include <stdio.h>
int operNum[4], N, list[11], max, min;
int isFirst = 1;

void dfs(int depth, int result);
int NumOper();


int main() {

    scanf("%d", &N);
    for (int i = 0; i < N; i++)
        scanf("%d", &list[i]);

    for (int i = 0; i < 4; i++) {
        scanf("%d", &operNum[i]);
    }



    dfs(0, list[0]);
    printf("%d\n%d", max, min);

    return 0;
}

void dfs(int depth, int result) {
    if (depth == N - 1) {
        if (isFirst) {
            max = min = result;
            isFirst = 0;
        }
        else {
            if (result > max) max = result;
            if (result < min) min = result;
        }
        return;
    }

    for (int i = 0; i < 4; i++) {
        if (operNum[i] > 0){
            switch (i)
            {
            case 0:
                operNum[i]--;
                dfs(depth+1, result+list[depth+1]);
                operNum[i]++;
                break;
            case 1:
                operNum[i]--;
                dfs(depth+1, result-list[depth+1]);
                operNum[i]++;
                break;
            case 2:
                operNum[i]--;
                dfs(depth+1, result*list[depth+1]);
                operNum[i]++;
                break;
            case 3:
                operNum[i]--;
                dfs(depth+1, result/list[depth+1]);
                operNum[i]++;
                break;
            }
        }
    }
}

