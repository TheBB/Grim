#include <assert.h>

#include "murmur.h"
#include "gmp.h"

#include "grim.h"
#include "internal.h"

#define MURMUR_SEED 0xcafe8881

static uint64_t hash_buffer(const char *buf, size_t len, uint64_t h) {
    uint64_t out[2];
    MurmurHash3_x86_128(buf, len, (uint32_t) h, out);
    return out[1] + h;
}

static uint64_t hash_uint64(uint64_t n) {
    n = ~n + (n << 21);
    n = n ^ (n >> 24);
    n = n + (n << 3) + (n << 8);
    n = n ^ (n >> 14);
    n = n + (n << 2) + (n << 4);
    n = n ^ (n >> 28);
    n = n + (n << 31);
    return n;
}

static uint64_t hash_bigint(mpz_t n, uint64_t h) {
    h += hash_uint64(n->_mp_size);
    return hash_buffer((char *) n->_mp_d, n->_mp_size * sizeof(n->_mp_d[0]), h);
}

static uint64_t hash_int_float(uint64_t n, double f, uint64_t h) {
    uint64_t m = *((uint64_t *) (&f));
    return hash_uint64((3*n + m) - h);
}


uint64_t grim_hash(grim_object obj, uint64_t h) {
    h += hash_uint64(grim_type(obj));

    switch (grim_direct_tag(obj)) {
    case GRIM_INDIRECT_TAG:
        switch (grim_indirect_tag(obj)) {
        case GRIM_BIGINT_TAG:
            return hash_bigint(((grim_indirect *) obj)->bigint, h);
        case GRIM_STRING_TAG:
            return hash_buffer((char *) grim_strptr(obj), grim_strlen(obj), h);
        case GRIM_BUFFER_TAG:
            return hash_buffer(grim_bufptr(obj), grim_buflen(obj), h);
        default:
            assert(false);
            return 0;
        }
    }

    return hash_int_float((uint64_t) obj, (double) obj, h);
}
