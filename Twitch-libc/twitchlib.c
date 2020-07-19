/*

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   +-----------------------------------+
   |                                   | 
   |  @author: sam@bonnekamp.net       |
   |                                   |
   |  @version: 1.3.1                  |
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

SSL_CTX* InitCTX(void);

typedef struct {
  SSL_CTX* ctx;
  SSL* ssl;
  int socket;
  int usingSSL;
} twitch_connection;


twitch_connection* twlibc_init(int usingSSL){ //returns a pointer to a struct that has to be freed
  twitch_connection *temp;
  temp = malloc(sizeof(twitch_connection));
  struct sockaddr_in twitchaddr;
  SSL_CTX *ctx;
  SSL *ssl;
  int twitchsock;
  int PORT;
  
  if(usingSSL == 1){
    //using SSL
    SSL_library_init();
    ctx = InitCTX();  
    ssl = SSL_new(ctx);
  }else {
    ctx = NULL;
    ssl = NULL;
  }
  
  twitchsock = socket(AF_INET, SOCK_STREAM, 0);
  if (twitchsock==-1)
    return NULL;
  
  struct hostent* host = gethostbyname("irc.chat.twitch.tv");
  if(host == NULL)
    return NULL;

  if(usingSSL == 1){
    PORT = 6697;
    temp->usingSSL = 1;
  }else {
    PORT = 6667;
    temp->usingSSL = 0;
  }

  
  //setup address
  bzero(&twitchaddr, sizeof(twitchaddr));
  twitchaddr.sin_family = AF_INET;
  twitchaddr.sin_addr.s_addr = *(long *)host->h_addr_list[0];
  twitchaddr.sin_port = htons(PORT);

  if(connect(twitchsock, (struct sockaddr*)&twitchaddr, sizeof(twitchaddr)) != 0)
    return NULL;

  if(usingSSL == 1){
    SSL_set_fd(ssl, twitchsock);
    if (SSL_connect(ssl) == -1)
      return NULL;
  }
  
    
  if(usingSSL == 0){
    //not using SSL
    temp->ctx = NULL;
    temp->ssl = NULL;
  }else {
    //using SSL
    temp->ctx = ctx;
    temp->ssl = ssl;
  }
  
  temp->socket = twitchsock;
  
  return temp;
}

int twlibc_msgchannel(twitch_connection* twlibc, const char* channel, const char* message){
  char payload[12+strlen(channel)+strlen(message)];
  sprintf(payload, "PRIVMSG %s :%s\r\n", channel, message);
  if(twlibc->ssl == NULL){
    return write(twlibc->socket, payload, strlen(payload));
  }else{
    return SSL_write(twlibc->ssl, payload, strlen(payload));
  }
  return 0;
}

int twlibc_joinchannel(twitch_connection* twlibc, const char* channel, char* output, int length){
  char payload[7+strlen(channel)];
  sprintf(payload, "JOIN %s\r\n", channel);
  if(twlibc->ssl == NULL){
    //not using SSL
    if(write(twlibc->socket, payload, strlen(payload))==-1){
      return -1;
    }    
    if(output != NULL){
      if(read(twlibc->socket, output, length)==-1){
	return -1;
      }
    }
  }else{
    //using SSL
    if(SSL_write(twlibc->ssl, payload, strlen(payload))==-1){
      return -1;
    }
    if(output != NULL){
      if(SSL_read(twlibc->ssl, output, length)==-1){
	return -1;
      }
    }
  }
  return 0;
}

int twlibc_leavechannel(twitch_connection* twlibc, const char* channel, char* output, int length){
  char payload[7+strlen(channel)];
  sprintf(payload, "PART %s\r\n", channel);
  if(twlibc->ssl == NULL){
    //not using SSL
    if(write(twlibc->socket, payload, strlen(payload))==-1){
      return -1;
    }    
    if(output != NULL){
      if(read(twlibc->socket, output, length)==-1){
	return -1;
      }
    }
  }else{
    //using SSL
    if(SSL_write(twlibc->ssl, payload, strlen(payload))==-1){
      return -1;
    }
    if(output != NULL){
      if(SSL_read(twlibc->ssl, output, length)==-1){
	return -1;
      }
    }
  }
  return 0;
}


int twlibc_sendrawpacket(twitch_connection* twlibc, char* payload){
  /*
  int payloadlen = strlen(payload);
   
  if(payload[payloadlen] != '\n' || payload[payloadlen-1] != '\r'){
    strcat(payload, "\r\n");
  }
  */
  if((int)twlibc->usingSSL == 1){
    return SSL_write(twlibc->ssl, payload, strlen(payload));
  }else {
    if(write(twlibc->socket, payload, strlen(payload))==-1){
      return -1;
    }
  }
  return 1;
}

int twlibc_setupauth(twitch_connection* twlibc,
		     const char* oauth,
		     const char* nick,
		     char* output,
		     int length)
{
  char payload[14 + strlen(oauth) + strlen(nick)];
  sprintf(payload, "PASS %s\r\nNICK %s\r\n", oauth, nick);

  if(twlibc->usingSSL == 1){
    //using SSL
    return SSL_write(twlibc->ssl, payload, strlen(payload)); 
  }else {
    //not using SSL
    return write(twlibc->socket, payload, strlen(payload)); 
  }
   
  if(output!=NULL){
    if(twlibc->usingSSL == 0){
      //not using SSL
      return read(twlibc->socket, output, length);
    }else {
      return SSL_read(twlibc->ssl, output, length);
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

int twlibc_whisper(twitch_connection* twlibc,
		   const char* user,
		   const char* message,
		   const char* channel){
  char buffer[16 + strlen(channel) + strlen(user) + strlen(message)];
  sprintf(buffer, "PRIVMSG %s :/w %s %s\r\n", channel, user, message);
  if(twlibc->ssl != NULL){
    return SSL_write(twlibc->ssl, buffer, strlen(buffer));
  }else {
    return write(twlibc->socket, buffer, strlen(buffer));
  }
}



SSL_CTX* InitCTX(void){
  SSL_METHOD *tls_method = TLS_client_method();   // Create new client-method instance 
  SSL_CTX *ctx;
  OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
  SSL_load_error_strings();   /* Bring in and register error */ 
  ctx = SSL_CTX_new(tls_method);   /* Create new context */
  if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

void ShowCerts(SSL* ssl){
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);       /* free the malloc'ed string */
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);       /* free the malloc'ed string */
        X509_free(cert);     /* free the malloc'ed certificate copy */
    }
    else
        printf("Info: No client certificates configured.\n");
}
