#ifndef EDGEFLOW_H
#define EDGEFLOW_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EF_NAME_MAX 64
#define EF_KIND_MAX 32
#define EF_LOCATION_MAX 48
#define EF_UNIT_MAX 24
#define EF_PATH_MAX 260
#define EF_MAX_SENSORS 64
#define EF_MAX_ROUTES 64
#define EF_MAX_TRANSFORMS 64
#define EF_MAX_ERROR 160

typedef enum {
    EF_OK = 0,
    EF_ERR_INVALID = -1,
    EF_ERR_NOMEM = -2,
    EF_ERR_IO = -3,
    EF_ERR_FULL = -4,
    EF_ERR_EMPTY = -5,
    EF_ERR_NOT_FOUND = -6,
    EF_ERR_PARSE = -7
} EfStatus;

typedef enum {
    EF_LEVEL_DEBUG,
    EF_LEVEL_INFO,
    EF_LEVEL_WARN,
    EF_LEVEL_ERROR
} EfLogLevel;

typedef enum {
    EF_SENSOR_TEMPERATURE,
    EF_SENSOR_HUMIDITY,
    EF_SENSOR_VIBRATION,
    EF_SENSOR_VOLTAGE,
    EF_SENSOR_UNKNOWN
} EfSensorKind;

typedef enum {
    EF_TRANSFORM_SCALE,
    EF_TRANSFORM_THRESHOLD,
    EF_TRANSFORM_SMOOTH,
    EF_TRANSFORM_UNKNOWN
} EfTransformKind;

typedef struct {
    char id[EF_NAME_MAX];
    EfSensorKind kind;
    char location[EF_LOCATION_MAX];
    char unit[EF_UNIT_MAX];
    double reliability;
} EfSensorDef;

typedef struct {
    EfSensorKind kind;
    char destination[EF_NAME_MAX];
    double minimum_value;
} EfRouteRule;

typedef struct {
    EfSensorKind kind;
    EfTransformKind type;
    double a;
    double b;
    double memory;
    int has_memory;
} EfTransformRule;

typedef struct {
    char pipeline_name[EF_NAME_MAX];
    size_t queue_capacity;
    char storage_path[EF_PATH_MAX];
    char network_endpoint[EF_PATH_MAX];
    EfSensorDef sensors[EF_MAX_SENSORS];
    size_t sensor_count;
    EfRouteRule routes[EF_MAX_ROUTES];
    size_t route_count;
    EfTransformRule transforms[EF_MAX_TRANSFORMS];
    size_t transform_count;
} EfConfig;

typedef struct {
    uint64_t id;
    time_t ts;
    char sensor_id[EF_NAME_MAX];
    EfSensorKind kind;
    char location[EF_LOCATION_MAX];
    char unit[EF_UNIT_MAX];
    double value;
    double quality;
    char route[EF_NAME_MAX];
    unsigned flags;
} EfEvent;

typedef struct {
    uint64_t accepted;
    uint64_t rejected;
    uint64_t dropped;
    uint64_t transformed;
    uint64_t published;
    uint64_t stored;
    uint64_t routed_default;
    double min_value;
    double max_value;
    double sum_value;
} EfMetrics;

typedef struct EfQueue EfQueue;

typedef struct {
    EfConfig config;
    EfQueue *queue;
    EfMetrics metrics;
    FILE *storage;
    uint64_t next_event_id;
    char last_error[EF_MAX_ERROR];
} EfEngine;

typedef struct {
    unsigned seed;
    size_t event_count;
    int verbose;
} EfRunOptions;

const char *ef_status_name(EfStatus status);
const char *ef_sensor_kind_name(EfSensorKind kind);
EfSensorKind ef_sensor_kind_parse(const char *text);
const char *ef_transform_kind_name(EfTransformKind kind);
EfTransformKind ef_transform_kind_parse(const char *text);

void ef_log_set_level(EfLogLevel level);
void ef_log(EfLogLevel level, const char *fmt, ...);

void ef_config_init(EfConfig *config);
EfStatus ef_config_load_file(EfConfig *config, const char *path, char *err, size_t err_len);
EfStatus ef_config_validate(const EfConfig *config, char *err, size_t err_len);
const EfSensorDef *ef_config_find_sensor(const EfConfig *config, const char *id);

void ef_metrics_init(EfMetrics *metrics);
void ef_metrics_observe(EfMetrics *metrics, const EfEvent *event);
void ef_metrics_print(const EfMetrics *metrics, FILE *out);

EfQueue *ef_queue_create(size_t capacity);
void ef_queue_destroy(EfQueue *queue);
EfStatus ef_queue_push(EfQueue *queue, const EfEvent *event);
EfStatus ef_queue_pop(EfQueue *queue, EfEvent *event);
size_t ef_queue_size(const EfQueue *queue);
size_t ef_queue_capacity(const EfQueue *queue);

EfStatus ef_apply_transforms(EfConfig *config, EfEvent *event);
EfStatus ef_route_event(const EfConfig *config, EfEvent *event);

EfStatus ef_storage_open(EfEngine *engine);
void ef_storage_close(EfEngine *engine);
EfStatus ef_storage_write(EfEngine *engine, const EfEvent *event);

EfStatus ef_network_publish(EfEngine *engine, const EfEvent *event);

EfStatus ef_engine_init(EfEngine *engine, const EfConfig *config);
void ef_engine_destroy(EfEngine *engine);
EfStatus ef_engine_ingest(EfEngine *engine, const EfSensorDef *sensor, double value, double quality);
EfStatus ef_engine_drain(EfEngine *engine);
EfStatus ef_engine_run_simulation(EfEngine *engine, const EfRunOptions *options);

#ifdef __cplusplus
}
#endif

#endif

