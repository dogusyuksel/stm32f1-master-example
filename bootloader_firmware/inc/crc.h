#ifndef _CRC32_H
#define _CRC32_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/**
 * Calculate single step of crc32
 */
uint32_t chksum_crc32_step(uint32_t crc, uint8_t byte);

/**
 * Caluclate crc32 of a block
 */
uint32_t chksum_crc32(uint8_t *block, unsigned int length);

/** Initialize a crc32 calculation */
uint32_t chksum_crc32_start(void);
/** Continue calculating the crc32 of several blocks, with a single block */
uint32_t chksum_crc32_continue(uint32_t crc, uint8_t *block,
                               unsigned int length);
/** Finish the crc32 of one or several blocks */
uint32_t chksum_crc32_end(uint32_t crc);
/** Calculate the CRC-16/ARC */
uint16_t calculate_crc16(uint8_t *buffer, uint32_t length);

#ifdef __cplusplus
}
#endif
#endif
