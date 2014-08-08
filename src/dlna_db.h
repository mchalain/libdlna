#ifndef __DLNA_DB_H__
#define __DLNA_DB_H__
int dms_db_open (dlna_t *dlna, char *dbname);
int dms_db_check (dlna_t *dlna);
int dms_db_create (dlna_t *dlna);
dlna_item_t *dms_db_get (dlna_t *dlna, uint32_t id);
int dms_db_add (dlna_t *dlna, uint32_t id, dlna_item_t *item);
void dms_db_close (dlna_t *dlna);

#ifndef HAVE_SQLITE
inline int dms_db_open (dlna_t *dlna, char *dbname) { return -1;}
inline int dms_db_check (dlna_t *dlna) { return 0;}
inline int dms_db_create (dlna_t *dlna) { return -1;}
inline dlna_item_t *dms_db_get (dlna_t *dlna, uint32_t id) { return NULL;}
inline int dms_db_add (dlna_t *dlna, dlna_item_t *item) { return -1}
inline void dms_db_close (dlna_t *dlna) {}
#endif
#endif
