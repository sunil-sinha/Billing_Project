#include "edgeflow.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void set_error(char *err, size_t err_len, const char *message) {
    if (err && err_len) {
        snprintf(err, err_len, "%s", message);
    }
}

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    return s;
}

static int parse_size(const char *text, size_t *out) {
    char *end = NULL;
    errno = 0;
    unsigned long value = strtoul(text, &end, 10);
    if (errno || end == text || *end != '\0') {
        return 0;
    }
    *out = (size_t)value;
    return 1;
}

static int parse_double(const char *text, double *out) {
    char *end = NULL;
    errno = 0;
    double value = strtod(text, &end);
    if (errno || end == text || *end != '\0') {
        return 0;
    }
    *out = value;
    return 1;
}

void ef_config_init(EfConfig *config) {
    if (!config) return;
    memset(config, 0, sizeof(*config));
    snprintf(config->pipeline_name, sizeof(config->pipeline_name), "edgeflow");
    config->queue_capacity = 128;
    snprintf(config->storage_path, sizeof(config->storage_path), "edgeflow-events.log");
    snprintf(config->network_endpoint, sizeof(config->network_endpoint), "memory://publisher");
}

const EfSensorDef *ef_config_find_sensor(const EfConfig *config, const char *id) {
    if (!config || !id) return NULL;
    for (size_t i = 0; i < config->sensor_count; i++) {
        if (strcmp(config->sensors[i].id, id) == 0) {
            return &config->sensors[i];
        }
    }
    return NULL;
}

static EfStatus parse_assignment(EfConfig *config, char *line, char *err, size_t err_len) {
    char *eq = strchr(line, '=');
    if (!eq) {
        set_error(err, err_len, "expected key = value assignment");
        return EF_ERR_PARSE;
    }
    *eq = '\0';
    char *key = trim(line);
    char *value = trim(eq + 1);

    if (strcmp(key, "pipeline_name") == 0) {
        snprintf(config->pipeline_name, sizeof(config->pipeline_name), "%s", value);
    } else if (strcmp(key, "queue_capacity") == 0) {
        if (!parse_size(value, &config->queue_capacity)) {
            set_error(err, err_len, "queue_capacity must be a number");
            return EF_ERR_PARSE;
        }
    } else if (strcmp(key, "storage_path") == 0) {
        snprintf(config->storage_path, sizeof(config->storage_path), "%s", value);
    } else if (strcmp(key, "network_endpoint") == 0) {
        snprintf(config->network_endpoint, sizeof(config->network_endpoint), "%s", value);
    } else {
        set_error(err, err_len, "unknown assignment key");
        return EF_ERR_PARSE;
    }
    return EF_OK;
}

static EfStatus parse_sensor(EfConfig *config, char **tokens, int count, char *err, size_t err_len) {
    if (count != 6) {
        set_error(err, err_len, "sensor requires: sensor id kind location unit reliability");
        return EF_ERR_PARSE;
    }
    if (config->sensor_count >= EF_MAX_SENSORS) {
        set_error(err, err_len, "too many sensors");
        return EF_ERR_PARSE;
    }

    EfSensorDef *sensor = &config->sensors[config->sensor_count++];
    snprintf(sensor->id, sizeof(sensor->id), "%s", tokens[1]);
    sensor->kind = ef_sensor_kind_parse(tokens[2]);
    snprintf(sensor->location, sizeof(sensor->location), "%s", tokens[3]);
    snprintf(sensor->unit, sizeof(sensor->unit), "%s", tokens[4]);
    if (!parse_double(tokens[5], &sensor->reliability)) {
        set_error(err, err_len, "sensor reliability must be numeric");
        return EF_ERR_PARSE;
    }
    if (sensor->kind == EF_SENSOR_UNKNOWN) {
        set_error(err, err_len, "unknown sensor kind");
        return EF_ERR_PARSE;
    }
    return EF_OK;
}

static EfStatus parse_route(EfConfig *config, char **tokens, int count, char *err, size_t err_len) {
    if (count != 4) {
        set_error(err, err_len, "route requires: route kind destination minimum_value");
        return EF_ERR_PARSE;
    }
    if (config->route_count >= EF_MAX_ROUTES) {
        set_error(err, err_len, "too many routes");
        return EF_ERR_PARSE;
    }

    EfRouteRule *route = &config->routes[config->route_count++];
    route->kind = ef_sensor_kind_parse(tokens[1]);
    snprintf(route->destination, sizeof(route->destination), "%s", tokens[2]);
    if (!parse_double(tokens[3], &route->minimum_value)) {
        set_error(err, err_len, "route minimum_value must be numeric");
        return EF_ERR_PARSE;
    }
    if (route->kind == EF_SENSOR_UNKNOWN) {
        set_error(err, err_len, "unknown route sensor kind");
        return EF_ERR_PARSE;
    }
    return EF_OK;
}

