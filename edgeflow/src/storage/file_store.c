#include "edgeflow.h"

#include <stdio.h>

EfStatus ef_storage_open(EfEngine *engine) {
    if (!engine) return EF_ERR_INVALID;
    engine->storage = fopen(engine->config.storage_path, "a");
    if (!engine->storage) {
        snprintf(engine->last_error, sizeof(engine->last_error), "could not open storage path: %s", engine->config.storage_path);
        return EF_ERR_IO;
    }
    return EF_OK;
}

void ef_storage_close(EfEngine *engine) {
    if (!engine || !engine->storage) return;
    fclose(engine->storage);
    engine->storage = NULL;
}

EfStatus ef_storage_write(EfEngine *engine, const EfEvent *event) {
    if (!engine || !engine->storage || !event) return EF_ERR_INVALID;

    int written = fprintf(
        engine->storage,
        "{\"id\":%llu,\"ts\":%lld,\"sensor\":\"%s\",\"kind\":\"%s\",\"location\":\"%s\","
        "\"value\":%.3f,\"unit\":\"%s\",\"quality\":%.3f,\"route\":\"%s\",\"flags\":%u}\n",
        (unsigned long long)event->id,
        (long long)event->ts,
        event->sensor_id,
        ef_sensor_kind_name(event->kind),
        event->location,
        event->value,
        event->unit,
        event->quality,
        event->route,
        event->flags
    );

    if (written < 0 || fflush(engine->storage) != 0) {
        snprintf(engine->last_error, sizeof(engine->last_error), "failed writing event to storage");
        return EF_ERR_IO;
    }
    return EF_OK;
}

