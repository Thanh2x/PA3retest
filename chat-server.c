#include <stdio.h>
#include "http-server.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

typedef struct Reaction Reaction;
typedef struct Chat Chat;

uint32_t const MAX_USERNAME_SIZE = 16;
uint32_t const MAX_MESSAGE_SIZE = 256;
uint32_t const MAX_BUFFER_SIZE = 2048;

char const * HTTP_200_OK = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=UTF-8\r\n\r\n";
char const * HTTP_404_NOT_FOUND = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n";

uint32_t currID = 0;
uint32_t currChatSize = 0;

struct Reaction {
	char user[16];
	char message[16];
};

struct Chat {	
	char user[16];
	char message[256];
	char timestamp[100];
	uint32_t num_reaction;
	Reaction * reaction;
};

Chat * universal = NULL;

void handle_response(char * request, int client_socket);
void url_decode(char * source, char * dest);
uint8_t hextobyte(char c1, char c2);
void add_new_chat(char * path, int client_socket);
void add_new_reaction(char * path, int client_socket);
void print_chat(int client_socket);
void handle_404(char * path, int client_socket);
void free_stuff();

int main(int argc, char ** argv) {

	uint32_t port = 0;

	if (argc >= 2) {
		port = atoi(argv[1]);
	}

	start_server(&handle_response, port);

	return 0;

}

void handle_response(char *request, int client_socket) {

	printf("The user sent a request with: %s\n", request);
	write(client_socket, HTTP_200_OK, strlen(HTTP_200_OK));
	printf("The client socket is %d\n", client_socket);
	
	char buffer[MAX_BUFFER_SIZE];
	char path[256];

	if (sscanf(request, "GET %255s", path) != 1) {
		printf("Invalid request line");
	}
	
	printf("==THE PATH IS %s==\n", path);

	if ((strstr(path, "/post?user=") != 0) && (strstr(path, "&message=") != 0)) {
		printf("Hello you reached POST\n");
		add_new_chat(path, client_socket);
	}
	else if ((strstr(path, "/react?user=") != 0) && (strstr(path, "&message=") != 0) && (strstr(path, "&id=") != 0)) {
		printf("Hello you reached REACT\n");
		add_new_reaction(path, client_socket);
	}
	else if (strcmp(path, "/reset") == 0) {
		printf("Hello you reached RESET\n");
		free_stuff();
	}
	else if (strcmp(path, "/chats") == 0) {
		printf("Hello you reached CHATS\n");
		print_chat(client_socket);
	}
	else {
		printf("Hello you reached 404\n");
		handle_404(path, client_socket);
	}


}

void add_new_chat(char * path, int client_socket) {

	if (currChatSize == 100000) {
		handle_404(path, client_socket);
		return;
	}

	char decoded_path[strlen(path)];
	url_decode(path, decoded_path);

	printf("The decoded path is %s\n", decoded_path);

	char * startP1 = strstr(decoded_path, "/post?user=") + strlen("/post?user=");
	char * endP1 = strstr(decoded_path, "&message=");
	int userNameLength = endP1 - startP1;

	printf("Name Length: %ld\n", userNameLength);

	if ((userNameLength == 0) || (userNameLength >= MAX_USERNAME_SIZE)) {
		handle_404(path, client_socket);
		return;
	}
	
	char * startP2 = strstr(decoded_path, "&message=") + strlen("&message=");
	
	int messageLength = strlen(startP2);
	
	printf("Message Length: %d\n", messageLength);

	if ((messageLength == 0) || (messageLength > MAX_MESSAGE_SIZE)) {
		handle_404(path, client_socket);
		return;
	}
	
	currChatSize++;
	universal = realloc(universal, currChatSize * sizeof(Chat));
	

	strncpy(universal[currID].user, startP1, userNameLength);
	strncpy(universal[currID].message, startP2, messageLength);
	universal[currID].user[userNameLength] = '\0';
	universal[currID].message[messageLength] = '\0';
	universal[currID].num_reaction = 0;
	universal[currID].reaction = NULL;
	
	time_t now = time(NULL);
	struct tm *tm_info = localtime(&now);
	strftime(universal[currID].timestamp, 100, "%Y-%m-\%d %H:%M:\%S", tm_info);
		
	

	printf("The user name is: %s\n", universal[currID].user);
	printf("The message is: %s\n", universal[currID].message);
	
	currID++;

	print_chat(client_socket);


}

