#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

const int MAX_COMMAND_AMOUNT = 256;
const int MAX_COMMAND_LENGTH = 256;
char commands_arr[256][256];

void executeCommand(int com_id) {
    char s[MAX_COMMAND_LENGTH];
    strcpy(s, commands_arr[com_id]);

    char* token;
    int cur_word = 0;

    char timeout[MAX_COMMAND_LENGTH];
    char command[MAX_COMMAND_LENGTH];
    int com_len = 0;

    // Parse timeout and command from input string
    for (token = strtok(s, " "); token; token = strtok(NULL, " ")) {
        switch (cur_word) {
            case 0:  // First token is the timeout
                strcpy(timeout, token);
                break;
            default: // Remaining tokens form the command
                if (cur_word != 1) {
                    command[com_len++] = ' ';
                }
                for (int i = 0; i < strlen(token); i++) {
                    command[com_len++] = token[i];
                }
                break;
        }
        ++cur_word;
    }
    command[com_len] = '\0';

    // Sleep for the specified timeout
    sleep(atoi(timeout));

    // Prepare the command and arguments for execvp
    char* args[MAX_COMMAND_LENGTH];
    int argc = 0;
    token = strtok(command, " ");
    while (token != NULL) {
        args[argc++] = token;
        token = strtok(NULL, " ");
    }
    args[argc] = NULL;

    // Execute the command with execvp
    if (execvp(args[0], args) == -1) {
        perror("execvp failed");
        exit(1); // Exit child process on execvp failure
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s FILE_NAME\n", argv[0]);
        return 1;
    }

    char* src_name = argv[1];
    FILE* le_file = fopen(src_name, "r");
    if (le_file == NULL) {
        fprintf(stderr, "Couldn't open %s: %s\n", src_name, strerror(errno));
        return EXIT_FAILURE;
    }

    char line[MAX_COMMAND_LENGTH];
    int curr_command = 0;

    // Read commands from file
    while (fgets(line, sizeof(line), le_file)) {
        line[strcspn(line, "\n")] = 0;  // Remove newline character
        strcpy(commands_arr[curr_command], line);
        curr_command++;
    }
    fclose(le_file);

    // Fork and execute each command in a separate process
    for (int i = 0; i < curr_command; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            executeCommand(i);
            exit(0);  // Exit child process after executing command
        } else if (pid < 0) {
            // Error in fork
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
    }

    // Parent process waits for all child processes to complete
    for (int i = 0; i < curr_command; i++) {
        int status;
        wait(&status);  // Wait for each child process
        if (!WIFEXITED(status)) {
            printf("Child process did not terminate normally\n");
        }
    }

    return 0;
}
