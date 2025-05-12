#
#	Makefile for the s65 simulator
#

#	VARIABLES
OUT			:= bin/a65
FLAGS		:= -static -std=c17 -O2 -Iinclude -Wall -g
SRC			:= $(wildcard src/*c)

#	BRANCHES

# 	Windows compilation
main:
	gcc $(SRC) -o $(OUT).exe $(FLAGS)

# 	Unix compilation
unix:
	gcc $(SRC) -o $(OUT).out $(FLAGS)