void add_new_reaction(char * path, int client_socket) {
	
	char decoded_path[strlen(path)];
	url_decode(path, decoded_path);

	printf("The decoded path is %s\n", decoded_path);

	char * startP1 = strstr(decoded_path, "/react?user=") + strlen("/react?user=");
	char * endP1 = strstr(decoded_path, "&message=");
	int userNameLength = endP1 - startP1;

	printf("Name Length: %d\n", userNameLength);

	if ((userNameLength == 0) || (userNameLength >= MAX_USERNAME_SIZE)) {
		handle_404(path, client_socket);
		return;
	}
	
	char * startP2 = strstr(decoded_path, "&message=") + strlen("&message=");
	char * endP2 = strstr(decoded_path, "&id=");
	int messageLength = endP2 - startP2;	
	printf("Message Length: %d\n", messageLength);

	if ((messageLength == 0) || (messageLength >= MAX_USERNAME_SIZE)) {	
		handle_404(path, client_socket);
		return;
	}

	char * startP3 = strstr(decoded_path, "&id=") + strlen("&id=");
	char tempStringID[10];
	strcpy(tempStringID, startP3);
	tempStringID[strlen(startP3)] = 0;
	int i = 0;
	int tempStringIDLength = strlen(tempStringID);
	for ( ; i < tempStringIDLength; i++) {
		if (!(tempStringID[i] >= '0' && tempStringID[i] <= '9')) {
			handle_404(path, client_socket);
			return;
		}
	}
	
	int realID = atoi(tempStringID) - 1;

	if (realID >= currID || realID < 0) {
		handle_404(path, client_socket);
		return;
	}

	if (universa[realID].num_reaction == 100) {
		handle_404(path, client_socket);
		return;
	}

	universal[realID].num_reaction++;

	universal[realID].reaction = realloc(universal[realID].reaction, universal[realID].num_reaction * sizeof(Reaction));
	
	int rIndex = universal[realID].num_reaction - 1;

	strncpy(universal[realID].reaction[rIndex].user, startP1, userNameLength);
	strncpy(universal[realID].reaction[rIndex].message, startP2, messageLength);
	universal[realID].reaction[rIndex].user[userNameLength] = '\0';
	universal[realID].reaction[rIndex].message[messageLength] = '\0';

	print_chat(client_socket);

}	

void url_decode(char * source, char * dest) {

	char *p_src = source;
	char *p_dest = dest;

	int i = 0;

	while (*p_src) {
		if (*p_src == '%') {
			*p_dest = hextobyte(*(p_src + 1), *(p_src + 2));
			p_src += 2;
		}
		else {
			*p_dest = *p_src;
		}
		p_src++;
		p_dest++;
	}
	*p_dest = 0;
}

uint8_t hextobyte(char c1, char c2) {

	char f;
	char s;
	
	if (c1 >= '0' && c1 <= '9') {
		f = c1 - '0';
	}
	else if (c1 >= 'a' && c1 <= 'f') {
		f = c1 - 'a' + 10;
	}
	else if (c1 >= 'A' && c1 <= 'F') {
		f = c1 - 'A' + 10;
	}

	
	if (c2 >= '0' && c2 <= '9') {
		s = c2 - '0';
	}
	else if (c2 >= 'a' && c2 <= 'f') {
		s = c2 - 'a' + 10;
	}
	else if (c2 >= 'A' && c2 <= 'F') {
		s = c2 - 'A' + 10;
	}

	return (f << 4) | s;

}

void free_stuff() {

	if (currChatSize == 0) {
		return;
	}

	int i = 0;

	for ( ; i < currChatSize; i++) {
		free(universal[i].reaction);
	}

	free(universal);
	universal = NULL;
	currChatSize = 0;
	currID = 0;



}


void print_chat(int client_socket) {
	
	char buffer[MAX_BUFFER_SIZE];
	char buffer2[MAX_BUFFER_SIZE];
	char * index;

	int i = 0;
	for ( ; i < currChatSize; i++) {
		snprintf(buffer, MAX_BUFFER_SIZE, "[#%d %s] %40s: %s\r\n", i + 1, universal[i].timestamp, universal[i].user, universal[i].message);
		write(client_socket, buffer, strlen(buffer));
		int j = 0;
		for ( ; j < universal[i].num_reaction; j++) {
			snprintf(buffer2, MAX_BUFFER_SIZE, "%64s) %s\r\n", universal[i].reaction[j].user, universal[i].reaction[j].message);
			index = strstr(buffer2, universal[i].reaction[j].user) - 1;
			*index = '(';
			write(client_socket, buffer2, strlen(buffer2));
		}
	}	


}

void handle_404(char * path, int client_socket) {

	printf("SERVER LOG: Got request for unrecognized path \"%s\"\n", path);

	char buffer[MAX_BUFFER_SIZE];

	snprintf(buffer, MAX_BUFFER_SIZE, "Error 404:\r\n Unrecognized \"%s\"\n", path);
	write(client_socket, HTTP_404_NOT_FOUND, strlen(HTTP_404_NOT_FOUND));
	write(client_socket, buffer, strlen(buffer));
	

}