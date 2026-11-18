#ifndef RLE_H
#define RLE_H

#include <stdbool.h>
#include <stddef.h>

/* Format: [count byte, value byte], with counts from 1 through 255.
 * Input and output must not overlap. NULL buffers are allowed only with zero
 * respective input length / output capacity. written must be non-NULL.
 * On failure, *written is zero; ignore any partially written output.
 * No terminator is encoded or appended. Pass byte lengths, not strlen for
 * binary data. Output capacity is always measured in bytes.
 */
bool rle_encode(const unsigned char *input, size_t length,
                unsigned char *output, size_t capacity, size_t *written);
bool rle_decode(const unsigned char *input, size_t length,
                unsigned char *output, size_t capacity, size_t *written);

#endif
