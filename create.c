/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cdb.h>
#include <fcntl.h>
#include <unistd.h>

void test(void);
void create_cdb(const char *filename, const char *routes_file) {
    struct cdb_make cdbm;
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
        perror("open");
        return;
    }

    cdb_make_start(&cdbm, fd);

    FILE *file = fopen(routes_file, "r");
    if (!file) {
        perror("fopen");
        close(fd);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *key = strtok(line, ",");
        char *url = strtok(NULL, "\n");
        if (key && url) {
            cdb_make_add(&cdbm, key, strlen(key), url, strlen(url));
        }
    }

    fclose(file);
    cdb_make_finish(&cdbm);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <cdb_filename> <routes_file>\n", argv[0]);
        return 1;
    }

    create_cdb(argv[1], argv[2]);
    return 0;
}
