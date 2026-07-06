#include "edgeflow.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static double random_unit(void) {
    return (double)rand() / (double)RAND_MAX;
}

static double simulated_base(EfSensorKind kind) {
    switch (kind) {
        case EF_SENSOR_TEMPERATURE: return 25.0 + random_unit() * 70.0;
        case EF_SENSOR_HUMIDITY: return 35.0 + random_unit() * 65.0;
        case EF_SENSOR_VIBRATION: return 20.0 + random_unit() * 90.0;
        case EF_SENSOR_VOLTAGE: return 190.0 + random_unit() * 70.0;
        default: return random_unit() * 100.0;
    }
}

EfStatus ef_engine_init(EfEngine *engine, const EfConfig *config) {
    if (!engine || !config) return EF_ERR_INVALID;
    memset(engine, 0, sizeof(*engine));
    engine->config = *config;
    ef_metrics_init(&engine->metrics);
    engine->queue = ef_queue_create(config->queue_capacity);
    if (!engine->queue) {
        snprintf(engine->last_error, sizeof(engine->last_error), "could not allocate event queue");
        return EF_ERR_NOMEM;
    }
    engine->next_event_id = 1;

    EfStatus status = ef_storage_open(engine);
    if (status != EF_OK) {
        ef_queue_destroy(engine->queue);
        engine->queue = NULL;
        return status;
    }
    return EF_OK;
}

void ef_engine_destroy(EfEngine *engine) {
    if (!engine) return;
    ef_storage_close(engine);
    ef_queue_destroy(engine->queue);
    engine->queue = NULL;
}

EfStatus ef_engine_ingest(EfEngine *engine, const EfSensorDef *sensor, double value, double quality) {
    if (!engine || !sensor) return EF_ERR_INVALID;

    EfEvent event;
    memset(&event, 0, sizeof(event));
    event.id = engine->next_event_id++;
    event.ts = time(NULL);
    snprintf(event.sensor_id, sizeof(event.sensor_id), "%s", sensor->id);
    event.kind = sensor->kind;
    snprintf(event.location, sizeof(event.location), "%s", sensor->location);
    snprintf(event.unit, sizeof(event.unit), "%s", sensor->unit);
    event.value = value;
    event.quality = quality * sensor->reliability;

    EfStatus status = ef_apply_transforms(&engine->config, &event);
    if (status != EF_OK || event.quality < 0.20) {
        engine->metrics.rejected++;
        return EF_ERR_INVALID;
    }

    ef_route_event(&engine->config, &event);
    if (event.flags & 0x01u) {
        engine->metrics.routed_default++;
    }
    if (event.flags & (0x02u | 0x08u)) {
        engine->metrics.transformed++;
    }

    status = ef_queue_push(engine->queue, &event);
    if (status != EF_OK) {
        engine->metrics.dropped++;
        return status;
    }

    engine->metrics.accepted++;
    ef_metrics_observe(&engine->metrics, &event);
    return EF_OK;
}

EfStatus ef_engine_drain(EfEngine *engine) {
    if (!engine || !engine->queue) return EF_ERR_INVALID;

    EfEvent event;
    EfStatus last = EF_OK;
    while (ef_queue_pop(engine->queue, &event) == EF_OK) {
        EfStatus stored = ef_storage_write(engine, &event);
        EfStatus published = ef_network_publish(engine, &event);
        if (stored == EF_OK) engine->metrics.stored++;
        if (published == EF_OK) engine->metrics.published++;
        if (stored != EF_OK) last = stored;
        if (published != EF_OK) last = published;
    }
    return last;
}

EfStatus ef_engine_run_simulation(EfEngine *engine, const EfRunOptions *options) {
    if (!engine || !options) return EF_ERR_INVALID;
    srand(options->seed);

    for (size_t i = 0; i < options->event_count; i++) {
        const EfSensorDef *sensor = &engine->config.sensors[i % engine->config.sensor_count];
        double value = simulated_base(sensor->kind);
        double noise = (random_unit() - 0.5) * 6.0;
        double quality = 0.65 + random_unit() * 0.35;

        if ((i % 53) == 17) {
            quality = 0.08;
        }
        ef_engine_ingest(engine, sensor, value + noise, quality);

        if (ef_queue_size(engine->queue) > ef_queue_capacity(engine->queue) / 2) {
            ef_engine_drain(engine);
        }
    }

    return ef_engine_drain(engine);
}

