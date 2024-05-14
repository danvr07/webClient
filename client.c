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
#include "parson/parson.h"
#include <ctype.h>

void register_user(int sockfd, const char *username, const char *password, int *is_register)
{
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);
    json_object_set_string(json_object, "username", username);
    json_object_set_string(json_object, "password", password);

    char *message = compute_post_request(HOST, REGISTER, "application/json", json_value, NULL, 0, NULL);

    send_to_server(sockfd, message);
    free(message);

    // Așteptați și primiți răspunsul de la server
    char *response = receive_from_server(sockfd);

    char *last_line = strrchr(response, '\n');
    // printf("last_line: %s\n", last_line);

    last_line = strtok(last_line, "\n");
    if (strncmp(last_line, "ok", 2) == 0)
    {
        // cout << "SUCCES, Creat cu succes!\n";
        *is_register = 1;
        printf("SUCCES, Creat cu succes!\n");
    }
    else
    {
        // cout << "EROARE, Nu s-a putut crea contul!\n";
        printf("EROARE, Nu s-a putut crea contul!\n");
    }

    free(response);
}

void login_user(int sockfd, const char *username, const char *password, char *cookies)
{

    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);
    json_object_set_string(json_object, "username", username);
    json_object_set_string(json_object, "password", password);

    char *message = compute_post_request(HOST, LOGIN, "application/json", json_value, NULL, 0, NULL);
    send_to_server(sockfd, message);
    free(message);
    // Așteptare și primire răspuns de la server
    char *response = receive_from_server(sockfd);

    char *cookie_start = strstr(response, "Set-Cookie: ");
    if (cookie_start != NULL)
    {
        cookie_start += strlen("Set-Cookie: ");
        char *cookie_end = strstr(cookie_start, "\r\n");
        if (cookie_end != NULL)
        {
            int cookie_length = cookie_end - cookie_start;
            strcpy(cookies, "Cookie: ");                        // Adăugăm header-ul "Cookie: " înainte de valoarea cookie-ului
            strncat(cookies, cookie_start, cookie_length);      // Adăugăm valoarea cookie-ului
            cookies[cookie_length + strlen("Cookie: ")] = '\0'; // Terminăm string-ul
        }
    }

    // Verificare răspuns și afișare mesaj corespunzător
    char *last_line = strrchr(response, '\r\n');
    if (strstr(response, "HTTP/1.1 200 OK") != NULL)
    {
        printf("SUCCES, S-a logat cu succes!\n");
    }
    else
    {
        if (strstr(last_line, "No account with this username") != NULL)
        {
            printf("EROARE, Nu exista asa utilizator!\n");
        }
        else if (strstr(last_line, "Credentials are not good") != NULL)
        {
            printf("EROARE, Parola gresita!\n");
        }
    }

    free(response);
}

void enter_library(int sockfd, char *cookies, char *token)
{

    char *message = compute_get_request(HOST, ACCESS, NULL, &cookies, 1, NULL);
    send_to_server(sockfd, message);

    free(message);

    char *response = receive_from_server(sockfd);
    const char *token_start = strstr(response, "token\":\"");
    if (token_start != NULL)
    {
        token_start += strlen("token\":\""); // Mutăm pointerul la începutul valorii token-ului
        const char *token_end = strstr(token_start, "\"");
        if (token_end != NULL)
        {
            int token_length = token_end - token_start; // Calculăm lungimea token-ului
            strncpy(token, token_start, token_length);  // Copiem token-ul în buffer-ul nostru
            token[token_length] = '\0';                 // Terminăm șirul cu un caracter nul
                                                        // printf("Token: %s\n", token); // Afișăm token-ul
        }
        else
        {
            printf("EROARE, Token-ul nu este formatat corect!\n");
        }
    }
    else
    {
        printf("EROARE, Token-ul nu a fost găsit în răspuns!\n");
    }

    if (strstr(response, "HTTP/1.1 200 OK") != NULL)
    {
        printf("SUCCES, Acces permis!\n");
    }
    else
    {
        printf("EROARE, Acces nepermis!\n");
    }

    free(response);
}

