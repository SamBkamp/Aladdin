#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h> //hostent
#include <stdlib.h>
#include <unistd.h> //write
#include <string.h> //strtok
#include <errno.h>
#include <pthread.h>
#include "userinput.h" //username and password
#include "scanfuck.h"
#include <signal.h>
#include "twitch.h"

struct connectionData *connData;


//I hate this entire function
char* analyseInput(char* strinput){
  char command[10];
  char body[100];
  int i;

  //gets the first 'word' of the input 
  for(i=0; strinput[i] != ' '  && i < strlen(strinput); i++){
    char buff[10] = "";
    buff[strlen(buff)] = strinput[i];
    buff[strlen(buff)+1] = '\0';
    strcat(command, buff);
    bzero(buff, sizeof(buff));
  }
  //I hate this loop
  for(int j=i; j < strlen(strinput); j++){
    char buff[10] = "";
    buff[strlen(buff)] = strinput[j];
    buff[strlen(buff)+1] = '\0';
    strcat(body, buff);
    bzero(buff, sizeof(buff));
  }
  
  if(strcmp(command, "say")==0){
    char buuf[50];
    sprintf(buuf, "PRIVMSG %s :%s\r\n", current, body); 
    char* addr = malloc(50);
    memcpy(addr, &buuf, sizeof(buuf));
    return addr;
  }else if (strcmp(command, "quit")==0){
    pthread_kill(connData->writerThread, SIGINT);
    pthread_kill(connData->readerThread, SIGINT);
  }
  return NULL;
}



void* readerTHEThread(void* context){
  char buff[1024];
  
  for (;;){
    bzero(buff, sizeof(buff));
    read(twitchsock, buff, sizeof(buff));
    
    if(strcmp(buff, "PING :tmi.twitch.tv\r\n") == 0){
      char* payload = "PONG :tmi.twitch.tv\r\n";
      sendMsg(payload);
    }else {
      printf("\r%s", buff);
      sleep(0.5);
      printf("[%s]> ", current);
      fflush(stdout);
    }
  }
  
  printf("closing writer thread\n");
}

void* writerTHEThread(void* context){
  char payload[50];
  
  sprintf(payload,"PRIVMSG %s :botbkamp is here! HeyGuys\r\n", current);
  sendMsg(payload);
  for (;;){
    printf("[%s]> ", current);
    fflush(stdout);
    char* message = scanfuck();
    char* payload = analyseInput(message);
    if(payload == NULL){
      printf("unrecognised command\n");
    }else {
      sendMsg(payload);
      free(message);
    }
    free(payload);
  }
}



int main(){
  struct connectionData conData;

  setup();
  joinChannel("#bkamp_");
  
  
  pthread_t writerThread;
  pthread_t readerThread;
  
  
  pthread_create(&writerThread, NULL, writerTHEThread, (void *) &conData);
  pthread_create(&readerThread, NULL, readerTHEThread, (void *) &conData);
  
  conData.writerThread = writerThread;
  conData.readerThread = readerThread;
  
  connData = &conData;
  
  pthread_join(writerThread, NULL);
  sleep(1);
  pthread_join(readerThread, NULL);
  
  
  return 0;
}




