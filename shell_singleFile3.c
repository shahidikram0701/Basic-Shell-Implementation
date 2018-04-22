#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>
#include <fcntl.h>

#define BUFF 1024
void print_args(char **args, int pos);
int run_shell();
void spawn(char **args, int pos, int child_no);

typedef struct{
  char command[100];
  char timestamp[64];
}HISTORY;

typedef struct{
  char alias[30];
  char alias_args[100];
}ALIAS;

ALIAS aliases[100];
int alias_index = 0;

HISTORY hist[100];
int hist_iter = 0;
char filename_in[100];
char filename_out[100];
int i;
bool redirect_in = false;
bool redirect_out = false;
int pos_redirect_in = -1;
int pos_redirect_out = -1;
int fd_in;
int fd_out;

char last_file[50][50];
int last_file_index = -1;

int pipe_pos = -1;
int fd[2];

int main(int argc, char const *argv[]) {
  while(run_shell());
  return 0;
}

void print_args(char **args, int pos){
  printf("\nPrinting the arguments\n");
  int i;
  for(i = 0; i < pos; i++){
    printf("%s\n", args[i]);
  }
}

int run_shell(){
  printf("\n $ ");

  char **args;
  args = (char **)malloc(BUFF);
  int i;
  for(i = 0; i < BUFF; i++){
    args[i] = (char *)malloc(BUFF);
  }

  int pos = 0;
  char *buffer = (char*)malloc(BUFF);
  size_t size = 0;

  //gets(buffer, BUFF, stdin);
  //char *string = (char*)malloc(sizeof(100));
  char c = getchar();
  i = 0;
  while(c != '\n'){
    *(buffer + i) = c;
    i++;
    c = getchar();
  }
  *(buffer + i) = '\0';


  int x ;
  for(x = 0; x < alias_index; x++){
    if(strcmp(buffer, aliases[x].alias) == 0){
      strcpy(buffer, aliases[x].alias_args);
      break;
    }
  }
  label:
  printf("command = %s\n", buffer);
  // push it to history structure
  if(buffer[0] != '%'){
    strcpy(hist[hist_iter].command, buffer);

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char s[64];
    strftime(s, sizeof(s), "%c", tm);
    //sleep(1);
    strcpy(hist[hist_iter++].timestamp, s);
  }

  char *token = strtok(buffer, " ");
  while(token != NULL){
    strcpy(args[pos++], token);
    token = strtok(NULL, " ");
  }
  if(pos == 1){
    if(strcmp(args[0], "exit") == 0){
      return 0;
    }
  }

  print_args(args, pos);

  if(strcmp(args[0], "cd") == 0){
      chdir(args[1]);
  }
  else if(strcmp(args[0], "history") == 0){
    int j = 0;
    printf("\n");
    while(j < hist_iter){
      printf("%d. %s\t\t\t%s\n", j+1, hist[j].command, hist[j].timestamp);
      j ++;
    }
  }
  else if(strcmp(args[0], "alias") == 0){
    strcpy(aliases[alias_index].alias, args[1]);
    int k = 0;
    while(buffer[k] != '='){
      k++;
    }
    printf("i = %d\n", i);
    k += 2;
    int m = 0;
    while(k < i){
      //printf("buffer[k] = %c\n", buffer[k]);
      if(buffer[k] != '\0')
        aliases[alias_index].alias_args[m++] = buffer[k];
      else
        aliases[alias_index].alias_args[m++] = ' ';
      k++;
    }
    aliases[alias_index].alias_args[m] = '\0';
    //printf("alias command = %s\n", aliases[alias_index].alias_args);
    alias_index++;
  }
  else if(strcmp(args[0], "aliases") == 0){
    printf("ALIASES\n\n");
    int k;
    for(k = 0; k < alias_index; k++){
      printf("%d. alias = %s\t\t", k + 1, aliases[k].alias);
      printf("alias_command = %s\n", aliases[k].alias_args);
    }
  }
  else{
    for(i = 0; i < pos; i++){
      if(strcmp(args[i], "<") == 0){
        pos_redirect_in = i;
        redirect_in = true;
        strcpy(filename_in, args[i+1]);
      }
      else if(strcmp(args[i], ">") == 0){
        pos_redirect_out = i;
        redirect_out = true;
        strcpy(filename_out, args[i+1]);
      }
      else if(strcmp(args[i], "|") == 0){
        //printf("pipe symbol\n");
        pipe_pos = i;
      }
    }
    if(pipe_pos == -1){
      //printf("no pipe\n");
      if(strcmp(args[0], "editor") == 0){
          strcpy(args[0], "python");
          strcpy(args[1], "/stuff/USP_Shell/editor.py");
          pos = 2;
      }
      else if(strcmp(args[0], "translate") == 0){
          char file[100];
          char language[100];
          strcpy(file, args[1]);
          strcpy(language, args[2]);
          strcpy(args[0], "python3");
          strcpy(args[1], "/stuff/USP_Shell/trans.py");
          strcpy(args[2], file);
          strcpy(args[3], language);
          pos = 4;
      } else if (strcmp(args[0], "open-file") == 0) {
        strcpy(args[0], "xdg-open");
        last_file_index++;
        strcpy(last_file[last_file_index], args[1]);
      }
      else if (strcmp(args[0], "open-prev-file") == 0) {
        strcpy(args[0], "xdg-open");
        int n = atoi(args[1]);
        strcpy(args[1], last_file[last_file_index - n + 1]);
      }
      else if(args[0][0] == '.'){
        char file[100];
        strcpy(file, args[0] + 2);
        //printf("Shell script - %s\n", file);
        strcpy(args[0], "bash");
        strcpy(args[1], file);
        pos = 2;
        //printf("%s\n", args[0]);
        //printf("%s\n", args[1]);
      }
      else if(args[0][0] == '%'){
        //printf("History\n");
        char rew[4];
        strcpy(rew, args[0] + 1);
        int prev = atoi(rew);
        printf("latest %dth command\n", prev);
        printf("hist = %d - %s\n", hist_iter - prev, hist[hist_iter - prev].command);
        strcpy(buffer, hist[hist_iter - prev].command);
        printf("buffer = %s\n", buffer);
        pos = 0;
        goto label;
      }
      spawn(args, pos, -1);
    }
    else{
      //printf("pipe\n");
      if(fork() == 0){
        pipe(fd);
        spawn(args, pos, 1);

        if(pipe_pos != -1){
          int x = pipe_pos;
          args[pos] = (char*)0;
          //printf("Before spawning New Child : pipe_pos = %d\n", pipe_pos);
          //pipe(fd);
          spawn(&args[x + 1], pos, 2);
          //printf("I am here!!!\n");
        }
      }
      else{
        wait(NULL);
        pipe_pos = -1;
      }
    }

  }
  return 1;
}

