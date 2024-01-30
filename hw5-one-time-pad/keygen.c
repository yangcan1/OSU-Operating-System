#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
    srand(time(NULL));

    if (argc != 2) {
        printf("Usage: %s <key_len>\n", argv[0]);
        return 1;
    }
    int num = atoi(argv[1]);

    int i;
    for (i = 0; i < num; i++) {
        int rand_num = 65 + rand() % 27;

        if (rand_num == 91) {
            rand_num = 32;
        }
        
        printf("%c", rand_num);
    }

    printf("\n");

    return 0;
}