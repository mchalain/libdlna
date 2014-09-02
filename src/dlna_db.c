#include <stdint.h>

#include "dlna_internals.h"
#include "vfs.h"

int dms_db_open (vfs_t *vfs dlna_unused, char *dbname dlna_unused) { return -1;}
int dms_db_check (vfs_t *vfs dlna_unused) { return 0;}
int dms_db_create (vfs_t *vfs dlna_unused) { return -1;}
dlna_item_t *dms_db_get (vfs_t *vfs dlna_unused, uint32_t id dlna_unused) { return NULL;}
int dms_db_add (vfs_t *vfs dlna_unused, uint32_t id dlna_unused, dlna_item_t *item dlna_unused) { return -1;}
void dms_db_close (vfs_t *vfs dlna_unused) {}
