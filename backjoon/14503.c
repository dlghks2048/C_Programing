#include <cstdio>
int g_rect[50][50] = { 0 };

int main () {
    int N, M, x1, y1, dir, count = 0;

    scanf("%d %d", &N, &M);
    scanf("%d %d %d", &y1, &x1, &dir);

    for(int i = 0; i < N; i++){
        for(int j = 0; j < M; j++)
            scanf("%d", &g_rect[i][j]);
    }
    
    while(1){
        if(g_rect[y1][x1] == 0){
            g_rect[y1][x1] = -1;
            count++;
        }
        else if (g_rect[y1+1][x1] != 0 && g_rect[y1-1][x1] != 0 &&
                 g_rect[y1][x1+1] != 0 && g_rect[y1][x1-1] != 0 ){
            if(dir == 0){
                if(g_rect[y1+1][x1] != 1){ 
                    y1 += 1;
                    continue;
                }
            }
            if(dir == 1){
                if(g_rect[y1][x1-1] != 1){
                    x1 -= 1;
                    continue;
                }
            }
            if(dir == 2){
                if(g_rect[y1-1][x1] != 1){
                    y1 -= 1;
                    continue;
                }
            }
            if(dir == 3){
                if(g_rect[y1][x1+1] != 1){
                    x1 += 1;
                    continue;
                }
            }
            break;
        }
        else if (g_rect[y1+1][x1] == 0 || g_rect[y1-1][x1] == 0 ||
                 g_rect[y1][x1+1] == 0 || g_rect[y1][x1-1] == 0 ){
            dir -= 1;
            if(dir < 0) dir = 3;

            if(dir == 0){
                if(g_rect[y1-1][x1] == 0){ 
                    y1 -= 1;
                    continue;
                }
            }
            if(dir == 1){
                if(g_rect[y1][x1+1] == 0){
                    x1 += 1;
                    continue;
                }
            }
            if(dir == 2){
                if(g_rect[y1+1][x1] == 0){
                    y1 += 1;
                    continue;
                }
            }
            if(dir == 3){
                if(g_rect[y1][x1-1] == 0){
                    x1 -= 1;
                    continue;
                }
            }   
        }
    }
    
    printf("%d", count);
    return 0;
}