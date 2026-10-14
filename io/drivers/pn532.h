#ifndef PN532_H
#define PN532_H
 
typedef struct nfc_context_t nfc_context_t;
typedef struct nfc_init_info_t nfc_init_info_t;

ERROR_TYPE_T pn532_init(nfc_context_t* a_ctx, nfc_init_info_t* a_init_info);
ERROR_TYPE_T pn532_read(const nfc_context_t* a_ctx, u8 a_offset, void* a_data, u32 a_len);
ERROR_TYPE_T pn532_write(const nfc_context_t* a_ctx, u8 a_offset, const void* a_data, u32 a_len);
ERROR_TYPE_T pn532_detect_tag(const nfc_context_t* a_ctx);
ERROR_TYPE_T pn532_deinit(const nfc_context_t* a_ctx);

#endif // PN532_H
