#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>

extern char **environ;

// --- ФУНКЦИИ КЛАССА CHILD ---
int run_child(int argc, char *argv[], char *envp[]) {
    printf("CHILD: Name: %s, PID: %d, PPID: %d\n", argv[0], getpid(), getppid());

    char *mode = (argc > 1) ? argv[1] : "";
    char **env_to_print = NULL;

    if (strcmp(mode, "+") == 0) {
        printf("CHILD: Environment via 'environ':\n");
        env_to_print = environ;
    } else if (strcmp(mode, "*") == 0) {
        printf("CHILD: Environment via 'envp':\n");
        env_to_print = envp;
    }

    if (env_to_print) {
        for (int i = 0; env_to_print[i] != NULL; i++) {
            printf("  %s\n", env_to_print[i]);
        }
    }
    return 0;
}

// --- ФУНКЦИИ КЛАССА PARENT ---
int compare_env(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void print_parent_env() {
    int count = 0;
    while (environ[count]) count++;
    char **copy = malloc((count + 1) * sizeof(char *));
    for (int i = 0; i < count; i++) copy[i] = environ[i];
    qsort(copy, count, sizeof(char *), compare_env);
    printf("--- PARENT SORTED ENV ---\n");
    for (int i = 0; i < count; i++) printf("%s\n", copy[i]);
    printf("-------------------------\n");
    free(copy);
}

int main(int argc, char *argv[], char *envp[]) {
    // Если имя программы начинается на "child", запускаем логику ребенка
    if (strstr(argv[0], "child") != NULL) {
        return run_child(argc, argv, envp);
    }

    // ЛОГИКА РОДИТЕЛЯ
    print_parent_env();

    // Формируем среду из файла 'env'
    char **reduced_env = malloc(100 * sizeof(char *));
    int env_count = 0;
    FILE *f = fopen("env", "r");
    if (f) {
        char var_name[256];
        while (fscanf(f, "%255s", var_name) == 1) {
            char *val = getenv(var_name);
            if (val) {
                char *entry = malloc(strlen(var_name) + strlen(val) + 2);
                sprintf(entry, "%s=%s", var_name, val);
                reduced_env[env_count++] = entry;
            }
        }
        fclose(f);
    }
    reduced_env[env_count] = NULL;

    int child_idx = 0;
    char command;
    char *child_path_env = getenv("CHILD_PATH");

    

    while (printf("\nCommand (+, *, q): ") && scanf(" %c", &command) == 1) {
        if (command == 'q') break;
        if (command != '+' && command != '*') continue;

        pid_t pid = fork();
        if (pid == 0) { // Дочерний процесс
            char child_name[16];
            sprintf(child_name, "child_%02d", child_idx);

            char full_path[512];
            // Используем ту же самую программу, но переименованную или по пути
            if (child_path_env) sprintf(full_path, "%s/child", child_path_env);
            else strcpy(full_path, "./child");

            char *child_argv[] = {child_name, (command == '+' ? "+" : "*"), NULL};
            
            execve(full_path, child_argv, reduced_env);
            perror("execve failed (убедитесь, что файл 'child' существует)");
            exit(1);
        } else if (pid > 0) {
            child_idx++;
            wait(NULL);
        }
    }

    for (int i = 0; i < env_count; i++) free(reduced_env[i]);
    free(reduced_env);
    return 0;
}
