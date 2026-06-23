#include <stdlib.h>
#include "fs.h"
#include "pico/stdlib.h"
#include "pico/flash.h"
#include "string.h"

#define FS_FLASH_BASE 0x10000000
#define FS_INDEX_OFFSET (1 * 1024 * 1024) // mb
#define FS_DATA_START (FS_INDEX_OFFSET + FLASH_SECTOR_SIZE)

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

static fs_t s_fs;

bool fs_init(void)
{
    fs_t* fs = (fs_t*)(FS_FLASH_BASE + FS_INDEX_OFFSET);
    if (fs->sentinel != FS_SENTINEL)
    {    
        fs->sentinel = FS_SENTINEL;
        fs->entry_count = 0;
        memset(&fs->entries, 0, sizeof(fs->entries));
    }
    memcpy(&s_fs, fs, sizeof(fs_t));
    return true;
}

static fs_entry_t* fs_find_entry(const char* a_name)
{
    for (size_t i = 0; i < s_fs.entry_count; i++)
    {
        if (strncmp(s_fs.entries[i].name, a_name, FS_MAX_NAME_LEN) == 0)
            return &s_fs.entries[i];
    }
    return NULL;
}

bool fs_read(const char* a_name, size_t a_name_len, void* a_buf, size_t a_buf_len)
{
    if (a_name_len > FS_MAX_NAME_LEN)
        return false;
    fs_entry_t* entry = fs_find_entry(a_name);
    if (entry == NULL || entry->size != a_buf_len)
        return false;
    
    void* f_data = (void*)(FS_FLASH_BASE + entry->offset);
    memcpy(a_buf, f_data, a_buf_len);
    return true;
}

static void fs_flush_index(void)
{
    uint8_t sec[FLASH_SECTOR_SIZE];
    memset(sec, 0XFF, sizeof(sec));
    memcpy(sec, &s_fs, sizeof(fs_t));

    uint32_t interupts = save_and_disable_interrupts();
    flash_range_erase(FS_INDEX_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FS_INDEX_OFFSET, sec, FLASH_SECTOR_SIZE);
    restore_interupts(interupts);
}

bool fs_write(const char* a_name, size_t a_name_len, const void* a_buf, size_t a_buf_len)
{
    if (a_name_len > FS_MAX_NAME_LEN)
        return false;
    fs_entry_t* entry = fs_find_entry(a_name);
    if (entry == NULL)
    {
        entry = &s_fs.entries[s_fs.entry_count++];
        memcpy(entry->name, a_name, a_name_len);
        entry->offset = (FS_DATA_START + FS_INDEX_OFFSET * (s_fs.entry_count - 1));
        return true;
    }
    entry->size = a_buf_len;

    uint8_t page[FLASH_PAGE_SIZE];
    memset(page, 0XFF, sizeof(page));
    memcpy(page, a_buf, a_buf_len);

    uint32_t interupts = save_and_disable_interrupts();
    flash_range_erase(entry->offset, FLASH_SECTOR_SIZE);
    flash_range_program(entry->offset, page, FLASH_PAGE_SIZE);
    restore_interupts(interupts);

    fs_flush_index();
    return true;
}

bool fs_exist(const char* a_name)
{
    return fs_find_entry(a_name) != NULL;   
}
