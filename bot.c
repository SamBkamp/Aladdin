#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h> //hostent
#include <stdlib.h>
#include <unistd.h> //write
#include <string.h> //strtok
#include <errno.h>
#include <pthread.h>
#include "userinput.h" //username and password


struct sockaddr_in twitchaddr;
int twitchsock;
char current[100];

struct connectionData{
  int sockfd;
};

void joinChannel(char* message){
  char payload[50];
  sprintf(payload, "JOIN %s\r\n", message);
  
  if(write(twitchsock, payload, strlen(payload)) == -1){
    printf("failed to write to socket \n");
    return;
  }
  sprintf(current, "%s", message);
}

//I hate this entire function
char* analyseInput(char* strinput){
  char command[10];
  char body[100];
  int i;
  for(i=0; strinput[i] != ' ' && i < strlen(strinput)-1; i++){
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
  }
  return NULL;
}

//TODO: move this method to header file
char* scanfuck(){ //gets char* of 'infinite' length | TODO: make a cap, because you could crash a system with a 9gb string. Could literally use fgets()
  int len_max = 10;
  int current_size = len_max;

  char* pStr = malloc(len_max);

  if (pStr != NULL) {
    int c = EOF;
    int i = 0;
    while ((c = fgetc(stdin)) != '\n') {
      pStr[i++] = (char)c;
      if (i == current_size){
	current_size = i + 1; //i++ wouldnt work for some reason
	pStr = realloc(pStr, current_size);
      }
    }
    pStr[i] = '\0';
  }
  return pStr;
  
}


void readerTHEThread(void* context){
  struct connectionData* connectionData = context;
  char buff[1024];
  for (;;){
    bzero(buff, sizeof(buff));
    read(twitchsock, buff, sizeof(buff));
    
    if(strcmp(buff, "PING :tmi.twitch.tv\r\n") == 0){
      char* payload = "PONG :tmi.twitch.tv\r\n";
      if(write(connectionData->sockfd, payload, strlen(payload)) == -1){
	perror("failed to write to socket");
	return;
      }
    }else {
      printf("\r%s", buff);
      sleep(0.5);
      printf("[%s]> ", current);
      fflush(stdout);
    }
  }
  
  printf("closing writer thread\n");
}

void writerTHEThread(void* context){
  struct connectionData *connectionData = context;
  const char* payload = "PRIVMSG #bkamp_ :botbkamp is here! HeyGuys\r\n";
  if(write(connectionData->sockfd, payload, strlen(payload)) == -1){
    perror("failed to write to socket");
    
  }
  for (;;){
    printf("[%s]> ", current);
    fflush(stdout);
    char* message = scanfuck();
    char* payload = analyseInput(message);
    if(payload == NULL){
      printf("unrecognised command\n");
    }else {
      if(write(connectionData->sockfd, payload, strlen(payload)) == -1){
	perror("failed to write to socket\n"); 
      }
      free(message);
    }
    free(payload);
  }
}



int main(){
  struct connectionData connectionData;
  twitchsock = socket(AF_INET, SOCK_STREAM, 0);
  if(twitchsock == -1){
    printf("couldn't connect to socket");
    exit(0);
  }
  //printf("connected to socket\n");
  struct hostent* host = gethostbyname("irc.chat.twitch.tv");
  if(host == NULL){
    printf("error connecting to twitch servers");
    exit(0);

  }
  //setup addresses
  bzero(&twitchaddr, sizeof(twitchaddr));
  twitchaddr.sin_family = AF_INET;
  twitchaddr.sin_addr.s_addr = *(long *)host->h_addr_list[0];
  twitchaddr.sin_port = htons(6667);
  connectionData.sockfd = twitchsock;
   if (connect(twitchsock, (struct sockaddr*)&twitchaddr, sizeof(twitchaddr)) != 0) {
    printf("-----[failed to connect to server]-----\n");
    exit(0);
  }
  
  //inital login to twitch irc servers
   char buff[1000];
   char payload[100] = "PASS ";
   //this entire strcat section will be changed when get config files/db implemented
   strcat(payload, password);
   strcat(payload, "\r\nNICK ");
   strcat(payload, nick);
   strcat(payload, "\r\n");
   
   if(write(twitchsock, payload, strlen(payload)) == -1){
     printf("failed to write to socket \n");
   }
   
   
   read(twitchsock, buff, sizeof(buff));
   printf("%s", buff);
   //TODO 1: move the next two writes to their own function
   //lets move the JOIN command to its own function so streamer can change on the go and lets move the
   //hello message as the first write in the writer thread
   
   joinChannel("#bkamp_");
   
   bzero(buff, sizeof(buff));
   
   read(twitchsock, buff, sizeof(buff));
   printf("%s", buff);
  

   pthread_t writerThread;
   pthread_t readerThread;
   
   pthread_create(&writerThread, NULL, writerTHEThread, (void *) &connectionData);
   pthread_create(&readerThread, NULL, readerTHEThread, (void *) &connectionData);
   
   pthread_join(writerThread, NULL);
   sleep(1);
   pthread_join(readerThread, NULL);

   
  return 0;
}




