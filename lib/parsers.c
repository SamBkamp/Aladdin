#include <stdio.h>
#include <stdlib.h>

//TODO 35: add GOTO for failing conditions
//returns the sender of a twitch chat command, expects line of twitch chat 
char* commandSender(char* strinput){
  char* tok = strtok(strinput, "!"); //expected input: :usr!usr@usr.tmi.twitch.tv PRIVMSG #chan :msg
  tok++; //skips the first character ':'
  return tok;
}

//analyses twitch chat for commands
char* returnCommand(char* strinput){
  sscanf(strinput, "%[^\r\n]", strinput);

  char* token = strtok(strinput, ":");
  token = strtok(NULL, ":"); //parses for contents of packet
  char* command = strtok(token, " "); //parses for first word of message
  char finalCommand[strlen(command-2)];
  char* retval = (char *)malloc(strlen(command));
  strcpy(retval, command);
  return retval;
}

//helper function for oauthsetup to improve ux
void askUser(char* authbuffer, char* nickbuffer){
  printf("You can get your twitch token from twitchapps.com/tmi\ntwitch Oauth key: ");
  fgets(authbuffer, 1024, stdin); 
  printf("twitch bot account name: ");
  fgets(nickbuffer, 1024, stdin);
}

//setups userinfo.txt
int oauthsetup(){
  FILE* fp = fopen("userinfo.txt", "w");
  if(fp==NULL){
    printf("Error: could't open userinfo.txt");
    return -1;
  }
  char twitchauth[37];
  char botnick[100];
  askUser(twitchauth, botnick);
  //user input validation i.e checks for 'oauth:' prefix
  char buffer[100];
  int i;
  for(i=0; i<=strlen(twitchauth) && twitchauth[i] != ':'; i++){
    buffer[i] = twitchauth[i];
  }
  buffer[i] = '\0';
  if(strcmp(buffer, "oauth")!=0){
    printf("invalid format. Please add the 'oauth:' prefix to your key\n");
    askUser(twitchauth, botnick);
  }
  if(fprintf(fp, "pass=%s\nnick=%s", twitchauth, botnick)==-1){
    perror("Error: couldn't write to file");
    return -1;
  }
  return 1;
}
