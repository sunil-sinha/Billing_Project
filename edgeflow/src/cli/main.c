#include "edgeflow.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(FILE *out) {
    fprintf(out, "Usage: edgeflow --config PATH [--events N] [--seed N] [--verbose]\n");
}

static int parse_args(int argc, char **argv, const char **config_path, EfRunOptions *options) {
    *config_path = NULL;
    options->event_count = 100;
    options->seed = 1;
    options->verbose = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            *config_path = argv[++i];
        } else if (strcmp(argv[i], "--events") == 0 && i + 1 < argc) {
            options->event_count = (size_t)strtoull(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            options->seed = (unsigned)strtoul(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            options->verbose = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            usage(stdout);
            exit(0);
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            return 0;
        }
    }

    return *config_path != NULL;
}

int main(int argc, char **argv) {
    const char *config_path = NULL;
    EfRunOptions options;

    if (!parse_args(argc, argv, &config_path, &options)) {
        usage(stderr);
        return 2;
    }

    if (options.verbose) {
        ef_log_set_level(EF_LEVEL_DEBUG);
    }

    EfConfig config;
    char err[EF_MAX_ERROR] = {0};
    EfStatus status = ef_config_load_file(&config, config_path, err, sizeof(err));
    if (status != EF_OK) {
        fprintf(stderr, "Config error: %s\n", err);
        return 1;
    }

    EfEngine engine;
    status = ef_engine_init(&engine, &config);
    if (status != EF_OK) {
        fprintf(stderr, "Engine error: %s\n", engine.last_error[0] ? engine.last_error : ef_status_name(status));
        return 1;
    }

    status = ef_engine_run_simulation(&engine, &options);
    if (status != EF_OK) {
        fprintf(stderr, "Run completed with warning: %s\n", engine.last_error[0] ? engine.last_error : ef_status_name(status));
    }

    ef_metrics_print(&engine.metrics, stdout);
    ef_engine_destroy(&engine);
    return status == EF_OK ? 0 : 1;
}

