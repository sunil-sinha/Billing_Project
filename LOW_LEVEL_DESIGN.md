# Low Level Design — Fifteen Columns

**Document Type:** Low Level Design (LLD)  
**Audience:** Software Developers, Code Reviewers  
**Date:** July 6, 2026

---

## 1. Project Structure

```
fifteen_columns/
├── include/
│   └── fifteen_columns.h     # Public API declarations and type definitions
├── src/
│   ├── main.c                # Entry point — argument parsing and orchestration
│   └── procedures.c          # Core logic — multiply, select, file I/O
├── data/
│   ├── multiplied_values.txt       # Runtime output (generated on each run)
│   └── selected_columns.txt        # Runtime output (generated on each run)
├── CMakeLists.txt            # CMake build definition
├── Makefile                  # GNU Make alternative
└── .gitignore
```

---

## 2. Build System

### CMake (preferred)
```sh
cmake -S . -B build
cmake --build build
# Binary: build/fifteen_columns  (or build\Debug\fifteen_columns.exe on Windows)
```

### GNU Make
```sh
make
# Binary: build/fifteen_columns
```

Compiler requirements: GCC or Clang with C99 or later.

---

## 3. Constants and Types (`fifteen_columns.h`)

### Preprocessor Constants

| Constant | Value | Meaning |
|---|---|---|
| `FC_INPUT_COUNT` | `15` | Total number of input values expected |
| `FC_SELECTED_COUNT` | `4` | Number of columns written to the selection output |

### `FcStatus` Enum

Return type used by all public functions.

| Enumerator | Value | Meaning |
|---|---|---|
| `FC_OK` | `0` | Operation completed successfully |
| `FC_ERR_INVALID_ARGUMENT` | `1` | A required pointer argument was `NULL` |
| `FC_ERR_FILE_OPEN` | `2` | `fopen()` returned `NULL` |
| `FC_ERR_FILE_READ` | `3` | `fscanf()` could not read expected columns |
| `FC_ERR_FILE_WRITE` | `4` | `fprintf()` or `fclose()` returned an error |

### Public Function Signatures

```c
const char *fc_status_message(FcStatus status);

FcStatus save_multiplied_inputs(
    const char *output_file,
    long input1, long input2, long input3, long input4, long input5,
    long input6, long input7, long input8, long input9, long input10,
    long input11, long input12, long input13, long input14, long input15
);

FcStatus copy_selected_columns(
    const char *input_file,
    const char *output_file
);
```

---

## 4. Module: `main.c`

### Responsibilities
- Parse and validate command-line arguments.
- Convert string arguments to `long` integers.
- Call `save_multiplied_inputs()` and `copy_selected_columns()` in sequence.
- Print success messages or error diagnostics to stdout/stderr.
- Return appropriate exit codes.

### Exit Codes

| Code | Condition |
|---|---|
| `0` | Both operations completed successfully |
| `1` | A library function returned a non-OK `FcStatus` |
| `2` | Wrong number of arguments or a non-numeric argument was supplied |

### Static Helper: `parse_long()`

```c
static int parse_long(const char *text, long *value);
```

- Uses `strtol()` with base 10.
- Sets `errno = 0` before calling `strtol` to detect range errors.
- Returns `0` (failure) if:
  - `errno != 0` (overflow/underflow)
  - `end == text` (no digits consumed)
  - `*end != '\0'` (trailing non-numeric characters)
- Returns `1` (success) and writes the parsed value into `*value`.

### Control Flow

```
main(argc, argv)
  │
  ├── argc != 16  →  print_usage()  →  exit(2)
  │
  ├── for i in 0..14:
  │     parse_long(argv[i+1], &input[i])
  │     failure  →  fprintf(stderr, ...)  →  exit(2)
  │
  ├── save_multiplied_inputs(MULTIPLIED_OUTPUT_FILE, input[0..14])
  │     failure  →  fprintf(stderr, fc_status_message(...))  →  exit(1)
  │
  ├── copy_selected_columns(MULTIPLIED_OUTPUT_FILE, SELECTED_OUTPUT_FILE)
  │     failure  →  fprintf(stderr, fc_status_message(...))  →  exit(1)
  │
  └── printf("Created ...")  →  exit(0)
```

### Hardcoded File Paths (static constants in `main.c`)

```c
static const char *MULTIPLIED_OUTPUT_FILE = "data/multiplied_values.txt";
static const char *SELECTED_OUTPUT_FILE   = "data/selected_columns.txt";
```

These paths are relative to the working directory at runtime. The `data/` directory must exist before the program runs.

---

## 5. Module: `procedures.c`

### 5.1 `fc_status_message()`

```c
const char *fc_status_message(FcStatus status);
```

