[![Bkamp_](https://img.shields.io/badge/twitch.tv-bkamp_-purple?logo=twitch&style=for-the-badge)](https://www.twitch.tv/bkamp_)
[![Bkamp_](https://img.shields.io/github/issues-raw/SamBkamp/Aladdin/good%20first%20issue?style=flat-square)](https://github.com/SamBkamp/Aladdin/issues?utf8=%E2%9C%93&q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22)

# **Aladdin**

command line twitch bot written in c

**Warning: this bot is not yet SSL encrypted, This means that your oauth key will be sent in plain text over the network. So, please don't use this bot on public networks until SSL is implemented.**

## Quick Start
```
$ make
$ ./Aladdin
```
## Usage
1. compile with `$ make`
2. setup program with `$ ./Aladdin --setup` you will need your [oath key](https://twitchapps.com/tmi/)(`oauth:` is included in the key) and bot's nick name
3. run `$ ./Aladdin --join <channel name>` where the channel name is the twitch account name you want to connect to

## Commands
- `[#channel]> say <message>`

send a message to the currently selected channel
- `[#channel]> quit`

quits the application
- `[#channel]> addcmd <command> <command body>`

adds command `<command>`. When it is typed in chat, the bot responds with `<command body>`
- `[#channel]> rmcmd <command>`

removes command `<command>`

## Chat Commands
- by default the bot accepts the command `!credits` from twitch chat,
You can add commands to the bot by using the `addcmd` function as mentioned in the Commands section.


## Support

I am open to collaboration from anyone who wants to. If youre not sure what to work on, check the issues page or the projects page. When you do submit a PR. Make sure that you include a reference to the issue or project, or a description of the changes. Thanks!
