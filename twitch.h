#ifndef twitch

void sendMsg(char* payload){
  if(write(twitchsock, payload, strlen(payload)) == -1){
	perror("failed to write to socket\n"); 
      }
}

//TODO 20: move joinChannels to headerfile
void joinChannel(char* message){
  char payload[50];
  sprintf(payload, "JOIN %s\r\n", message);
  sendMsg(payload); 
  sprintf(current, "%s", message);
}

#endif
