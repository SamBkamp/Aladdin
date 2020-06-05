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
#include "lib/parsers.c"
#include "Twitch-libc/twitchlib.h"
#include <ncurses.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

struct connectionData{
  int sockfd;
  pthread_t writerThread;
  pthread_t readerThread;
  SSL* ssl;
};

struct sockaddr_in twitchaddr;
struct connectionData *connData;
int twitchsock;
char currentChannel[100];

int  analyseInput(char* strinput);
void* readerTHEThread(void* context);
void* writerTHEThread(void* context);
int writeToFile(char* command, char* body);
int setupSocket();
void close_cycle();
void closeHandler(int signal);
void printToScreen(char* message, WINDOW* window);
void printFormatted(char* message, WINDOW* window);
SSL_CTX* InitCTX(void);
void ShowCerts(SSL* ssl);
int windowHeight, windowWidth;
WINDOW* mainwin;
WINDOW* textWin;
WINDOW* inputWin;
int lineIterator = -1;

#include "lib/cmdfile.h" //its ugly, I know but please bear with me

int main(int argc, char* argv[]){
  struct connectionData conData;
  
  if(argc < 2){
    printf("Copyright 2020 Sam Bonnekamp (sam@bonnekamp.net) under the GPLv3 license\n\n");
    printf("Aladdin --<option> [channel name]\n\n Option can be either:\n - join\n - setup\n");
    printf("if you join a channel you must supply the channel name you want to join\n");
    exit(0);
  }
  
  if (strcmp(argv[1], "--join")==0){
    if(argc < 3){
      printf("no channel name supplied... exiting\n");
      exit(0);
    }
  }else if (strcmp(argv[1], "--setup")==0){
    if(oauthsetup()==-1){
      printf("fatal error, exiting now\n");
      exit(-1);
    }
    exit(0);
  }else {
    printf("Copyright 2020 Sam Bonnekamp (sam@bonnekamp.net) under the GPLv3 license\n\n");
    printf("Aladdin --<option> [channel name]\n\n Option can be either:\n - join\n - setup\n");
    printf("if you join a channel you must supply the channel name you want to join\n");
    exit(0);
  }

  //dealing with data files
  
  if (parseInfo()==-1){
    printf("run with --setup to add twitch auth\n");
    return -1;
  } 

  if(init()==-1){
    printf("Warning: couldn't load commands\n");
  }
  
  if(banlist_init()==-1){
    printf("Warning: couldn't load banlist\n");
  }

 

  //signal handler

  signal(SIGINT, closeHandler);

  //sets up networking stuff
  SSL_library_init();
  SSL_CTX* ctx = InitCTX();
  SSL* ssl = SSL_new(ctx);
  twitchsock = twlibc_init(ssl); //sets up address
  SSL_set_fd(ssl, twitchsock);
  if(SSL_connect(ssl) < 1){
    printf("FATAL: Failed to connect to SSL socket\n");
    ERR_print_errors_fp(stderr);
    return 0;
  }
  ShowCerts(ssl);
  
  //set up ncurses
  
  mainwin = initscr();
  start_color();
  keypad(mainwin, TRUE);
  getmaxyx(mainwin, windowHeight, windowWidth);
  
  textWin = newwin(windowHeight-5, windowWidth-1, 0, 0);
  inputWin = newwin(3, windowWidth-1, windowHeight-4, 0);
  scrollok(textWin, TRUE);
  box(inputWin, '|', '-');
  wrefresh(inputWin);
  
  char buff[500];
  if(twlibc_setupauth(twitchsock, password, nick, buff, sizeof(buff))==-1){
    perror("failed to authenticate");
    close_cycle();
    return -1;
  }
  printFormatted(buff, textWin);

  
  char channelName[20];
  sprintf(channelName, "#%s", argv[2]);
  sprintf(currentChannel, "#%s", argv[2]);
  
  bzero(buff, sizeof(buff));
  
  if(twlibc_joinchannel(twitchsock, channelName, buff, sizeof(buff))==-1){
    perror("failed to write to socket");
    return -1;
  }
  
  printFormatted(buff, textWin);
  
  strcpy(currentChannel, channelName);
  
  pthread_t writerThread;
  pthread_t readerThread;
  
  pthread_create(&writerThread, NULL, writerTHEThread, (void *) &conData);
  pthread_create(&readerThread, NULL, readerTHEThread, (void *) &conData);
  
  conData.writerThread = writerThread;
  conData.readerThread = readerThread;
  conData.ssl = ssl;
  
 connData = &conData;
  
  pthread_join(writerThread, NULL);
  sleep(1);
  pthread_join(readerThread, NULL);
  
  
  return 0;
  endwin();
}



