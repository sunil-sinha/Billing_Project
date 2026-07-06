# Fifteen Columns Project

This is a small C project created for code-query and modification practice.

## What It Does

1. `save_multiplied_inputs(...)`
   - Accepts 15 input values.
   - Multiplies them by `1, 2, 3, ... 15` respectively.
   - Saves one row with 15 columns into a text file.

2. `copy_selected_columns(...)`
   - Reads the generated text file.
   - Extracts column numbers `1, 3, 5, 7`.
   - Writes those four columns into another text file.

Column numbers are human-friendly and start at `1`.

## Build

With GCC or Clang:

```sh
make
```

With CMake:

```sh
cmake -S . -B build
cmake --build build
```

## Run

```sh
./build/fifteen_columns 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150
```

On Windows after CMake build:

```sh
build\Debug\fifteen_columns.exe 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150
```

The program writes:

- `data/multiplied_values.txt`
- `data/selected_columns.txt`

## Example

Input:

```text
10 20 30 40 50 60 70 80 90 100 110 120 130 140 150
```

First output row:

```text
10 40 90 160 250 360 490 640 810 1000 1210 1440 1690 1960 2250
```

Selected column output:

```text
10 90 250 490
```

