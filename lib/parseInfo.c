#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define inputfile "userinfo.txt"


char password[50];
char nick[50];

int parseInfo(){
  
  FILE* fp = fopen(inputfile, "r");

  if (fp == NULL){
    printf("couldn't open userinfo.txt");
    return -1;
  }

  char buffer[2048];
  while(fgets(buffer, 2048, fp)!= NULL){ //goes over file line by line
    sscanf(buffer, "%[^\n]", buffer);
    char buff[10];
    for(int i = 0; i < strlen(buffer) && buffer[i] != '='; i++){ //parses character at ':'
      buff[i] = buffer[i];
      buff[i+1] = 0;
    }
    if(strcmp(buff, "nick")!=0 && strcmp(buff, "pass")!=0){ //checks to make sure prefix is correct
      printf("incorrectly formatted userinfo.txt\n%s\n", buff);
      return -1;
      }
    
    if(strcmp(buff, "nick")==0){ //assigns correct value to password || nick
      strtok(buffer, "=");
      strcpy(nick, strtok(NULL, "="));
    }else{
      strtok(buffer, "=");
      strcpy(password, strtok(NULL, "="));
    }
  }
  printf("nick is: %s\npass is: %s\n", nick, password);
  return 0;
}

