#             Makefile
#     ~~~~~~~~~~~~~~~~~~~~~~~~

CC     = gcc
CFLAGS = -g3 -std=c99 -pedantic -Wall

all: clean prsim_add8_writer prsim_gen_writer

prsim_add8_writer: prsim_add8_writer.o
	${CC} ${CFLAGS} -o $@ $^

prsim_gen_writer: prsim_gen_writer.o
	${CC} ${CFLAGS} -o $@ $^


clean:
	${RM} prsim_add8_writer prsim_gen_writer vgcore*
