#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdfile.h"
#include <ncurses.h>

typedef struct {
    char *cmd;
    char *msg;
} Commands;

typedef struct {
  char* word;
} banwords;

Commands *allCommands;
banwords* allBanWords;
int cmdlen;
int banlen;

int lines = 0;
int banLines = 0;

#define cmdFile "commands.csv"
#define MAX_LEN 4096
#define banFile "banlist.csv"


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

int banlist_init() {
    FILE *fp = fopen(banFile, "r");
    
    //if file doesn't exist exit function call
    //temporary
    if (fp == NULL)
        return -1;

    char buffer[MAX_LEN + 1];
    
    while(fgets(buffer, MAX_LEN, fp) != NULL)
        banLines++;
    
    
    //reset position in file to beginning
    fseek(fp, 0, SEEK_SET);
    allBanWords = (banwords *)malloc(sizeof(banwords)*banLines);
    banwords *tmp = allBanWords;
    banlen = 0;
    while(fgets(buffer, MAX_LEN, fp) != NULL) {
        int lineSize = strlen(buffer);

        tmp->word = (char *)malloc(lineSize);
	sscanf(buffer, "%[^\n]", tmp->word);
	tmp++;
	banlen++;
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


int banlist_test_command(const char *test_bw) {
  int i;
  banwords *tmp = allBanWords;
  for(i = 0; i < banlen; i++, tmp++) {
    if (strcmp(test_bw,tmp->word) == 0) {
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
  for(int j = i; j < cmdlen; j++, tmp++, buff++){
    buff->cmd = tmp->cmd;
    buff->msg = tmp->msg;
  }

  lines--;
  allCommands = (Commands *)realloc(allCommands, sizeof(Commands)*lines);
  cmdlen--;
  return 0;
}


int banlist_remove_command(char* remove_bw){
  int i;
  banwords* tmp = allBanWords;
  banwords* buff;
  for(i=0; i < banlen; i++, tmp++){
    if (strcmp(remove_bw,tmp->word) == 0) {
      buff = tmp;
      break;
    }
  }
  tmp++; //tmp is one ahead of buff
  for(int j = i; j < cmdlen; j++, tmp++, buff++){
    buff->word = tmp->word;
  }

  banLines--;
  allBanWords = (banwords *)realloc(allBanWords, sizeof(banwords)*banLines);
  banlen--;
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


int banlist_add_command(char* banword){
  banLines++;
  banwords* tmp = (banwords *)realloc(allBanWords, sizeof(banwords)*banLines);
  banwords* lastcmd = (tmp + banlen);
  lastcmd->word = (char *)malloc(strlen(banword));
  strcpy(lastcmd->word, banword);
  allBanWords = tmp;
  banlen++;
  return 0;
}


void list_bot_commands(WINDOW* window){ //for debugging
  int i;
  Commands* tmp = allCommands;
  char buffer[1024];
  char intBuffer[20];
  sprintf(intBuffer, "total commands: %d", cmdlen);
  printToScreen(intBuffer, window);
  for(i=0; i < cmdlen; i++, tmp++){
    sprintf(buffer, "%s --> %s", tmp->cmd, tmp->msg);
    printToScreen(buffer, window);
  }
}


void list_ban_list(WINDOW* window){ //for debugging
  int i;
  banwords* tmp = allBanWords;
  char buffer[1024];
  char intBuffer[20];
  sprintf(intBuffer, "total ban words: %d", banlen);
  printToScreen(intBuffer, window);
  for(i=0; i < banlen; i++, tmp++){
    sprintf(buffer, "%s", tmp->word);
    printToScreen(buffer, window);
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


int banlist_finish() {
  int i;
  FILE* fp = fopen("banlist.csv", "w");
  if(fp==NULL){
    printf("couldn't open commands.csv\n");
    return -1;
  }
  for(i = 0; i < banlen; i++) {
    banwords *curr = (allBanWords + i);
    if(fprintf(fp, "%s\n", curr->word) < 0){
      printf("failed to save command `%s`\n", curr->word);
    }
    free(curr->word);
  }
  free(allBanWords);
  fclose(fp);
  return 0;
}
