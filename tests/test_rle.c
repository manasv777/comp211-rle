#include "rle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

static size_t round_trips = 0;

static void check_round_trip(const unsigned char *input, size_t length)
{
    unsigned char encoded[4096];
    unsigned char decoded[2048];
    size_t encoded_size = 0;
    size_t decoded_size = 0;
    CHECK(length <= sizeof decoded);
    CHECK(rle_encode(input, length, encoded, sizeof encoded, &encoded_size));
    CHECK(encoded_size <= 2 * length);
    CHECK(rle_decode(encoded, encoded_size, decoded, sizeof decoded, &decoded_size));
    CHECK(decoded_size == length);
    CHECK(length == 0 || memcmp(input, decoded, length) == 0);
    ++round_trips;
}

static void known_bytes(void)
{
    /* Catches counting across different symbols, lost final runs, and swapped
     * count/value fields. Expected bytes are hand-derived, not re-encoded. */
    const unsigned char input[] = {'A', 'A', 'A', 'B', 'B', 'C'};
    const unsigned char want[] = {3, 'A', 2, 'B', 1, 'C'};
    unsigned char encoded[12];
    unsigned char decoded[6];
    size_t size = 0;
    CHECK(rle_encode(input, sizeof input, encoded, sizeof encoded, &size));
    CHECK(size == sizeof want);
    CHECK(memcmp(encoded, want, sizeof want) == 0);
    CHECK(rle_decode(want, sizeof want, decoded, sizeof decoded, &size));
    CHECK(size == sizeof input);
    CHECK(memcmp(decoded, input, sizeof input) == 0);

    /* Catches treating values as decimal count text or stopping at NUL. */
    const unsigned char binary[] = {0, 0, 255, 255, 255, '1', '1'};
    const unsigned char binary_want[] = {2, 0, 3, 255, 2, '1'};
    CHECK(rle_encode(binary, sizeof binary, encoded, sizeof encoded, &size));
    CHECK(size == sizeof binary_want);
    CHECK(memcmp(encoded, binary_want, size) == 0);
    check_round_trip(binary, sizeof binary);
}

static void count_boundaries(void)
{
    /* Catches count overflow and incorrect splitting at 255. */
    unsigned char input[511];
    unsigned char encoded[8];
    size_t size = 0;
    memset(input, 'A', sizeof input);
    const size_t lengths[] = {1, 255, 256, 510, 511};
    const size_t sizes[] = {2, 2, 4, 4, 6};
    const unsigned char want[][6] = {
        {1, 'A', 0, 0, 0, 0},
        {255, 'A', 0, 0, 0, 0},
        {255, 'A', 1, 'A', 0, 0},
        {255, 'A', 255, 'A', 0, 0},
        {255, 'A', 255, 'A', 1, 'A'}
    };
    for (size_t i = 0; i < sizeof lengths / sizeof lengths[0]; ++i) {
        CHECK(rle_encode(input, lengths[i], encoded, sizeof encoded, &size));
        CHECK(size == sizes[i]);
        CHECK(memcmp(encoded, want[i], size) == 0);
        check_round_trip(input, lengths[i]);
    }
}

static void empty_and_invalid(void)
{
    /* Catches empty-input underflow, missing pair validation, and writes past
     * caller-provided capacities. Sentinel bytes must remain untouched. */
    size_t size = 99;
    unsigned char output[8];
    memset(output, 0xCC, sizeof output);
    CHECK(rle_encode(NULL, 0, NULL, 0, &size) && size == 0);
    CHECK(rle_decode(NULL, 0, NULL, 0, &size) && size == 0);
    const unsigned char one[] = {'A'};
    const unsigned char pair[] = {3, 'A'};
    const unsigned char odd[] = {3, 'A', 2};
    const unsigned char zero[] = {0, 'A'};
    const unsigned char two_runs[] = {'A', 'B'};
    CHECK(!rle_encode(one, 1, output, 1, &size) && size == 0);
    CHECK(output[0] == 0xCC && output[1] == 0xCC);
    CHECK(!rle_encode(two_runs, 2, output, 3, &size) && size == 0);
    CHECK(output[3] == 0xCC);
    CHECK(!rle_decode(pair, 2, output, 2, &size) && size == 0);
    CHECK(output[2] == 0xCC);
    CHECK(!rle_decode(odd, 3, output, sizeof output, &size) && size == 0);
    CHECK(!rle_decode(zero, 2, output, sizeof output, &size) && size == 0);
    CHECK(!rle_encode(NULL, 1, output, sizeof output, &size));
    CHECK(!rle_decode(NULL, 2, output, sizeof output, &size));
    CHECK(!rle_encode(one, 1, NULL, 2, &size));
    CHECK(!rle_decode(pair, 2, NULL, 3, &size));
    CHECK(!rle_encode(one, 1, output, sizeof output, NULL));
    CHECK(!rle_decode(pair, 2, output, sizeof output, NULL));
}

static void exhaustive_short_inputs(void)
{
    /* Every 0–8 byte string over three distinct byte values. Catches off-by-one
     * behavior across all short run patterns, including NUL and high bytes. */
    const unsigned char alphabet[] = {0, 'A', 255};
    unsigned char input[8];
    size_t combinations = 1;
    for (size_t length = 0; length <= sizeof input; ++length) {
        for (size_t code = 0; code < combinations; ++code) {
            size_t value = code;
            for (size_t i = 0; i < length; ++i) {
                input[i] = alphabet[value % 3];
                value /= 3;
            }
            check_round_trip(input, length);
        }
        combinations *= 3;
    }
}

int main(void)
{
    known_bytes();
    count_boundaries();
    empty_and_invalid();
    exhaustive_short_inputs();
    unsigned char all_bytes[256];
    for (size_t i = 0; i < sizeof all_bytes; ++i) {
        all_bytes[i] = (unsigned char)i;
    }
    check_round_trip(all_bytes, sizeof all_bytes);
    unsigned char alternating[2048];
    for (size_t i = 0; i < sizeof alternating; ++i) {
        alternating[i] = (unsigned char)(i % 2);
    }
    check_round_trip(alternating, sizeof alternating);
    printf("PASS: exact byte fixtures, count boundaries, invalid input, capacities\n");
    printf("PASS: %zu lossless round trips (including 9841 exhaustive short inputs)\n",
           round_trips);
    return EXIT_SUCCESS;
}