A simple `switch` over `FcStatus` returning a string literal. The `default` branch returns `"unknown error"` for any unmapped value.

---

### 5.2 `save_multiplied_inputs()`

**Purpose:** Multiply each of the 15 inputs by its 1-based index and write all 15 results as a space-separated row to a file.

**Parameters:**
- `output_file` — path to the file to create/overwrite (must not be `NULL`)
- `input1` through `input15` — the 15 `long` values

**Algorithm:**

```
1. Guard: return FC_ERR_INVALID_ARGUMENT if output_file == NULL

2. Pack the 15 named parameters into:
     long values[15] = { input1, input2, ..., input15 }

3. fopen(output_file, "w")
     → NULL: return FC_ERR_FILE_OPEN

4. for i = 0 to 14:
     multiplied_value = values[i] * (long)(i + 1)
     fprintf(file, "%ld%s", multiplied_value,
             i == 14 ? "\n" : " ")
     → fprintf < 0: fclose + return FC_ERR_FILE_WRITE

5. fclose(file)
     → non-zero: return FC_ERR_FILE_WRITE

6. return FC_OK
```

**Output format example** (input `10 20 30 ... 150`):
```
10 40 90 160 250 360 490 640 810 1000 1210 1440 1690 1960 2250
```

One line, values separated by single spaces, terminated with `\n`.

---

### 5.3 `copy_selected_columns()`

**Purpose:** Read all 15 columns from the multiplied-values file and write only the values at 0-based indices `{0, 2, 4, 6}` (columns 1, 3, 5, 7 in human terms) to a new file.

**Parameters:**
- `input_file` — path to `multiplied_values.txt` (must not be `NULL`)
- `output_file` — path to the selection output file (must not be `NULL`)

**Algorithm:**

```
1. Guard: return FC_ERR_INVALID_ARGUMENT if either pointer is NULL

2. fopen(input_file, "r")
     → NULL: return FC_ERR_FILE_OPEN

3. for i = 0 to 14:
     fscanf(source, "%ld", &columns[i])
     → != 1: fclose + return FC_ERR_FILE_READ

4. fclose(source)

5. fopen(output_file, "w")
     → NULL: return FC_ERR_FILE_OPEN

6. selected_indexes[4] = { 0, 2, 4, 6 }

7. for i = 0 to 3:
     fprintf(destination, "%ld%s",
             columns[selected_indexes[i]],
             i == 3 ? "\n" : " ")
     → fprintf < 0: fclose + return FC_ERR_FILE_WRITE

8. fclose(destination)
     → non-zero: return FC_ERR_FILE_WRITE

9. return FC_OK
```

**Output format example:**
```
10 90 250 490
```

One line, four values separated by single spaces, terminated with `\n`.

---

## 6. Data Flow

```
CLI Arguments (15 strings)
        │
        │  parse_long() × 15
        ▼
long input[15]
        │
        │  save_multiplied_inputs()
        │  values[i] = input[i] × (i + 1)
        ▼
data/multiplied_values.txt   ←── 15 long integers, space-separated, one line
        │
        │  copy_selected_columns()
        │  fscanf() reads all 15, picks indices {0,2,4,6}
        ▼
data/selected_columns.txt    ←── 4 long integers, space-separated, one line
```

---

## 7. Error Handling Strategy

- All functions return `FcStatus`; callers check the return value before proceeding.
- On failure inside a function that has opened a file, `fclose()` is called before returning to avoid resource leaks.
- `fprintf()` return values are checked; a negative return indicates a write failure.
- `fclose()` return values are checked; a non-zero return on write mode indicates buffered data was not flushed.
- `strtol()` is used instead of `atoi()` to safely detect invalid input and integer overflow.

---

## 8. Memory Management

This project uses no dynamic memory allocation (`malloc`/`free`). All storage is:
- Stack-allocated arrays (`long input[15]`, `long values[15]`, `long columns[15]`).
- String literals for error messages and file paths.

There are no heap allocations and therefore no risk of memory leaks in normal operation.

---

## 9. Concurrency and Re-entrancy

- The program is single-threaded with no shared global mutable state.
- `fc_status_message()` returns string literals (read-only); it is safe to call from multiple contexts.
- The output file paths are `static const` in `main.c` — they are not writable.

---

## 10. Known Limitations / Future Improvement Areas

| Area | Current Behavior | Possible Improvement |
|---|---|---|
| Input count | Fixed at 15 | Accept variable count via config or flag |
| Selected columns | Hardcoded as `{0,2,4,6}` | Accept column indices as arguments |
| Output paths | Hardcoded static strings | Accept paths as CLI arguments |
| Row count | Single row per run | Loop over input file for batch processing |
| Data type | `long` only | Support floating-point inputs |
| File mode | Always overwrites output | Add append mode option |
