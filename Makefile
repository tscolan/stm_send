#stm_send Makefile

all: stm_send

stm_send: main.c
	gcc main.c -o stm_send

clean:
	$(RM) -f stm_send

