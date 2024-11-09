#include <stdio.h>
#include "http-server.h"
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>


typedef struct Reaction Reaction;
typedef struct Chat Chat;


int chat_id = 0;
int chat_size = 0;
char const* HTTP_200_OK = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=UTF-8\r\n\r\n";
char const* HTTP_404_NOT_FOUND = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n";

struct Reaction {
	char * user;
	char * message;
};

struct Chat {
	char * user;
	char * message;
	char timestamp[100];
	uint32_t num_reaction;
	Reaction * reactions;
};




Chat * universal;



void handle_response(char *request, int client_socket);
void create_new_chat(char *path, int client_socket);
void url_decode(char * source, char * dest);
void print_chat(int client_socket);
void create_new_reaction(char * path, int client_socket);
void handle_404(char *path, int client_socket);
void free_stuff();

	
	
	
	
void handle_404(char *path, int client_socket) {

	printf("SERVER LOG: Got request for unrecognized path \"%s\"\r\n", path);

	char response_buff[BUFFER_SIZE];
	snprintf(response_buff, BUFFER_SIZE, "Error 404:\r\nUnrecognized path \"%s\"\r\n", path);
	write(client_socket, HTTP_404_NOT_FOUND, strlen(HTTP_404_NOT_FOUND));
	write(client_socket, response_buff, strlen(response_buff));

}




void handle_response(char *request, int client_socket) {
	printf("The user sent a request with: %s\n", request);
	write(client_socket, HTTP_200_OK, strlen(HTTP_200_OK));
	printf("\n\n%d\n\n", client_socket);
	
	char buffer[2056];

	char path[256];

	if (sscanf(request, "GET %255s", path) != 1) {
		printf("Invalid request line\n");
	}

	printf("This is the path: \'%s\'\n", path);


	if ((strstr(path, "/post?user=") != 0) && (strstr(path, "&message=") != 0)) {
		create_new_chat(path, client_socket);

	}
	else if ((strstr(path, "/react?user=") != 0) && (strstr(path, "&message=") != 0) && (strstr(path, "&id=") != 0)) {
		if (chat_size == 0) {
			handle_404(path, client_socket);
		}
		else {
			create_new_reaction(path, client_socket);
		}
	}
	else if (strcmp(path, "/reset") == 0) {
			free_stuff();
	}
	else if (strcmp(path, "/chats") == 0) {
		print_chat(client_socket);
	}
	else {
		handle_404(path, client_socket);
	}





}

void create_new_chat(char *path, int client_socket) {

	if (chat_size > 100000) {
		handle_404(path, client_socket);
		return;
	}

		
	printf("You have entered created new chat with path \'%s\'\n", path);
	//DECODE THE PATH TO THE RIGHT FORMAT
	char decoded_path[strlen(path)];
	url_decode(path, decoded_path);
	printf("\n\n\nHHHHHHHHHHHHH%sHHHHHHHHHHHHH\n\n\n", decoded_path);

	//GET THE LENGTH OF THE USERNAME	
	char * startP = strstr(decoded_path, "/post?user=") + strlen("/post?user=");
	char * endP = strstr(decoded_path, "&message=");
	int length = endP - startP;

	if (length == 0) {
		handle_404(path, client_socket);
		return;
	}

	char * tempUser = calloc(length, sizeof(char));
	strncpy(tempUser, startP, length);
	printf("%s\n", tempUser);
	printf("%d\n", length);

	
	//CHECK IF USERNAME BIGGER THAN 16 BYTES
	if (strlen(tempUser) > 15 || strlen(tempUser) < 1) {
		handle_404(path, client_socket);
		free(tempUser);
		return;
	}	

	//GET THE LENGTH OF THE MESSAGE, endP points to the first character and strlen iterates till we get a null term
	length = strlen(endP + strlen("&message="));
	
	if (length == 0) {
		handle_404(path, client_socket);
		free(tempUser);
		return;
	}

	char * tempMessage = calloc(length, sizeof(char));
	strncpy(tempMessage, endP + strlen("&message="), length);
	printf("%s\n", tempMessage);
	printf("%d\n", length);
	
	//CHECK IF MESSAGE LESS THAN 1 BYTES
	if (strlen(tempMessage) < 1 || strlen(tempMessage) > 255) {
		handle_404(path, client_socket);
		free(tempUser);
		free(tempMessage);
		return;
	}	

	//REALLOCATE NEW STRUCTS EVERYTIME WE CALL NEW CHATS	
	if (chat_size == 0) {
		chat_size++;
		universal = calloc(chat_size, sizeof(Chat));
	}
	else {
		chat_size++;
		universal = realloc(universal, chat_size * sizeof(Chat));
	}
	//PUT IT IN THE GLOBAL STRUCT OF CHAT
	universal[chat_id].user = tempUser;
	universal[chat_id].message = tempMessage;
	universal[chat_id].num_reaction = 0;
	universal[chat_id].reactions = NULL;

	printf("%s\n%s\n", universal[chat_id].user, universal[chat_id].message);
	
	//GET THE TIMe
	
	char buffer[100];
	time_t now = time(NULL);
	struct tm *tm_info = localtime(&now);
	strftime(universal[chat_id].timestamp, 100, "%Y-%m-\%d %H:%M:\%S", tm_info);
/*	printf("\n%s\n", buffer);
	char * timeInHeap = calloc(strlen(buffer), sizeof(char));
	strcpy(timeInHeap, buffer); */

	//strcpy(universal[chat_id].timestamp, buffer);
	printf("\n%s\n", universal[chat_id].timestamp);

	
	chat_id++;

	//WRITE TO THE BOWSER
	print_chat(client_socket);

}


