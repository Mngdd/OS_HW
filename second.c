#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

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

void listFilesRecursively(char* origPath, char* backupPath) {
    char currOrigPath[1000];
    char currBackupPath[1000];
    char errmsg[1000];
    char command[1000];

    struct dirent* dp;
    struct dirent* bckpath_readable;
    DIR* origDir = opendir(origPath);  //TODO: CLOSE THEM
    DIR* backupDir = opendir(backupPath);

    // Unable to open directory stream
    if (!origDir || !backupDir){
        strcpy(errmsg, "Unable to open directory: ");
        if (!origDir){
            strcat(errmsg, origPath);
            strcat(errmsg, " ");
        }
        if (!backupDir)
            strcat(errmsg, backupPath);

        perror("Unable to open directory(-ies): ");
        return;}

    //printf("%s", origDir->dd_name);
    while ((dp = readdir(origDir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) // skip linux spec files
        {
            printf("%s\n", dp->d_name);

            // Construct new "path" from our base path
            strcpy(currOrigPath, origPath); // replace with new path
            strcat(currOrigPath, "/");
            strcat(currOrigPath, dp->d_name);

            strcpy(currBackupPath, backupPath);
            strcat(currBackupPath, "/");
            strcat(currBackupPath, dp->d_name);

            if (is_regular_file(currOrigPath)) {
                //compare that file with backup
                strcat(currBackupPath, ".gz");
                _Bool need_create_file = access(currBackupPath, 0) != 0; // check if backup doesnt exists...

                if (!need_create_file) {
                    // file exists
                    time_t orig_t = getFileModTime(currOrigPath);
                    time_t backup_t = getFileModTime(currBackupPath);
                    printf("og %s date: %d, bckp %s date: %d\n", currOrigPath, orig_t, currBackupPath, backup_t);

                    // comp them lol
                    if (orig_t > backup_t) {
                        // update backup
                        printf("----------UPDATE NEEDED for %s", backupPath);
                        strcpy(command, "rm ");  // bash
                        strcat(command, currBackupPath);
                        system(command);
                        need_create_file = 1;
                    }
                }

                if (need_create_file) {
                    bckpath_readable = readdir(backupDir);

                    // file doesn't exist
                    // copy it
                    strcpy(command, "cp -r ");  // bash
                    strcat(command, currOrigPath);
                    strcat(command, " ");
                    strcat(command, backupPath);
                    strcat(command, "/");
                    //printf("--%s-----COPY? COMM: %s\n", backupPath, command);
                    system(command);


                    // gzip it
                    strcpy(command, "gzip ");
                    strcat(command, backupPath);
                    strcat(command, "/");
                    strcat(command, dp->d_name);
                    //printf("-------GZIP COMM: %s", command);
                    system(command);
                }
            }
            else {
                DIR* bckp_test_dir = opendir(currBackupPath);
                if (bckp_test_dir) {
                    closedir(bckp_test_dir);
                } else if (ENOENT == errno) {
                    closedir(bckp_test_dir);
                    strcpy(command, "mkdir ");
                    strcat(command, currBackupPath);
                    system(command);
                } else {
                    closedir(bckp_test_dir);
                    strcpy(errmsg, "Unable to open directory: ");
                    strcat(errmsg, currBackupPath);
                    strcat(errmsg, ", got unexpected error: ");
                    strcpy(errmsg, errno);
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
    if (argc != 3)
        perror("argc != 3; 2 args required: SOURCE DEST");
    char* src_name = argv[1];
    char* dest_name = argv[2];

    listFilesRecursively(src_name, dest_name);
}
