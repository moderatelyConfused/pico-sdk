/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
// Include sys/types.h before inttypes.h to work around issue with
// certain versions of GCC and newlib which causes omission of PRIu64
#include <sys/types.h>
#include <inttypes.h>
#include "pico/stdlib.h"
#include "pico/bit_ops.h"
#include <stdlib.h>

void test_builtin_bitops() {
    // Use int64_t to avoid signed integer overflow (UBSAN trap)
    // The __rev() results can be large negative numbers when cast to int32_t
    int64_t x = 0;
    for (uint32_t i = 0; i < 10000; i++) {
        if (i % 1000 == 0) {
            printf("  iteration %u\n", (unsigned)i);
        }
        uint32_t vals32[] = {
                i,
                1u << (i & 31u),
                i * 12355821u,
        };
        uint64_t vals64[] = {
                i,
                1ull << (i & 63u),
                i * 12345678123125ull,
        };
        for(int j=0; j<count_of(vals32); j++) {
            x += __builtin_popcount(vals32[j]);
            x += __builtin_popcountl(vals32[j]);
            x += (int32_t)__rev(vals32[j]);
            // clz/ctz on 0 is undefined behavior - compiler-rt may infinite loop
            if (vals32[j]) {
                x += __builtin_clz(vals32[j]);
                x += __builtin_ctz(vals32[j]);
#if PICO_ON_DEVICE
                // check l variants are the same
                if (__builtin_clz(vals32[j]) != __builtin_clzl(vals32[j])) x += 17;
                if (__builtin_ctz(vals32[j]) != __builtin_ctzl(vals32[j])) x += 23;
#endif
            } else {
                x += 64; // clz(0) + ctz(0) = 32 + 32
            }
        }
        for(int j=0; j<count_of(vals64); j++) {
            x += __builtin_popcountll(vals64[j]);
            x += (int32_t)__revll(vals64[j]);
            // clzll/ctzll on 0 is undefined behavior - compiler-rt may infinite loop
            if (vals64[j]) {
                x += __builtin_clzll(vals64[j]);
                x += __builtin_ctzll(vals64[j]);
            } else {
                x += 128; // clzll(0) + ctzll(0) = 64 + 64
            }
        }
    }
    printf("Count is %" PRId64 "\n", x);
    int64_t expected = 1475508680;
    if (x != expected) {
        printf("FAILED (expected count %" PRId64 ")\n", expected);
        exit(1);
    }
}

int main() {
    setup_default_uart();

    puts("Hellox, world!");
    printf("Hello world %d\n", 2);
#if PICO_NO_HARDWARE
    puts("This is native");
#endif
#if PICO_NO_FLASH
    puts("This is no flash");
#endif

    for (int i = 0; i < 64; i++) {
        uint32_t x = (i < 32) ? (1u << i) : 0;
        uint64_t xl = 1ull << i;
//        printf("%d %u %u %u %u \n", i, (uint)(x%10u), (uint)(x%16u), (uint)(xl %10u), (uint)(xl%16u));
        printf("%08x %08x %016llx %016llx\n", (uint) x, (uint) __rev(x), (unsigned long long) xl,
               (unsigned long long) __revll(xl));
    }

    test_builtin_bitops();

    for (int i = 0; i < 8; i++) {
        sleep_ms(500);
        printf( "%" PRIu64 "\n", to_us_since_boot(get_absolute_time()));
    }
    absolute_time_t until = delayed_by_us(get_absolute_time(), 500000);
    printf("\n");
    for (int i = 0; i < 8; i++) {
        sleep_until(until);
        printf("%" PRIu64 "\n", to_us_since_boot(get_absolute_time()));
        until = delayed_by_us(until, 500000);
    }
    puts("DONE");
}
