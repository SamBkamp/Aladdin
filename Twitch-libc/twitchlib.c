/*

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   +-----------------------------------+
   |                                   | 
   |  @author: sam@bonnekamp.net       |
   |                                   |
   |  @version: 0.2.1                  |
   |                                   |  
   +-----------------------------------+
*/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


int usingSSL;
SSL* ssl_connection;

int twlibc_init(SSL* ssl){
  struct sockaddr_in twitchaddr;
  int twitchsock = socket(AF_INET, SOCK_STREAM, 0);
  if (twitchsock==-1)
    return -1;
  
  struct hostent* host = gethostbyname("irc.chat.twitch.tv");
  if(host == NULL)
    return -1;
  
  //setup address
  bzero(&twitchaddr, sizeof(twitchaddr));
  twitchaddr.sin_family = AF_INET;
  twitchaddr.sin_addr.s_addr = *(long *)host->h_addr_list[0];
  twitchaddr.sin_port = htons(6697);

  if(connect(twitchsock, (struct sockaddr*)&twitchaddr, sizeof(twitchaddr)) != 0)
    return -1;
  
  if(ssl != NULL){
    ssl_connection = ssl;
    usingSSL = 1;
  }else {
    usingSSL = 0; 
  }
  
  return twitchsock;
}

int twlibc_msgchannel(int sockfd, const char* channel, const char* message){
  char payload[12+strlen(channel)+strlen(message)];
  sprintf(payload, "PRIVMSG %s :%s\r\n", channel, message);
  if(usingSSL == 0){
    if(write(sockfd, payload, strlen(payload))==-1){
      return -1;
    } 
  }else{
    if(SSL_write(ssl_connection, payload, strlen(payload))==-1){
      return -1;
    }
  }
  return 0;
}

int twlibc_joinchannel(int sockfd, const char* channel, char* output, int length){
  char payload[7+strlen(channel)];
  sprintf(payload, "JOIN %s\r\n", channel);
  if(usingSSL == 0){
    //not using SSL
    if(write(sockfd, payload, strlen(payload))==-1){
      return -1;
    }    
    if(output != NULL){
      if(read(sockfd, output, length)==-1){
	return -1;
      }
    }
  }else{
    //using SSL
    if(SSL_write(ssl_connection, payload, strlen(payload))==-1){
      return -1;
    }
    if(output != NULL){
      if(SSL_read(ssl_connection, output, length)==-1){
	return -1;
      }
    }
  }
  return 0;
}

int twlibc_leavechannel(int sockfd, const char* channel, char* output, int length){
  char payload[7+strlen(channel)];
  sprintf(payload, "PART %s\r\n", channel);
  if(write(sockfd, payload, strlen(payload))==-1){
    return -1;
  }
  if(output != NULL){
    if(read(sockfd, output, length)==-1){
      return -1;
    }
  }
  return 0;
}


int twlibc_sendrawpacket(int sockfd, char* payload){
  int payloadlen = strlen(payload);
  /* 
  if(payload[payloadlen] != '\n' || payload[payloadlen-1] != '\r'){
    strcat(payload, "\r\n");
  }
  */
  if(write(sockfd, payload, strlen(payload))==-1){
    return -1;
  }
  return 1;
}

int twlibc_setupauth(int sockfd, const char* oauth, const char* nick, char* output, int length){
  char payload[14 + strlen(oauth) + strlen(nick)];
  sprintf(payload, "PASS %s\r\nNICK %s\r\n", oauth, nick);

  if(write(sockfd, payload, strlen(payload))==-1){
    return -1;
  }
  
  if(output!=NULL){
    if(read(sockfd, output, length)==-1){
      return -1;
    }
  }
  return 0;
}

char* twlibc_parseSender(char* message){
  char* name = strtok(message, "!");
  if(name==NULL)
    return NULL;
  name++;
  char* parsedName = (char *)malloc(1+strlen(name));
  if(parsedName==NULL)
    return NULL;
  strcpy(parsedName, name);
  return parsedName;
}

int twlibc_whisper(int sockfd, const char* user, const char* message, const char* channel){
  char buffer[16 + strlen(channel) + strlen(user) + strlen(message)];
  sprintf(buffer, "PRIVMSG %s :/w %s %s\r\n", channel, user, message);
  if(write(sockfd, buffer, strlen(buffer))==-1){
    return -1;
  }
  return 0 ;
}
