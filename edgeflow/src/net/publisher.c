#include "edgeflow.h"

#include <stdio.h>
#include <string.h>

EfStatus ef_network_publish(EfEngine *engine, const EfEvent *event) {
    if (!engine || !event) return EF_ERR_INVALID;

    if (strncmp(engine->config.network_endpoint, "memory://", 9) == 0) {
        return EF_OK;
    }
    if (strncmp(engine->config.network_endpoint, "udp://", 6) == 0) {
        ef_log(EF_LEVEL_DEBUG, "publish %llu to %s/%s",
               (unsigned long long)event->id,
               engine->config.network_endpoint,
               event->route);
        return EF_OK;
    }

    snprintf(engine->last_error, sizeof(engine->last_error), "unsupported network endpoint: %s", engine->config.network_endpoint);
    return EF_ERR_INVALID;
}

