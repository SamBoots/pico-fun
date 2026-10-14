#ifndef NFC_H
#define NFC_H

#include "hardware/i2c.h"

#define NTAG_215_START_PAGE 4
#define NTAG_215_END_PAGE 129

typedef enum nfc_driver_t
{
    DRIVER_PN532
} nfc_driver_t;

typedef struct nfc_init_info_t
{
    u32 pin_sda;
    u32 pin_scl;
    i2c_inst_t* i2c;
} nfc_init_info_t;

typedef struct nfc_context_t
{
    u8 page_size;  // 4 bytes NTAG or 16 for MIFARE
    u8 address;
    u32 pin_sda;
    u32 pin_scl;
    i2c_inst_t* i2c;

    ERROR_TYPE_T (*read)(const struct nfc_context_t* a_ctx, u8 a_offset, void* a_data, u32 a_len);
    ERROR_TYPE_T (*write)(const struct nfc_context_t* a_ctx, u8 a_offset, const void* a_data, u32 a_len);
    ERROR_TYPE_T (*detect_tag)(const struct nfc_context_t* a_ctx);
    ERROR_TYPE_T (*close)(const struct nfc_context_t* a_ctx);
} nfc_context_t;

ERROR_TYPE_T nfc_init(nfc_context_t* a_ctx, nfc_init_info_t* a_connect_type, nfc_driver_t a_driver);
ERROR_TYPE_T nfc_read(const nfc_context_t* a_ctx, u8 a_offset, void* a_data, u32 a_len);
ERROR_TYPE_T nfc_write(const nfc_context_t* a_ctx, u8 a_offset, const void* a_data, u32 a_len);
ERROR_TYPE_T nfc_detect_tag(const nfc_context_t* a_ctx);
ERROR_TYPE_T nfc_deinit(const nfc_context_t* a_ctx);

#endif // NFC_H
