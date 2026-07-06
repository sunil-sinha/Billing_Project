# Kiro Prompt Ideas

Use these prompts against the whole `edgeflow` folder.

## Design Understanding

```text
Explain the architecture of this C project. Describe the major modules, the data flow, and ownership rules for memory and file handles.
```

```text
Create a design diagram for how an EfEvent moves from sensor simulation to transform, routing, queueing, storage, and publishing.
```

## Bug-Finding

```text
Review the queue pressure behavior. Are dropped events counted correctly, and are there situations where the simulation hides backpressure problems?
```

```text
Find edge cases in the config parser. Focus on invalid directives, partial parser state, long values, and error messages.
```

```text
Look for metrics that could become misleading after rejected events, transformed events, or failed storage writes.
```

## Feature Changes

```text
Add a pressure sensor kind with unit "bar", simulation behavior, routing support, config parsing support, and tests.
```

```text
Add a clamp transform that takes minimum and maximum values and clamps the event value instead of rejecting it.
```

```text
Refactor storage into a vtable-based interface with file storage and memory storage implementations.
```

```text
Make route rules optionally match both sensor kind and location, while keeping old configs working.
```

## Refactoring

```text
Separate transform runtime state from EfConfig so the same config can be shared safely across multiple engines.
```

```text
Improve the config parser so it supports quoted values and inline comments.
```

