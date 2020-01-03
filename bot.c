#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h> //hostent
#include <stdlib.h>
#include <unistd.h> //write
#include <string.h> //strtok
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include "lib/parseInfo.c"
#include "lib/twitch.c"
#include "lib/cmdfile.h"
struct connectionData *connData;


int  analyseInput(char* strinput);
void* readerTHEThread(void* context);
void* writerTHEThread(void* context);
char* returnCommand(char* strinput);
int writeToFile(char* command, char* body);
char* commandSender(char* strinput);
int oauthsetup();

int main(int argc, char* argv[]){
  struct connectionData conData;

  if(argc < 2){
    printf("usage: Aladdin --join <channel name>\n");
    exit(0);
  }
  if (strcmp(argv[1], "--join")==0){
    if(argc < 3){
      printf("usage: Aladdin --join <channel name>\n");
      exit(0);
    }
  }else if (strcmp(argv[1], "--setup")==0){
    //printf("--setup has not been implemented yet\n");
    //exit(0);
    if(oauthsetup()==-1){
      printf("fatal error, exiting now\n");
      exit(-1);
    }
    exit(0);
  }else {
    printf("usage: Aladdin --join <channel name>\n");
    exit(0);
  }
  
  if (parseInfo()==-1){
    printf("run with --setup to add twitch auth\n");
    return -1;
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
  sscanf(strinput, "%[^\r\n]", strinput);

  char* token = strtok(strinput, ":");
  token = strtok(NULL, ":"); //parses for contents of packet
  char* command = strtok(token, " "); //parses for first word of message
  char finalCommand[strlen(command-2)];
  char* retval = (char *)malloc(strlen(command));
  strcpy(retval, command);
  return retval;
}

//TODO 35: add GOTO for failing conditions
int writeToFile(char* command, char* body){
  FILE* f = fopen("commands.csv", "a");
  if (f == NULL){
    perror("error: couldn't open commands.csv");
    fclose(f);
    return -1;
  }
  if(fprintf(f, "%s,%s\n", command, body)==-1){
    perror("couldn't write to file");
    fclose(f);
    return -1;
  }
  fclose(f);
  return 0;
};

//returns the sender of a twitch chat command, expects line of twitch chat 
char* commandSender(char* strinput){
  char* tok = strtok(strinput, "!"); //expected input: :usr!usr@usr.tmi.twitch.tv PRIVMSG #chan :msg
  tok++; //skips the first character ':'
  return tok;
}

//setups userinfo.txt
int oauthsetup(){
  FILE* fp = fopen("userinfo.txt", "w");
  if(fp==NULL){
    printf("Error: could't open userinfo.txt");
    return -1;
  }
  printf("You can get your twitch token from twitchapps.com/tmi\ntwitch Oauth key: ");
  char twitchauth[37];
  char botnick[100];
  scanf("%s", twitchauth);
  printf("twitch bot account name: ");
  scanf("%s", botnick);
  if(fprintf(fp, "pass=%s\nnick=%s", twitchauth, botnick)==-1){
    perror("Error: couldn't write to file");
    return -1;
  }
  return 1;
}


//analyses the user input (streamer side, not input from twitch channel)
int analyseInput(char* strinput){
  int commandLen = strlen(strinput);
  char* token = strtok(strinput, " ");
  
  if(strcmp(token, "say")==0){

    char buuf[50];
    char commandBody[commandLen];
    
    sprintf(commandBody, ""); //init the char array 
    token = strtok(NULL, " ");
    while(token != NULL){
      sprintf(commandBody, "%s ", strcat(commandBody, token));
      token = strtok(NULL, " ");
    }
    
    sprintf(buuf, "PRIVMSG %s :%s\r\n", current, commandBody);
    if(sendMsg(buuf)==-1){
      return -1;
    } 
  }else if (strcmp(token, "quit\n")==0){
    finish();
    pthread_kill(connData->writerThread, SIGTERM);
    pthread_kill(connData->readerThread, SIGTERM);
  }else if(strncmp(token, "addcmd", 6)==0){

    if(strlen(strinput)==7){ //checks for arguments
      printf("addcmd <command> <message>\n");
      return 0;
    }
    
    char* commandName = strtok(NULL, " ");
    if (strcmp(commandName, "!credits")==0 || strcmp(commandName, "!vanish")==0){
      printf("Error: can't override command '%s'\n", commandName);
      return 0;
    }
    char buffer[10]; //TODO : add method in lib to not require output buffer
    if(test_command(commandName, buffer, 1)==1){
      printf("Error: command '%s' already exists\n", commandName);
      return 0;
    }
    char body[strlen(strinput)-strlen(commandName)-6]; //char body has to be size of command body, so total length - length of commandName - length of strin 'addcmd' (6)
    token = strtok(NULL, " ");
    sprintf(body, ""); 
    while(token != NULL){
      sprintf(body, "%s ", strcat(body, token));
      token = strtok(NULL, " ");
    }
    body[strlen(body)-2] = 0;
    if(strlen(commandName)==1 || strlen(body)==0){
      printf("addcmd <command> <message>\n");
      return 0;
    }
    if(writeToFile(commandName, body)==-1){
      return -1;
    }
    //TODO 34: hacky way to add new command to struct of commands
    finish();
    init();
    return 0;
  }
  return -1;
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
      if(strcmp(command, "!credits")==0){ 
	//hard coded command
	char payload[100];
	sprintf(payload,"PRIVMSG %s :This bot was written by SamBkamp at: https://github.com/SamBkamp/Aladdin\r\n", current);
	if(sendMsg(payload)==-1){
	  return NULL;
	}
      }else if(strcmp(command, "!vanish")==0){
	char* payload = (char *)malloc(1024);
	sprintf(payload, "PRIVMSG %s :/timeout %s 1\r\n", current, commandSender(buff));
	sendMsg(payload);
	free(payload);
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
    analyseInput(buffer);
  }
}




