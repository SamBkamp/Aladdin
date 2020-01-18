[![Bkamp_](https://img.shields.io/badge/twitch.tv-bkamp_-purple?logo=twitch&style=for-the-badge)](https://www.twitch.tv/bkamp_)
# Twitch libc

A library to interact with the twitch irc chat written in c

## Quick Start

To use the functions in this library all you have to include this headerfile like this:
```
#include "twitchlib.h"
```

## Functions

currently this library has a couple of callable functions:

- `msgchannel(int sockfd, const char* channel, const char* message)`

- `joinchannel(int sockfd, const char* channel, char output, int length)`

- `leavechannel(int sockfd, const char* channel, char output, int length)`

- `setupauth(int sockfd, const char* oauth, const char* nick, const char* output, int length)`

**Note: you can find more details about these methods in the Wiki section**

 
