#ifndef FS_H
#define FS_H

bool fs_init(void);
bool fs_read(const char* a_name, size_t a_name_len, void* a_buf, size_t a_buf_len);
bool fs_write(const char* a_name, size_t a_name_len, const void* a_buf, size_t a_buf_len);
bool fs_exist(const char* a_name);

#endif // FS_H
