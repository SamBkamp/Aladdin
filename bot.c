#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h> //hostent
#include <stdlib.h>
#include <unistd.h> //write
#include <string.h> //strtok
#include <errno.h>

struct sockaddr_in twitchaddr;
int twitchsock;

void login(){
  //inital login to twitch irc servers
  char buff[1000];
  char payload[100] = "PASS oauth:n7g340l7nz7txpvr8tgj6y2rkhzyhv\r\nNICK BotBkamp\r\n";

  if(write(twitchsock, payload, strlen(payload)) == -1){
    printf("failed to write to socket \n");
    return NULL;
  }

  read(twitchsock, buff, sizeof(buff));
  printf("%s", buff);
  //TODO 1: move the next two writes to their own function
  //lets move the JOIN command to its own function so streamer can change on the go and lets move the
  //hello message as the first write in the writer thread
  strcpy(payload, "JOIN #bkamp_\r\n");

  if(write(twitchsock, payload, strlen(payload)) == -1){
    printf("failed to write to socket \n");
    return NULL;
  }

  bzero(buff, sizeof(buff));
  
  read(twitchsock, buff, sizeof(buff));
  printf("%s", buff);

  sleep(1);

  strcpy(payload, "PRIVMSG #bkamp_ :botbkamp is here! HeyGuys\r\n");
  
  if(write(twitchsock, payload, strlen(payload)) == -1){
    printf("failed to write to socket \n");
    return NULL;
  }

}


int main(){
  twitchsock = socket(AF_INET, SOCK_STREAM, 0);
  if(twitchsock == -1){
    printf("couldn't connect to socket");
    exit(0);
  }
  printf("connected to socket\n");
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
  
  if (connect(twitchsock, (struct sockaddr*)&twitchaddr, sizeof(twitchaddr)) != 0) {
    printf("-----[failed to connect to server]-----\n");
    exit(0);
  }
  
  login(twitchsock);
  //int log = login(twitchsock);
  /*if(log  == -1){
  printf("failed to send data to server");
    exit(0);
    }*/ 
  //printf("\n**logged in successfully**\n");
  
  return 0;
}




