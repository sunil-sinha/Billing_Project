#include "fifteen_columns.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static const char *MULTIPLIED_OUTPUT_FILE = "data/multiplied_values.txt";
static const char *SELECTED_OUTPUT_FILE = "data/selected_columns.txt";

static void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s n1 n2 n3 n4 n5 n6 n7 n8 n9 n10 n11 n12 n13 n14 n15 n16\n", program_name);
}

static int parse_long(const char *text, long *value) {
    char *end = NULL;
    errno = 0;
    long parsed = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0') {
        return 0;
    }
    *value = parsed;
    return 1;
}

int main(int argc, char **argv) {
    if (argc != FC_INPUT_COUNT + 1) {
        print_usage(argv[0]);
        return 2;
    }

    long input[FC_INPUT_COUNT];
    for (size_t i = 0; i < FC_INPUT_COUNT; i++) {
        if (!parse_long(argv[i + 1], &input[i])) {
            fprintf(stderr, "Invalid number at argument %lu: %s\n", (unsigned long)(i + 1), argv[i + 1]);
            return 2;
        }
    }

    FcStatus status = save_multiplied_inputs(
        MULTIPLIED_OUTPUT_FILE,
        input[0],
        input[1],
        input[2],
        input[3],
        input[4],
        input[5],
        input[6],
        input[7],
        input[8],
        input[9],
        input[10],
        input[11],
        input[12],
        input[13],
        input[14],
        input[15]
    );
    if (status != FC_OK) {
        fprintf(stderr, "save_multiplied_inputs failed: %s\n", fc_status_message(status));
        return 1;
    }

    status = copy_selected_columns(MULTIPLIED_OUTPUT_FILE, SELECTED_OUTPUT_FILE);
    if (status != FC_OK) {
        fprintf(stderr, "copy_selected_columns failed: %s\n", fc_status_message(status));
        return 1;
    }

    printf("Created %s\n", MULTIPLIED_OUTPUT_FILE);
    printf("Created %s\n", SELECTED_OUTPUT_FILE);
    return 0;
}

