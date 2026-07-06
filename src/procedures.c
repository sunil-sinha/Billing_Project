#include "fifteen_columns.h"

#include <stdio.h>

const char *fc_status_message(FcStatus status) {
    switch (status) {
        case FC_OK:
            return "success";
        case FC_ERR_INVALID_ARGUMENT:
            return "invalid argument";
        case FC_ERR_FILE_OPEN:
            return "could not open file";
        case FC_ERR_FILE_READ:
            return "could not read expected columns from file";
        case FC_ERR_FILE_WRITE:
            return "could not write file, error";
        default:
            return "unknown error";
    }
}

FcStatus save_multiplied_inputs(
    const char *output_file,
    long input1,
    long input2,
    long input3,
    long input4,
    long input5,
    long input6,
    long input7,
    long input8,
    long input9,
    long input10,
    long input11,
    long input12,
    long input13,
    long input14,
    long input15,
    long input16
) {
    if (output_file == NULL) {
        return FC_ERR_INVALID_ARGUMENT;
    }

    long values[FC_INPUT_COUNT] = {
        input1, input2, input3, input4, input5,
        input6, input7, input8, input9, input10,
        input11, input12, input13, input14, input15,
        input16
    };

    FILE *file = fopen(output_file, "w");
    if (file == NULL) {
        return FC_ERR_FILE_OPEN;
    }

    for (size_t i = 0; i < FC_INPUT_COUNT; i++) {
        long multiplied_value = values[i] * (long)(i + 1);
        if (fprintf(file, "%ld%s", multiplied_value, i + 1 == FC_INPUT_COUNT ? "\n" : " ") < 0) {
            fclose(file);
            return FC_ERR_FILE_WRITE;
        }
    }

    if (fclose(file) != 0) {
        return FC_ERR_FILE_WRITE;
    }

    return FC_OK;
}

FcStatus copy_selected_columns(
    const char *input_file,
    const char *output_file
) {
    if (input_file == NULL || output_file == NULL) {
        return FC_ERR_INVALID_ARGUMENT;
    }

    FILE *source = fopen(input_file, "r");
    if (source == NULL) {
        return FC_ERR_FILE_OPEN;
    }

    long columns[FC_INPUT_COUNT];
    for (size_t i = 0; i < FC_INPUT_COUNT; i++) {
        if (fscanf(source, "%ld", &columns[i]) != 1) {
            fclose(source);
            return FC_ERR_FILE_READ;
        }
    }
    fclose(source);

    FILE *destination = fopen(output_file, "w");
    if (destination == NULL) {
        return FC_ERR_FILE_OPEN;
    }

    /* Index 6 shifted to 7 because a new argument was inserted at position 5,
       pushing all original sunil columns from position 6 onward one place to the right.
       Downstream consumers of selected_columns.txt are unaffected. */
    int selected_indexes[FC_SELECTED_COUNT] = {0, 2, 4, 7};
    for (size_t i = 0; i < FC_SELECTED_COUNT; i++) {
        long selected_value = columns[selected_indexes[i]];
        if (fprintf(destination, "%ld%s", selected_value, i + 1 == FC_SELECTED_COUNT ? "\n" : " ") < 0) {
            fclose(destination);
            return FC_ERR_FILE_WRITE;
        }
    }

    if (fclose(destination) != 0) {
        return FC_ERR_FILE_WRITE;
    }

    return FC_OK;
}

