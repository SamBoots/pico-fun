#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pm532.h"
#include "../nfc.h"

#define PN532_I2C_ADDRESS 0x24

#define PN532_I2C_ADDR    0x24

#define PN532_CMD_SAMCONFIGURATION    0x14
#define PN532_CMD_INLISTPASSIVETARGET 0x4A
#define PN532_CMD_INDATAEXCHANGE      0x40

#define NTAG_CMD_READ  0x30
#define NTAG_CMD_WRITE 0xA2

#define PN532_PREAMBLE  0x00
#define PN532_START1    0x00
#define PN532_START2    0xFF
#define PN532_POSTAMBLE 0x00
#define PN532_TFI_HOST  0xD4
#define PN532_TFI_PN532 0xD5

static uint8_t calc_lcs(uint8_t a_len)
{
    return (~a_len) + 1;
}

static uint8_t calc_dcs(const uint8_t* a_data, size_t a_len)
{
    uint8_t sum = 0;
    for (size_t i = 0; i < a_len; i++) sum += a_data[i];
    return (~sum) + 1;
}

static void pn532_write_raw(const nfc_context_t* a_ctx, const uint8_t* a_buf, size_t a_len)
{
    i2c_write_blocking(a_ctx->i2c.i2c, a_ctx->i2c.address, a_buf, a_len, false);
}

static void pn532_read_raw(const nfc_context_t* a_ctx, uint8_t* a_buf, size_t a_len)
{
    i2c_read_blocking(a_ctx->i2c.i2c, a_ctx->i2c.address, a_buf, a_len, false);
}

static bool pn532_wait_ready(const nfc_context_t* a_ctx)
{
    uint8_t  status  = 0;
    uint32_t timeout = 1000; // ms
    uint32_t start   = to_ms_since_boot(get_absolute_time());

    do {
        pn532_read_raw(a_ctx, &status, 1);
        if (status == 0x01) return true;
        sleep_ms(10);
    } while (to_ms_since_boot(get_absolute_time()) - start < timeout);

    return false; // timed out
}

static bool pn532_read_ack(const nfc_context_t* a_ctx)
{
    static const uint8_t ACK[] = { 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00 };
    uint8_t buf[7]; // status byte + 6 ack bytes

    if (!pn532_wait_ready(a_ctx)) return false;

    pn532_read_raw(a_ctx, buf, sizeof(buf));

    // buf[0] is status byte, buf[1..6] is ACK frame
    return memcmp(buf + 1, ACK, sizeof(ACK)) == 0;
}

static bool pn532_send_command(const nfc_context_t* a_ctx, const uint8_t* a_data, uint8_t a_data_len)
{
    uint8_t frame[32];
    uint8_t idx = 0;

    uint8_t payload_len = a_data_len + 1;

    frame[idx++] = PN532_PREAMBLE;
    frame[idx++] = PN532_START1;
    frame[idx++] = PN532_START2;
    frame[idx++] = payload_len;
    frame[idx++] = calc_lcs(payload_len);
    frame[idx++] = PN532_TFI_HOST;

    memcpy(&frame[idx], a_data, a_data_len);
    idx += a_data_len;

    // DCS covers TFI + data
    uint8_t dcs_buf[32];
    dcs_buf[0] = PN532_TFI_HOST;
    memcpy(dcs_buf + 1, a_data, a_data_len);
    frame[idx++] = calc_dcs(dcs_buf, a_data_len + 1);
    frame[idx++] = PN532_POSTAMBLE;

    pn532_write_raw(a_ctx, frame, idx);
    return pn532_read_ack(a_ctx);
}

static bool pn532_read_response(const nfc_context_t* a_ctx, uint8_t* a_buf, size_t a_len)
{
    if (!pn532_wait_ready(a_ctx)) return false;

    uint8_t raw[64];
    pn532_read_raw(a_ctx, raw, sizeof(raw));

    // data starts at index 8
    size_t copy_len = a_len < (sizeof(raw) - 8) ? a_len : sizeof(raw) - 8;
    memcpy(a_buf, raw + 8, copy_len);
    return true;
}

