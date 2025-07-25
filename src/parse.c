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

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {

}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
    return 0;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    return 0;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    
    if (lseek(fd, SEEK_SET, 0) == -1){
        printf("Error seeking file in file descriptor!\n");
        free(dbhdr);
        free(employees);
        return STATUS_ERROR;
    }
    
    // Convert Database Header host endianess to network endianess
    dbhdr->count = htons(dbhdr->count);
    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->version = htons(dbhdr->version);
    dbhdr->filesize = htonl(dbhdr->filesize);

    if (write(fd, dbhdr, sizeof(struct dbheader_t)) == -1){
        printf("Error writing header to database file.\n");
        return STATUS_ERROR;
    }
    if (employees){
        if (write(fd, employees, dbhdr->filesize - sizeof(struct dbheader_t)) == -1){
            printf("Error writing employees to database file.\n");
            free(dbhdr);
            free(employees);
            return STATUS_ERROR;
        }
    }
    close(fd);
    return EXIT_SUCCESS;
    
}	

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    struct dbheader_t *header = malloc(sizeof(struct dbheader_t));
    if (header == NULL){
        perror("Failed to allocate memory for Database Header!");
        return STATUS_ERROR;
    }
    if( read(fd, header, sizeof(struct dbheader_t)) == -1){
        printf("Cannot read struct from dbfile!");
        return STATUS_ERROR;
    }
    
    // Convert from network to host endianess
    header->magic = ntohl(header->magic);
    header->count = ntohs(header->count);
    header->filesize = ntohl(header->filesize);
    header->version = ntohs(header->version);


    if (header->version != VERSION){
        printf("Invalid database version!\n");
        free(header);
        return STATUS_ERROR;
    }
    if (header->magic != HEADER_MAGIC){
        printf("Invalid Header Magic value!\n");
        free(header);
        return STATUS_ERROR;
    }
    if (header->filesize < sizeof(struct dbheader_t)){
        printf("Invalid filesize in Database Header!\n");
        free(header);
        return STATUS_ERROR;
    }
    *headerOut = header;
    return STATUS_SUCCESS;
}

int create_db_header(int fd, struct dbheader_t **headerOut) {
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL){
        perror("Failed to allocate memory for header!");
        return STATUS_ERROR;
    }

    header->magic = HEADER_MAGIC;
    header->version = 1;
    header->count = 0;
    header->filesize = sizeof(struct dbheader_t);
    
    *headerOut = header;
    return STATUS_SUCCESS;
}


