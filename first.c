#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

const int MAX_COMNAD_AMOUNT = 256;
const int MAX_COMNAD_LENGTH = 256;
char commands_arr[256][256];

void* myThreadFun(int com_id) {
    char s[MAX_COMNAD_LENGTH];
    strcpy(s, commands_arr[com_id]);

    char* token;

    int cur_word = 0;

    char timeout[MAX_COMNAD_LENGTH];
    char command[MAX_COMNAD_LENGTH];

    int com_len = 0;
    int time_len = 0;

    for (token = strtok(s, " "); token; token = strtok(NULL, " ")) {
        switch (cur_word) {
            case 0:
                for (int char_ = 0; char_ < strlen(token); ++char_, ++time_len) {
                    timeout[time_len] = token[char_];
                }
                break;
            default:
                if (cur_word != 1) {
                    command[com_len] = ' ';
                    com_len += 1;
                }

                for (int char_ = 0; char_ < strlen(token); ++char_, ++com_len) {
                    command[com_len] = token[char_];
                }
                break;
        }

        ++cur_word;
    }
    command[com_len - 1] = '\0';
    sleep(atoi(timeout));

    system(command);
    printf("time passed=%i; executed=%s\n", atoi(timeout), command);
    return NULL;
}

int main(void) {
    FILE* le_file = fopen("./poop.txt", "r");
    pthread_t thread[MAX_COMNAD_AMOUNT];

    if (le_file == NULL)
        return -1;

    char line[MAX_COMNAD_LENGTH];

    int curr_thread = 0;

    while (fgets(line, sizeof(line), le_file)) {
        strcpy(commands_arr[curr_thread], line);

        pthread_create(&thread[curr_thread], NULL, myThreadFun, curr_thread);
        ++curr_thread;
    }
    fclose(le_file);

    for (int i = 0; i != 4; i++) {
        if (pthread_join(thread[i], NULL) != 0) {
            printf("ERROR : pthread join failed.\n");
            return (0);
        }
    }
}
