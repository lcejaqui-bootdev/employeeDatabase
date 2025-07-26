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
    struct employee_t *emp;
    for (int i = 0; i < dbhdr->count; i++){
        emp = &employees[i];
        printf("Employee %d\n", i);
        printf("\t Name: %s\n", emp->name);
        printf("\t Address: %s\n", emp->address);
        printf("\t Hours: %d\n", emp->hours);
    }
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
    char *name = strtok(addstring, ",");
    char *address = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");
    
    strncpy(employees[dbhdr->count].name, name, MAX_DATA);
    strncpy(employees[dbhdr->count].address, address, MAX_DATA);
    employees[dbhdr->count].hours = atoi(hours);
    
    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    if (fd < 0){
        printf("File Descriptor invalid in read_employees()\n");
        return STATUS_ERROR;
    }
    int count = dbhdr->count;
    
    if (count == 0)
        return STATUS_SUCCESS;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == NULL){
        perror("Error Allocating memory for employees!");
        return STATUS_ERROR;
    }
    read(fd, employees, count * sizeof(struct employee_t));
        
    for (int i = 0; i < count ; i++){
        employees[i].hours = ntohl(employees[i].hours);
    }
    *employeesOut = employees;
    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    
    if (lseek(fd, SEEK_SET, 0) == -1){
        printf("Error seeking file in file descriptor!\n");
        return STATUS_ERROR;
    }
    // Converting to network endianness may change the value of count!
    int count = dbhdr->count;

    // Convert Database Header host endianess to network endianess
    dbhdr->count = htons(dbhdr->count);
    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->version = htons(dbhdr->version);
    dbhdr->filesize = htonl(dbhdr->filesize);

    if (write(fd, dbhdr, sizeof(struct dbheader_t)) == -1){
        printf("Error writing header to database file.\n");
        return STATUS_ERROR;
    }
    if (count > 0){
        struct employee_t *emp;
        for (int i = 0; i < count; i++){
            emp = &employees[i];
            emp->hours = htonl(emp->hours);
            if(write(fd, emp, sizeof(struct employee_t)) == -1){
                printf("Error writing employee[%d] to database!\n", i);
                return STATUS_ERROR;
            }
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
        
        return STATUS_ERROR;
    }
    if (header->magic != HEADER_MAGIC){
        printf("Invalid Header Magic value!\n");
        
        return STATUS_ERROR;
    }
    if (header->filesize < sizeof(struct dbheader_t)){
        printf("Invalid filesize in Database Header!\n");
        
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


