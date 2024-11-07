#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int is_regular_file(const char* path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

time_t getFileModTime(char* path) {
    struct stat attr;
    stat(path, &attr);
    return attr.st_mtime;
}

void execute_command(char* const args[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Command exited with status %d\n", WEXITSTATUS(status));
        } else {
            printf("Command did not terminate normally\n");
        }
    } else {
        perror("fork failed");
    }
}

void listFilesRecursively(char* origPath, char* backupPath) {
    char currOrigPath[1000];
    char currBackupPath[1000];
    char errmsg[1000];

    struct dirent* dp;
    DIR* origDir = opendir(origPath);
    DIR* backupDir = opendir(backupPath);

    if (!origDir || !backupDir) {
        strcpy(errmsg, "Unable to open directory: ");
        if (!origDir) {
            strcat(errmsg, origPath);
            strcat(errmsg, " ");
        }
        if (!backupDir)
            strcat(errmsg, backupPath);

        perror(errmsg);
        return;
    }

    while ((dp = readdir(origDir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            printf("%s\n", dp->d_name);


            sprintf(currOrigPath, "%s/%s", origPath, dp->d_name);

            sprintf(currBackupPath, "%s/%s", backupPath, dp->d_name);
		
            if (is_regular_file(currOrigPath)) {
		struct stat buf;
	    	int x;

		x = lstat(currOrigPath, &buf);
		if (!S_ISLNK(buf.st_mode)){
                	strcat(currBackupPath, ".gz");
                	_Bool need_create_file = access(currBackupPath, F_OK) != 0;

               		if (!need_create_file) {
                    		time_t orig_t = getFileModTime(currOrigPath);
                    		time_t backup_t = getFileModTime(currBackupPath);
                    		printf("Original %s date: %ld, Backup %s date: %ld\n", currOrigPath, orig_t, currBackupPath, backup_t);

                    		if (orig_t > backup_t) {
                        		printf("----------UPDATE NEEDED for %s\n", backupPath);

                        		char* rm_args[] = {"rm", currBackupPath, NULL};
                        		execute_command(rm_args);
                        		need_create_file = 1;
                    		}
                	}

                	if (need_create_file) {
                    		char* cp_args[] = {"cp", "-r", currOrigPath, backupPath, NULL};
                    		execute_command(cp_args);

                    		char* tmp_fname[1000];
                    		sprintf(tmp_fname, "%s/%s", backupPath, dp->d_name);
                    		char* gzip_args[] = {"gzip", tmp_fname, NULL};
                    		execute_command(gzip_args);
                	}
		}
            } else {
                DIR* bckp_test_dir = opendir(currBackupPath);
                if (bckp_test_dir) {
                    closedir(bckp_test_dir);
                } else if (ENOENT == errno) {
                    char* mkdir_args[] = {"mkdir", currBackupPath, NULL};
                    execute_command(mkdir_args);
                } else {
                    closedir(bckp_test_dir);
                    snprintf(errmsg, sizeof(errmsg), "Unable to open directory: %s, unexpected error: %s", currBackupPath, strerror(errno));
                    perror(errmsg);
                }

                listFilesRecursively(currOrigPath, currBackupPath);
            }
        }
    }

    closedir(origDir);
    closedir(backupDir);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s SOURCE DEST\n", argv[0]);
        return 1;
    }

    char* src_name = argv[1];
    char* dest_name = argv[2];

    listFilesRecursively(src_name, dest_name);
    return 0;
}
