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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "twitchlib.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <resolv.h>
#include <malloc.h>
#include <errno.h>

//prototypes
int OpenConnection(const char *hostname, int port);
SSL_CTX* InitCTX(void);
void ShowCerts(SSL* ssl);

int main(){
  SSL_CTX *ctx;
  SSL *ssl;
  int twitchsock;
  
  SSL_library_init();
  ctx = InitCTX();

  ssl = SSL_new(ctx);
  twitchsock = twlibc_init(ssl);
  SSL_set_fd(ssl, twitchsock);

  if (SSL_connect(ssl) == -1){
    ERR_print_errors_fp(stderr);
    exit(0);
  }
  
  printf("Connected with %s encryption\n", SSL_get_cipher(ssl));

  ShowCerts(ssl);
  
  if(twitchsock == -1){
    perror("FATAL: Couldn't initialise socket");
    return 0;
  }
  
  char buffer[1024];

  //SSL_write(ssl, "PASS oauth:zxh966o7dqsxy78mh5m1xja5jqiybk\r\nNICK botbkamp\r\n", 31);
  
  if(twlibc_setupauth(twitchsock, "oauth:zxh966o7dqsxy78mh5m1xja5jqiybk", "botbkamp", buffer, 1024)==-1){
   perror("FATAL: Couldn't authenticate with twitch servers");
  }
  //  SSL_read(ssl, buffer, 1024);
  
  printf("%s", buffer);

  bzero(buffer, 1024);
  if(twlibc_joinchannel(twitchsock, "#bkamp_", buffer, 1024)==-1){
    perror("FATAL: Couldn't join server");
  }
  printf("%s", buffer);

  bzero(buffer, 1024);
  if(twlibc_msgchannel(twitchsock, "#bkamp_", "HeyGuys")==-1){
    perror("FATAL: Couldn't send message");
  }
  printf("%s", buffer);

  char returnString[1024];

  if(SSL_read(ssl, returnString, sizeof(returnString))==-1){
    perror("FATAL: couldn't read from twitch socket");
  }

  printf("%s", returnString);

  char* senderName = twlibc_parseSender(returnString);
  printf("parsed name is: %s\n", senderName);
  free(senderName);

  if(twlibc_whisper(twitchsock, "Bkamp_", "test", "#bkamp_") == -1){
    perror("FATAL: couldn't whisper");
  }
  
  bzero(buffer, 1024);
  if(twlibc_leavechannel(twitchsock, "#bkamp_", buffer, 1024)==-1){
    perror("FATAL: Couldn't join server");
  }
  printf("%s", buffer);
  
}

SSL_CTX* InitCTX(void){
    SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = TLSv1_2_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
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
