#ifndef NVS_DRIVER_H
#define NVS_DRIVER_H

#include <stddef.h>

bool nvs_init();
bool nvs_read_blob(const char *key, void *data, size_t length);
bool nvs_write_blob(const char *key, const void *data, size_t length);
bool nvs_delete_key(const char *key);
bool nvs_clear();

#endif // NVS_DRIVER_H