#ifdef TEST

#include "unity.h"

#include "crc.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_crc(void)
{
    uint8_t buffer[1024] = {0};
    snprintf((char *)buffer, sizeof(buffer), "this is a test string");
    uint32_t crc = chksum_crc32_start();

    for (uint32_t i = 0; i < sizeof(buffer); i++) {
        crc = chksum_crc32_continue(crc, &buffer[i], 1);
    }

    crc = chksum_crc32_end(crc);

    TEST_ASSERT_EQUAL(crc, 0xE160A8BE);
}

#endif // TEST
