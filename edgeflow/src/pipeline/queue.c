#include "edgeflow.h"

#include <stdlib.h>
#include <string.h>

struct EfQueue {
    EfEvent *items;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t size;
};

EfQueue *ef_queue_create(size_t capacity) {
    if (capacity == 0) return NULL;
    EfQueue *queue = (EfQueue *)calloc(1, sizeof(*queue));
    if (!queue) return NULL;
    queue->items = (EfEvent *)calloc(capacity, sizeof(EfEvent));
    if (!queue->items) {
        free(queue);
        return NULL;
    }
    queue->capacity = capacity;
    return queue;
}

void ef_queue_destroy(EfQueue *queue) {
    if (!queue) return;
    free(queue->items);
    free(queue);
}

EfStatus ef_queue_push(EfQueue *queue, const EfEvent *event) {
    if (!queue || !event) return EF_ERR_INVALID;
    if (queue->size == queue->capacity) return EF_ERR_FULL;

    queue->items[queue->tail] = *event;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->size++;
    return EF_OK;
}

EfStatus ef_queue_pop(EfQueue *queue, EfEvent *event) {
    if (!queue || !event) return EF_ERR_INVALID;
    if (queue->size == 0) return EF_ERR_EMPTY;

    *event = queue->items[queue->head];
    memset(&queue->items[queue->head], 0, sizeof(queue->items[queue->head]));
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size--;
    return EF_OK;
}

size_t ef_queue_size(const EfQueue *queue) {
    return queue ? queue->size : 0;
}

size_t ef_queue_capacity(const EfQueue *queue) {
    return queue ? queue->capacity : 0;
}

