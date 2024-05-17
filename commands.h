#ifndef _COMMANDS_
#define _COMMANDS_

void register_user(int sockfd, const char *username, const char *password);

void login_user(int sockfd, const char *username, const char *password, char *cookies);

void enter_library(int sockfd, char *cookies, char *token);

void get_books(int sockfd, char *cookies, char *token);

void get_book_id(int sockfd, char *cookies, char *token);

void add_book(int sockfd, char *cookies, char *token);

void delete_book(int sockfd, char *cookies, char *token);

void logout(int sockfd, char *cookies, char *token);

#endif