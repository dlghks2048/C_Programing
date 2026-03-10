#include <stdio.h>

int main () {
    int num;
    long long b, c;

    scanf("%d", &num);

    for(int i = 0; i <= num; i++){
        scanf("%lld %lld", &b, &c);
        
        int found = 0;
        long long N1;
        for(N1 = 1; N1*N1 <= c;  N1++){
            if(c%N1 == 0){
                long long N2 = b-N1;
                if(N2*N1 == c){
                    found = 1;
                    break;
                }
            }
        }

        if(found)
            printf("Yes\n");
        else 
            printf("No\n");
    }
    
    return 0;
}