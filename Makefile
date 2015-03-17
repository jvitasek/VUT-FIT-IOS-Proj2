# Makefile - IOS PROJECT 2 - FIT VUT 1BIT
################################################################################
# Author:	Jakub Vitásek
#			FIT VUT Brno, 1BIT
# E-mail:	xvitas02@stud.fit.vutbr.cz
# Date:		29/04/2014
################################################################################

CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
CC=gcc

all: rivercrossing
rivercrossing: rivercrossing.c
	$(CC) $(CFLAGS) rivercrossing.c -o rivercrossing

################################################################################
################################################################################