#include <arpa/inet.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

#include "commands.h"
#include "helpers.h"
#include "requests.h"

int convert_string_to_int_command(char *line) {
	if (strcmp(line, "register") == 0) {
		return 0;
	}
	if (strcmp(line, "login") == 0) {
		return 1;
	}
	if (strcmp(line, "enter_library") == 0) {
		return 2;
	}
	if (strcmp(line, "get_books") == 0) {
		return 3;
	}
	if (strcmp(line, "get_book") == 0) {
		return 4;
	}
	if (strcmp(line, "add_book") == 0) {
		return 5;
	}
	if (strcmp(line, "delete_book") == 0) {
		return 6;
	}
	if (strcmp(line, "logout") == 0) {
		return 7;
	}
	if (strcmp(line, "exit") == 0) {
		return 8;
	}

	return -1;
}

int main(int argc, char *argv[]) {
	char *session_cookie = NULL;
	char *library_access_token = NULL;

	char line[MAX_LINE];

	while (1) {
		fgets(line, MAX_LINE, stdin);
		line[strlen(line) - 1] = '\0';
		int cmd_id = convert_string_to_int_command(line);

		switch (cmd_id) {
			case 0:
				register_user();
				break;
			case 1:
				session_cookie = login_user();
				break;
			case 2:
				library_access_token = access_library(session_cookie);
				break;
			case 3:
				get_books(library_access_token);
				break;
			case 4:
				get_book(library_access_token);
				break;
			case 5:
				add_book(library_access_token);
				break;
			case 6:
				delete_book(library_access_token);
				break;
			case 7:
				logout_user(&session_cookie, &library_access_token);
				break;
			case 8:
				return 0;  // exit
			default:
				printf("Unkown command\n");
		}
	}

	return 0;
}
