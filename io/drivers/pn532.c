#include "core/types.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pn532.h"
#include "io/nfc.h"
#include "string.h"
#include "os/OS.h"

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

#define I2C_TIMEOUT_US 50000 // 50ms per raw I2C transaction

static u8 calc_lcs(u8 a_len)
{
    return (~a_len) + 1;
}

static u8 calc_dcs(const u8* a_data, usize a_len)
{
    u8 sum = 0;
    for (usize i = 0; i < a_len; i++) sum += a_data[i];
    return (~sum) + 1;
}

static bool pn532_write_raw(const nfc_context_t* a_ctx, const u8* a_buf, usize a_len)
{
    absolute_time_t timeout = make_timeout_time_us(I2C_TIMEOUT_US);
    isize ret = i2c_write_blocking_until(a_ctx->i2c, a_ctx->address, a_buf, a_len, false, timeout);
    return ret == (isize)a_len;
}

static bool pn532_read_raw(const nfc_context_t* a_ctx, u8* a_buf, usize a_len)
{
    absolute_time_t timeout = make_timeout_time_us(I2C_TIMEOUT_US);
    isize ret = i2c_read_blocking_until(a_ctx->i2c, a_ctx->address, a_buf, a_len, false, timeout);
    return ret == (int)a_len;
}

static bool pn532_wait_ready(const nfc_context_t* a_ctx)
{
    u8  status  = 0;
    u32 timeout = 1000; // ms
    u32 start   = to_ms_since_boot(get_absolute_time());

    do {
        pn532_read_raw(a_ctx, &status, 1);
        if (status == 0x01) return true;
        sleep_ms(10);
    } while (to_ms_since_boot(get_absolute_time()) - start < timeout);

    return false; // timed out
}

static bool pn532_read_ack(const nfc_context_t* a_ctx)
{
    static const u8 ACK[] = { 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00 };
    u8 buf[7]; // status byte + 6 ack bytes
    u32 timeout = 1000; // ms
    u32 start   = to_ms_since_boot(get_absolute_time());

    do {
        if (pn532_read_raw(a_ctx, buf, sizeof(buf)) && buf[0] == 0x01)
            return memcmp(buf + 1, ACK, sizeof(ACK)) == 0;

        sleep_ms(10);
    } while (to_ms_since_boot(get_absolute_time()) - start < timeout);

    return false; // timed out
}

static bool pn532_send_command(const nfc_context_t* a_ctx, const u8* a_data, usize a_data_len)
{
    u8 frame[32];
    usize idx = 0;

    usize payload_len = a_data_len + 1;

    frame[idx++] = PN532_PREAMBLE;
    frame[idx++] = PN532_START1;
    frame[idx++] = PN532_START2;
    frame[idx++] = payload_len;
    frame[idx++] = calc_lcs(payload_len);
    frame[idx++] = PN532_TFI_HOST;

    memcpy(&frame[idx], a_data, a_data_len);
    idx += a_data_len;

    // DCS covers TFI + data
    u8 dcs_buf[32];
    dcs_buf[0] = PN532_TFI_HOST;
    memcpy(dcs_buf + 1, a_data, a_data_len);
    frame[idx++] = calc_dcs(dcs_buf, a_data_len + 1);
    frame[idx++] = PN532_POSTAMBLE;

    if (!pn532_write_raw(a_ctx, frame, idx))
        return false;
    return pn532_read_ack(a_ctx);
}

static bool pn532_read_response(const nfc_context_t* a_ctx, u8* a_buf, usize a_len)
{
    if (!pn532_wait_ready(a_ctx)) return false;

    u8 raw[64];
    if (!pn532_read_raw(a_ctx, raw, sizeof(raw)))
        return false;

    // data starts at index 8
    usize copy_len = a_len < (sizeof(raw) - 8) ? a_len : sizeof(raw) - 8;
    memcpy(a_buf, raw + 8, copy_len);
    return true;
}

static bool pn532_sam_config(const nfc_context_t* a_ctx) {
    u8 cmd[] = {
        PN532_CMD_SAMCONFIGURATION, // 0x14
        0x01,  // normal mode
        0x14,  // timeout
        0x01
    };
    return pn532_send_command(a_ctx, cmd, sizeof(cmd));
}

