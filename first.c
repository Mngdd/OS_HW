#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

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


int main(int argc, char** argv) {
    if (argc != 2)
        perror("argc != 2; 1 arg required: FILE_NAME");
    char* src_name = argv[1];

    FILE* le_file = fopen("./poop.txt", "r");
    if( le_file == NULL ) {
        printf(stderr, "Couldn't open %s: %s\n", src_name, strerror(errno));
        exit(1);
    }
    char word[5];
    if (fscanf(le_file, "%4s", word) != 1) {
        fprintf(stderr, "Error parsing\n");
        return EXIT_FAILURE;
    }
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
