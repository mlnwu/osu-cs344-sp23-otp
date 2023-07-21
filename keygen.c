#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    int n;

    srand(time(NULL));

    if (argc > 1 && atoi(argv[1]) > 0) {
        int num = atoi(argv[1]);
        
        for (int i = 0; i < num; i++) {
            n = rand() % 27;
            putchar(chars[n]);
        }
        putchar('\n');
    } else {
        fprintf(stderr, "Invalid argv in keygen\n");
        return 1;
    }
    return 0;
}
