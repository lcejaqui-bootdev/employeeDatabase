#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"


void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database file> -a <Employee> -r\n", argv[0]);
    printf("\t -n - create new database file\n");
    printf("\t -f - (required) path to database file\n");
    printf("\t -a - Employee to add to database file\n");
    printf("\t -r - Reads employee?\n");
}

int main(int argc, char *argv[]) { 
	
    int c;
    int dbfd = -1;
    char *employee_str = NULL;
    char *filepath = NULL;

    bool newfile = false;
    bool list = false;

    struct dbheader_t *dbhdr = NULL;
    struct employee_t *employees = NULL;

    while((c = getopt(argc, argv, "nf:a:l")) != -1){
        switch (c){
            case 'n':
                newfile = true;
                break;
            case 'f':
                filepath = optarg;
                break;
            case 'a':
                employee_str = optarg;
                break;
            case 'l':
                list = true;
                break;
            case '?':
                printf("Unknown option -%c\n", c);
                break;
            default:
                return -1;
        }
    }

    if (filepath == NULL){
        printf("Filepath is a required argument\n");
        print_usage(argv);
        return -1; 
    }

    if (newfile){
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR){
            printf("Unable to create database!\n");
            return -1;
        }
        if(create_db_header(dbfd, &dbhdr) == STATUS_ERROR){
            return -1;
        }
    }else{   
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR){
            printf("Unable to open database!\n");
            return -1;
        }
        if(validate_db_header(dbfd, &dbhdr) == STATUS_ERROR){
            printf("Invalid Database Header!\n");
            free(dbhdr);
            close(dbfd);
            return -1;
        }
    }

    if (read_employees(dbfd, dbhdr, &employees) == STATUS_ERROR){
        printf("Error reading employees from database!\n");
        free(dbhdr);
        free(employees);
        close(dbfd);
        return -1;
    }

    if (employee_str){
        if (employees)
            employees = realloc(employees, (dbhdr->count + 1) * sizeof(struct employee_t));
        else
            employees = calloc(1, sizeof(struct employee_t));
        if (!employees){
            perror("Error Reallocating memory for employees.\n");
            free(dbhdr);
            close(dbfd);
            return -1;
        }
        add_employee(dbhdr, employees, employee_str);
        dbhdr->count++;
        dbhdr->filesize += sizeof(struct employee_t);
    }

    if (list){
        list_employees(dbhdr, employees);
    }

    if (output_file(dbfd, dbhdr, employees) == STATUS_ERROR){
        printf("Error writing to file!\n");
        free(dbhdr);
        free(employees);
        close(dbfd);
        return -1;
    }

    return 0;
}
