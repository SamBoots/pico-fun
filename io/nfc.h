#ifndef NFC_H
#define NFC_H

#include "hardware/i2c.h"

typedef enum nfc_driver_t
{
    DRIVER_PN532
} nfc_driver_t;

typedef struct nfc_init_info_t
{
    uint pin_sda;
    uint pin_scl;
    i2c_inst_t* i2c;
} nfc_init_info_t;

typedef struct nfc_context_t
{
    uint8_t page_size;  // 4 bytes NTAG or 16 for MIFARE
    uint8_t address;
    uint pin_sda;
    uint pin_scl;
    i2c_inst_t* i2c;

    bool (*read)(const struct nfc_context_t* a_ctx, uint8_t a_offset, void* a_data, uint32_t a_len);
    bool (*write)(const struct nfc_context_t* a_ctx, uint8_t a_offset, const void* a_data, uint32_t a_len);
    bool (*detect_tag)(const struct nfc_context_t* a_ctx);
} nfc_context_t;

bool nfc_init(nfc_context_t* a_ctx, nfc_init_info_t* a_connect_type, nfc_driver_t a_driver);
bool nfc_read(const nfc_context_t* a_ctx, uint8_t a_offset, void* a_data, uint32_t a_len);
bool nfc_write(const nfc_context_t* a_ctx, uint8_t a_offset, const void* a_data, uint32_t a_len);
bool nfc_detect_tag(const nfc_context_t* a_ctx);

#endif // NFC_H
