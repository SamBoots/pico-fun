#include "core/types.h"
#include "hardware/i2c.h"
#include "drivers/pn532.h"
#include "nfc.h"

ERROR_TYPE_T nfc_init(nfc_context_t* a_ctx, nfc_init_info_t* a_connect_type, nfc_driver_t a_driver)
{
    switch (a_driver)
    {
    case DRIVER_PN532:
        return pn532_init(a_ctx, a_connect_type);
        break;
    }
    return ERROR_TYPE_INVALID_PARAM;
}

ERROR_TYPE_T nfc_read(const nfc_context_t* a_ctx, u8 a_offset, void* a_data, u32 a_len)
{
    return a_ctx->read(a_ctx, a_offset, a_data, a_len);
}

ERROR_TYPE_T nfc_write(const nfc_context_t* a_ctx, u8 a_offset, const void* a_data, u32 a_len)
{
    return a_ctx->write(a_ctx, a_offset, a_data, a_len);
}

ERROR_TYPE_T nfc_detect_tag(const nfc_context_t* a_ctx)
{
    return a_ctx->detect_tag(a_ctx);
}

ERROR_TYPE_T nfc_deinit(const nfc_context_t* a_ctx)
{
    return a_ctx->close(a_ctx);
}