ERROR_TYPE_T pn532_init(nfc_context_t* a_ctx, nfc_init_info_t* a_init_info)
{
    a_ctx->page_size = 4;
    a_ctx->pin_sda = a_init_info->pin_sda;
    a_ctx->pin_scl = a_init_info->pin_scl;
    a_ctx->i2c = a_init_info->i2c;
    a_ctx->address = PN532_I2C_ADDRESS;
    a_ctx->read = pn532_read;
    a_ctx->write = pn532_write;
    a_ctx->detect_tag = pn532_detect_tag;
    a_ctx->close = pn532_deinit;

    i2c_init(a_ctx->i2c, 400000);
    gpio_set_function(a_ctx->pin_sda, GPIO_FUNC_I2C);
    gpio_set_function(a_ctx->pin_scl, GPIO_FUNC_I2C);
    gpio_pull_up(a_ctx->pin_sda);
    gpio_pull_up(a_ctx->pin_scl);

    sleep_ms(500);

    u8 buf;
    absolute_time_t probe_timeout = make_timeout_time_us(I2C_TIMEOUT_US);
    isize ret = i2c_read_blocking_until(a_ctx->i2c, PN532_I2C_ADDR, &buf, 1, false, probe_timeout);
    if (ret < 0)
        return ERROR_TYPE_TIMEOUT;

    if (pn532_sam_config(a_ctx))
        return ERROR_TYPE_OK;

    return ERROR_TYPE_I2C;
}

ERROR_TYPE_T pn532_deinit(const nfc_context_t* a_ctx)
{
    i2c_deinit(a_ctx->i2c);
    gpio_set_function(a_ctx->pin_sda, GPIO_FUNC_SIO);
    gpio_set_function(a_ctx->pin_scl, GPIO_FUNC_SIO);
    gpio_set_dir(a_ctx->pin_sda, GPIO_IN);
    gpio_set_dir(a_ctx->pin_scl, GPIO_IN);
    return ERROR_TYPE_OK;
}

ERROR_TYPE_T pn532_read(const nfc_context_t* a_ctx, u8 a_offset, void* a_data, u32 a_len)
{
    if (!pn532_detect_tag(a_ctx)) return ERROR_TYPE_NFC_NO_TAGS_FOUND;

    u8* dst = (u8*)a_data;
    u8 pages = a_len / a_ctx->page_size;
    u8 current_pg = a_offset;

    for (u8 i = 0; i < pages; i++) {
        u8 cmd[] = {
            PN532_CMD_INDATAEXCHANGE, // 0x40
            0x01,                     // target 1
            NTAG_CMD_READ,            // 0x30
            current_pg
        };

        if (!pn532_send_command(a_ctx, cmd, sizeof(cmd))) return ERROR_TYPE_TIMEOUT;

        u8 response[20];
        if (!pn532_read_response(a_ctx, response, sizeof(response))) return ERROR_TYPE_TIMEOUT;

        if (response[0] != 0x00) return ERROR_TYPE_NFC_NO_TAGS_FOUND; // status != success

        memcpy(dst + (i * 4), response + 1, 4);
        current_pg++;
    }
    return true;
}

ERROR_TYPE_T pn532_write(const nfc_context_t* a_ctx, u8 a_offset, const void* a_data, u32 a_len)
{
    if (!pn532_detect_tag(a_ctx)) return ERROR_TYPE_NFC_NO_TAGS_FOUND;

    const u8* src = (const u8*)a_data;
    u8 pages = a_len / a_ctx->page_size;
    u8 current_pg = a_offset;

    for (u8 i = 0; i < pages; i++) {
        u8 cmd[8];
        cmd[0] = PN532_CMD_INDATAEXCHANGE;
        cmd[1] = 0x01;
        cmd[2] = NTAG_CMD_WRITE;  // 0xA2
        cmd[3] = current_pg;
        memcpy(&cmd[4], src + (i * 4), 4);

        if (!pn532_send_command(a_ctx, cmd, sizeof(cmd))) return ERROR_TYPE_TIMEOUT;

        u8 response[4];
        if (!pn532_read_response(a_ctx, response, sizeof(response))) return ERROR_TYPE_TIMEOUT;

        if (response[0] != 0x00) return ERROR_TYPE_NFC_NO_TAGS_FOUND;

        current_pg++;
        sleep_ms(10); // NTAG write cycle time
    }
    return ERROR_TYPE_OK;
}

ERROR_TYPE_T pn532_detect_tag(const nfc_context_t* a_ctx)
{
    u8 cmd[] = {
        PN532_CMD_INLISTPASSIVETARGET,
        0x01,  // max 1 tag
        0x00   // 106kbps ISO14443A (NTAG type)
    };

    if (!pn532_send_command(a_ctx, cmd, sizeof(cmd))) return ERROR_TYPE_TIMEOUT;

    u8 response[20];
    if (!pn532_read_response(a_ctx, response, sizeof(response))) return ERROR_TYPE_TIMEOUT;

    // response[0] = tags found
    if (response[0] == 0) return ERROR_TYPE_NFC_NO_TAGS_FOUND;

    // parse UID
    // response: tags_found, target_num, ATQA(2), SAK(1), UID_len, UID...
    u8 uid_len = response[5];
    if (uid_len > 7) uid_len = 7;

    // store UID in context for caller to inspect if needed
    // we don't have uid field in context yet but could add it
    // for now just confirm tag found

    return ERROR_TYPE_OK;
}
