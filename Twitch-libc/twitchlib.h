/*

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   +-----------------------------------+
   |                                   | 
   |  @author: sam@bonnekamp.net       |
   |                                   |
   |  @version: 0.2                    |
   |                                   |  
   +-----------------------------------+
*/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int msgchannel(int sockfd, const char* channel, const char* message){
  char* payload = (char *)malloc(12+strlen(channel)+strlen(message));
  sprintf(payload, "PRIVMSG %s :%s\r\n", channel, message);
  if(write(sockfd, payload, strlen(payload))==-1){
    free(payload);
    return -1;
  }
  free(payload);
  return 0;
}

int joinchannel(int sockfd, const char* channel, char* output, int length){
  char payload[7+strlen(channel)];
  sprintf(payload, "JOIN %s\r\n", channel);
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

int leavechannel(int sockfd, const char* channel, char* output, int length){
  char* payload = (char *)malloc(7+strlen(channel));
  sprintf(payload, "PART %s\r\n", channel);
  if(write(sockfd, payload, strlen(payload))==-1){
    free(payload);
    return -1;
  }
  if(output != NULL){
    //TODO : read directly into output rather than sticking it in a buffer first
    if(read(sockfd, output, length)==-1){
      free(payload);
      return -1;
    }
  }
  free(payload);
  return 0;
}


int sendrawpacket(int sockfd, char* payload){
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

int setupauth(int sockfd, const char* oauth, const char* nick, char* output, int length){
  char payload[14 + strlen(oauth) + strlen(nick)];
  sprintf(payload, "PASS %s\r\nNICK %s\r\n", oauth, nick);

  if(write(sockfd, payload, strlen(payload))==-1){
    //free(payload);
    return -1;
  }
  
  if(output!=NULL){
    if(read(sockfd, output, length)==-1){
      //free(payload);
      return -1;
    }
  }
  //free(payload);
  return 0;
}

