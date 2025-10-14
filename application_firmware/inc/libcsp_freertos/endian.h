#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// // ---------------------------------------------------------------------------
// // STM32 (ARM Cortex-M) microcontrollers are always little-endian
// // ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Byte-swap helpers using GCC built-ins
// These map directly to efficient CPU instructions where available
// ---------------------------------------------------------------------------
#ifndef bswap_16
#define bswap_16(x) __builtin_bswap16(x)
#endif

#ifndef bswap_32
#define bswap_32(x) __builtin_bswap32(x)
#endif

#ifndef bswap_64
#define bswap_64(x) __builtin_bswap64(x)
#endif

// ---------------------------------------------------------------------------
// POSIX-style endian conversion macros
// For STM32 (little-endian), htoleXX and leXXtoh are identity operations,
// while htobeXX and beXXtoh perform byte-swapping.
// ---------------------------------------------------------------------------
#ifndef htobe16
#define htobe16(x) bswap_16(x)
#define htole16(x) (x)
#define be16toh(x) bswap_16(x)
#define le16toh(x) (x)

#define htobe32(x) bswap_32(x)
#define htole32(x) (x)
#define be32toh(x) bswap_32(x)
#define le32toh(x) (x)

#define htobe64(x) bswap_64(x)
#define htole64(x) (x)
#define be64toh(x) bswap_64(x)
#define le64toh(x) (x)
#endif

#ifdef __cplusplus
}
#endif

#endif // _ENDIAN_H_
