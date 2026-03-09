# csv_stream

Streaming CSV reader for C++.

`csv_stream` provides a small deterministic toolkit for reading
CSV files row by row without loading the entire file into memory.

Header-only. No dependencies.

## Download

https://vixcpp.com/registry/pkg/gk/csv_stream

## Why csv_stream?

Many applications need to process CSV files efficiently,
especially when working with large datasets.

Common examples include:

- data import pipelines
- analytics processing
- log processing
- ETL jobs
- reporting systems
- batch data processing

Loading a full CSV file into memory can be expensive
or even impossible when files are very large.

`csv_stream` solves this by reading CSV rows sequentially.

Example CSV:
```text
name,age,city
alice,30,kampala
bob,28,nairobi
```

Rows can be processed one by one without storing
the entire file in memory.

This makes it suitable for very large CSV files.

---

# Features

- Streaming CSV reader
- Row-by-row parsing
- Optional header support
- Custom delimiter support
- Quoted field parsing
- Escaped quote support
- Strict column validation
- Header-only implementation
- No external dependencies

Supported syntax examples:
```text
alice,30,kampala
"alice, bob",42,nairobi
"hello ""world""",example
```

---

# Installation

## Using Vix Registry

```bash
vix add @gk/csv_stream
vix deps
```

Manual
```bash
git clone https://github.com/Gaspardkirira/csv_stream.git
```

Add the include/ directory to your project.

## Dependency

- Requires **C++17** or newer.
- No external libraries required.

## Quick examples

### Basic streaming
```cpp
#include <csv_stream/csv_stream.hpp>

int main()
{
    csv_stream::Reader reader("data.csv");

    csv_stream::Row row;

    while (reader.read_row(row))
    {
        // process row
    }
}
```

### Reading CSV with header
```cpp
#include <csv_stream/csv_stream.hpp>

int main()
{
    csv_stream::Options opt;
    opt.has_header = true;

    csv_stream::Reader reader("users.csv", opt);

    auto header = reader.header();
}
```

### Custom delimiter
```cpp
#include <csv_stream/csv_stream.hpp>

int main()
{
    csv_stream::Options opt;
    opt.delimiter = ';';

    csv_stream::Reader reader("data.csv", opt);
}
```
### Parsing a single CSV line
```cpp
#include <csv_stream/csv_stream.hpp>

int main()
{
    csv_stream::Row row;

    auto r = csv_stream::parse_row("alice,30,kampala", row);
}
```

## API overview

### Main types

- `csv_stream::Result`
- `csv_stream::Row`
- `csv_stream::Options`
- `csv_stream::Reader`

### Parsing utilities

- `parse_row(line)`
- `parse_row_or_throw(line)`

### Streaming reader

- `Reader::read_row(row)`
- `Reader::try_read_row(row)`
- `Reader::header()`
- `Reader::rows_read()`

## Typical workflow

Typical CSV streaming workflow:

1. Open a CSV reader
2. Read rows sequentially
3. Process each row
4. Continue until end-of-file

### Example

**CSV file:**

```csv
name,age

alice,30
bob,28
```

### Processing flow:

```text
read header
read row 1
read row 2
EOF
```

This allows efficient processing of very large CSV files.

## Complexity

| Operation       | Time complexity |
|-----------------|-----------------|
| Parsing row     | O(n)            |
| Streaming file  | O(n)            |

Where **n** is the number of characters processed.

The implementation focuses on **predictable deterministic behavior**.

## Design principles

- Deterministic behavior
- Minimal implementation
- Header-only simplicity
- Streaming-first design
- No external dependencies

This library focuses strictly on **CSV streaming utilities**.

If you need:

- Full CSV dialect support
- Schema validation
- Type conversion
- CSV writing

Build them on top of this layer.

## Tests

Run:

```bash
vix build
vix test
```

**Tests verify:**

- CSV parsing
- Quoted fields
- Escaped quotes
- Streaming reader
- Column validation

## License

MIT License
Copyright (c) Gaspard Kirira
