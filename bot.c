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
#include "lib/parsers.c"

struct connectionData *connData;


int  analyseInput(char* strinput);
void* readerTHEThread(void* context);
void* writerTHEThread(void* context);
int writeToFile(char* command, char* body);
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



//analyses the user input (streamer side, not input from twitch channel)
int analyseInput(char* strinput){
  int commandLen = strlen(strinput);
  sscanf(strinput, "%[^\n]", strinput);
  char* strinput2 = strdup(strinput);
  char* token = strtok(strinput, " ");
  
  if(strcmp(token, "say")==0){

    if(strlen(strinput2) < 5){
      printf("usage: say <message>");
    }
    char buuf[50];
    char* commandBody;

    commandBody = strchr(strinput2,' ');
    commandBody++;
    
    sprintf(buuf, "PRIVMSG %s :%s\r\n", current, commandBody);
    if(sendMsg(buuf)==-1){
      return -1;
    }
    return 0;
  }else if (strcmp(token, "quit")==0){
    finish();
    pthread_kill(connData->writerThread, SIGTERM);
    pthread_kill(connData->readerThread, SIGTERM);
  }else if(strcmp(token, "ls")==0){
    list_bot_commands();
    return 0;
  }else if(strncmp(token, "rmcmd", 5)==0){
    if(strlen(strinput2)<=6){ //checks for arguments
      printf("rmcmd <command>   %s\n", strinput2);
      return 0;
    }
    char* commandName = strtok(NULL, " ");
    if(test_command(commandName, NULL, 100)!=1){
      printf("'%s' is not a command\n", commandName);
      return 0;
    }
    if(remove_command(commandName)==-1){
      return -1;
    }
    printf("removed command '%s'\n", commandName);
    return 0;
  }else if(strcmp(token, "addcmd")==0){

    if(strlen(strinput2)<=7){ //checks for arguments
      printf("addcmd <command> <message>\n");
      return 0;
    }
    
    char* commandName = strtok(NULL, " ");
    if (strcmp(commandName, "!credits")==0 || strcmp(commandName, "!vanish")==0){
      printf("Error: can't override command '%s'\n", commandName);
      return 0;
    } 
    if(test_command(commandName, NULL, 1)==1){
      printf("Error: command '%s' already exists\n", commandName);
      return 0;
    }

    char* body = strchr(strinput2, ' ');
    body++;
    body = strchr(body, ' ');
    body++; //all of this just gets the pointer to string after the first two spaces
    
    if(strlen(commandName)==1 || strlen(body)==0){
      printf("addcmd <command> <message>\n");
      return 0;
    }
    add_command(commandName, body);
    return 0;
  }
  return -2;
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
    //get streamer input
    fgets(buffer, 1024, stdin);
    if(analyseInput(buffer)==-2){
      printf("unrecognised command\n");
    }
  }
}




