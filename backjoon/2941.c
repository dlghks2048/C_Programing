#include <cstdio>
#include <string.h>
char* list[] = { "c=", "c-", "dz=", "d-", "lj", "nj", "s=", "z=" };

int main () {
    char word[101] = { 0 };
    int sum = 0;

    scanf("%s", word);

    for(int i = 0; word[i] != '\0';){
        int found = 0;
        for(int j = 0; j < 8; j++){
            int len = strlen(list[j]);
            if(strncmp(word + i, list[j], len) == 0){
                i += len;
                sum++;
                found = 1;
                break;
            }
        }
        if(found == 0) {
            i++;
            sum++;
        }
    }
    printf("%d", sum);

    return 0;
}