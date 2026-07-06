#include "edgeflow.h"

#include <stdio.h>
#include <string.h>

EfStatus ef_route_event(const EfConfig *config, EfEvent *event) {
    if (!config || !event) return EF_ERR_INVALID;

    const EfRouteRule *best = NULL;
    for (size_t i = 0; i < config->route_count; i++) {
        const EfRouteRule *rule = &config->routes[i];
        if (rule->kind == event->kind && event->value >= rule->minimum_value) {
            if (!best || rule->minimum_value > best->minimum_value) {
                best = rule;
            }
        }
    }

    if (best) {
        snprintf(event->route, sizeof(event->route), "%s", best->destination);
    } else {
        snprintf(event->route, sizeof(event->route), "default-%s", ef_sensor_kind_name(event->kind));
        event->flags |= 0x01u;
    }

    return EF_OK;
}
