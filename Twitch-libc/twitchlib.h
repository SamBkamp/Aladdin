#include <openssl/ssl.h>
#include <openssl/err.h>

#ifndef twitchlib
typedef struct{
  SSL_CTX* ctx;
  SSL* ssl;
  int socket;
  int usingSSL;
} twitch_connection;
int twlibc_msgchannel(twitch_connection* twlibc, const char* channel, const char* message);
int twlibc_joinchannel(twitch_connection* twlibc, const char* channel, char* output, int length);
int twlibc_leavechannel(twitch_connection* twlibc, const char* channel, char* output, int length);
int twlibc_sendrawpacket(twitch_connection* twlibc, char* payload);
int twlibc_setupauth(twitch_connection* twlibc, const char* oauth, const char* nick, char* output, int length);
char* twlibc_parseSender(char* message);
twitch_connection* twlibc_init();
int twlibc_whisper();
#endif
