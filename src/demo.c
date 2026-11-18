#include "rle.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void preview(const unsigned char *bytes, size_t length)
{
    putchar('"');
    const size_t limit = length < 48 ? length : 48;
    for (size_t i = 0; i < limit; ++i) {
        if (bytes[i] >= 32 && bytes[i] <= 126 && bytes[i] != '"' && bytes[i] != '\\') {
            putchar(bytes[i]);
        } else {
            printf("\\x%02X", (unsigned int)bytes[i]);
        }
    }
    if (limit < length) {
        printf("... (preview)");
    }
    putchar('"');
}

static int experiment(const char *name, const unsigned char *input,
                      size_t length, int csv)
{
    if (length > SIZE_MAX / 2) {
        fprintf(stderr, "Input too large.\n");
        return 1;
    }
    const size_t capacity = length == 0 ? 1 : 2 * length;
    unsigned char *encoded = malloc(capacity);
    unsigned char *decoded = malloc(length == 0 ? 1 : length);
    if (encoded == NULL || decoded == NULL) {
        fprintf(stderr, "Could not allocate demonstration buffers.\n");
        free(encoded);
        free(decoded);
        return 1;
    }
    size_t encoded_size = 0;
    size_t decoded_size = 0;
    if (!rle_encode(input, length, encoded, capacity, &encoded_size)
        || !rle_decode(encoded, encoded_size, decoded, length, &decoded_size)
        || decoded_size != length
        || (length != 0 && memcmp(input, decoded, length) != 0)) {
        fprintf(stderr, "Lossless round trip failed: %s\n", name);
        free(encoded);
        free(decoded);
        return 1;
    }

    if (csv) {
        printf("%s,%zu,%zu,%zu,", name, length, encoded_size / 2, encoded_size);
        if (length != 0) {
            printf("%.6f,%.6f", (double)encoded_size / (double)length,
                   100.0 * (1.0 - (double)encoded_size / (double)length));
        } else {
            printf("NA,NA");
        }
        printf(",PASS\n");
    } else {
        printf("\n%s\n  Input: ", name);
        preview(input, length);
        printf("\n  Encoded [count value] pairs (hex): ");
        const size_t limit = encoded_size < 32 ? encoded_size : 32;
        for (size_t i = 0; i < limit; i += 2) {
            printf("[%02X %02X] ", (unsigned int)encoded[i], (unsigned int)encoded[i + 1]);
        }
        if (limit < encoded_size) {
            printf("... ");
        }
        printf("\n  Original: %zu bytes | Records: %zu | Encoded: %zu bytes\n",
               length, encoded_size / 2, encoded_size);
        if (length != 0) {
            printf("  Encoded/original: %.3f | Space saved: %.1f%%\n",
                   (double)encoded_size / (double)length,
                   100.0 * (1.0 - (double)encoded_size / (double)length));
        } else {
            printf("  Encoded/original and space saved: N/A (empty input)\n");
        }
        printf("  Decoded equals input: PASS\n");
    }
    free(encoded);
    free(decoded);
    return 0;
}

int main(int argc, char **argv)
{
    int csv = 0;
    if (argc == 3 && strcmp(argv[1], "--input") == 0) {
        printf("Format: two binary bytes per record, count then value.\n");
        return experiment("custom", (const unsigned char *)argv[2], strlen(argv[2]), 0);
    }
    if (argc == 2 && strcmp(argv[1], "--csv") == 0) {
        csv = 1;
    } else if (argc != 1) {
        fprintf(stderr, "Usage: %s [--csv | --input TEXT]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (csv) {
        printf("case,input_bytes,records,encoded_bytes,encoded_over_original,space_saved_percent,round_trip\n");
    } else {
        printf("RLE experiment: two binary bytes per record (count, value).\n");
        printf("Payload sizes only; hex displays, terminators and file headers are excluded.\n");
        printf("A count of 06 hex means 6; value 41 hex is ASCII 'A'.\n");
    }

    const struct {
        const char *name;
        const char *text;
    } cases[] = {
        {"long_runs", "AAAAAABBBBBBCCCCCC"},
        {"one_run", "AAAAAAAAAAAA"},
        {"pairs", "AABBCCDD"},
        {"alternating", "ABABABABABAB"},
        {"repeated_sequence", "ABCABCABCABC"},
        {"digit_values", "1111222333"},
        {"pixel_row", "00000000111111110000000011111111"},
        {"empty", ""}
    };
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        if (experiment(cases[i].name, (const unsigned char *)cases[i].text,
                       strlen(cases[i].text), csv) != 0) {
            return EXIT_FAILURE;
        }
    }
    unsigned char long_run[300];
    memset(long_run, 'A', sizeof long_run);
    if (experiment("run_300", long_run, sizeof long_run, csv) != 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
