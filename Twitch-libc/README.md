[![Bkamp_](https://img.shields.io/badge/twitch.tv-bkamp_-purple?logo=twitch&style=for-the-badge)](https://www.twitch.tv/bkamp_)
# Twitch libc

A library to interact with the twitch irc chat written in c

## Quick Start

To use the functions in this library all you have to include this headerfile like this:
```
#include "twitchlib.h"
```
and include the c file at compilation. Eg;
```bash
$ gcc -o twlibc example.c twitchlib.c
```

## Functions

currently this library has a couple of callable functions:

- `twlibc_msgchannel(int sockfd, const char* channel, const char* message)`

- `twlibc_joinchannel(int sockfd, const char* channel, char output, int length)`

- `twlibc_leavechannel(int sockfd, const char* channel, char output, int length)`

- `twlibc_setupauth(int sockfd, const char* oauth, const char* nick, const char* output, int length)`

**Note: you can find more details about these methods in the Wiki section**

 
