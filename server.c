#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cdb.h>

#define PORT 8080
#define BUFFER_SIZE 1024

char *find_redirect(const char *key) {
    struct cdb cdb;
    int fd = open("redirects.cdb", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return NULL;
    }

    cdb_init(&cdb, fd);

    unsigned klen = strlen(key);
    char *value = NULL;
    unsigned vlen;
    char *result = NULL;

    if (cdb_find(&cdb, key, klen) > 0) {
        vlen = cdb_datalen(&cdb);
        value = malloc(vlen + 1);
        cdb_read(&cdb, value, vlen, cdb_datapos(&cdb));
        value[vlen] = '\0';
        result = strdup(value);
        free(value);
    }

    cdb_free(&cdb);
    close(fd);
    return result;
}

void handle_redirect(int client_socket, const char *url) {
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "HTTP/1.1 302 Found\nLocation: %s\nContent-Length: 0\n\n", url);
    write(client_socket, response, strlen(response));
}

void handle_not_found(int client_socket) {
    char *response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nContent-Length: 9\n\nNot Found";
    write(client_socket, response, strlen(response));
}

void route(int client_socket, const char *path) {
    char *url = find_redirect(path + 1); // Eliminar el '/' inicial
    if (url) {
        handle_redirect(client_socket, url);
        free(url);
    } else {
        handle_not_found(client_socket);
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Crear el socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección y puerto del servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Adjuntar el socket a la dirección y puerto
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Escuchar en el socket
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        read(new_socket, buffer, BUFFER_SIZE);
        printf("%s\n", buffer);

        // Parsear la primera línea del request
        char method[16], path[256], version[16];
        sscanf(buffer, "%s %s %s", method, path, version);

        // Enrutar el request
        route(new_socket, path);

        close(new_socket);
    }

    close(server_fd);
    return 0;
}

