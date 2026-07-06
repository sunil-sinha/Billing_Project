# EdgeFlow Design Notes

EdgeFlow is a compact event-processing system. The code is split by responsibility rather than by object hierarchy, which keeps ownership visible and makes it friendly for C debugging.

## Data Flow

```text
config file
   |
   v
EfConfig -> EfEngine
   |
   v
simulated sensor reading
   |
   v
EfEvent -> transforms -> router -> bounded queue -> storage + publisher
```

## Ownership Rules

- `EfEngine` owns the runtime copy of `EfConfig`.
- `EfEngine` owns the queue returned by `ef_queue_create`.
- `EfEngine` owns the storage `FILE *` after `ef_storage_open`.
- Callers own config loading errors and pass an error buffer into `ef_config_load_file`.
- Events are copied into the queue by value, so producers do not need to keep them alive.

## Important Modules

- `config.c` turns a small line-based config language into `EfConfig`.
- `queue.c` implements a bounded circular queue.
- `transform.c` mutates event values and can reject out-of-range events.
- `router.c` chooses the most specific route threshold for each event kind.
- `engine.c` coordinates ingest, transform, route, queue, drain, storage, and publishing.
- `file_store.c` writes one JSON-like event per line.
- `publisher.c` simulates outbound network publication.

## Extension Ideas

- Add a new sensor type, such as `pressure`.
- Add transform chaining metrics by transform kind.
- Replace the file store with a storage interface and multiple implementations.
- Support config includes or environment-variable interpolation.
- Make the queue optionally overwrite oldest entries instead of rejecting new events.
- Add location-aware route matching.

## Known Design Tradeoffs

- The config parser is simple and easy to inspect, but it does not support quoting.
- The router only matches by sensor kind and value threshold.
- Transform state lives inside `EfConfig`, which is convenient but not thread-safe.
- Storage flushes every event, which is durable but slower than batching.
- The publisher is simulated so the project remains easy to run offline.

