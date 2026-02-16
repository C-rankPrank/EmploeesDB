#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage:\n\t %s -n -f <database file path>\n", argv[0]);
    printf("\t -n - create new database file\n");
    printf("\t -f - path to DB file\n");
}

int main(int argc, char *argv[]) { 
    bool newfile = false;
    char *filepath = NULL;
    int c;

    int dbfd = -1;
    struct dbheader_t *dbheader = NULL;

    while ((c = getopt(argc, argv, "nf:")) != -1) {
        switch (c) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filepath = optarg;
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
            return -1;
        }

    }


    printf("New file: %d\n", newfile);
    printf("Filepath: %s\n", filepath != NULL ? filepath : "NULL");

    output_file(dbfd, dbheader, NULL);

    return 0;
}
