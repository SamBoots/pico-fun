#ifndef PN532_H
#define PN532_H
 
typedef struct nfc_context_t nfc_context_t;
typedef struct nfc_init_info_t nfc_init_info_t;

bool pn532_init(nfc_context_t* a_ctx, nfc_init_info_t* a_init_info);
bool pn532_read(const nfc_context_t* a_ctx, uint8_t a_offset, void* a_data, uint32_t a_len);
bool pn532_write(const nfc_context_t* a_ctx, uint8_t a_offset, const void* a_data, uint32_t a_len);
bool pn532_detect_tag(const nfc_context_t* a_ctx);

#endif // PN532_H
