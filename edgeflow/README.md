# EdgeFlow C

EdgeFlow is a deliberately non-trivial C project for experimenting with code understanding, design reviews, bug fixes, and feature changes in AI coding tools.

It models a small telemetry pipeline:

- sensors emit readings
- events enter a bounded queue
- routing rules choose a destination stream
- transforms normalize or reject bad readings
- storage persists accepted events
- metrics summarize pipeline behavior
- the CLI runs deterministic simulations from a config file

## Build

```sh
make
```

## Run

```sh
./build/edgeflow --config examples/basic.edge --events 250 --seed 42
```

On Windows with a MinGW-style toolchain:

```sh
mingw32-make
build\edgeflow.exe --config examples\basic.edge --events 250 --seed 42
```

## Project Map

- `include/edgeflow.h` - public API and shared types
- `src/core` - allocator, logging, config, metrics, and utility code
- `src/pipeline` - queue, router, transforms, and engine
- `src/storage` - in-memory and file-backed event stores
- `src/net` - simulated network publisher
- `src/cli` - command-line app
- `tests` - focused tests for parsing, routing, and pipeline behavior

## Good Kiro Queries To Try

- "Explain the design of this C project and draw the data flow."
- "Find a bug related to queue pressure or dropped events."
- "Add a new transform that clamps humidity between 0 and 100."
- "Refactor routing rules so they can match by sensor location."
- "Make storage pluggable through a vtable interface."
- "Add tests for malformed config files."

The code is intentionally written in a realistic, slightly old-school C style: explicit ownership, manual error handling, fixed-size buffers, and modules that are easy to inspect but large enough to have interesting behavior.

