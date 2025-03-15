#
#	Makefile for the s65 simulator
#

#	VARIABLES
OUT			:= bin/a65.exe
FLAGS		:= -static -std=c17 -O0 -m32 -Iinclude -Wall -g
SRC			:= $(wildcard src/*c)

#	BRANCHES

# 	Windows x86 compilation
main:
	gcc $(SRC) -o $(OUT) $(FLAGS)