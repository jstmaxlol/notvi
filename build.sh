#!/usr/bin/env bash

gcc -o nvi src/*.c -lncurses || exit
printf "nvi built.\n"
