#include "rle.h"

#include <limits.h>
#include <string.h>

_Static_assert(CHAR_BIT == 8, "This format uses 8-bit bytes.");

bool rle_encode(const unsigned char *input, size_t length,
                unsigned char *output, size_t capacity, size_t *written)
{
    if (written == NULL) {
        return false;
    }
    *written = 0;
    if ((input == NULL && length != 0) || (output == NULL && capacity != 0)) {
        return false;
    }

    size_t used = 0;
    size_t i = 0;
    while (i < length) {
        size_t count = 1;
        /* Stop at a new value, the input end, or the largest count byte. */
        while (count < 255 && count < length - i && input[i + count] == input[i]) {
            ++count;
        }
        if (capacity - used < 2) {
            return false;
        }
        output[used++] = (unsigned char)count;
        output[used++] = input[i];
        i += count;
    }
    *written = used;
    return true;
}

bool rle_decode(const unsigned char *input, size_t length,
                unsigned char *output, size_t capacity, size_t *written)
{
    if (written == NULL) {
        return false;
    }
    *written = 0;
    if ((input == NULL && length != 0) || (output == NULL && capacity != 0)
        || length % 2 != 0) {
        return false;
    }

    size_t used = 0;
    for (size_t i = 0; i < length; i += 2) {
        size_t count = input[i];
        if (count == 0 || count > capacity - used) {
            return false;
        }
        memset(output + used, input[i + 1], count);
        used += count;
    }
    *written = used;
    return true;
}
