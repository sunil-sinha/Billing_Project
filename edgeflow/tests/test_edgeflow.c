#include "edgeflow.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

static void check_int(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

static void test_queue_order(void) {
    EfQueue *queue = ef_queue_create(2);
    EfEvent a = {.id = 1, .value = 10.0};
    EfEvent b = {.id = 2, .value = 20.0};
    EfEvent out;

    check_int(queue != NULL, "queue allocates");
    check_int(ef_queue_push(queue, &a) == EF_OK, "push a");
    check_int(ef_queue_push(queue, &b) == EF_OK, "push b");
    check_int(ef_queue_push(queue, &b) == EF_ERR_FULL, "queue reports full");
    check_int(ef_queue_pop(queue, &out) == EF_OK && out.id == 1, "pop preserves order 1");
    check_int(ef_queue_pop(queue, &out) == EF_OK && out.id == 2, "pop preserves order 2");
    check_int(ef_queue_pop(queue, &out) == EF_ERR_EMPTY, "queue reports empty");
    ef_queue_destroy(queue);
}

static EfConfig sample_config(void) {
    EfConfig config;
    ef_config_init(&config);
    config.sensor_count = 1;
    snprintf(config.sensors[0].id, sizeof(config.sensors[0].id), "s1");
    config.sensors[0].kind = EF_SENSOR_TEMPERATURE;
    snprintf(config.sensors[0].location, sizeof(config.sensors[0].location), "lab");
    snprintf(config.sensors[0].unit, sizeof(config.sensors[0].unit), "c");
    config.sensors[0].reliability = 1.0;
    config.route_count = 1;
    config.routes[0].kind = EF_SENSOR_TEMPERATURE;
    snprintf(config.routes[0].destination, sizeof(config.routes[0].destination), "hot");
    config.routes[0].minimum_value = 50.0;
    return config;
}

static void test_routing(void) {
    EfConfig config = sample_config();
    EfEvent event;
    memset(&event, 0, sizeof(event));
    event.kind = EF_SENSOR_TEMPERATURE;
    event.value = 55.0;

    check_int(ef_route_event(&config, &event) == EF_OK, "route ok");
    check_int(strcmp(event.route, "hot") == 0, "route threshold match");

    memset(&event, 0, sizeof(event));
    event.kind = EF_SENSOR_TEMPERATURE;
    event.value = 20.0;
    ef_route_event(&config, &event);
    check_int(strcmp(event.route, "default-temperature") == 0, "default route");
    check_int((event.flags & 0x01u) != 0, "default route flag");
}

static void test_engine_counts(void) {
    EfConfig config = sample_config();
    snprintf(config.storage_path, sizeof(config.storage_path), "test-edgeflow.log");
    snprintf(config.network_endpoint, sizeof(config.network_endpoint), "memory://tests");
    EfEngine engine;
    check_int(ef_engine_init(&engine, &config) == EF_OK, "engine init");
    check_int(ef_engine_ingest(&engine, &config.sensors[0], 60.0, 1.0) == EF_OK, "ingest accepted");
    check_int(ef_engine_ingest(&engine, &config.sensors[0], 60.0, 0.1) == EF_ERR_INVALID, "ingest rejects low quality");
    check_int(ef_engine_drain(&engine) == EF_OK, "drain");
    check_int(engine.metrics.accepted == 1, "accepted count");
    check_int(engine.metrics.rejected == 1, "rejected count");
    check_int(engine.metrics.stored == 1, "stored count");
    ef_engine_destroy(&engine);
    remove("test-edgeflow.log");
}

int main(void) {
    test_queue_order();
    test_routing();
    test_engine_counts();
    if (failures) {
        fprintf(stderr, "%d test(s) failed\n", failures);
        return 1;
    }
    puts("all tests passed");
    return 0;
}