void get_books(int sockfd, char *cookies, char *token)
{
    if (errorCommandLogin(cookies))
    {
        return;
    }
    if (errorCommandAcces(token))
    {
        return;
    }

       char *message = compute_get_request(HOST, BOOKS, NULL, &cookies, 1, token);

    send_to_server(sockfd, message);
    free(message);

    char *response = receive_from_server(sockfd);
    // printf("response: %s\n", response);
    char *last_line = strrchr(response, '\n');
    //    printf("last_line: %s\n", last_line);

    if (strstr(response, "HTTP/1.1 200 OK") != NULL)
    {
        //  Parsarea șirului JSON
        JSON_Value *root_value = json_parse_string(last_line);

        // Obținerea tabloului JSON
        JSON_Array *json_array = json_value_get_array(root_value);

        // Afisarea inceputului array-ului
        printf("[\n");

        // Parcurgerea fiecarui element din array
        for (size_t i = 0; i < json_array_get_count(json_array); i++)
        {
            // Obținerea obiectului JSON din array
            JSON_Object *json_object = json_array_get_object(json_array, i);

            // Obținerea valorii pentru fiecare cheie din obiect
            int id = json_object_get_number(json_object, "id");
            const char *title = json_object_get_string(json_object, "title");

            // Afisarea elementului in formatul specificat
            printf("    {\n");
            printf("        id: %d,\n", id);
            printf("        title: %s\n", title);
            printf("    }");

            // Dacă nu este ultimul element, adaugăm virgula
            if (i < json_array_get_count(json_array) - 1)
            {
                printf(",");
            }

            // Trecem la următorul rând
            printf("\n");
        }

        // Afisarea sfarsitului array-ului
        printf("]\n");

        // Eliberarea memoriei
        json_value_free(root_value);
    }
    else
    {
        printf("EROARE, Nu s-au putut primi cartile!\n");
    }
    free(response);
}

void get_book_id(int sockfd, char *cookies, char *token)
{
    if (errorCommandLogin(cookies))
    {
        return;
    }
    if (errorCommandAcces(token))
    {
        return;
    }
    char bookid[1000] = BOOKS;

    char id[1000];
    printf("id=");
    scanf("%s", id);
    strcat(bookid, "/");
    strcat(bookid, id);
    char *message = compute_get_request(HOST, bookid, NULL, &cookies, 1, token);
    // printf("message: %s\n", message);

    send_to_server(sockfd, message);
    free(message);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "HTTP/1.1 200 OK") != NULL)
    {
        char *last_line = strrchr(response, '\n');
        // Parsarea șirului JSON
        JSON_Value *root_value = json_parse_string(last_line);
        // Obținerea obiectului JSON din răspuns
        JSON_Object *json_object = json_value_get_object(root_value);

        // Obținerea valorilor din obiectul JSON
        int id = json_object_get_number(json_object, "id");
        const char *title = json_object_get_string(json_object, "title");
        const char *author = json_object_get_string(json_object, "author");
        const char *genre = json_object_get_string(json_object, "genre");
        const char *publisher = json_object_get_string(json_object, "publisher");
        int page_count = json_object_get_number(json_object, "page_count");

        printf("    {\n");
        printf("        id: %d,\n", id);
        printf("        title: %s\n", title);
        printf("        author: %s\n", author);
        printf("        genre: %s\n", genre);
        printf("        publisher: %s\n", publisher);
        printf("        page_count: %d\n", page_count);
        printf("    }\n");

        // Eliberarea memoriei
        json_value_free(root_value);
    }
    else
    {
        printf("EROARE, Nu s-a putut primi cartea!\n");
    }
}

