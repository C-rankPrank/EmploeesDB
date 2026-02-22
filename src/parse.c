#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "common.h"
#include "parse.h"

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    if (fd < 0) {
        printf("Incorrect file FD received\n");
        return STATUS_ERROR;
    }
    
    int count = dbhdr->count;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == NULL) {
        printf("Malloc failed\n");
        return STATUS_ERROR;
    }

    /* After reading header, coursor of the file is at employee data start*/
    read(fd, employees, count*sizeof(struct employee_t));

    for(int i = 0; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;
    
    return STATUS_SUCCESS;
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *addstring) {
    if ((dbhdr == NULL) ||
        (employees == NULL) ||
        (*employees == NULL) ||
        (addstring == NULL)) {
        printf("Failed to add employee\n");
        return STATUS_ERROR;
    }

    char *name = strtok(addstring, ",");
    
    while(name != NULL) {
        char *address = strtok(NULL, ",");
        if(NULL == address) break;
        char *hours = strtok(NULL, ",");
        if(NULL == hours) break;
        
        struct employee_t *e = *employees;
        e = realloc(e, (dbhdr->count+1) * sizeof(struct employee_t));
        if (e == NULL) {
            break;
        }
        *employees = e;
        
        
        dbhdr->count++;
        /* sizeof() -1 because to leave room for \0 */
        strncpy(e[dbhdr->count-1].name, name, sizeof(e[dbhdr->count-1].name)-1);
        strncpy(e[dbhdr->count-1].address, address, sizeof(e[dbhdr->count-1].address)-1);
        e[dbhdr->count-1].hours = atoi(hours);
        
        name = strtok(NULL, ",");
    }  

    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    if (fd < 0) {
        printf("Incorrect file FD received\n");
        return STATUS_ERROR;
    }

    if ((employees == NULL) || (dbhdr == NULL)) {
        return STATUS_ERROR;
    }
    
    int realcount = dbhdr->count;

    /* Manage endianess*/
    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->version = htons(dbhdr->version);
    dbhdr->count = htons(dbhdr->count);
    dbhdr->filesize = htonl(sizeof(struct dbheader_t) + realcount * sizeof(struct employee_t));

    /* Mover cursor inside the file*/
    lseek(fd, 0, SEEK_SET);
    write(fd, dbhdr, sizeof(struct dbheader_t));

    for(int i = 0; i < realcount; i++) {
        employees[i].hours = htonl(employees[i].hours);
        write(fd, &employees[i], sizeof(struct employee_t));
    }

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
