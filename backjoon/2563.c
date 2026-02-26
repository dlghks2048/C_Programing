#include <stdio.h>
int area[100][100] = { 0 };

int main() {
    int num, x1, y1, sum = 0;

    scanf("%d", &num);

    for(int i = 0; i < num; i++){
        scanf("%d %d", &x1, &y1 );
        for(int j = x1; j < x1 + 10; j++){
            for (int k = y1; k < y1 + 10; k++){
                area[j][k] = 1;
            }
        }
    }

    for(int j = 0; j < 100; j++){
        for (int k = 0; k < 100; k++){
            sum += area[j][k];
        }
    }
    printf("%d", sum);
}