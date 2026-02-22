#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage:\n\t %s -n -f <database file path>\n", argv[0]);
    printf("\t %s -a <name,adress,hours>\n", argv[0]);
    printf("\t -n - create new database file\n");
    printf("\t -f - path to DB file\n");
    printf("\t -a - add employee to database\n");
    printf("\t -l - list employees\n");
    printf("\t -r - <name> remove employee from database\n");
    printf("\t -u - <name,hours> update employee hours\n");
}

int main(int argc, char *argv[]) { 
    bool newfile = false;
    bool listemployees = false;
    char *filepath = NULL;
    char *addstring = NULL;
    char *removeemployee = NULL;
    char *updateemployee = NULL;
    int c;

    int dbfd = -1;
    struct dbheader_t *dbheader = NULL;
    struct employee_t *employees = NULL;

    while ((c = getopt(argc, argv, "nf:a:lr:u:")) != -1) {
        switch (c) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filepath = optarg;
                break;
            case 'a':
                addstring = optarg;
                break;
            case 'l':
                listemployees = true;
            break;
            case 'r':
                removeemployee = optarg;
            break;
            case 'u':
                updateemployee = optarg;
            break;
            case '?':
                printf("Unexpected option -%c\n", c);
                break;
            default:
                return -1;
        }
    }

    if (filepath == NULL) {
        printf("file path is required option\n");
        print_usage(argv);
        return 0;
    }

    if (newfile) {
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to create DB file, already exists\n");
            return -1;
        }
        if (create_db_header(&dbheader) == STATUS_ERROR) {
            printf("Faield to create DB header\n");
            return -1;
        }
    } else {
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to open DB file, it does not exist\n");
            return -1;
        }
        if (validate_db_header(dbfd, &dbheader) == STATUS_ERROR) {
            printf("Failed to validate DB header\n");
            close_db_file(dbfd);
            return -1;
        }

    }


    printf("New file: %d\n", newfile);
    printf("Filepath: %s\n", filepath != NULL ? filepath : "NULL");

    if ((read_employees(dbfd, dbheader, &employees)) != STATUS_SUCCESS) {
        printf("Failed reading employees data from the DB\n");
        close_db_file(dbfd);
        return 0;
    }

    if (addstring != NULL) {
        add_employee(dbheader, &employees, addstring);
    }
    
    if (removeemployee != NULL) {
        remove_employee(dbheader, employees, removeemployee);
    }
    
    if (updateemployee != NULL) {
        update_employee(dbheader, employees, updateemployee);
    }

    if (listemployees) {
        list_employees(dbheader, employees);
    }

    output_file(dbfd, dbheader, employees);

    close_db_file(dbfd);
    return 0;
}
