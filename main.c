#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

void lsh_loop(void);
char *lsh_read_line(void);
char **lsh_split_line(char *line);
int lsh_launch(char **args);
int lsh_execute(char **args);

// 为shell本身添加命令
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

int main(int argc, char **argv)
{
    // 1. 读取并执行配置文件

    // 2. 读取命令并执行
    lsh_loop();

    // 3. 执行任何关闭命令、释放所有内存并终止


    return EXIT_SUCCESS;
}


/**
 * 处理命令步骤：
 * 1. 读取
 * 2. 解析
 * 3. 执行
 * */
void lsh_loop(void)
{
    char *line;
    char **args;
    int status;

    // 在执行中，打印一个提示，调用一个函数读取一行
    // 调用一个函数将行拆分为参数
    // 然后执行参数
    do {
        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        // 释放之前创建的行和参数
        free(line);
        free(args);
    }while(status); // 使用lsh_execute()返回的状态值确定何时退出
}


char *lsh_read_line(void)
{
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1) {
        // 读取字符
        c = getchar();

        if(c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        }else{
            buffer[position] = c;
        }
        position++;

        if(position >= bufsize){
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer){
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}


char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if(!tokens){
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;

        if(position >= bufsize){
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens){
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}


int lsh_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0){
        if(execvp(args[0], args) == -1){
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }else if(pid < 0) {
        perror("lsh");
    }else{
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}


char *builtin_str[] = {
        "cd",
        "help",
        "exit"
};

int (*builtin_func[]) (char **) = {
        &lsh_cd,
        &lsh_help,
        &lsh_exit
};

int lsh_num_builtins(){
    return sizeof(builtin_str) / sizeof(char *);
}

int lsh_cd(char **args)
{
    if(args[1] == NULL){
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    }else{
        if(chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}


int lsh_help(char **args)
{
    int i;
    printf("Stephen Brennan's LSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for(i = 0; i < lsh_num_builtins(); i++){
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}


int lsh_exit(char **args)
{
    return 0;
}

int lsh_execute(char **args)
{
    int i;

    if(args[0] == NULL){
        return 1;
    }

    for(i = 0; i < lsh_num_builtins(); i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }
    return lsh_launch(args);
}
