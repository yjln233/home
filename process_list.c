#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_comm(pid_t pid, char *buffer, size_t buffer_len) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return -1;
    }

    if (fgets(buffer, (int)buffer_len, file) == NULL) {
        fclose(file);
        return -1;
    }

    buffer[strcspn(buffer, "\n")] = '\0';
    fclose(file);
    return 0;
}

static int is_numeric(const char *name) {
    if (name == NULL || *name == '\0') {
        return 0;
    }

    for (const char *ch = name; *ch != '\0'; ++ch) {
        if (!isdigit((unsigned char)*ch)) {
            return 0;
        }
    }

    return 1;
}

int main(void) {
    DIR *proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        perror("Failed to open /proc");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        if (!is_numeric(entry->d_name)) {
            continue;
        }

        pid_t pid = (pid_t)atoi(entry->d_name);
        char comm[256];

        if (read_comm(pid, comm, sizeof(comm)) == 0) {
            printf("%5d  %s\n", pid, comm);
        } else {
            printf("%5d  [unknown]\n", pid);
        }
    }

    closedir(proc_dir);
    return EXIT_SUCCESS;
}
