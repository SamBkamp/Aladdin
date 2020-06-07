#!/bin/bash

gcc -Wall -o example example_ssl.c -lssl -lcrypto -ggdb twitchlib.c
