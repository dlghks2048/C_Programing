#include <stdio.h>
char listOper[] = { '+',  '-', '*', '/' }, char_list[10], char_Result[10] = { '\0' };
int operNum[4], operNumUse[10] = { 0 }, N, list[11], max, min;
int isFirst = 1;

void dfs(int depth);
int NumOper();


int main() {

    scanf_s("%d", &N);
    for (int i = 0; i < N; i++)
        scanf_s("%d", &list[i]);

    for (int i = 0; i < 4; i++) {
        scanf_s("%d", &operNum[i]);
    }

    int j = 0;
    for (int i = 0; i < 4;) {
        if (operNum[i] != 0) {
            char_list[j++] = listOper[i];
            operNum[i]--;
        }
        else i++;
    }

    dfs(0);
    printf("%d\n%d", max, min);

    return 0;
}

void dfs(int depth) {
    if (depth == N - 1) {
        int result;
        result = NumOper();
        if (isFirst) {
            max = min = result;
            isFirst = 0;
        }
        else {
            if (result > max) max = result;
            else if (result < min) min = result;
        }
        return;
    }

    for (int i = 0; i < N - 1; i++) {
        if (operNumUse[i] == 1) continue;
        char_Result[depth] = char_list[i];
        operNumUse[i] = 1;
        dfs(depth + 1);
        operNumUse[i] = 0;
    }
}

int NumOper() {
    int i1 = list[0], i2, result;
    for (int i = 0; i < N - 1; i++) {
        i2 = list[i + 1];
        switch (char_Result[i]) {
        case '+':
            i1 = i1 + i2;
            break;
        case '-':
            i1 = i1 - i2;
            break;
        case '*':
            i1 = i1 * i2;
            break;
        case '/':
            i1 = i1 / i2;
            break;
        }
        result = i1;
    }

    return result;
}