void create_new_reaction(char * path, int client_socket) {


	printf("You have entered created new chat with path \'%s\'\n", path);
	//DECODE THE PATH TO THE RIGHT FORMAT
	char decoded_path[strlen(path)];
	url_decode(path, decoded_path);

	//GET THE LENGTH OF THE USERNAME	
	char * startP = strstr(decoded_path, "/react?user=") + strlen("/react?user=");
	char * endP = strstr(decoded_path, "&message=");
	int length = endP - startP;
	
	if (length == 0) {
		handle_404(path, client_socket);
		return;
	}
	
	char * tempUser = calloc(length, sizeof(char));
	strncpy(tempUser, startP, length);
	printf("%s\n", tempUser);
	printf("%d\n", length);
	
	//CHECK IF USERNAME BIGGER THAN 16 BYTES
	if (strlen(tempUser) > 15 || strlen(tempUser) < 1) {
		handle_404(path, client_socket);
		free(tempUser);
		return;
	}	


	//GET THE LENGTH OF THE MESSAGE
	endP = strstr(decoded_path, "&id=");
	startP = strstr(decoded_path, "&message=") + strlen("&message=");
	length = endP - startP;	
	
	if (length == 0) {
		handle_404(path, client_socket);
		return;
	}
	
	char * tempMessage = calloc(length, sizeof(char));
	strncpy(tempMessage, startP, length);
	printf("%s\n", tempMessage);
	printf("%d\n", length);

	
	//CHECK IF MESSAGE BIGGER THAN 16 BYTES
	if (strlen(tempMessage) > 15) {
		handle_404(path, client_socket);
		free(tempUser);
		free(tempMessage);
		return;
	}	

	startP = endP + strlen("&id=");
	length = strlen(startP);
	
	if (length == 0) {
		handle_404(path, client_socket);
		return;
	}
	
	char * tempID = calloc(length, sizeof(char));
	strncpy(tempID, startP, length);
	printf("%s\n", tempID);
	printf("%d\n", length);
	int realID = atoi(tempID) - 1;
	
	free(tempID);
	
	//CHECK IF ID IS BIGGER THAN CHAT SIZE OR NEGATIVE
	if ((realID < 0) || (realID > chat_id - 1) || (universal[realID].num_reaction > 100)) {
		handle_404(path, client_socket);
		return;
	}


	if (universal[realID].num_reaction == 0) {
		universal[realID].num_reaction++;
		universal[realID].reactions = calloc(universal[realID].num_reaction, sizeof(Reaction));
	}
	else {
		universal[realID].num_reaction++;
		universal[realID].reactions = realloc(universal[realID].reactions, universal[realID].num_reaction * sizeof(Reaction));
	}

	universal[realID].reactions[universal[realID].num_reaction - 1].user = tempUser;
	universal[realID].reactions[universal[realID].num_reaction - 1].message = tempMessage;
	
	printf("%s\n", universal[realID].reactions[universal[realID].num_reaction - 1].user);

	printf("%s\n", universal[realID].reactions[universal[realID].num_reaction - 1].message);


	print_chat(client_socket);



}




void print_chat(int client_socket) {
	
	char bufferP[2056];
	int i = 0;
	
	//write(client_socket, HTTP_200_OK, strlen(HTTP_200_OK));
	
	for (; i < chat_id; i++) {
		snprintf(bufferP, 2056, "[#%d %s]        %20s: %s\r\n", i + 1, universal[i].timestamp, universal[i].user, universal[i].message);
		write(client_socket, bufferP, strlen(bufferP));
		int j = 0;
		for (;j < universal[i].num_reaction; j++) {
			snprintf(bufferP, 2056, "                             %24s) %s\r\n", universal[i].reactions[j].user, universal[i].reactions[j].message);
			
			char * index = strstr(bufferP, universal[i].reactions[j].user) - 1;
			*index = '(';
			write(client_socket, bufferP, strlen(bufferP));
		}
	}

}

void free_stuff() {

	if (chat_size == 0) {
		return;
	}

	int i = 0;
	for ( ; i < chat_size; i++) {
		free(universal[i].user);
		free(universal[i].message);
//		free(universal[i].timestamp);

		int j = 0;
		for ( ; j < universal[i].num_reaction; j++) {
			free(universal[i].reactions[j].user);
			free(universal[i].reactions[j].message);
		}
		free(universal[i].reactions);
	}

	free(universal);

	chat_size = 0;
	chat_id = 0;

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


void register_name() {

	





}




int main(int argc, char ** argv) {

	int port = 0;

	if (argc >= 2) {
		port = atoi(argv[1]);
	}
	
	start_server(&handle_response, port);

	return 0;
}
