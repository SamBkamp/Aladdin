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

#define cmdFile "commands.csv"
#define MAX_LEN 4096



int init() {
    FILE *fp = fopen(cmdFile, "r");
    
    //if file doesn't exist exit function call
    //temporary
    if (fp == NULL)
        return -1;

    char buffer[MAX_LEN + 1];
    int lines = 0;

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

        //fprintf(stderr, "Buffer: \"%s\"\n\tlinesize: %d\t cmdSize: %d\t msgSize: %d\n", buffer, lineSize, cmdSize, msgSize);

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


int finish() {
    int i;
    for(i = 0; i < cmdlen; i++) {
        Commands *curr = (allCommands + i);
        free(curr->cmd);
        free(curr->msg);
    }

    free(allCommands);
    return 0;
}
