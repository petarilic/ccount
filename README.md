# ccount - Lines of Code Counter

A C program that counts lines of code in source files, providing detailed LOC statistics.

## Features

- Strips single-line (`//`) and multi-line (`/* */`) comments before counting
- Counts lines excluding blank lines
- Reports:
  - Total LOC (excluding comments and empty lines)
  - Total characters (including whitespace, excluding comments)
  - Number of `#include` statements
  - Number of global variable lines
  - Per-function LOC breakdown

## Usage

```bash
ccount <source_file>
```

## Example

```bash
ccount main.c
```

Sample output:
```
=== LOC REPORT ===
Total LOC (excl. comments/empty lines): 123
Total chars (incl. whitespace, excl. comments/empty): 4567

Includes: 4
Global variables: 2

Functions:
  main: 45 lines
  is_function_start: 28 lines
```

## Compilation

```bash
gcc -o ccount main.c
```

Or use the provided batch file:
```bash
compile.bat
```

## How It Works

1. Reads the source file line by line
2. Strips comments from each line
3. Skips blank lines
4. Tracks brace depth to detect function boundaries
5. Counts include statements and global variable lines
6. Reports per-function LOC

## Limitations

- Naive function detection (looks for patterns like `name(...) {`)
- May not detect all function types (e.g., inline functions, lambdas)
- Global variable detection is approximate
