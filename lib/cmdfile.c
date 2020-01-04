#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdfile.h"


typedef struct {
    char *cmd;
    char *msg;
} Commands;

Commands *allCommands;
int cmdlen;

int lines = 0;

#define cmdFile "commands.csv"
#define MAX_LEN 4096



int init() {
    FILE *fp = fopen(cmdFile, "r");
    
    //if file doesn't exist exit function call
    //temporary
    if (fp == NULL)
        return -1;

    char buffer[MAX_LEN + 1];

    //count number of lines in file
    while(fgets(buffer, MAX_LEN, fp) != NULL)
        lines++;
    
    //reset position in file to beginning
    fseek(fp, 0, SEEK_SET);
    allCommands = (Commands *)malloc(sizeof(Commands)*lines);
    Commands *tmp = allCommands;
    cmdlen = 0;
    while(fgets(buffer, MAX_LEN, fp) != NULL) {
        int lineSize = strlen(buffer);
        int cmdSize = strcspn(buffer, ",")+2;
	int msgSize = lineSize-cmdSize-1;

        tmp->cmd = (char *)malloc(sizeof(char)*cmdSize);
        tmp->msg = (char *)malloc(sizeof(char)*msgSize); 
	sscanf(buffer, "%[^,],%[^\n]", tmp->cmd, tmp->msg);
	//strcat(tmp->cmd, "\r\n");
	tmp++; 
        cmdlen++;
    }




    fclose(fp); 
    return 0;
}

int test_command(const char *test_cmd, char *outputMsg, int maxOutputLen) {
  int i;
  Commands *tmp = allCommands;
  for(i = 0; i < cmdlen; i++, tmp++) {
    if (strcmp(test_cmd,tmp->cmd) == 0) {
      if(outputMsg != NULL){
	strncpy(outputMsg, tmp->msg, maxOutputLen);
      }
      return 1;
    }
    
  }
  
  return 0;
}

int remove_command(char* remove_cmd){
  int i;
  Commands* tmp = allCommands;
  Commands* buff;
  for(i=0; i < cmdlen; i++, tmp++){
    if (strcmp(remove_cmd,tmp->cmd) == 0) {
      buff = tmp;
      break;
    }
  }
  tmp++;
  for(int j = i; j < cmdlen-i; j++, tmp++, buff++){
    buff->cmd = tmp->cmd;
    buff->msg = tmp->msg;
  }

  lines--;
  allCommands = (Commands *)realloc(allCommands, sizeof(Commands)*lines);
  cmdlen--;
  return 0;
}

int add_command(char* add_cmd, char* add_msg){
  lines++;
  Commands* tmp = (Commands *)realloc(allCommands, sizeof(Commands)*lines);
  Commands* lastcmd = (tmp + cmdlen);
  lastcmd->cmd = (char *)malloc(strlen(add_cmd));
  lastcmd->msg = (char *)malloc(strlen(add_msg));
  strcpy(lastcmd->cmd, add_cmd);
  strcpy(lastcmd->msg, add_msg);
  allCommands = tmp;
  cmdlen++;
  return 0;
}

void list_bot_commands(){ //for debugging
  int i;
  Commands* tmp = allCommands;
  printf("total commands: %d\n", cmdlen);
  for(i=0; i < cmdlen; i++, tmp++){
    printf("%s --> %s\n", tmp->cmd, tmp->msg);
  }
}

int finish() {
  int i;
  FILE* fp = fopen("commands.csv", "w");
  if(fp==NULL){
    printf("couldn't open commands.csv\n");
    return -1;
  }
  for(i = 0; i < cmdlen; i++) {
    Commands *curr = (allCommands + i);
    if(fprintf(fp, "%s,%s\n", curr->cmd, curr->msg) < 0){
      printf("failed to save command `%s`\n", curr->cmd);
    }
    free(curr->cmd);
    free(curr->msg);
  }
  free(allCommands);
  fclose(fp);
  return 0;
}
