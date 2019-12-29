#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h> //hostent
#include <stdlib.h>
#include <unistd.h> //write
#include <string.h> //strtok
#include <errno.h>
#include <pthread.h>
#include "userinput.h" //username and password
#include <signal.h>
#include "twitch.h"
#include "cmdfile.h"

struct connectionData *connData;


char* analyseInput(char* strinput);
void* readerTHEThread(void* context);
void* writerTHEThread(void* context);
char* returnCommand(char* strinput);


int main(int argc, char* argv[]){
  struct connectionData conData;
  
  if(argc < 3){
    printf("usage: Aladdin --join <channel name>\n");
    exit(0);
  }
  if (strcmp(argv[1], "--join")!=0){
    printf("usage: Aladdin --join <channel name>\n");
    exit(0);
  }
 
  if(init()==-1){
    printf("Warning: couldn't load commands\n");
  }
  
  setup();
  char channelName[20];
  sprintf(channelName, "#%s", argv[2]);
  joinChannel(channelName);
  
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

//analyses twitch chat for commands
char* returnCommand(char* strinput){
  char* token = strtok(strinput, ":");
  char* retval = malloc(20);
  token = strtok(NULL, ":");
  memcpy(retval, token, 19);
  return retval;
}

//analyses the user input (streamer side, not input from twitch channel)
char* analyseInput(char* strinput){

  char* token = strtok(strinput, " ");
  
  if(strcmp(token, "say")==0){
    char buuf[50];
    sprintf(buuf, "PRIVMSG %s :%s\r\n", current, strtok(NULL, " ")); 
    char* addr = malloc(50);
    memcpy(addr, &buuf, sizeof(buuf));
    return addr;
  }else if (strcmp(token, "quit\n")==0){
    finish();
    pthread_kill(connData->writerThread, SIGTERM);
    pthread_kill(connData->readerThread, SIGTERM);
    printf("-------------------\n");
  }
  
  return NULL;
}

//thread that reads from socket (twitch chat)
void* readerTHEThread(void* context){
  char buff[500];
  char outputmsg[1024];
  
  for (;;){
    bzero(buff, sizeof(buff));
    read(twitchsock, buff, sizeof(buff));    
    //catches ping from twitch servers 
    if(strcmp(buff, "PING :tmi.twitch.tv\r\n") == 0){

      char* payload = "PONG :tmi.twitch.tv\r\n";
      if(sendMsg(payload)==-1){
	return NULL;
      }
    }else {
      printf("\r%s", buff);
      sleep(0.5);
      printf("[%s]> ", current);
      fflush(stdout);
      
      char* command = returnCommand(buff);
      if(strcmp(command, "!credits\r\n")==0){ 
	//hard coded command
	char payload[100];
	sprintf(payload,"PRIVMSG %s :This bot was written by SamBkamp at: https://github.com/SamBkamp/Aladdin\r\n", current);

	if(sendMsg(payload)==-1){
	  return NULL;
	}
	
      }else if(test_command(command, outputmsg, 100)==1){
	char* addr = (char *)malloc(1024);
	sprintf(addr, "PRIVMSG %s :%s\r\n", current, outputmsg); 
        sendMsg(addr);
	free(addr);
      }
      free(command);
    }
  }
  
  printf("closing writer thread\n");
}

//thread that writes to socket
void* writerTHEThread(void* context){
  char payload[50];
  
  sprintf(payload,"PRIVMSG %s :%s is here! HeyGuys\r\n", current, nick);

  if(sendMsg(payload)==-1){
    return NULL;
  }
  
  for (;;){
    printf("[%s]> ", current);
    char buffer[1024];
    //get streamer inputt
    fgets(buffer, 1024, stdin);
    char* payload = analyseInput(buffer);
    if(payload == NULL){
      printf("unrecognised command\n");
    }else {
      if(sendMsg(payload)==-1){
	return NULL;
      }
    }
    free(payload);
  }
}