static bool pn532_sam_config(const nfc_context_t* a_ctx) {
    uint8_t cmd[] = {
        PN532_CMD_SAMCONFIGURATION, // 0x14
        0x01,  // normal mode
        0x14,  // timeout
        0x01
    };
    return pn532_send_command(a_ctx, cmd, sizeof(cmd));
}

bool pn532_init(nfc_context_t* a_ctx, nfc_init_info_t* a_init_info)
{
    a_ctx->page_size = 4;
    a_ctx->pin_sda = a_init_info->pin_sda;
    a_ctx->pin_scl = a_init_info->pin_scl;
    a_ctx->i2c = a_init_info->i2c;
    a_ctx->address = PN532_I2C_ADDRESS;
    a_ctx->read = pn532_read;
    a_ctx->write = pn532_write;
    a_ctx->detect_tag = pn532_detect_tag;

    i2c_init(a_ctx->i2c, 400000);
    gpio_set_function(a_ctx->pin_sda, GPIO_FUNC_I2C);
    gpio_set_function(a_ctx->pin_scl, GPIO_FUNC_I2C);
    gpio_pull_up(a_ctx->pin_sda);
    gpio_pull_up(a_ctx->pin_scl);

    sleep_ms(100);

    return pn532_sam_config(a_ctx);
}

bool pn532_read(const nfc_context_t* a_ctx, uint8_t a_offset, void* a_data, uint32_t a_len)
{
    if (!pn532_detect_tag(a_ctx)) return false;

    uint8_t* dst = (uint8_t*)a_data;
    uint8_t pages = a_len / a_ctx->page_size;
    uint8_t current_pg = a_offset / a_ctx->page_size;

    for (uint8_t i = 0; i < pages; i++) {
        uint8_t cmd[] = {
            PN532_CMD_INDATAEXCHANGE, // 0x40
            0x01,                     // target 1
            NTAG_CMD_READ,            // 0x30
            current_pg
        };

        if (!pn532_send_command(a_ctx, cmd, sizeof(cmd))) return false;

        uint8_t response[20];
        if (!pn532_read_response(a_ctx, response, sizeof(response))) return false;

        if (response[0] != 0x00) return false; // status != success

        memcpy(dst + (i * 4), response + 1, 4);
        current_pg++;
    }
    return true;
}

bool pn532_write(const nfc_context_t* a_ctx, uint8_t a_offset, const void* a_data, uint32_t a_len)
{
    if (!pn532_detect_tag(a_ctx)) return false;

    const uint8_t* src = (const uint8_t*)a_data;
    uint8_t pages = a_len / a_ctx->page_size;
    uint8_t current_pg = a_offset / a_ctx->page_size;

    for (uint8_t i = 0; i < pages; i++) {
        uint8_t cmd[8];
        cmd[0] = PN532_CMD_INDATAEXCHANGE;
        cmd[1] = 0x01;
        cmd[2] = NTAG_CMD_WRITE;  // 0xA2
        cmd[3] = current_pg;
        memcpy(&cmd[4], src + (i * 4), 4);

        if (!pn532_send_command(a_ctx, cmd, sizeof(cmd))) return false;

        uint8_t response[4];
        if (!pn532_read_response(a_ctx, response, sizeof(response))) return false;

        if (response[0] != 0x00) return false;

        current_pg++;
        sleep_ms(10); // NTAG write cycle time
    }
    return true;
}

static bool pn532_detect_tag(const nfc_context_t* a_ctx)
{
    uint8_t cmd[] = {
        PN532_CMD_INLISTPASSIVETARGET,
        0x01,  // max 1 tag
        0x00   // 106kbps ISO14443A (NTAG type)
    };

    if (!pn532_send_command(a_ctx, cmd, sizeof(cmd))) return false;

    uint8_t response[20];
    if (!pn532_read_response(a_ctx, response, sizeof(response))) return false;

    // response[0] = tags found
    if (response[0] == 0) return false;

    // parse UID
    // response: tags_found, target_num, ATQA(2), SAK(1), UID_len, UID...
    uint8_t uid_len = response[5];
    if (uid_len > 7) uid_len = 7;

    // store UID in context for caller to inspect if needed
    // we don't have uid field in context yet but could add it
    // for now just confirm tag found

    return true;
}
