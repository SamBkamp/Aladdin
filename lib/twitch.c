#ifndef twitch

struct sockaddr_in twitchaddr;
int twitchsock;
char current[100];

struct connectionData{
  int sockfd;
  pthread_t writerThread;
  pthread_t readerThread;
};


int sendMsg(char* payload){
  if(write(twitchsock, payload, strlen(payload)) == -1){
	perror("failed to write to socket\n"); 
	return -1;
  }
  return 0;
}


int setup(){
    //here to joinChannel could be put in its own function in the twitch.h file?
  struct connectionData conData;
  twitchsock = socket(AF_INET, SOCK_STREAM, 0);
  if(twitchsock == -1){
    perror("couldn't connect to socket");
    return -1;
  }
  //printf("connected to socket\n");
  struct hostent* host = gethostbyname("irc.chat.twitch.tv");
  if(host == NULL){
    perror("error connecting to twitch servers");
    return -1;

  }
  //setup addresses
  bzero(&twitchaddr, sizeof(twitchaddr));
  twitchaddr.sin_family = AF_INET;
  twitchaddr.sin_addr.s_addr = *(long *)host->h_addr_list[0];
  twitchaddr.sin_port = htons(6667);
  conData.sockfd = twitchsock;
   if (connect(twitchsock, (struct sockaddr*)&twitchaddr, sizeof(twitchaddr)) != 0) {
    perror("failed to connect to server\n");
    return -1;
  }

   return 0;
}

int joinChannel(char* message){
  char payload[50];
  char buff[500];
  sprintf(payload, "JOIN %s\r\n", message);

  if(sendMsg(payload)==-1){
    perror("couldn't send message");
    return -1;
  }
  sprintf(current, "%s", message);

  if(read(twitchsock, buff, sizeof(buff))==-1){
    perror("Warning: couldn't read from socket");
    return 0;
  }
  printf("%s", buff);
  bzero(buff, sizeof(buff));

  return 0;
}

#endif
