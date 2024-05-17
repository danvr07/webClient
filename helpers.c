#include <stdlib.h> /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "buffer.h"
#include "parson/parson.h"

#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void compute_message(char *message, const char *line)
{
    strcat(message, line);
    strcat(message, "\r\n");
}

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(ip_type, socket_type, flag);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(portno);
    inet_aton(host_ip, &serv_addr.sin_addr);

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    return sockfd;
}

void close_connection(int sockfd)
{
    close(sockfd);
}

void send_to_server(int sockfd, char *message)
{
    int bytes, sent = 0;
    int total = strlen(message);

    do
    {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0)
        {
            error("ERROR writing message to socket");
        }

        if (bytes == 0)
        {
            break;
        }

        sent += bytes;
    } while (sent < total);
}

char *receive_from_server(int sockfd)
{
    char response[BUFLEN];
    buffer buffer = buffer_init();
    int header_end = 0;
    int content_length = 0;

    do
    {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0)
        {
            error("ERROR reading response from socket");
        }

        if (bytes == 0)
        {
            break;
        }

        buffer_add(&buffer, response, (size_t)bytes);

        header_end = buffer_find(&buffer, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE);

        if (header_end >= 0)
        {
            header_end += HEADER_TERMINATOR_SIZE;

            int content_length_start = buffer_find_insensitive(&buffer, CONTENT_LENGTH, CONTENT_LENGTH_SIZE);

            if (content_length_start < 0)
            {
                continue;
            }

            content_length_start += CONTENT_LENGTH_SIZE;
            content_length = strtol(buffer.data + content_length_start, NULL, 10);
            break;
        }
    } while (1);
    size_t total = content_length + (size_t)header_end;

    while (buffer.size < total)
    {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0)
        {
            error("ERROR reading response from socket");
        }

        if (bytes == 0)
        {
            break;
        }

        buffer_add(&buffer, response, (size_t)bytes);
    }
    buffer_add(&buffer, "", 1);
    return buffer.data;
}

char *basic_extract_json_response(char *str)
{
    return strstr(str, "{\"");
}

void display_success(const char *message)
{
    printf("SUCCES, %s\n", message);
}

void display_error(const char *message)
{
    printf("EROARE, %s\n", message);
}

int errorCommandAcces(char *token)
{
    if (strcmp(token, "") == 0)
    {
        // printf("EROARE, Nu aveti acces\n");
        display_error("Nu aveti acces");
        return 1; // Indică o eroare
    }
    return 0; // Nu a apărut nicio eroare
}
int errorCommandLogin(char *cookies)
{
    if (strcmp(cookies, "") == 0)
    {
        // printf("EROARE, Nu sunteti logat\n");
        display_error("Nu sunteti logat");
        return 1; // Indică o eroare
    }
    return 0; // Nu a apărut nicio eroare
}

void display_book_info(JSON_Object *json_object)
{
    // Obținerea valorilor din obiectul JSON
    int id = json_object_get_number(json_object, "id");
    const char *title = json_object_get_string(json_object, "title");
    const char *author = json_object_get_string(json_object, "author");
    const char *genre = json_object_get_string(json_object, "genre");
    const char *publisher = json_object_get_string(json_object, "publisher");
    int page_count = json_object_get_number(json_object, "page_count");

    // Afisarea informatiilor despre carte
    printf("    {\n");
    printf("        id: %d,\n", id);
    printf("        title: %s\n", title);
    printf("        author: %s\n", author);
    printf("        genre: %s\n", genre);
    printf("        publisher: %s\n", publisher);
    printf("        page_count: %d\n", page_count);
    printf("    }\n");
}

void display_books(JSON_Array *json_array)
{
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
}

void extract_cookie(const char *response, char *cookies)
{
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
}

void extract_token(const char *response, char *token)
{
    const char *token_start = strstr(response, "token\":\"");
    if (token_start != NULL)
    {
        token_start += strlen("token\":\""); // Mutăm pointerul la începutul valorii token-ului
        const char *token_end = strstr(token_start, "\"");

        int token_length = token_end - token_start; // Calculăm lungimea token-ului

        strncpy(token, token_start, token_length); // Copiem token-ul în buffer-ul nostru
        token[token_length] = '\0';                // Terminăm șirul cu un caracter nul
    }
    else
    {
        token[0] = '\0'; // Setăm primul caracter al șirului la NULL dacă token-ul nu a fost găsit în răspuns
    }
}

int contains_spaces(char *str)
{
    while (*str)
    {
        if (isspace(*str))
        {
            return 1; // Returnează 1 dacă găsește un spațiu
        }
        str++;
    }
    return 0; // Returnează 0 dacă nu găsește spații
}
