#             Makefile
#     ~~~~~~~~~~~~~~~~~~~~~~~~

CC     = gcc
CFLAGS = -g3 -std=c99 -pedantic -Wall

all: clean chp2act

chp2act: chp.o partB.o
	${CC} ${CFLAGS} -o $@ $^

clean:
	${RM} chp2act vgcore*
