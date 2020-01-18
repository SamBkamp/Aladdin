/*

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   +-----------------------------------+
   |                                   | 
   |  @author: sam@bonnekamp.net       |
   |                                   |
   |  @version: 0.1                    |
   |                                   |  
   +-----------------------------------+
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "twitchlib.h"

int main(){
  struct sockaddr_in twitchaddr;
  int twitchsock = socket(AF_INET, SOCK_STREAM, 0);
  if (twitchsock==-1){
    printf("FATAL: couldn't create socket\n");
    return 0;
  }
  struct hostent* host = gethostbyname("irc.chat.twitch.tv");
  if(host == NULL){
    perror("FATAL: couldn't get host");
    return 0;
  }
  //setup address
  bzero(&twitchaddr, sizeof(twitchaddr));
  twitchaddr.sin_family = AF_INET;
  twitchaddr.sin_addr.s_addr = *(long *)host->h_addr_list[0];
  twitchaddr.sin_port = htons(6667);

  if(connect(twitchsock, (struct sockaddr*)&twitchaddr, sizeof(twitchaddr)) != 0){
    perror("FATAL: couldn't connect to server\n");
    return 0;
  }

  char buffer[1024];

  if(setupauth(twitchsock, "oauth:xyz", "botbkamp", buffer, 1024)==-1){
    perror("FATAL: Couldn't authenticate with twitch servers");
  }
  printf("%s", buffer);

  bzero(buffer, 1024);
  if(joinchannel(twitchsock, "#bkamp_", buffer, 1024)==-1){
    perror("FATAL: Couldn't join server");
  }
  printf("%s", buffer);

  bzero(buffer, 1024);
  if(msgchannel(twitchsock, "#bkamp_", "HeyGuys")==-1){
    perror("FATAL: Couldn't send message");
  }
  printf("%s", buffer);

  bzero(buffer, 1024);
  if(leavechannel(twitchsock, "#bkamp_", buffer, 1024)==-1){
    perror("FATAL: Couldn't join server");
  }
  printf("%s", buffer);
  
}
