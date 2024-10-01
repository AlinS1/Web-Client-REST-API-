#include "commands.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

#include "helpers.h"
#include "parson.h"
#include "requests.h"

// =================================
// ======= HELPER FUNCTIONS ========
// =================================

// Verifies if the username or password follow the accepted format.
int verify_user_or_pass(char *input) {
	for (int i = 0; i < strlen(input); i++) {
		if (isspace(input[i])) {
			return 0;  // not ok
		}
	}
	return 1;  // ok
}

// Verifies if a string can be converted to an integer.
int verifiyInt(char *p) {
	for (int i = 0; i < strlen(p); i++) {
		if (!isdigit(p[i])) {
			return 0;  // false
		}
	}

	return 1;  // true
}

// Verifies if all the book fields have been given as input.
int verifyBookFields(char *title, char *author, char *genre, char *publisher) {
	if (strlen(title) == 0 || strlen(author) == 0 || strlen(genre) == 0 || strlen(publisher) == 0) {
		return 0;
	}
	return 1;
}

// =================================
// ======= COMMAND FUNCTIONS =======
// =================================

void register_user() {
	// Read and verify the input.
	char username[MAX_LINE];
	char password[MAX_LINE];
	printf("username= ");
	fgets(username, MAX_LINE, stdin);
	username[strlen(username) - 1] = '\0';
	printf("password= ");
	fgets(password, MAX_LINE, stdin);
	password[strlen(password) - 1] = '\0';

	if (!verify_user_or_pass(username) || !verify_user_or_pass(password)) {
		printf("ERROR - Password or username should not contain spaces!\n");
		return;
	}

	// Create the JSON object that contains the username and password.
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	char *serialized_string = NULL;
	json_object_set_string(root_object, "username", username);
	json_object_set_string(root_object, "password", password);
	serialized_string = json_serialize_to_string_pretty(root_value);
	char *body_data[] = {serialized_string};

	// POST Request
	int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *message = compute_post_request(HOST, ROUTE_REGISTER, "application/json", body_data, 1,
	                                     NULL, 0, NULL, 0);
	send_to_server(sockfd, message);
	char *response = receive_from_server(sockfd);

	// The output depends on the response from the server.
	if (strstr(response, "error") != NULL) {
		printf("ERROR - User already exists!\n");
	} else if (strstr(response, "ok") != NULL) {
		printf("200 - OK - User registered with SUCCESS!\n");
	}

	// Free the memory.
	free(message);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	close_connection(sockfd);
}

char *login_user() {
	// Read and verify the input.
	char username[MAX_LINE];
	char password[MAX_LINE];
	printf("username= ");
	fgets(username, MAX_LINE, stdin);
	username[strlen(username) - 1] = '\0';
	printf("password= ");
	fgets(password, MAX_LINE, stdin);
	password[strlen(password) - 1] = '\0';

	if (!verify_user_or_pass(username) || !verify_user_or_pass(password)) {
		printf("ERROR - Password or username should not contain spaces!\n");
		return NULL;
	}

	// Create the JSON object that contains the username and password.
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	char *serialized_string = NULL;
	json_object_set_string(root_object, "username", username);
	json_object_set_string(root_object, "password", password);
	serialized_string = json_serialize_to_string_pretty(root_value);
	char *body_data[] = {serialized_string};

	// POST Request
	int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *message =
	    compute_post_request(HOST, ROUTE_LOGIN, "application/json", body_data, 1, NULL, 0, NULL, 0);
	send_to_server(sockfd, message);
	char *response = receive_from_server(sockfd);

	// Free the memory.
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	free(message);
	close_connection(sockfd);

	// The output depends on the response from the server. If the request was
	// done successfully, we extract the session cookie.
	char *session_cookie = NULL;
	if (strstr(response, "error") != NULL) {
		printf("ERROR - User could not log in!\n");
	} else if (strstr(response, "ok") != NULL) {
		printf("200 - OK - User logged in with SUCCESS!\n");
		session_cookie = strstr(response, "Set-Cookie: ");
		session_cookie = strtok(session_cookie, " ");
		session_cookie = strtok(NULL, " ");
	}

	return session_cookie;
}

