#include "edgeflow.h"

#include <float.h>
#include <stdio.h>

void ef_metrics_init(EfMetrics *metrics) {
    if (!metrics) return;
    metrics->accepted = 0;
    metrics->rejected = 0;
    metrics->dropped = 0;
    metrics->transformed = 0;
    metrics->published = 0;
    metrics->stored = 0;
    metrics->routed_default = 0;
    metrics->min_value = DBL_MAX;
    metrics->max_value = -DBL_MAX;
    metrics->sum_value = 0.0;
}

void ef_metrics_observe(EfMetrics *metrics, const EfEvent *event) {
    if (!metrics || !event) return;
    if (event->value < metrics->min_value) metrics->min_value = event->value;
    if (event->value > metrics->max_value) metrics->max_value = event->value;
    metrics->sum_value += event->value;
}

void ef_metrics_print(const EfMetrics *metrics, FILE *out) {
    uint64_t total = metrics->accepted + metrics->rejected + metrics->dropped;
    double avg = metrics->accepted ? metrics->sum_value / (double)metrics->accepted : 0.0;
    double min_value = metrics->accepted ? metrics->min_value : 0.0;
    double max_value = metrics->accepted ? metrics->max_value : 0.0;

    fprintf(out, "EdgeFlow metrics\n");
    fprintf(out, "  total input:      %llu\n", (unsigned long long)total);
    fprintf(out, "  accepted:         %llu\n", (unsigned long long)metrics->accepted);
    fprintf(out, "  rejected:         %llu\n", (unsigned long long)metrics->rejected);
    fprintf(out, "  dropped:          %llu\n", (unsigned long long)metrics->dropped);
    fprintf(out, "  transformed:      %llu\n", (unsigned long long)metrics->transformed);
    fprintf(out, "  published:        %llu\n", (unsigned long long)metrics->published);
    fprintf(out, "  stored:           %llu\n", (unsigned long long)metrics->stored);
    fprintf(out, "  default routed:   %llu\n", (unsigned long long)metrics->routed_default);
    fprintf(out, "  value min/avg/max %.2f / %.2f / %.2f\n", min_value, avg, max_value);
}

