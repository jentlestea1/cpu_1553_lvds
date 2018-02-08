#!/bin/sh

sparc-elf-gcc -std=c89 -pedantic -Wall -Wstrict-prototypes -Wno-missing-braces -O6 -I. $*
