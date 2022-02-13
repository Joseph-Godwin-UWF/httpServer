src = $(wildcard *.c)
obj = $(src:.c=.o)

httpServer: $(obj)
	gcc -g -Wall -o $@ $^

clean:
	rm -f $(obj) httpServer