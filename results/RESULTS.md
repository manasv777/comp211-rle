# Measured RLE results

Generated from the included C demo on 2026-11-18. Reproduce with `make results`.
The raw data is in [experiments.csv](experiments.csv); the complete terminal
output is in [demo-output.txt](demo-output.txt).

Each record consists of a one-byte count and a one-byte value. An empty payload
uses zero records. Percentages below are rounded to one decimal place.

| Case | Original bytes | Records | Encoded bytes | Encoded/original | Space saved | Decodes exactly? |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| `long_runs`: six As, six Bs, six Cs | 18 | 3 | 6 | 0.333333 | 66.7% | Yes |
| `one_run`: twelve As | 12 | 1 | 2 | 0.166667 | 83.3% | Yes |
| `pairs`: AABBCCDD | 8 | 4 | 8 | 1.000000 | 0.0% | Yes |
| `alternating`: ABABABABABAB | 12 | 12 | 24 | 2.000000 | −100.0% | Yes |
| `repeated_sequence`: ABCABCABCABC | 12 | 12 | 24 | 2.000000 | −100.0% | Yes |
| `digit_values`: 1111222333 | 10 | 3 | 6 | 0.600000 | 40.0% | Yes |
| `pixel_row`: four groups of eight digit bytes | 32 | 4 | 8 | 0.250000 | 75.0% | Yes |
| `empty` | 0 | 0 | 0 | N/A | N/A | Yes |
| `run_300`: 300 As | 300 | 2 | 4 | 0.013333 | 98.7% | Yes |

## Interpreting the experiments

The three long runs encode as `[06 41] [06 42] [06 43]`, six total bytes.
Those are hexadecimal bytes: ASCII `A`, `B`, and `C` are `41`, `42`, and `43`.

The alternating and repeated-sequence cases each need twelve records. RLE here
does not have a way to refer back to an earlier substring. Losing the size
comparison does not make the encoding lossy: both still decode exactly.

Two identical bytes become one two-byte record, explaining the break-even
`pairs` case. For `n > 0`, the condition for compression is `2 × records < n`.
Equivalently, the average number of original bytes represented by each record
must exceed two. Count splitting must be included in the record total.

The 300-byte case becomes `(255, A)` and `(45, A)`: `[FF 41] [2D 41]`.
An eight-bit count cannot store 300 directly. The empty case has no defined
encoded/original ratio because its original size is zero.

## Important limits

- These are deterministic payload-size experiments, not runtime benchmarks.
- Payload sizes exclude headers, stored lengths, end markers, and filesystem
  overhead. A real stored file needs a way to delimit or describe its contents.
- The `pixel_row` case is **32 ASCII bytes**, using `0` and `1` as a toy row.
  It is not a measured BMP image. A truly bit-packed 32-pixel monochrome row
  would have a four-byte raw pixel payload; do not claim a 75% saving over that.
- RLE variants can use literal blocks, escape markers, different count widths,
  or other representations. These precise sizes and the two-byte break-even
  condition apply to this project's chosen format.
- The demo allocates worst-case buffers. Its temporary memory use is not the
  same as its encoded payload size, and it does not write compressed files.

## Try an independent prediction

Before running these commands, calculate the expected encoded byte counts:

```sh
./build/rle_demo --input 'AAAAAABBBBBB'
./build/rle_demo --input 'ABABABABABAB'
```

Both inputs contain twelve bytes, six of each letter. Their record counts are
different. Explain that difference without reading the program's output.
