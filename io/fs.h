#ifndef FS_H
#define FS_H

#define FS_MAX_FILES 16
#define FS_MAX_NAME_LEN 16
#define FS_SENTINEL 0xDEADBEEF

typedef struct fs_entry_t
{
    char name[FS_MAX_NAME_LEN];
    uint32_t offset;
    uint32_t size;
} fs_entry_t;

typedef struct fs_t
{
    uint32_t sentinel;
    uint32_t entry_count;
    fs_entry_t entries[FS_MAX_FILES];
} fs_t;

bool fs_init(void);
bool fs_read(const char* a_name, size_t a_name_len, void* a_buf, size_t a_buf_len);
bool fs_write(const char* a_name, size_t a_name_len, const void* a_buf, size_t a_buf_len);
bool fs_exist(const char* a_name);

#endif // FS_H