void spawn(char **args, int pos, int child_no){

  int child = fork();

  if(child == 0){
    /*
    char base_path[20] = "bin/";
    char path[BUFF];
    strcat(path, base_path);
    strcat(path, args[0])
    */
    if(child_no == 1){
      //printf("Entered\n");
      //close(1);
      close(fd[0]);
      dup2(fd[1], 1);
      close(fd[1]);
    }

    if(child_no == 2){
      //close(0);
      //dup2(fd[1], 1);
      exit(0);
  }
    if(pos_redirect_out != -1){
      if(pos_redirect_in != -1){
        if(pos_redirect_in < pos_redirect_out){
          args[pos_redirect_in] = (char*)0;
          close(0);
          fd_in = open(filename_in, O_RDWR);
        }
        else{
          args[pos_redirect_out] = (char*)0;
          close(1);
          fd_out = open(filename_out, O_CREAT|O_RDWR, 0777);
        }
      }
      else{
          printf("pos_redirect_out = %d\n", pos_redirect_out);
          printf("filename_out = %s\n", filename_out);
          args[pos_redirect_out] = NULL;
          close(1);
          fd_out = open(filename_out, O_CREAT|O_RDWR, 0777);
      }
    }
    else{
      if(pos_redirect_in != -1){
        args[pos_redirect_in] = (char*)0;
        close(0);
        fd_in = open(filename_in, O_RDWR);
      }
      else{
        args[pos++] = (char*)0;
      }
    }

    if(pipe_pos != -1){
      args[pipe_pos] = (char*)0;
      // execvp(args[pipe_pos+1], &args[pipe_pos + 1])
    }

    //printf("\nChild process - %d\n", getpid());
    //printf("Spawned to execute the command - %s\n", args[0]);

    printf("\n\n");
    //printf("Child number %d exiting\n", child_no);
    if(execvp(args[0], args) == -1){
      printf("Command Unrecognized!!!\n");
    }

    printf("Child number %d exiting\n", child_no);
    exit(0);
  }
  else if(child < 0){
    printf("Could not spawn child\n");
  }
  else{
    wait(NULL);
    if(pos_redirect_in != -1){
      pos_redirect_in = -1;
    }
    if(pos_redirect_out != -1){
      pos_redirect_out = -1;
    }
  }

  if(child_no == 2){
      close(fd[1]);
      dup2(fd[0], 0);
      close(fd[0]);
      execvp(args[0], args);
  }

}
