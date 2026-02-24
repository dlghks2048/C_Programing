#include <stdio.h>
#include <stdlib.h>

void printSet(int *list, int listPointer[], int step, int extra);

int main() {
    int size, extra, *list, listPointer[6] = {0, 1, 2, 3, 4, 5};

    while(1){
        scanf("%d", &size);
        if(size == 0) break;

        extra = size - 6;
        list = (int*)malloc(sizeof(int)*size);

        for(int i = 0; i < size; i++){
            scanf("%d", &list[i]);
        }

        printSet(list, listPointer, 5, extra);
        printf("\n");
    }

    return 0;
}

void printSet(int *list, int listPointer[], int step, int extra) {
    if(step == -1) return;
    int i = 0;

    for(i = 0; i < 6; i++){
        printf("%d ", list[listPointer[i]]);
    }
    printf("\n");
    if(extra == 0) return;

    if(listPointer[step] < step+extra){
        int num;
        if(listPointer[5] < 5+extra){
            listPointer[5]++;
        }
        else {listPointer[5] = listPointer[4] + 1;            
            for (int i = 4; i >= step; i--){
                if(listPointer[i] < i+extra){
                    num = i;
                    listPointer[i]++;
                    break;
                }
            }
            for (int i = num; i < 5; i++){
                listPointer[i+1] = listPointer[i] + 1;
            }
        }
        printSet(list, listPointer, step, extra);
    }
    else if (listPointer[step] == step+extra){
        step--;
        int value = 0;
        for (int i = 0; i < 6; i++) {
            if (value == step) {
                value++; 
            }
            listPointer[i] = value;
            value++;
        }
        printSet(list, listPointer, step, extra);
    }
}