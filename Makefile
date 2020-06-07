all:
	gcc lib/cmdfile.c Twitch-libc/twitchlib.c bot.c -Wall -o Aladdin -lpthread -lncurses -ggdb -lssl -lcrypto		
#openssl req -x509 -nodes -days 365 -newkey rsa:1024 -keyout mycert.pem -out mycert.pem
