TARGETS=server

all: $(TARGETS) 

server: server.c
	gcc -g server.c -o server -lpthread 
clean:
	rm -f $(TARGETS)
