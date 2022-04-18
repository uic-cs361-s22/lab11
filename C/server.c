#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_SIZE 1460
#define MAX_CLIENTS 10

void insert(int fd);
void remove_inactives();
void broadcast_all(int fd, char *buffer);
void *client_handler(void *args);

typedef struct client{
	int fd;
	enum {ACTIVE = 1, INACTIVE = 2} status;
	pthread_t thread;
	struct client* next;
} client;

client* head = NULL;
int actives = 0;
char *reply = "Server: Message Broadcasted\n";

// insert first
void insert(int fd) {
	client* c = (client*) malloc(sizeof(client));
	c->fd = fd;
	c->next = head;
	c->status = ACTIVE;
	if (pthread_create(&c->thread,NULL,client_handler,(void *)c)) {
		printf("pthread_create failed\n");
		close(c->fd);
		free(c);
		return;
	}
	head = c;
	actives++;
	printf("Currently %d active, %d is added\n", actives, c->fd);
}

void remove_inactives() {
	if (head == NULL) return;
	// if in head
	while (head && head->status == INACTIVE) {
		client* temp = head;
		head = head->next;
		actives--;
		printf("Currently %d active, %d is removed\n", actives, temp->fd);
		free(temp);
	}

	// elsewhere
	client* current = head;
	client* previous = NULL;
	while(current != NULL) {
		if (current->status == INACTIVE) {
			previous->next = current->next;
			actives--;
			printf("Currently %d active, %d is removed\n", actives, current->fd);
			free(current);
			current = previous->next;
		} else {
			previous = current;
			current = current->next;
		}
	}
}


void printList() {
	client* c = head;
	while (c != NULL) {
		printf("%d -> ", c->fd);
		c = c->next;
	}
	printf("\n");
}

void broadcast_all(int fd, char *buffer){
	char broadcast_message[MAX_SIZE];
	snprintf(broadcast_message,sizeof(broadcast_message),"Client %d: %s\n",fd,buffer);
	int message_len = strlen(broadcast_message);
	for(client* c = head;c != NULL; c = c->next){
		if(c->fd==fd){
			continue;
		}
		printf("Sending to client %d\n", c->fd);
		//To-Do 6: Broadcast Message across all clients
		if (0/*Call send here*/) {
			perror("Broadcast send failed!");
			close(c->fd);
			c->status = INACTIVE;
		}
	}
}

void *client_handler(void *args){
	client *c = (client*) args;
	while(1){
		char buffer[MAX_SIZE];
		//To-Do 5(a): Read from the client
		int size = -1/*Call read here*/;
		if (size <= 0) {
			perror("Read failed");
			close(c->fd);
			c->status = INACTIVE;
			break;
		} else {
			broadcast_all(c->fd, buffer);
			//To-Do 5(b): Reply back to client with reply message
			if (1/*Call send here*/) {
				perror("Send failed");
				close(c->fd);
				c->status = INACTIVE;
			}
		}
	}
	remove_inactives();
}

int main(int argc, char const* argv[]){
	
	int PORT = 5000;
	if(argc!=2){
		printf("Expected format: ./server <port number>\n");
		exit(0);
	}
	PORT = atoi(argv[1]);
	//To-Do 1: Create a IPv4 Socket for TCP and store fd in socketfd
	int socketfd = -1/*Call socket here*/;
	if(socketfd == -1){
		printf("Could not create Socket\n");
		exit(0);
	}	
	//To-Do 2: Bind to localhost use server_address
	struct sockaddr_in server_address;
	if(1/*Call bind here*/){
		perror("Could not bind the Socket");
		close(socketfd);
		exit(0);
	}
	//To-Do 3: Start listening for clients
	if(1/*Call listen here*/){
		perror("Not able to listen on the Socket");
		close(socketfd);
		exit(0);
	}
	printf("Server running on localhost: %d \n",PORT);
	while(1){
		if(actives == MAX_CLIENTS){
			printf("Server Capacity Full\n");
			continue;
		}
		struct sockaddr_in client_address;
		int client_addr_len = sizeof(client_address);
		//To-Do 4: Accept Connections from the clients and store fd associated with connection in connfd
		int connfd = -1/*Call accept here*/;
		if(connfd < 0){
			printf("Server Could not accept the connection\n");
			continue;
		}	
		insert(connfd);
		printf("Connection Accepted for the Client with fd %d\n",connfd);
		printList();
		
	}
	return 0;
}