//analyses the user input (streamer side, not input from twitch channel)
int analyseInput(char* strinput){
  sscanf(strinput, "%[^\n]", strinput);
  char* strinput_raw = strdup(strinput);
  char* token = strtok(strinput_raw, " ");
  
  if(strcmp(token, "say")==0){

    if(strlen(strinput) < 5){
      printf("usage: say <message>\n");
      return 0;
    }
    char* commandBody;

    commandBody = strchr(strinput,' ');
    commandBody++;
    if(twlibc_msgchannel(twitchsock, currentChannel, commandBody)==-1){
      perror("could't send message");
      return -1;
    }
    return 0;
  }else if (strcmp(token, "quit")==0){
    close_cycle();
  }else if(strcmp(token, "ls")==0){
    list_bot_commands(textWin);
    return 0;
  }else if(strcmp(token, "ld")==0){
    list_ban_list(textWin);
    return 0;
  }else if(strncmp(token, "rmcmd", 5)==0){
    if(strlen(strinput)<=6){ //checks for arguments
      char buffer[1024];
      sprintf(buffer, "rmcmd <command>   %s\n", strinput);
      printToScreen(buffer, textWin);
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
    char printBUffer[1024];
    sprintf(printBUffer,"removed command '%s'", commandName);
    printToScreen(printBUffer, textWin);
    return 0;
    
  }else if(strcmp(token, "rmbw")==0){
    if(strlen(strinput) <= 5){
      printToScreen("rmbw <command>", textWin);
      return 0;
    }
    char* body = strchr(strinput, ' ');
    body++;
    banlist_remove_command(body);
    printToScreen("ban word removed", textWin);
    return 0;
  }else if(strcmp(token, "addcmd")==0){

    if(strlen(strinput)<=7){ //checks for arguments
      printToScreen("addcmd <command> <message>", textWin);
      return 0;
    }
    
    char* commandName = strtok(NULL, " ");
    if (strcmp(commandName, "!credits")==0 || strcmp(commandName, "!vanish")==0){
      printToScreen("Can't override that command", textWin);
      return 0;
    } 
    if(test_command(commandName, NULL, 1)==1){
      printToScreen("command already exists", textWin);
      return 0;
    }

    char* body = strchr(strinput, ' ');
    body++;
    body = strchr(body, ' ');
    body++; //all of this just gets the pointer to string after the first two spaces
    
    if(strlen(commandName)==1 || strlen(body)==0){
      printf("addcmd <command> <message>\n");
      return 0;
    }
    add_command(commandName, body);
    char namebuff[strlen(commandName) + strlen("added command ")];
    sprintf(namebuff, "added command %s", commandName);
    printToScreen(namebuff, textWin);
    return 0;
  }else if(strcmp(token, "addbw")==0){
    if(strlen(strinput) <= 6){
      printToScreen("addbw <ban word>", textWin);
      return 0;
    }
    char* body = strchr(strinput, ' ');
    body++;
    
    if(banlist_test_command(body) == 1){
      printToScreen("This banword already exists", textWin);
      return 0;
    }
    
    banlist_add_command(body);
    printToScreen("ban word added", textWin);
    return 0;
  }else if(strcmp(token, "join")==0){
    char* newChannelName = strtok(NULL, " ");
    char newChannel[1+strlen(newChannelName)];
    char returnBuff[500];
    char payload[50];

    sprintf(newChannel, "#%s", newChannelName);
    twlibc_leavechannel(twitchsock, currentChannel, returnBuff, 200);
    strcpy(currentChannel, newChannel);
    printFormatted(returnBuff, textWin);
    bzero(returnBuff, sizeof(returnBuff));
    twlibc_joinchannel(twitchsock, newChannel, returnBuff, 200);
    printFormatted(returnBuff, textWin);
    sleep(1);
    sprintf(payload,"%s is here! HeyGuys", nick);
    twlibc_msgchannel(twitchsock, currentChannel, payload);

    return 0;
  }else if(strcmp(token, "whisper")==0 || strcmp(token, "w")==0){
    char* user = strtok(NULL, " ");
    char* message = strchr(strinput, ' ');
    message = strchr(message++, ' ');
    if(user==NULL || strtok(NULL, " ")==NULL){
      printf("usage: whisper|w <user> <message>\n");
      return -1;
    }
    if(twlibc_whisper(twitchsock, user, message, currentChannel)==-1){
      perror("Coulnd't send whisper");
    }
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
      if(twlibc_sendrawpacket(twitchsock, payload)==-1){
	return NULL;
      }
    }else {
      //sprintf(buff, "[^\r\n]", buff);
      char* buff_raw = strdup(buff);
      buff[strlen(buff)-2] = 0;
      for (char* token = strtok(buff, "\r\n"); token != NULL; token = strtok(NULL, "\r\n")){
	printToScreen(token, textWin);
      }
      sleep(0.5);
      
      char* command = returnCommand(buff);
      if(strcmp(command, "!credits")==0){ 
	//hard coded command
	if(twlibc_msgchannel(twitchsock,
		      currentChannel,
		      "This bot was written by SamBkamp at: https://github.com/SamBkamp/Aladdin\r\n")==-1){
	  return NULL;
	}
      }else if(strcmp(command, "!vanish")==0){
	char* payload = (char *)malloc(1024);
	sprintf(payload, "/timeout %s 1", commandSender(buff));
        if(twlibc_msgchannel(twitchsock, currentChannel, payload)==-1){
	  perror("coulnd't send message");
	  return NULL;
	}
	free(payload);
      }else if(test_command(command, outputmsg, 100)==1){ 
        if(twlibc_msgchannel(twitchsock, currentChannel, outputmsg)==-1){
	  perror("coulnd't send message");
	  return NULL;
	}
      }else {
	char* buff_really_raw = strdup(buff_raw);
	buff_raw++;
	char* message = strchr(buff_raw, ':');
	message++;
	sscanf(message, "%[^\r\n]", message);
	for (char* token = strtok(message, " "); token != NULL; token = strtok(NULL, " ")){
	  if(banlist_test_command(token) == 1){
	    char* payload = (char *)malloc(1024);
	    sprintf(payload, "/timeout %s 60", commandSender(buff_really_raw));
	    if(twlibc_msgchannel(twitchsock, currentChannel, payload)==-1){
	      perror("coulnd't send message");
	      return NULL;
	    }
	    free(payload);
	  }
	}
	
      }
      free(command);
    }
  }
  
  printf("closing writer thread\n");
}

