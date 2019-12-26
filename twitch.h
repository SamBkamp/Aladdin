#ifndef twitch

struct sockaddr_in twitchaddr;
int twitchsock;
char current[100];

struct connectionData{
  int sockfd;
  pthread_t writerThread;
  pthread_t readerThread;
};


void sendMsg(char* payload){
  if(write(twitchsock, payload, strlen(payload)) == -1){
	perror("failed to write to socket\n"); 
      }
}


void setup(){
    //here to joinChannel could be put in its own function in the twitch.h file?
  struct connectionData conData;
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
  conData.sockfd = twitchsock;
   if (connect(twitchsock, (struct sockaddr*)&twitchaddr, sizeof(twitchaddr)) != 0) {
    printf("-----[failed to connect to server]-----\n");
    exit(0);
  }
  
  //inital login to twitch irc servers
   char buff[1000];
   char payload[100] = "PASS ";
   //this entire strcat section will be changed when get config files/db implemented
   //TODO 17: change this to sprintf lmao
   strcat(payload, password);
   strcat(payload, "\r\nNICK ");
   strcat(payload, nick);
   strcat(payload, "\r\n");
   
   if(write(twitchsock, payload, strlen(payload)) == -1){
     printf("failed to write to socket \n");
   }
   
   
   read(twitchsock, buff, sizeof(buff));
   printf("%s", buff);


}

//TODO 20: move joinChannels to headerfile
void joinChannel(char* message){
  char payload[50];
  char buff[1024];
  sprintf(payload, "JOIN %s\r\n", message);
  sendMsg(payload); 
  sprintf(current, "%s", message);
  read(twitchsock, buff, sizeof(buff));
  printf("%s", buff);
  bzero(buff, sizeof(buff));
}

#endif