void add_book(int sockfd, char *cookies, char *token)
{
    if (errorCommandLogin(cookies))
    {
        return;
    }
    if (errorCommandAcces(token))
    {
        return;
    }
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);
    char title[1000];
    char author[1000];
    char genre[1000];
    char publisher[1000];
    char page_count[1000];
    printf("title=");
    fgets(title, sizeof(title), stdin);
    title[strcspn(title, "\n")] = '\0'; // Eliminăm caracterul newline capturat de fgets
    printf("author=");
    fgets(author, sizeof(author), stdin);
    author[strcspn(author, "\n")] = '\0'; // Eliminăm caracterul newline capturat de fgets
    printf("genre=");
    fgets(genre, sizeof(genre), stdin);
    genre[strcspn(genre, "\n")] = '\0'; // Eliminăm caracterul newline capturat de fgets
    printf("publisher=");
    fgets(publisher, sizeof(publisher), stdin);
    publisher[strcspn(publisher, "\n")] = '\0'; // Eliminăm caracterul newline capturat de fgets
    printf("page_count=");
    scanf("%s", page_count);

    for (int i = 0; i < strlen(page_count); i++)
    {
        if (!isdigit(page_count[i]))
        {
            printf("EROARE, Page count trebuie să fie un număr!\n");
            return;
        }
    }
    int pages = strtol(page_count, NULL, 10); // Convertirea la întreg
    json_object_set_string(json_object, "title", title);
    json_object_set_string(json_object, "author", author);
    json_object_set_string(json_object, "genre", genre);
    json_object_set_string(json_object, "publisher", publisher);
    json_object_set_number(json_object, "page_count", pages);

    char *message = compute_post_request(HOST, BOOKS, "application/json", json_value, &cookies, 1, token);
    send_to_server(sockfd, message);

    free(message);

    char *response = receive_from_server(sockfd);
    if (strstr(response, "HTTP/1.1 200 OK") != NULL)
    {
        printf("SUCCES, Cartea a fost adaugata!\n");
    }
    else
    {
        printf("EROARE, Nu s-a putut adauga cartea!\n");
    }
    free(response);
}

void delete_book(int sockfd, char *cookies, char *token)
{
    if (errorCommandLogin(cookies))
    {
        return;
    }
    if (errorCommandAcces(token))
    {
        return;
    }
    char bookid[1000] = BOOKS;

    char id[1000];
    printf("id=");
    scanf("%s", id);
    strcat(bookid, "/");
    strcat(bookid, id);
    char *message = compute_delete_request(HOST, bookid, NULL, &cookies, 1, token);

    send_to_server(sockfd, message);
    free(message);

    char *response = receive_from_server(sockfd);

    if (strstr(response, "HTTP/1.1 200 OK") != NULL)
    {
        printf("SUCCES, Cartea a fost stearsa!\n");
    }
    else
    {
        printf("EROARE, Nu s-a putut sterge cartea!\n");
    }
}

void logout(int sockfd, char *cookies, char *token)
{
    if (errorCommandLogin(cookies))
    {
        return;
    }
    char *message = compute_get_request(HOST, LOGOUT, NULL, &cookies, 1, token);
    send_to_server(sockfd, message);
    free(message);
    *cookies = '\0';
    *token = '\0';
    char *response = receive_from_server(sockfd);

    if (strstr(response, "HTTP/1.1 200 OK") != NULL)
    {
        printf("SUCCES, S-a delogat cu succes!\n");
    }
    else
    {
        printf("EROARE, Nu s-a putut deloga!\n");
    }
    free(response);
}

int main(int argc, char *argv[])
{
    // char *message;
    // char *response;
    int sockfd;

    char command[1000];
    char username[1000];
    char password[1000];
    // char cookies[BUFLEN] = "";
    char *cookies = calloc(BUFLEN, sizeof(char));
    char *token = calloc(BUFLEN, sizeof(char));
    int is_register = 0;

    while (1)
    {
        // printf("Enter a command: ");
        fgets(command, sizeof(command), stdin); // Citirea comenzii de la utilizator
                                                // Înlătură newline-ul de la sfârșitul comenzii (înlocuiește '\n' cu '\0')
                                                // command[strcspn(command, "\n")] = '\0';
        strtok(command, "\n");
        if (strcmp(command, "register") == 0)
        {
            // Deschideți conexiunea către server
            printf("username=");
            // std:: cin >> username;
            scanf("%s", username);
            printf("password=");
            scanf("%s", password);
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

            // Înregistrați un utilizator
            register_user(sockfd, username, password, &is_register);
            close_connection(sockfd);
        }
        else if (strcmp(command, "login") == 0)
        {
            // printf("is_registdder: %d\n", is_register);
            if (strcmp(cookies, "") != 0)
            {
                printf("EROARE, Sunteti logat deja!\n");
                continue;
            }
            else if (is_register == 0)
            {
                printf("EROARE, Trebuie sa va inregistrati!\n");
                continue;
            }
            printf("username=");
            scanf("%s", username);
            printf("password=");
            scanf("%s", password);
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

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
    }

    return 0;
}
