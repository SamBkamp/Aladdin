# Aladdin debugging tools

As Aladdin is now a CLI application, i've created these files to help with debugging this application with gdb using gdb's remote debugging feature. There should be two files in this folder:
- debugging.gdb
- debugsession.sh

two use this you will need two terminal windows, one will serve as the output window and one will serve as the gdb debugging window. 

Start the output window first by running `./debugging/debugsession.sh`. If you want to run any arguments with the program, just append it like it were the executable. (eg `./debugging/debugsession.sh --setup`)
**Note:** Make sure that this is run in the root folder 

Next, in a second terminal start the gdb window by running `gdb -x debugging.gdb`

Lastly, debug away. 
Good luck.
