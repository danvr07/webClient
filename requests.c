#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        // Assuming each cookie is added as a separate header
        for (int i = 0; i < cookies_count; i++) {
            compute_message(message, cookies[i]);
        }
    }

    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // Step 4: add final new line
   compute_message(message, "");
    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, JSON_Value *json_value,
                            char **cookies, int cookies_count, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = NULL;

    // Convert JSON value to string
    body_data_buffer = json_serialize_to_string_pretty(json_value);
    if (!body_data_buffer) {
        fprintf(stderr, "Error: Failed to convert JSON to string\n");
        free(message);
        free(line);
        return NULL;
    }

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
    
    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    int body_size = strlen(body_data_buffer);
    sprintf(line, "Content-Length: %d", body_size);
    compute_message(message, line);
    
    // Step 4 (optional): add cookies
    if (cookies != NULL) {
        // Assuming each cookie is added as a separate header
        for (int i = 0; i < cookies_count; i++) {
            compute_message(message, cookies[i]);
        }
    }

    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // Step 5: add new line at end of headers
    compute_message(message, "");

    // Step 6: add the actual payload data (JSON)
    compute_message(message, body_data_buffer);

    free(line);
    json_free_serialized_string(body_data_buffer); // Free the serialized JSON string
    return message;
}

char* compute_delete_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Construim prima linie a cererii DELETE
    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);

    // Adăugăm antetul Host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Dacă există parametrii de interogare, îi adăugăm
    if (query_params != NULL)
    {
        sprintf(line, "query_params");
        compute_message(message, line);
    }

    // Adăugăm antetul Cookie, dacă există
    if (cookies != NULL && cookies_count > 0)
    {
        for (int i = 0; i < cookies_count; i++)
        {
            compute_message(message, cookies[i]);
        }
    }

    // Adăugăm antetul Authorization, dacă există un token
    if (token != NULL && strcmp(token, "") != 0)
    {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // Adăugăm antetul de conexiune
    compute_message(message, "Connection: close");

    // Adăugăm linia goală pentru a marca sfârșitul antetelor
    compute_message(message, "");

    free(line);
    return message;
}
