# Run-Length Encoding in C

A COMP 211 exploration of how run-length encoding works and when it saves space.

## What this project explores

RLE stores a run of equal neighboring bytes as a count and a value. For example,
six As followed by six Bs can be represented as `(6, A), (6, B)`. Decoding repeats
the values in order to recover the exact original bytes.

The question behind this project is: **when does that actually save space?**

The C program uses a simple format with two binary bytes per record: one count
byte and one value byte. Counts range from 1 to 255. Longer runs are split.
This is a simple format, not a complete image-file compressor.

## How to run it

You need a C11 compiler and Make. No additional libraries are needed.
Open a terminal in this folder, then run:

```sh
make
make demo
make test
./build/rle_demo --input 'AAAAAABBBBBB'
./build/rle_demo --input 'ABABABABABAB'
make results
```

`make results` regenerates `results/experiments.csv` and `results/demo-output.txt`.
On a compiler that supports the sanitizers, `make sanitize` checks for address
and undefined-behavior errors. The commands work on macOS or Linux with those
tools installed; Windows users can use WSL.

Without Make:

```sh
mkdir -p build
cc -std=c11 -Wall -Wextra -Wpedantic -Werror -O2 -Isrc src/demo.c src/rle.c -o build/rle_demo
./build/rle_demo
cc -std=c11 -Wall -Wextra -Wpedantic -Werror -O2 -Isrc tests/test_rle.c src/rle.c -o build/test_rle
./build/test_rle
```

## Results worth explaining

| Input | Original bytes | Encoded bytes | What it shows |
| --- | ---: | ---: | --- |
| `AAAAAABBBBBB` | 12 | 4 | Long runs can save space. |
| `ABABABABABAB` | 12 | 24 | Alternation creates one record per byte. |
| `AABBCCDD` | 8 | 8 | Runs of length two break even in this format. |
| `ABCABCABCABC` | 12 | 24 | Repeated substrings do not become single runs. |

The first two inputs have the same length and the same letter counts. The
arrangement changes the number of records. With `n` input bytes and `r` records,
the encoded payload uses `2r` bytes and saves space when `2r < n`.

Every demonstration decodes the result and checks its length and all its bytes
against the original. The test suite passes **9,849 lossless round trips**, with
additional exact-byte and invalid-input checks. The larger output from an
alternating input is still lossless.

See [all measured results and limits](results/RESULTS.md). The size comparison
excludes file headers, terminators, and the text used to print hexadecimal bytes.

## Presentation materials

- [PowerPoint slides](presentation/rle-example.pptx)
- [PDF slides](presentation/rle-example.pdf)
- [References](docs/RESOURCES.md)

## Project files

`src/rle.c` and `src/rle.h` contain the codec. `src/demo.c` runs the examples and
prints sizes. `tests/test_rle.c` checks exact encoded bytes, binary values,
count boundaries, invalid inputs, buffer capacity, and round trips. `results/`
contains reproducible output. `presentation/` contains the PowerPoint and PDF slides.

## Real-world connection

Some Windows bitmap formats use RLE. Microsoft's BI_RLE8 encoded mode uses a
count byte followed by a color index. Its full format includes additional
controls, so this project does not produce valid BMP files. See
[Microsoft's RLE overview](https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-wmf/b05fc2f2-ad74-41f0-88ee-55ed5c36aad2)
and [Bitmap Compression documentation](https://learn.microsoft.com/en-us/windows/win32/gdi/bitmap-compression).
