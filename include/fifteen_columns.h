#ifndef FIFTEEN_COLUMNS_H
#define FIFTEEN_COLUMNS_H

#include <stddef.h>

#define FC_INPUT_COUNT 16
#define FC_SELECTED_COUNT 4

typedef enum {
    FC_OK = 0,
    FC_ERR_INVALID_ARGUMENT = 1,
    FC_ERR_FILE_OPEN = 2,
    FC_ERR_FILE_READ = 3,
    FC_ERR_FILE_WRITE = 4
} FcStatus;

const char *fc_status_message(FcStatus status);

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
);

FcStatus copy_selected_columns(
    const char *input_file,
    const char *output_file
);

#endif