char *access_library(char *session_cookie) {
	if (session_cookie == NULL) {
		printf("ERROR - User not logged in!\n");
		return NULL;
	}

	// GET Request
	int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *cookies[] = {session_cookie};
	char *message = compute_get_request(HOST, ROUTE_ACCESS, NULL, cookies, 1, NULL, 0);
	send_to_server(sockfd, message);
	char *response = receive_from_server(sockfd);

	// The output depends on the response from the server. If the request was
	// done successfully, we extract the access token.
	char *access_token = NULL;
	if (strstr(response, "error") != NULL) {
		printf("ERROR - Session_cookie not ok!\n");
	} else if (strstr(response, "ok") != NULL) {
		printf("200 - OK - User accessed library with SUCCESS!\n");
		access_token = strstr(response, "token");
		access_token = strtok(access_token, ":");
		access_token = strtok(NULL, "\"");
	}

	// Free the memory.
	free(message);
	close_connection(sockfd);

	return access_token;
}

void get_books(char *library_access_token) {
	if (library_access_token == NULL) {
		printf("ERROR - User does not have access to the library!\n");
		return;
	}

	// In order to perform the request, we need the Authorization header that
	// will contain the access token.
	char *authorization_header = calloc(LINELEN, sizeof(char));
	sprintf(authorization_header, "Authorization: Bearer %s", library_access_token);
	char *headers[] = {authorization_header};

	// GET Request
	int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *message = compute_get_request(HOST, ROUTE_BOOKS, NULL, NULL, 0, headers, 1);
	send_to_server(sockfd, message);
	char *response = receive_from_server(sockfd);

	// The output will be exactly the response from the server. If there are no
	// books, the output will be "[]". Otherwise, it will be in JSON format.
	response = strstr(response, "[");
	printf("%s\n", response);

	// Free the memory.
	free(message);
	free(authorization_header);
	close_connection(sockfd);
}

void get_book(char *library_access_token) {
	// Read the book's id we want to get.
	char line[MAX_LINE];
	printf("id= ");
	fgets(line, MAX_LINE, stdin);
	line[strlen(line) - 1] = '\0';

	if(!verifiyInt(line)){
		printf("ERROR - Book id should be a number!\n");
		return;
	}
	int id = atoi(line);


	if (library_access_token == NULL) {
		printf("ERROR - User does not have access to the library!\n");
		return;
	}

	// In order to perform the request, we need the Authorization header that
	// will contain the access token.
	char *authorization_header = calloc(LINELEN, sizeof(char));
	sprintf(authorization_header, "Authorization: Bearer %s", library_access_token);
	char *headers[] = {authorization_header};

	// The path is unique to the book's id.
	char *path = calloc(LINELEN, sizeof(char));
	sprintf(path, "%s/%d", ROUTE_BOOKS, id);

	// GET Request
	int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *message = compute_get_request(HOST, path, NULL, NULL, 0, headers, 1);
	send_to_server(sockfd, message);
	char *response = receive_from_server(sockfd);

	// Free the memory.
	free(message);
	free(path);
	close_connection(sockfd);

	// The output depends on the response from the server.
	if (strstr(response, "404 Not Found") != NULL) {
		printf("ERROR - No book found with the given id!\n");
		return;
	}
	// If the request was done successfully, we extract the JSON object and
	// print it.
	response = strstr(response, "{");
	printf("%s\n", response);
}

