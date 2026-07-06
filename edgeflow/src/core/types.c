#include "edgeflow.h"

#include <ctype.h>
#include <string.h>

static int equals_ignore_case(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

const char *ef_status_name(EfStatus status) {
    switch (status) {
        case EF_OK: return "ok";
        case EF_ERR_INVALID: return "invalid";
        case EF_ERR_NOMEM: return "out-of-memory";
        case EF_ERR_IO: return "io-error";
        case EF_ERR_FULL: return "full";
        case EF_ERR_EMPTY: return "empty";
        case EF_ERR_NOT_FOUND: return "not-found";
        case EF_ERR_PARSE: return "parse-error";
        default: return "unknown";
    }
}

const char *ef_sensor_kind_name(EfSensorKind kind) {
    switch (kind) {
        case EF_SENSOR_TEMPERATURE: return "temperature";
        case EF_SENSOR_HUMIDITY: return "humidity";
        case EF_SENSOR_VIBRATION: return "vibration";
        case EF_SENSOR_VOLTAGE: return "voltage";
        default: return "unknown";
    }
}

EfSensorKind ef_sensor_kind_parse(const char *text) {
    if (!text) return EF_SENSOR_UNKNOWN;
    if (equals_ignore_case(text, "temperature")) return EF_SENSOR_TEMPERATURE;
    if (equals_ignore_case(text, "humidity")) return EF_SENSOR_HUMIDITY;
    if (equals_ignore_case(text, "vibration")) return EF_SENSOR_VIBRATION;
    if (equals_ignore_case(text, "voltage")) return EF_SENSOR_VOLTAGE;
    return EF_SENSOR_UNKNOWN;
}

const char *ef_transform_kind_name(EfTransformKind kind) {
    switch (kind) {
        case EF_TRANSFORM_SCALE: return "scale";
        case EF_TRANSFORM_THRESHOLD: return "threshold";
        case EF_TRANSFORM_SMOOTH: return "smooth";
        default: return "unknown";
    }
}

EfTransformKind ef_transform_kind_parse(const char *text) {
    if (!text) return EF_TRANSFORM_UNKNOWN;
    if (equals_ignore_case(text, "scale")) return EF_TRANSFORM_SCALE;
    if (equals_ignore_case(text, "threshold")) return EF_TRANSFORM_THRESHOLD;
    if (equals_ignore_case(text, "smooth")) return EF_TRANSFORM_SMOOTH;
    return EF_TRANSFORM_UNKNOWN;
}