static EfStatus parse_transform(EfConfig *config, char **tokens, int count, char *err, size_t err_len) {
    if (count != 5) {
        set_error(err, err_len, "transform requires: transform kind type a b");
        return EF_ERR_PARSE;
    }
    if (config->transform_count >= EF_MAX_TRANSFORMS) {
        set_error(err, err_len, "too many transforms");
        return EF_ERR_PARSE;
    }

    EfTransformRule *transform = &config->transforms[config->transform_count++];
    memset(transform, 0, sizeof(*transform));
    transform->kind = ef_sensor_kind_parse(tokens[1]);
    transform->type = ef_transform_kind_parse(tokens[2]);
    if (!parse_double(tokens[3], &transform->a) || !parse_double(tokens[4], &transform->b)) {
        set_error(err, err_len, "transform parameters must be numeric");
        return EF_ERR_PARSE;
    }
    if (transform->kind == EF_SENSOR_UNKNOWN || transform->type == EF_TRANSFORM_UNKNOWN) {
        set_error(err, err_len, "unknown transform kind or type");
        return EF_ERR_PARSE;
    }
    return EF_OK;
}

static EfStatus parse_directive(EfConfig *config, char *line, char *err, size_t err_len) {
    char *tokens[8] = {0};
    int count = 0;

    for (char *tok = strtok(line, " \t\r\n"); tok && count < 8; tok = strtok(NULL, " \t\r\n")) {
        tokens[count++] = tok;
    }

    if (count == 0) return EF_OK;
    if (strcmp(tokens[0], "sensor") == 0) return parse_sensor(config, tokens, count, err, err_len);
    if (strcmp(tokens[0], "route") == 0) return parse_route(config, tokens, count, err, err_len);
    if (strcmp(tokens[0], "transform") == 0) return parse_transform(config, tokens, count, err, err_len);

    set_error(err, err_len, "unknown directive");
    return EF_ERR_PARSE;
}

EfStatus ef_config_load_file(EfConfig *config, const char *path, char *err, size_t err_len) {
    if (!config || !path) {
        set_error(err, err_len, "config and path are required");
        return EF_ERR_INVALID;
    }

    FILE *fp = fopen(path, "r");
    if (!fp) {
        set_error(err, err_len, "could not open config file");
        return EF_ERR_IO;
    }

    ef_config_init(config);
    char line_buffer[512];
    unsigned line_no = 0;
    EfStatus status = EF_OK;

    while (fgets(line_buffer, sizeof(line_buffer), fp)) {
        line_no++;
        char *line = trim(line_buffer);
        if (*line == '\0' || *line == '#') continue;

        char local_err[EF_MAX_ERROR] = {0};
        status = strchr(line, '=') ? parse_assignment(config, line, local_err, sizeof(local_err))
                                   : parse_directive(config, line, local_err, sizeof(local_err));
        if (status != EF_OK) {
            snprintf(err, err_len, "%s:%u: %s", path, line_no, local_err);
            break;
        }
    }

    fclose(fp);
    if (status == EF_OK) {
        status = ef_config_validate(config, err, err_len);
    }
    return status;
}

EfStatus ef_config_validate(const EfConfig *config, char *err, size_t err_len) {
    if (!config) {
        set_error(err, err_len, "config is required");
        return EF_ERR_INVALID;
    }
    if (config->sensor_count == 0) {
        set_error(err, err_len, "at least one sensor is required");
        return EF_ERR_INVALID;
    }
    if (config->queue_capacity == 0 || config->queue_capacity > 4096) {
        set_error(err, err_len, "queue_capacity must be between 1 and 4096");
        return EF_ERR_INVALID;
    }
    for (size_t i = 0; i < config->sensor_count; i++) {
        if (config->sensors[i].reliability < 0.0 || config->sensors[i].reliability > 1.0) {
            set_error(err, err_len, "sensor reliability must be between 0 and 1");
            return EF_ERR_INVALID;
        }
    }
    return EF_OK;
}

