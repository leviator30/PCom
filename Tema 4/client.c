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

#include "parson.h"
#include <ctype.h>

#define SERVERADDR "34.246.184.49"
#define PORT 8080
#define HTTP_PREFIX_LENGTH 9
#define COOKIE_LENGTH 126
#define COOKIE_PREFIX_LENGTH 12
#define TOKEN_LENGTH 278

int main(int argc, char *argv[])
{
    char *message = NULL;
    char *response = NULL;

    char command[LINELEN];

    char *sessionCookie = NULL;
    char *sessionToken = NULL;
    int connectionStatus = 0;

    while(1) {

        int sockfd = open_connection(SERVERADDR, PORT, AF_INET, SOCK_STREAM, 0);

        scanf("%s", command);

        /* REGISTER command */
        if(!strncmp(command, "register", 9)) {

            char username[LINELEN];
            char password[LINELEN];

            while (getchar() != '\n');
            printf("username=");
            fgets(username, LINELEN, stdin);
            username[strcspn(username, "\n")] = '\0';

            printf("password=");
            fgets(password, LINELEN, stdin);
            password[strcspn(password, "\n")] = '\0';

            int contains_whitespace = 0;
            for (int i = 0; i < strlen(username); i++)
                if (isspace(username[i])) {
                    contains_whitespace = 1;
                    break;
                }

            if(contains_whitespace) {
                message = NULL;
                response = NULL;
                printf("ERROR! - Username invalid!\n");
            } else {

                JSON_Value *userValue = json_value_init_object();
                JSON_Object *userObject = json_value_get_object(userValue);

                json_object_set_string(userObject, "username", username);
                json_object_set_string(userObject, "password", password);

                char *serializedString = json_serialize_to_string(userValue);
                message = compute_post_request(SERVERADDR, "/api/v1/tema/auth/register", "application/json", &serializedString, 1, NULL, 0, NULL);
                send_to_server(sockfd, message);

                response = receive_from_server(sockfd);
                JSON_Value *json_response = json_parse_string(basic_extract_json_response(response));
                int resCode = atoi(response + HTTP_PREFIX_LENGTH);

                if (resCode==201)
                    printf("SUCCES - Utilizator inregistrat cu succes!\n");
                else
                    printf("ERROR! %d - %s\n", resCode, json_object_get_string(json_object(json_response), "error"));

                json_value_free(json_response);
                json_free_serialized_string(serializedString);
            }

        /* LOGIN command */
        } else if(!strncmp(command, "login", 6)) {

            char username[LINELEN];
            char password[LINELEN];

            while (getchar() != '\n');
            printf("username=");
            fgets(username, LINELEN, stdin);
            username[strcspn(username, "\n")] = '\0';

            printf("password=");
            fgets(password, LINELEN, stdin);
            password[strcspn(password, "\n")] = '\0';

            int contains_whitespace = 0;
            for (int i = 0; i < strlen(username); i++)
                if (isspace(username[i])) {
                    contains_whitespace = 1;
                    break;
                }

            message = NULL;
            response = NULL;

            if(connectionStatus==1) {
                printf("ERROR! - Sunteti deja logat cu un cont!\n");
            } else if(contains_whitespace) {
                printf("ERROR! - Username invalid!\n");
            } else {
                JSON_Value *userValue = json_value_init_object();
                JSON_Object *userObject = json_value_get_object(userValue);

                json_object_set_string(userObject, "username", username);
                json_object_set_string(userObject, "password", password);

                char *serializedString = json_serialize_to_string(userValue);
                message = compute_post_request(SERVERADDR, "/api/v1/tema/auth/login", "application/json", &serializedString, 1, NULL, 0, NULL);
                send_to_server(sockfd, message);

                response = receive_from_server(sockfd);
                JSON_Value *json_response = json_parse_string(basic_extract_json_response(response));
                int resCode = atoi(response + HTTP_PREFIX_LENGTH);

                if (resCode==200) {
                    printf("SUCCES! - Utilizator logat cu succes!\n");

                    connectionStatus = 1;
                    
                    char *cookie = strstr(response, "Set-Cookie");
                    if (sessionCookie != NULL) {
                        free(sessionCookie);
                        sessionCookie = NULL;
                    }
                    
                    sessionCookie = calloc(COOKIE_LENGTH + 1, sizeof(char));
                    memcpy(sessionCookie, cookie + COOKIE_PREFIX_LENGTH, COOKIE_LENGTH);
                    strtok(sessionCookie, ";");

                } else {
                    printf("ERROR! %d - %s\n", resCode, json_object_get_string(json_object(json_response), "error"));
                }

                json_value_free(json_response);
                json_free_serialized_string(serializedString);
            }

        /* ENTER_LIBRARY command */
        } else if(!strncmp(command, "enter_library", 14)) {

            message = NULL;
            response = NULL;

            if(connectionStatus==0) {
                printf("ERROR! - Nu sunteti conectat!\n");
            } else if(sessionToken != NULL) {
                printf("ERROR! - Sunteti deja conectat!\n");
            } else {
                message = compute_get_request(SERVERADDR, "/api/v1/tema/library/access", NULL, &sessionCookie, 1, NULL);
                send_to_server(sockfd, message);

                response = receive_from_server(sockfd);
                JSON_Value *json_response = json_parse_string(basic_extract_json_response(response));
                int resCode = atoi(response + HTTP_PREFIX_LENGTH);

                if(resCode == 200) {
                    printf("SUCCES - Utilizatorul are acces la biblioteca!\n");

                    if(sessionToken != NULL) {
                        free(sessionToken);
                        sessionToken = NULL;
                    }
                    
                    sessionToken = calloc(TOKEN_LENGTH + 1, sizeof(char));
                    strcat(sessionToken, json_object_get_string(json_object(json_response), "token"));

                } else {
                    printf("ERROR! %d - %s\n", resCode, json_object_get_string(json_object(json_response), "error"));
                }

                json_value_free(json_response);
            }

        /* GET_BOOKS command */
        } else if(!strncmp(command, "get_books", 10)) {
            if(sessionToken==NULL || connectionStatus==0) {
                message = NULL;
                response = NULL;
                printf("ERROR! - Nu aveti acces la bibioteca!\n");
            } else {
                message = compute_get_request(SERVERADDR, "/api/v1/tema/library/books", NULL, &sessionCookie, 1, sessionToken);
                send_to_server(sockfd, message);

                response = receive_from_server(sockfd);
                JSON_Value *json_response = json_parse_string(basic_extract_json_response(response));
                int resCode = atoi(response + HTTP_PREFIX_LENGTH);

                char *json_start = strstr(response, "\r\n\r\n");
                json_start += 4;
                JSON_Value *root_value = json_parse_string(json_start);

                if(resCode == 200) {
                    printf("SUCCES! - Cartile sunt:\n");
                    char *serialized_string = NULL;
                    serialized_string = json_serialize_to_string_pretty(root_value);
                    printf("%s\n", serialized_string);

                } else {
                    printf("ERROR! %d - %s\n", resCode, json_object_get_string(json_object(json_response), "error"));
                }

                json_value_free(json_response);
            }

        /* GET_BOOK command */
        } else if(!strncmp(command, "get_book", 9)) {

            message = NULL;
            response = NULL;

            if(sessionToken==NULL || connectionStatus==0) {
                printf("ERROR! - Nu aveti acces la bibioteca!\n");
            } else {
                char id[LINELEN];

                while (getchar() != '\n');
                printf("id=");
                fgets(id, LINELEN, stdin);
                id[strcspn(id, "\n")] = '\0';

                int is_valid = 1;
                    for (int i = 0; i < strlen(id); i++)
                        if (!isdigit(id[i])) {
                            is_valid = 0; 
                            break;
                        }

                if(!is_valid) {
                    printf("ERROR! - Id is invalid!\n");
                } else {

                    char str[LINELEN + 28];
                    snprintf(str, sizeof(str), "/api/v1/tema/library/books/%s", id);

                    message = compute_get_request(SERVERADDR, str, NULL, &sessionCookie, 1, sessionToken);
                    send_to_server(sockfd, message);

                    response = receive_from_server(sockfd);
                    JSON_Value *json_response = json_parse_string(basic_extract_json_response(response));
                    int resCode = atoi(response + HTTP_PREFIX_LENGTH);

                    char *json_start = strstr(response, "\r\n\r\n");
                    json_start += 4;
                    JSON_Value *root_value = json_parse_string(json_start);


                    if(resCode == 200) {
                        printf("SUCCES! - Cartea este:\n");
                        char *serialized_string = NULL;
                        serialized_string = json_serialize_to_string_pretty(root_value);
                        printf("%s\n", serialized_string);
                    } else {
                        printf("ERROR! %d - %s\n", resCode, json_object_get_string(json_object(json_response), "error"));
                    }

                    json_value_free(json_response);
                }
            }
        
        /* ADD_BOOK command*/
        } else if(!strncmp(command, "add_book", 9)) {

            message = NULL;
            response = NULL;

            if(sessionToken==NULL || connectionStatus==0) {
                printf("ERROR! - Nu aveti acces la bibioteca!\n");
            } else {
                char title[LINELEN];
                char author[LINELEN];
                char genre[LINELEN];
                char page_count[LINELEN];
                char publisher[LINELEN];

                while (getchar() != '\n');
                printf("title=");
                fgets(title, LINELEN, stdin);
                title[strcspn(title, "\n")] = '\0';

                printf("author=");
                fgets(author, LINELEN, stdin);
                author[strcspn(author, "\n")] = '\0';

                printf("genre=");
                fgets(genre, LINELEN, stdin);
                genre[strcspn(genre, "\n")] = '\0';

                printf("page_count=");
                fgets(page_count, LINELEN, stdin);
                page_count[strcspn(page_count, "\n")] = '\0';

                int is_valid = 1;
                    for (int i = 0; i < strlen(page_count); i++)
                        if (!isdigit(page_count[i])) {
                            is_valid = 0; 
                            break;
                        }

                printf("publisher=");
                fgets(publisher, LINELEN, stdin);
                publisher[strcspn(publisher, "\n")] = '\0';

                if(!is_valid) {
                    printf("ERROR! - Tip de date incorect pentru numarul de pagini!\n");
                } else {
                
                    JSON_Value *bookValue = json_value_init_object();
                    JSON_Object *book = json_value_get_object(bookValue);

                    json_object_set_string(book, "title", title);
                    json_object_set_string(book, "author", author);
                    json_object_set_string(book, "genre", genre);
                    json_object_set_string(book, "page_count", page_count);
                    json_object_set_string(book, "publisher", publisher);
                    
                    char *serializedString = json_serialize_to_string(bookValue);

                    message = compute_post_request(SERVERADDR, "/api/v1/tema/library/books", "application/json", &serializedString, 1, &sessionCookie, 1, sessionToken);
                    send_to_server(sockfd, message);

                    response = receive_from_server(sockfd);
                    JSON_Value *json_response = json_parse_string(basic_extract_json_response(response));
                    int resCode = atoi(response + HTTP_PREFIX_LENGTH);

                    if(resCode == 200) {
                        printf("SUCCES! - Carte adaugata cu succes!\n");
                    } else {
                        printf("ERROR! %d - %s\n", resCode, json_object_get_string(json_object(json_response), "error"));
                    }

                    json_free_serialized_string(serializedString);
                    json_value_free(json_response);
                }
            }

        /* DELETE_BOOK command */
        } else if(!strncmp(command, "delete_book", 12)) {

            message = NULL;
            response = NULL;

            if(sessionToken==NULL || connectionStatus==0) {
                printf("ERROR! - Nu aveti acces la bibioteca!\n");
            } else {
                char id[LINELEN];

                while (getchar() != '\n');
                printf("id=");
                fgets(id, LINELEN, stdin);
                id[strcspn(id, "\n")] = '\0';

                int is_valid = 1;
                    for (int i = 0; i < strlen(id); i++)
                        if (!isdigit(id[i])) {
                            is_valid = 0; 
                            break;
                        }

                if(!is_valid) {
                    printf("ERROR! - Id is invalid!\n");
                } else {

                    char str[LINELEN + 28];
                    snprintf(str, sizeof(str), "/api/v1/tema/library/books/%s", id);

                    message = compute_delete_request(SERVERADDR, str, NULL, &sessionCookie, 1, sessionToken);
                    send_to_server(sockfd, message);

                    response = receive_from_server(sockfd);
                    JSON_Value *json_response = json_parse_string(basic_extract_json_response(response));
                    int resCode = atoi(response + HTTP_PREFIX_LENGTH);

                    if(resCode == 200) {
                        printf("SUCCES! - Cartea cu id %s a fost stearsa cu succes!\n", id);
                    } else {
                        printf("ERROR! %d - %s\n", resCode, json_object_get_string(json_object(json_response), "error"));
                    }

                    json_value_free(json_response);
                }
            }

        /* LOGOUT command */
        } else if(!strncmp(command, "logout", 7)) {
            if(connectionStatus==0) {
                message = NULL;
                response = NULL;
                printf("ERROR! - Nu sunteti conectat!\n");
            } else {
                message = compute_get_request(SERVERADDR, "/api/v1/tema/auth/logout", NULL, &sessionCookie, 1, NULL);
                send_to_server(sockfd, message);

                response = receive_from_server(sockfd);
                JSON_Value *json_response = json_parse_string(basic_extract_json_response(response));
                int resCode = atoi(response + HTTP_PREFIX_LENGTH);

                if(resCode == 200) {
                    connectionStatus = 0;
                    sessionToken = NULL;
                    sessionCookie = NULL;
                    printf("SUCCES! - Utilizatorul s-a delogat cu succes!\n");
                } else {
                    printf("ERROR! %d - %s\n", resCode, json_object_get_string(json_object(json_response), "error"));
                }

                json_value_free(json_response);
            }

        /* EXIT command */
        } else if (!strncmp(command, "exit", 5)) {
            printf("Inchidere program!\n");
            break;

        /* BAD WRITTING */
        } else {
            message = NULL;
            response = NULL;
            printf("ERROR! - Comanda %s inexistenta!\n", command);
        }

        free(message);
        free(response);

        close_connection(sockfd);
    }

    return 0;
}
