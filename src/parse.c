#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

int output_file(int fd, struct dbheader_t *dbhdr, employee_t *employees) {
    (void)employees; /* Unused */
    if (fd < 0) {
        printf("Incorrect file FD received\n");
        return STATUS_ERROR;
    }
    
    /* Manage endianess*/
    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->version = htons(dbhdr->version);
    dbhdr->count = htons(dbhdr->count);
    dbhdr->filesize = htonl(dbhdr->filesize);

    /* Mover cursor inside the file*/
    lseek(fd, 0, SEEK_SET);
    write(fd, dbhdr, sizeof(struct dbheader_t));


    return 0;

}	

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (fd < 0) {
        printf("Incorrect file FD received\n");
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Failed to allocate header dynamic memory\n");
        return STATUS_ERROR;
    }

    if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    /*  ntohl - network to host long
    *   manage endianess
    *   by default network sends MSB first (big endian - MSB in lowest address)
    */
    header->magic = ntohl(header->magic);
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->filesize = ntohl(header->filesize);

    if (header->magic != HEADER_MAGIC) {
        printf("Incorrect header magic\n");
        free(header);
        return STATUS_ERROR;
    }
    if (header->version != 1) {
        printf("DB version mismatch\n");
        free(header);
        return STATUS_ERROR;
    }
    
    struct stat db_stat = {0};
    fstat(fd, &db_stat);
    
    if (header->filesize != db_stat.st_size) {
        printf("Corrupted DB header\n");
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;

    return 0;
}

int create_db_header(struct dbheader_t **headerOut) {
    if (headerOut == NULL) {
        return STATUS_ERROR;
    }
	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Failed to allocate header dynamic memory\n");
        return STATUS_ERROR;
    }

    header->magic = HEADER_MAGIC;
    header->version = 0x1;
    header->count = 0;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;

    return STATUS_SUCCESS;
}


