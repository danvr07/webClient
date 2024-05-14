#ifndef _REQUESTS_
#define _REQUESTS_

#include "parson/parson.h"

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(char *host, char *url, char *query_params,
							char **cookies, int cookies_count, char *token);

char *compute_post_request(char *host, char *url, char* content_type, JSON_Value *json_value,
							char **cookies, int cookies_count, char *token);

char *compute_delete_request(char *host, char *url, char *query_params,
							char **cookies, int cookies_count, char *token);

#endif
