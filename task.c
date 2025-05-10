#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#define PROC_DIR "/proc"

int is_digits(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isdigit(str[i])) return 0;
    }
    return 1;
}

void list_all_processes() {
    DIR *dir = opendir(PROC_DIR);
    struct dirent *entry;

    printf("\nList of all processes:\n");
    while ((entry = readdir(dir)) != NULL) {
        if (is_digits(entry->d_name)) {
            printf("PID: %s\n", entry->d_name);
        }
    }
    closedir(dir);
}

void list_processes_by_user() {
    DIR *dir = opendir(PROC_DIR);
    struct dirent *entry;

    printf("\nProcesses grouped by user:\n");
    while ((entry = readdir(dir)) != NULL) {
        if (is_digits(entry->d_name)) {
            char path[256];
            sprintf(path, PROC_DIR "/%s/status", entry->d_name);
            FILE *fp = fopen(path, "r");
            if (!fp) continue;

            char line[256];
            uid_t uid = -1;
            while (fgets(line, sizeof(line), fp)) {
                if (strncmp(line, "Uid:", 4) == 0) {
                    sscanf(line, "Uid:\t%u", &uid);
                    break;
                }
            }
            fclose(fp);
            if (uid != -1) {
                struct passwd *pwd = getpwuid(uid);
                printf("User: %s, PID: %s\n", pwd ? pwd->pw_name : "Unknown", entry->d_name);
            }
        }
    }
    closedir(dir);

}

void run_process(const char *cmd) {
    if (fork() == 0) {
        execlp(cmd, cmd, NULL);
        perror("Failed to start process");
        exit(EXIT_FAILURE);
    }
}

void stop_process(pid_t pid) {
    if (kill(pid, SIGTERM) == 0)
        printf("Process %d terminated.\n", pid);
    else
        perror("Failed to terminate process");
}

void send_signal(pid_t pid, int sig) {
    if (kill(pid, sig) == 0)
        printf("Signal %d sent to process %d.\n", sig, pid);
    else
        perror("Failed to send signal");
}

int main() {
    int choice;
    char cmd[100];
    pid_t pid;
    int sig;

    while (1) {
        printf("\n--- Process Manager ---\n");
        printf("1. List all processes\n");
        printf("2. List processes by user\n");
        printf("3. Run a specific process\n");
        printf("4. Stop a specific process\n");
        printf("5. Send signal to process\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                list_all_processes();
                break;
            case 2:
                list_processes_by_user();
                break;
            case 3:
                printf("Enter command to run: ");
                scanf("%s", cmd);
                run_process(cmd);
                break;
            case 4:
                printf("Enter PID to stop: ");
                scanf("%d", &pid);
                stop_process(pid);
                break;
            case 5:
                printf("Enter PID to signal: ");
                scanf("%d", &pid);
                printf("Enter signal number: ");
                scanf("%d", &sig);
                send_signal(pid, sig);
                break;
            case 0:
                exit(0);
            default:
                printf("Invalid choice!\n");
        }
    }
    return 0;
}
