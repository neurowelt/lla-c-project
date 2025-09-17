#include <stdio.h>
#include <sys/_endian.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "../include/common.h"
#include "../include/parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
    return 0;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    return 0;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    if (fd < 0) {
        printf("Bad file descriptor from the user\n");
        return STATUS_ERROR;
    }

    // Pack back to network endian
    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->version = htonl(dbhdr->version);
    dbhdr->filesize = htonl(dbhdr->filesize);
    dbhdr->count = htonl(dbhdr->count);

    // Bring cursor to the beginning of the file for proper closing
    lseek(fd, 0, SEEK_SET);

    write(fd, dbhdr, sizeof(struct dbheader_t));

    return 0;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (fd < 0) {
        printf("Bad file descriptor from the user\n");
        return STATUS_ERROR;
    }
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == -1) {
        printf("Malloc failed to create DB header\n");
        return STATUS_ERROR;
    }

    if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    // We're need to unpack data to host endian
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    if (header->version != 1) {
        printf("Improper header version\n");
        free(header);
        return STATUS_ERROR;
    }

    if (header->magic != HEADER_MAGIC) {
        printf("Improper header magic\n");
        free(header);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    if (header->filesize != dbstat.st_size) {
        printf("Corrupted database file (size not right)\n");
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;

    return STATUS_SUCCESS;
}

int create_db_header(int fd, struct dbheader_t **headerOut) {
    // Ask the heap to allocate memory space for this structure
    // and return the pointer to that memory outside of this function
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == -1) {
        printf("Malloc failed to create DB header\n");
        return STATUS_ERROR;
    }

    header->version = 0x1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;

    return STATUS_SUCCESS;
}
