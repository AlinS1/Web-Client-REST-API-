#ifndef _COMMANDS_
#define _COMMANDS_

#define HOST "34.246.184.49"
#define PORT 8080
#define ROUTE_REGISTER "/api/v1/tema/auth/register"
#define ROUTE_LOGIN "/api/v1/tema/auth/login"
#define ROUTE_ACCESS "/api/v1/tema/library/access"
#define ROUTE_BOOKS "/api/v1/tema/library/books"
#define ROUTE_LOGOUT "/api/v1/tema/auth/logout"
#define MAX_LINE 100
#define MAX_COOKIE 150

// =================================
// ======= HELPER FUNCTIONS ========
// =================================

int verify_user_or_pass(char *input);
int verifiyInt(char *p);
int verifyBookFields(char *title, char *author, char *genre, char *publisher);

// =================================
// ======= COMMAND FUNCTIONS =======
// =================================

void register_user();
char *login_user();
char *access_library(char *session_cookie);
void get_books(char *library_access_token);
void get_book(char *library_access_token);
void add_book(char *library_access_token);
void delete_book(char *library_access_token);
void logout_user(char **session_cookie, char **library_access_token);

#endif