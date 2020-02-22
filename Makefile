all:
	gcc lib/cmdfile.c Twitch-libc/twitchlib.c bot.c -Wall -o Aladdin -lpthread -lncurses -ggdb		
