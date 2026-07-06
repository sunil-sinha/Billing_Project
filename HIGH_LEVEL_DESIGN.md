# High Level Design — Sixteen Columns

**Document Type:** High Level Design (HLD)  
**Audience:** Senior Leaders, Product Owners, Business Stakeholders  
**Date:** July 6, 2026

---

## The Story — What This System Does

Imagine you are running a production line that takes in sixteen raw measurements. You want two things: a complete record of all transformed measurements, and a concise summary that your downstream partner system reads every single time.

That is exactly what this tool does.

You hand it sixteen numbers on the command line. It runs two back-to-back operations — first it transforms all sixteen values and saves the full picture, then it reads that picture and extracts only the four values your partner system cares about. The partner system never needs to change. It always gets the same four values in the same format, no matter what happens upstream.

Think of it as a two-stage pipeline:

```
You provide 16 numbers
        │
        ▼
┌─────────────────────────────────────────┐
│  STAGE 1 — Transform & Record           │
│                                         │
│  Multiply each number by its position.  │
│  Save all 16 results to File 1.         │
└──────────────────┬──────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────┐
│  STAGE 2 — Select & Deliver             │
│                                         │
│  Read File 1.                           │
│  Pick 4 specific values.                │
│  Write them to File 2.                  │
└──────────────────┬──────────────────────┘
                   │
                   ▼
        Downstream system reads File 2
        (always the same 4 values)
```

---

## Overall Functionality

### What Goes In

The user runs the program from the command line and provides exactly **16 integer numbers** as arguments.

```
fifteen_columns  9  8  7  6  NEW  4  3  2  1  10  11  12  13  14  15  16
```

Each number is validated before any processing begins. If a value is missing, non-numeric, or the wrong count is provided, the program stops immediately with a clear error message.

### What Comes Out

**File 1 — `data/multiplied_values.txt`**  
A single line with all 16 transformed values. Each input is scaled by its position in the list (1st number × 1, 2nd × 2, … 16th × 16). This is the full audit trail.

**File 2 — `data/selected_columns.txt`**  
A single line with exactly 4 values, extracted from File 1 at fixed positions. This is what the downstream system reads. Its content is stable and predictable regardless of changes to other inputs.

### Example

Input: `9  8  7  6  5  4  3  2  1  10  11  12  13  14  15  16`

File 1 output:
```
9 16 21 24 25 24 21 16 9 100 121 144 169 196 225 256
```

File 2 output:
```
9 21 25 21
```

---

## Function-by-Function Story

### 1. `parse_long` — The Gatekeeper

Before anything else happens, every number the user typed on the command line goes through this check. It makes sure each argument is a real, valid integer — no letters, no decimal points, no values too large or too small for the system to handle.

If even one argument fails this check, the program stops right there and tells the user exactly which argument was the problem. Nothing is written to disk until all 16 inputs are confirmed clean.

> **In plain terms:** This is the bouncer at the door. No bad data gets in.

---

### 2. `save_multiplied_inputs` — The Calculator and Recorder

Once all 16 numbers are validated, this function takes over. It multiplies each number by its own position in the list:

- The 1st number is multiplied by 1
- The 2nd number is multiplied by 2
- The 3rd number is multiplied by 3
- … and so on up to the 16th number being multiplied by 16

All 16 results are written as a single row into **File 1** (`multiplied_values.txt`), separated by spaces.

This file is the master record. It captures everything — all 16 transformed values — so nothing is lost and the full picture is always available for auditing or debugging.

> **In plain terms:** This is the worker who does the math and writes the full report.

---

### 3. `copy_selected_columns` — The Selector and Deliverer

Once File 1 is written, this function reads it back and picks out exactly four values — the ones at positions **1, 3, 5, and 7** (counting from 1). These four values are written into **File 2** (`selected_columns.txt`).

This is where the downstream system protection lives. Even though we added a new input at position 5, the selection logic was adjusted to still pick the same logical values that the downstream system has always expected. The downstream system reads File 2 and sees no difference whatsoever.

> **In plain terms:** This is the editor who highlights only the key numbers from the full report and hands them to the client. The client always gets the same four numbers they asked for.

---

### 4. `fc_status_message` — The Translator

Every operation in this system can succeed or fail. When something goes wrong — a file cannot be opened, a value cannot be read, a write fails — the system has a code for what happened. This function translates that internal error code into a plain English sentence so the user understands exactly what went wrong.

> **In plain terms:** This is the error message service. It turns machine codes into human-readable explanations.

---

## How the Pieces Connect

```
Command Line
    │
    │  16 integer arguments
    ▼
parse_long()  ──── invalid? ──── print error, exit
    │
    │  16 validated long integers
    ▼
save_multiplied_inputs()
    │  multiplies each by its position (×1 … ×16)
    │  writes 16 values to  →  data/multiplied_values.txt
    │
    ▼
copy_selected_columns()
    │  reads all 16 values from multiplied_values.txt
    │  picks positions {1, 3, 5, 7}  →  data/selected_columns.txt
    │
    ▼
Downstream system reads selected_columns.txt
(always 4 values, always in the same format)
```

At any point, if `save_multiplied_inputs` or `copy_selected_columns` encounters a problem (disk full, missing directory, corrupted file), `fc_status_message` produces a readable error and the program exits cleanly without leaving partial output.

---

## Key Design Decisions

| Decision | Reason |
|---|---|
| New argument inserted at position 5 | Business requirement to capture an additional measurement |
| File 2 selection adjusted from `{1,3,5,7}` to compensate | Downstream system must not be impacted — it always reads the same logical values |
| Input count and selected count defined as constants | One place to change if the counts ever need updating again |
| All processing stops on first validation error | Prevents bad data from silently flowing into output files |
| File 1 always written before File 2 | File 2 depends on File 1 — the sequence is fixed and enforced |

---

## Success Criteria

The system is working correctly when:

1. Providing 16 valid integers produces both output files with no errors.
2. Every value in File 1 equals its original input multiplied by its 1-based position.
3. File 2 contains exactly the values from positions 1, 3, 5, and 7 of File 1.
4. The downstream system reading File 2 sees no change in output after the new argument was added.
5. Any bad input, missing file, or write failure produces a clear error message and exits without partial output.