void add_book(char *library_access_token) {
	// Read and verify the input.
	char title[MAX_LINE];
	char author[MAX_LINE];
	char genre[MAX_LINE];
	char publisher[MAX_LINE];
	char page_count[MAX_LINE];

	printf("title= ");
	fgets(title, MAX_LINE, stdin);
	title[strlen(title) - 1] = '\0';

	printf("author= ");
	fgets(author, MAX_LINE, stdin);
	author[strlen(author) - 1] = '\0';

	printf("genre= ");
	fgets(genre, MAX_LINE, stdin);
	genre[strlen(genre) - 1] = '\0';

	printf("publisher= ");
	fgets(publisher, MAX_LINE, stdin);
	publisher[strlen(publisher) - 1] = '\0';

	printf("page_count= ");
	fgets(page_count, MAX_LINE, stdin);
	page_count[strlen(page_count) - 1] = '\0';

	if (!verifyBookFields(title, author, genre, publisher)) {
		printf("ERROR - Invalid input!\n");
		return;
	}
	if (!verifiyInt(page_count)) {
		printf("ERROR - Invalid input for page count!\n");
		return;
	}
	int page_count_nr = atoi(page_count);


	if (library_access_token == NULL) {
		printf("ERROR - User does not have access to the library!\n");
		return;
	}

	// Create the JSON object that contains the book information.
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	char *serialized_string = NULL;
	json_object_set_string(root_object, "title", title);
	json_object_set_string(root_object, "author", author);
	json_object_set_string(root_object, "genre", genre);
	json_object_set_string(root_object, "publisher", publisher);
	json_object_set_number(root_object, "page_count", page_count_nr);
	serialized_string = json_serialize_to_string_pretty(root_value);
	char *body_data[] = {serialized_string};

	// In order to perform the request, we need the Authorization header that
	// will contain the access token.
	char *authorization_header = calloc(LINELEN, sizeof(char));
	sprintf(authorization_header, "Authorization: Bearer %s", library_access_token);
	char *headers[] = {authorization_header};

	// POST Request
	int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *message = compute_post_request(HOST, ROUTE_BOOKS, "application/json", body_data, 1, NULL,
	                                     0, headers, 1);
	send_to_server(sockfd, message);
	char *response = receive_from_server(sockfd);

	// The output
	if (strstr(response, "ok") != NULL) {
		printf("200 - OK - Book added with SUCCESS!\n");
	} else {
		printf("ERROR - Book could not be added!\n");
	}

	// Free the memory.
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	free(message);
	close_connection(sockfd);
}

void delete_book(char *library_access_token) {
	// Read the input data
	char line[MAX_LINE];
	printf("id= ");
	fgets(line, MAX_LINE, stdin);
	line[strlen(line) - 1] = '\0';

	// Verify the input data.
	if (!verifiyInt(line)) {
		printf("ERROR - Book id should be a number!\n");
		return;
	}
	int id = atoi(line);

	if (library_access_token == NULL) {
		printf("ERROR - User does not have access to the library!\n");
		return;
	}

	// In order to perform the request, we need the Authorization header that
	// will contain the access token.
	char *authorization_header = calloc(LINELEN, sizeof(char));
	sprintf(authorization_header, "Authorization: Bearer %s", library_access_token);
	char *headers[] = {authorization_header};

	// The path is unique to the book's id.
	char *path = calloc(LINELEN, sizeof(char));
	sprintf(path, "%s/%d", ROUTE_BOOKS, id);

	// DELETE Request
	int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *message = compute_delete_request(HOST, path, headers, 1);
	send_to_server(sockfd, message);
	char *response = receive_from_server(sockfd);

	// The output depends on the response from the server.
	if (strstr(response, "404 Not Found") != NULL) {
		printf("ERROR - No book found with the given id!\n");
	} else {
		printf(
		    "200 - OK - The book with the given id was deleted with "
		    "SUCCESS!\n");
	}

	// Free the memory.
	free(message);
	free(path);
	free(authorization_header);
	close_connection(sockfd);
}

void logout_user(char **session_cookie, char **library_access_token) {
	if (*session_cookie == NULL) {
		printf("ERROR - You are not logged in!\n");
		return;
	}

	// GET Request
	int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
	char *cookies[] = {*session_cookie};
	char *message = compute_get_request(HOST, ROUTE_LOGOUT, NULL, cookies, 1, NULL, 0);
	send_to_server(sockfd, message);
	char *response = receive_from_server(sockfd);

	// The output depends on the response from the server.
	if (strstr(response, "error") != NULL) {
		printf("ERROR - User could not log out!\n");
	} else if (strstr(response, "ok") != NULL) {
		printf("200 - OK - User logged out with SUCCESS!\n");
	}

	// Free the memory and erase the cookie and token.
	free(message);
	*session_cookie = NULL;
	*library_access_token = NULL;
	close_connection(sockfd);
}