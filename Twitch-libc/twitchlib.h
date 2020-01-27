#ifndef twitchlib
int twlibc_msgchannel(int sockfd, const char* channel, const char* message);
int twlibc_joinchannel(int sockfd, const char* channel, char* output, int length);
int twlibc_leavechannel(int sockfd, const char* channel, char* output, int length);
int twlibc_sendrawpacket(int sockfd, char* payload);
int twlibc_setupauth(int sockfd, const char* oauth, const char* nick, char* output, int length);
char* twlibc_parseSender(char* message);
int twlibc_init();
int twlibc_whisper();
#endif
