#include "requests.h"

#include <arpa/inet.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

#include "helpers.h"

char *compute_get_request(char *host, char *url, char *query_params, char **cookies,
                          int cookies_count, char **headers, int header_count) {
	char *message = calloc(BUFLEN, sizeof(char));
	char *line = calloc(LINELEN, sizeof(char));

	// Write the method name, URL, request params (if any) and protocol type
	if (query_params != NULL) {
		sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
	} else {
		sprintf(line, "GET %s HTTP/1.1", url);
	}

	compute_message(message, line);

	// Add the host
	sprintf(line, "Host: %s", host);
	compute_message(message, line);

	// Add headers
	if (headers != NULL) {
		for (int i = 0; i < header_count; i++) {
			sprintf(line, "%s", headers[i]);
			compute_message(message, line);
		}
	}

	// Add cookies
	char *aux = calloc(LINELEN, sizeof(char));
	if (cookies != NULL) {
		// add the cookies to the message
		sprintf(line, "Cookie: %s; ", cookies[0]);
		for (int i = 1; i < cookies_count; i++) {
			strcpy(aux, line);
			if (i == cookies_count - 1) {
				sprintf(aux, "%s%s", line, cookies[i]);
			} else {
				sprintf(aux, "%s%s; ", line, cookies[i]);
			}
		}
		compute_message(message, line);
	}
	// Add final new line
	compute_message(message, "");

	free(line);
	free(aux);
	return message;
}

char *compute_post_request(char *host, char *url, char *content_type, char **body_data,
                           int body_data_fields_count, char **cookies, int cookies_count,
                           char **headers, int header_count) {
	char *message = calloc(BUFLEN, sizeof(char));
	char *line = calloc(LINELEN, sizeof(char));

	// Write the method name, URL and protocol type
	sprintf(line, "POST %s HTTP/1.1", url);
	compute_message(message, line);

	// Add the host
	sprintf(line, "Host: %s", host);
	compute_message(message, line);

	/* Add necessary headers (Content-Type and Content-Length are mandatory)
	        in order to write Content-Length you must first compute the message size
	*/
	sprintf(line, "Content-Type: %s", content_type);
	compute_message(message, line);

	int message_size = 0;
	for (int i = 0; i < body_data_fields_count; i++) {
		message_size += strlen(body_data[i]);
	}
	sprintf(line, "Content-Length: %d", message_size);
	compute_message(message, line);

	// Add additional headers
	if (headers != NULL) {
		for (int i = 0; i < header_count; i++) {
			sprintf(line, "%s", headers[i]);
			compute_message(message, line);
		}
	}

	// Add cookies
	char *aux = calloc(LINELEN, sizeof(char));
	if (cookies != NULL) {
		// add the cookies to the message
		sprintf(line, "Cookie: %s; ", cookies[0]);
		for (int i = 1; i < cookies_count; i++) {
			strcpy(aux, line);
			if (i == cookies_count - 1) {
				sprintf(aux, "%s%s", line, cookies[i]);
			} else {
				sprintf(aux, "%s%s; ", line, cookies[i]);
			}
		}
		compute_message(message, line);
	}

	// Add new line at end of header
	compute_message(message, "");

	// Add the actual payload data
	memset(line, 0, LINELEN);

	for (int i = 0; i < body_data_fields_count; i++) {
		strcat(line, body_data[i]);
	}
	strcat(message, line);

	free(line);
	free(aux);
	return message;
}

char *compute_delete_request(char *host, char *url, char **headers, int header_count) {
	char *message = calloc(BUFLEN, sizeof(char));
	char *line = calloc(LINELEN, sizeof(char));

	// Write the method name, URL and protocol type
	sprintf(line, "DELETE %s HTTP/1.1", url);
	compute_message(message, line);

	// Add the host
	sprintf(line, "Host: %s", host);
	compute_message(message, line);

	/* Add necessary headers (Content-Type and Content-Length are mandatory)
	        in order to write Content-Length you must first compute the message size
	*/
	if (headers != NULL) {
		for (int i = 0; i < header_count; i++) {
			sprintf(line, "%s", headers[i]);
			compute_message(message, line);
		}
	}

	// Add new line at end of header
	compute_message(message, "");

	free(line);
	return message;
}