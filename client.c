#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "commands.h"
#include "parson/parson.h"
#include <ctype.h>

int main(int argc, char *argv[])
{
    int sockfd;

    char command[1000];
    char username[1000];
    char password[1000];
    char *cookies = calloc(BUFLEN, sizeof(char));
    char *token = calloc(BUFLEN, sizeof(char));

    while (1)
    {
        fgets(command, sizeof(command), stdin); // Citirea comenzii de la utilizator
                                                // Înlătură newline-ul de la sfârșitul comenzii (înlocuiește '\n' cu '\0')
                                                // command[strcspn(command, "\n")] = '\0';
        strtok(command, "\n");
        if (strcmp(command, "register") == 0)
        {

            if (strcmp(cookies, "") != 0)
            {
                display_error("Trebuie sa va delogati inainte de a va inregistra!");
                continue;
            }
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            // Deschideți conexiunea către server
            printf("username=");
            fgets(username, sizeof(username), stdin);
            printf("password=");
            fgets(password, sizeof(password), stdin);

            // Elimină caracterul newline de la sfârșitul username-ului și parolei, dacă există
            username[strcspn(username, "\n")] = '\0';
            password[strcspn(password, "\n")] = '\0';

            if (contains_spaces(username) || contains_spaces(password))
            {
                display_error("Username sau parola contine spatii!");
                continue;
            }

            // Înregistrați un utilizator
            register_user(sockfd, username, password);
            close_connection(sockfd);
        }
        else if (strcmp(command, "login") == 0)
        {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (strcmp(cookies, "") != 0)
            {
                printf("EROARE, Sunteti logat deja!\n");
                continue;
            }

            printf("username=");
            fgets(username, sizeof(username), stdin);
            printf("password=");
            fgets(password, sizeof(password), stdin);
            // Elimină caracterul newline de la sfârșitul username-ului și parolei, dacă există
            username[strcspn(username, "\n")] = '\0';
            password[strcspn(password, "\n")] = '\0';

            if (contains_spaces(username) || contains_spaces(password))
            {
                display_error("Username sau parola contine spatii!");
                continue;
            }

            login_user(sockfd, username, password, cookies);

            close_connection(sockfd);
        }
        else if (strcmp(command, "enter_library") == 0)
        {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            enter_library(sockfd, cookies, token);
            close_connection(sockfd);
        }
        else if (strcmp(command, "get_books") == 0)
        {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

            get_books(sockfd, cookies, token);
            close_connection(sockfd);
        }
        else if (strcmp(command, "get_book") == 0)
        {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            get_book_id(sockfd, cookies, token);
            close_connection(sockfd);
        }
        else if (strcmp(command, "add_book") == 0)
        {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            add_book(sockfd, cookies, token);
            close_connection(sockfd);
        }
        else if (strcmp(command, "delete_book") == 0)
        {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            delete_book(sockfd, cookies, token);
            close_connection(sockfd);
        }
        else if (strcmp(command, "logout") == 0)
        {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            logout(sockfd, cookies, token);
            close_connection(sockfd);
        }
        else if (strcmp(command, "exit") == 0)
        {
            break;
        }
        else if(strcmp(command, "help") == 0)
        {
            printf("Comenzi disponibile:\n");
            printf("register\n");
            printf("login\n");
            printf("enter_library\n");
            printf("get_books\n");
            printf("get_book\n");
            printf("add_book\n");
            printf("delete_book\n");
            printf("logout\n");
            printf("exit\n");
        }
    }

    free(cookies);
    free(token);

    return 0;
}