//thread that writes to socket
void* writerTHEThread(void* context){
  char payload[50];
  
  sprintf(payload,"%s is here! HeyGuys", nick);
  
  if(twlibc_msgchannel(twitchsock, currentChannel, payload)==-1){
    perror("coulnd't send message");
    return NULL;
  }
  
  for (;;){
    char buffer[1024];
    //get streamer input
    //fgets(buffer, 1024, stdin);
    int offset = strlen(currentChannel)  + 5;
    mvwgetstr(inputWin, 1, offset, buffer);
    wmove(inputWin, 1, offset);
    wclrtoeol(inputWin);
    if(analyseInput(buffer)==-2){
      printToScreen("unrecognised command", textWin);
    }
  }
}


void printToScreen(char* message, WINDOW* window){
  char channelNameBuff[500];
  sprintf(channelNameBuff, "[%s]> ", currentChannel);
  
  if(lineIterator < windowHeight-6){
    lineIterator++;
    goto noscroll;
  }
  scroll(window);
 noscroll:
  mvwaddstr(window, lineIterator, 0, message);
  mvwaddstr(inputWin, 1, 1, channelNameBuff);
  wrefresh(window);
  wrefresh(inputWin);
}

void printFormatted(char* message, WINDOW* window){ //helper function that deals with strings ending in \r\n's, such as youd get from twitch irc
  for (char* tokky = strtok(message, "\r\n"); tokky != NULL; tokky = strtok(NULL, "\r\n")){
    printToScreen(tokky, window);    
  }
}

void closeHandler(int signal){
  close_cycle();
}

void close_cycle(){
  endwin();
  finish();
  banlist_finish();
  pthread_kill(connData->writerThread, SIGTERM);
  pthread_kill(connData->readerThread, SIGTERM);
}

SSL_CTX* InitCTX(void){ //create client-method instance & context
  SSL_CTX* ctx;
  const SSL_METHOD* method;
  
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  method = TLS_client_method();
  ctx = SSL_CTX_new(method);

  if(ctx == NULL){
    printf("%s", "CTX IS NULL - PANIC");
    ERR_print_errors_fp(stderr);
    abort();
  }
  return ctx;
}

void ShowCerts(SSL* ssl){
  X509* cert;
  char* line;

  cert = SSL_get_peer_certificate(ssl);

  if (cert != NULL){
    printf("Server certificates:\n");
    line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    printf("Subject: %s\n", line);
    free(line);       /* free the malloc'ed string */
    line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    printf("Issuer: %s\n", line);
    free(line);       /* free the malloc'ed string */
    X509_free(cert);	
  }else {
    printf("Info: No client certificates configured.\n"); 
  }
}

