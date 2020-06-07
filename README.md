[![Bkamp_](https://img.shields.io/badge/twitch.tv-bkamp_-purple?logo=twitch&style=for-the-badge)](https://www.twitch.tv/bkamp_)
[![Bkamp_](https://img.shields.io/github/issues-raw/SamBkamp/Aladdin/good%20first%20issue?style=flat-square)](https://github.com/SamBkamp/Aladdin/issues?utf8=%E2%9C%93&q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22)
[![Bkamp_](https://github.com/sambkamp/Aladdin/workflows/C%20CI/badge.svg)](https://github.com/SamBkamp/Aladdin/actions)

# **Aladdin**

a CLI twitch bot written in c. This bot is a way for you to interface with your twitch chat and also have chat commands and ban lists, all packed neatly into a single executable. The bot also encrypts data with SSL when talking to twitch. That means you can use this bot on IRL streams on sketchy public WIFI if you want...

## Features
This bot has two main features; Chat commands and ban words. The chat commands are a way for you as a streamer to program the bot to respond to twitch chat with automated messages, see the Commands section for more. The other feature is the ban words, this is a way for you to create a list that, if anyone says these words in your twitch chat, the bot will automatically time them out for 60 seconds. See more at the Commands section.  

## Quick Start
```
$ make
$ ./Aladdin
```

-- or --

you can download and run the precompiled binary on the release page
## Usage
1. compile with `$ make`
2. setup program with `$ ./Aladdin --setup` you will need your [oath key](https://twitchapps.com/tmi/)(`oauth:` is included in the key) and bot's nick name
3. run `$ ./Aladdin --join <channel name>` where the channel name is the twitch account name you want to connect to

## Commands
- `[#channel]> say <message>`

send a message to `channel`
- `[#channel]> quit`

quits the application
- `[#channel]> ls`

lists all the current active commands
- `[#channel]> addcmd <command> <command body>`

adds command `<command>`. When it is typed in chat, the bot responds with `<command body>`
- `[#channel]> rmcmd <command>`

removes command `<command>`
- `[#channel]> join <channel>`

leaves the currently active channel to move to new channel `<channel>`, sends a greeting same to the one sent on initial startup
- `[#channel]> w|whisper <user> <message>`

whispers `<message>` to the user given by `<user>` 
- `[#channel]> ld`

lists the current active ban words
- `[#channel]> addbw <ban word>`

adds `<ban word>` to current active ban words
- `[#channel]> rmbw <ban word>`

removes `<ban word>` from list of banwords
## Chat Commands
- by default the bot accepts the command `!credits` from twitch chat,
You can add commands to the bot by using the `addcmd` function as mentioned in the Commands section.
- The bot also ships with the !vanish command. This times the calling user out for 1 second, which removes their last couple of messages. This doesn't work if the invoking user has mod previliges 


## Support

I am open to collaboration from anyone who wants to. If youre not sure what to work on, check the issues page or the projects page. When you do submit a PR. Make sure that you include a reference to the issue or project, or a description of the changes. Thanks!
