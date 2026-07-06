#include "edgeflow.h"

#include <math.h>

static int is_bad_value(double value) {
    return isnan(value) || isinf(value);
}

static EfStatus apply_one(EfTransformRule *rule, EfEvent *event) {
    switch (rule->type) {
        case EF_TRANSFORM_SCALE:
            event->value = event->value * rule->a + rule->b;
            event->flags |= 0x02u;
            return EF_OK;
        case EF_TRANSFORM_THRESHOLD:
            if (event->value < rule->a || event->value > rule->b) {
                event->flags |= 0x04u;
                return EF_ERR_INVALID;
            }
            return EF_OK;
        case EF_TRANSFORM_SMOOTH:
            if (!rule->has_memory) {
                rule->memory = event->value;
                rule->has_memory = 1;
            } else {
                double alpha = rule->a;
                if (alpha < 0.0) alpha = 0.0;
                if (alpha > 1.0) alpha = 1.0;
                rule->memory = alpha * event->value + (1.0 - alpha) * rule->memory;
                event->value = rule->memory;
                event->flags |= 0x08u;
            }
            return EF_OK;
        default:
            return EF_ERR_INVALID;
    }
}

EfStatus ef_apply_transforms(EfConfig *config, EfEvent *event) {
    if (!config || !event) return EF_ERR_INVALID;
    if (is_bad_value(event->value) || is_bad_value(event->quality)) {
        event->flags |= 0x10u;
        return EF_ERR_INVALID;
    }

    for (size_t i = 0; i < config->transform_count; i++) {
        EfTransformRule *rule = &config->transforms[i];
        if (rule->kind != event->kind) {
            continue;
        }
        EfStatus status = apply_one(rule, event);
        if (status != EF_OK) {
            return status;
        }
    }
    return EF_OK;
}

