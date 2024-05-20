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

void register_user(int sockfd, const char *username, const char *password)
{
    if (strcmp(username, "") == 0 || strcmp(password, "") == 0)
    {
        display_error("Username-ul sau parola este goala!");
        return; // Ieșiți din funcție dacă username-ul sau parola este goală
    }

    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);
    json_object_set_string(json_object, "username", username);
    json_object_set_string(json_object, "password", password);

    char *message = compute_post_request(HOST, REGISTER, "application/json", json_value, NULL, 0, NULL);
    send_to_server(sockfd, message);
    free(message);

    // Așteptați și primiți răspunsul de la server
    char *response = receive_from_server(sockfd);
    if (strstr(response, "HTTP/1.1 201 Created") != NULL)
    {
        // user ul a fost creat cu succes
        display_success("Creat cu succes!");
    }
    else if (strstr(response, "is taken!"))
    {
        display_error("Username-ul este ocupat!");
    }

    free(response);
}

void login_user(int sockfd, const char *username, const char *password, char *cookies)
{
    // Verificare pentru username și parolă
    if (strcmp(username, "") == 0 || strcmp(password, "") == 0)
    {
        display_error("Username-ul sau parola este goala!");
        return; // Ieșiți din funcție dacă username-ul sau parola este goală
    }

    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);
    json_object_set_string(json_object, "username", username);
    json_object_set_string(json_object, "password", password);

    char *message = compute_post_request(HOST, LOGIN, "application/json", json_value, NULL, 0, NULL);
    send_to_server(sockfd, message);
    free(message);
    // Așteptare și primire răspuns de la server
    char *response = receive_from_server(sockfd);

    extract_cookie(response, cookies);

    // Verificare pentru răspunsul primit
    if (strstr(response, "HTTP/1.1 200 OK") != NULL)
    {
        display_success("V-ati logat cu succes!");
    }
    else
    {
        if (strstr(response, "No account with this username") != NULL)
        {
            display_error("Nu exista asa utilizator!");
        }
        else if (strstr(response, "Credentials are not good") != NULL)
        {
            display_error("Credentiale gresite!");
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
    // Daca tokenul este deja setat atunci sunt deja in biblioteca
    if (strcmp(token, "") != 0)
    {
        display_error("Acces deja permis!");
        return;
    }

    if (strstr(response, "token\":\"") != NULL)
    {
        extract_token(response, token);
        display_success("Acces permis!");
    }
    else if (strstr(response, "HTTP/1.1 401 Unauthorized") != NULL)
    {
        display_error("Nu aveti acces!");
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

    char *last_line = strrchr(response, '\n');
    if (strstr(response, "HTTP/1.1 200 OK") != NULL)
    {
        //  Parsarea șirului JSON
        JSON_Value *root_value = json_parse_string(last_line);

        // Obținerea tabloului JSON
        JSON_Array *json_array = json_value_get_array(root_value);

        // Afisarea listei de cărți
        display_books(json_array);

        // Eliberarea memoriei
        json_value_free(root_value);
    }
    else
    {
        display_error("Nu s-au putut primi cartile!");
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

    char bookid[LINELEN] = BOOKS;

    char id[LINELEN];
    printf("id=");
    fgets(id, sizeof(id), stdin);
    id[strcspn(id, "\n")] = '\0'; // Eliminăm caracterul newline capturat de fgets

    // Verificăm dacă ID-ul nu este gol
    if (strcmp(id, "") == 0)
    {
        display_error("ID-ul este gol!");
        return;
    }

    // Verificăm dacă ID-ul este format doar din cifre
    for (int i = 0; i < strlen(id); i++)
    {
        if (!isdigit(id[i]))
        {
            display_error("ID-ul trebuie să fie un număr!");
            return;
        }
    }

    strcat(bookid, "/");
    strcat(bookid, id);

    char *message = compute_get_request(HOST, bookid, NULL, &cookies, 1, token);
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

        // Afisarea informatiilor despre carte
        display_book_info(json_object);

        // Eliberarea memoriei
        json_value_free(root_value);
    }
    else if (strstr(response, "HTTP/1.1 404 Not Found") != NULL)
    {
        display_error("Cartea cu acest id nu exista!");
    }
    else if (strstr(response, "HTTP/1.1 401 Unauthorized") != NULL)
    {
        display_error("Nu aveti acces!");
    }
    else if (strstr(response, "HTTP/1.1 403 Forbidden") != NULL)
    {
        display_error("Nu aveti acces!");
    }
    else if (strstr(response, "HTTP/1.1 400 Bad Request") != NULL)
    {
        display_error("ID-ul cartii trebuie sa fie un numar!");
    }
    else if (strstr(response, "HTTP/1.1 500 Internal Server Error") != NULL)
    {
        display_error("Eroare interna!");
    }

    free(response);
}

void add_book(int sockfd, char *cookies, char *token)
{
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);
    char title[LINELEN];
    char author[LINELEN];
    char genre[LINELEN];
    char publisher[LINELEN];
    char page_count[LINELEN];

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
    fgets(page_count, sizeof(page_count), stdin);
    page_count[strcspn(page_count, "\n")] = '\0'; // Eliminăm caracterul newline capturat de fgets

    // Verificare pentru toate câmpurile
    if (strlen(title) == 0 || strlen(author) == 0 || strlen(genre) == 0 || strlen(publisher) == 0 || strlen(page_count) == 0)
    {
        display_error("Toate campurile sunt obligatorii");
        return;
    }

    // Verificam daca page_count este un numar
    for (int i = 0; i < strlen(page_count); i++)
    {
        if (!isdigit(page_count[i]))
        {
            display_error("Page count trebuie sa fie un numar!");
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
        display_success("Cartea a fost adaugata!");
    }
    else
    {
        display_error("Nu s-a putut adauga cartea!");
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

    char id[LINELEN];
    printf("id=");
    fgets(id, sizeof(id), stdin);
    id[strcspn(id, "\n")] = '\0'; // Eliminăm caracterul newline capturat de fgets

    // Verificăm dacă ID-ul nu este gol
    if (strlen(id) == 0)
    {
        display_error("ID-ul este gol!");
        return;
    }

    // Verificăm dacă ID-ul este format doar din cifre
    for (int i = 0; i < strlen(id); i++)
    {
        if (!isdigit(id[i]))
        {
            display_error("ID-ul trebuie să fie un număr!");
            return;
        }
    }

    // Construim URL-ul pentru ștergere
    char bookid[LINELEN] = BOOKS;
    strcat(bookid, "/");
    strcat(bookid, id);

    char *message = compute_delete_request(HOST, bookid, NULL, &cookies, 1, token);

    send_to_server(sockfd, message);
    free(message);

    char *response = receive_from_server(sockfd);

    if (strstr(response, "HTTP/1.1 200 OK") != NULL)
    {
        display_success("Cartea a fost ștearsă cu succes!");
    }
    else
    {
        display_error("Nu s-a putut șterge cartea!");
    }

    free(response);
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
        display_success("V-ati delogat cu succes!");
    }
    else
    {
        display_error("Nu sunteti logat!");
    }
    free(response);
}