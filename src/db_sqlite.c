#include <stdlib.h>
#include <sqlite3.h>

#include "dlna.h"
#include "dlna_internals.h"
#include "dlna_db.h"

#define xstr(x) str(x)
#define str(x) #x

#define ITEMS_TABLE "items_table"
#define DLNA_DB_ITEMS_FILENAME "filename"
#define DLNA_DB_ITEMS_FILESIZE "filesize"
#define DLNA_DB_ITEMS_PROFILEID "profileid"
#define DLNA_DB_ITEMS_CREATE_TABLE \
  "CREATE TABLE "ITEMS_TABLE"(" \
                      "UID INT PRIMARY KEY NOT NULL," \
                      DLNA_DB_ITEMS_FILENAME" TEXT," \
                      DLNA_DB_ITEMS_FILESIZE" INT," \
                      DLNA_DB_ITEMS_PROFILEID" CHAR(50)" \
                      ");"
#define DLNA_DB_ITEMS_INSERT \
  "INSERT INTO "ITEMS_TABLE" (UID,"DLNA_DB_ITEMS_FILENAME","DLNA_DB_ITEMS_FILESIZE","DLNA_DB_ITEMS_PROFILEID")" \
  "VALUES (%u,'%q',%u,'%q');"

#define METADATA_TABLE "metadata_table"
#define DLNA_DB_MEDIA_TITLE "title"
#define DLNA_DB_MEDIA_ALBUM "album"
#define DLNA_DB_MEDIA_AUTHOR "author"
#define DLNA_DB_MEDIA_COMMENT "comment"
#define DLNA_DB_MEDIA_GENRE "genre"
#define DLNA_DB_MEDIA_TRACK "track"
#define DLNA_DB_METADATA_CREATE_TABLE \
  "CREATE TABLE "METADATA_TABLE"(" \
                      "UID INT PRIMARY KEY NOT NULL," \
                      DLNA_DB_MEDIA_TITLE" TEXT," \
                      DLNA_DB_MEDIA_ALBUM" TEXT," \
                      DLNA_DB_MEDIA_AUTHOR " TEXT," \
                      DLNA_DB_MEDIA_COMMENT " TEXT," \
                      DLNA_DB_MEDIA_GENRE" TEXT," \
                      DLNA_DB_MEDIA_TRACK" INT" \
                      ");"
#define DLNA_DB_METADATA_INSERT \
  "INSERT INTO "METADATA_TABLE" (UID,"DLNA_DB_MEDIA_TITLE","DLNA_DB_MEDIA_ALBUM","DLNA_DB_MEDIA_AUTHOR ","DLNA_DB_MEDIA_COMMENT ","DLNA_DB_MEDIA_GENRE","DLNA_DB_MEDIA_TRACK ")" \
  "VALUES (%u,'%q','%q','%q','%q','%q',%d);"

#define PROPERTIES_TABLE "properties_table"
#define DLNA_DB_PROP_DURATION "duration"
#define DLNA_DB_PROP_BITRATE "bitrate"
#define DLNA_DB_PROP_SAMPLE_FREQUENCY "sample_frequency"
#define DLNA_DB_PROP_BPS "bps"
#define DLNA_DB_PROP_CHANNELS "channels"
#define DLNA_DB_PROP_RESOLUTION "resolution"
#define DLNA_DB_PROPERTIES_CREATE_TABLE \
  "CREATE TABLE "PROPERTIES_TABLE"(" \
                      "UID INT PRIMARY KEY NOT NULL," \
                      DLNA_DB_PROP_DURATION" CHAR(" xstr(DLNA_PROPERTIES_DURATION_MAX_SIZE) ")," \
                      DLNA_DB_PROP_BITRATE" INT," \
                      DLNA_DB_PROP_SAMPLE_FREQUENCY" INT," \
                      DLNA_DB_PROP_BPS" INT," \
                      DLNA_DB_PROP_CHANNELS" INT," \
                      DLNA_DB_PROP_RESOLUTION" CHAR(" xstr(DLNA_PROPERTIES_RESOLUTION_MAX_SIZE) ")" \
                      ");"
#define DLNA_DB_PROPERTIES_INSERT \
  "INSERT INTO "PROPERTIES_TABLE" (UID,"DLNA_DB_PROP_DURATION","DLNA_DB_PROP_BITRATE","DLNA_DB_PROP_SAMPLE_FREQUENCY ","DLNA_DB_PROP_BPS ","DLNA_DB_PROP_CHANNELS","DLNA_DB_PROP_RESOLUTION ")" \
  "VALUES (%u,'%" xstr(DLNA_PROPERTIES_DURATION_MAX_SIZE) "q',%d,%d,%d,%d,'%" xstr(DLNA_PROPERTIES_RESOLUTION_MAX_SIZE) "q');"

int dms_db_open (dlna_t *dlna, char *dbname)
{
  int res;
  sqlite3 *db = NULL;
  
  if (!dlna)
    return -1;

  if (!dbname)
  {
    dlna_log (dlna, DLNA_MSG_ERROR,
              "SQLite support is disabled. " \
              "No database name has been provided");
    return -1;
  }

  res = sqlite3_open (dbname, &db);
  if (res != SQLITE_OK)
  {
    dlna_log (dlna, DLNA_MSG_ERROR,
              "SQLite support is disabled. " \
              "Unable to open database '%s' (%s)",
              dbname, sqlite3_errmsg (db));
    sqlite3_close (db);
    return -1;
  }
  
  dlna->storage_type = DLNA_DMS_STORAGE_MEMORY;
  dlna_log (dlna, DLNA_MSG_INFO,
            "Use SQL database for VFS metadata storage.\n");
  dlna->db = (void*)db;

  res = dms_db_check(dlna);
  return 0;
}

int
dms_db_check (dlna_t *dlna)
{
  sqlite3 *db = (sqlite3 *)dlna->db;
  char* sql = NULL;
  int rc = -1;

  sql = sqlite3_mprintf("SELECT 1 FROM "ITEMS_TABLE);
  rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
  if ( rc != SQLITE_OK )
  {
    return -1;
  }
  return 0;
}

void
dms_db_close (dlna_t *dlna)
{
  sqlite3 *db = (sqlite3 *)dlna->db;
  if (db)
    sqlite3_close (db);
}

static int dms_db_items_callback(void *data, int argc, char **argv, char **colname)
{
  int i;
  dlna_item_t *item = (dlna_item_t *)data;

  for (i = 0; i < argc; i++)
  {
    if (!strcmp(colname[i], DLNA_DB_ITEMS_FILENAME))
      item->filename = strdup(argv[i]);
    if (!strcmp(colname[i], DLNA_DB_ITEMS_FILESIZE))
      item->filesize = strtoul(argv[i], NULL, 10);
    if (!strcmp(colname[i], DLNA_DB_ITEMS_PROFILEID))
      item->profileid = strdup(argv[i]);
  }
 
  return 0;
}

static int dms_db_metadata_callback(void *data, int argc, char **argv, char **colname)
{
  int i;
  dlna_item_t *item = (dlna_item_t *)data;

  item->metadata = calloc (1, sizeof (dlna_metadata_t));
  for (i = 0; i < argc; i++)
  {
    if (!strcmp(colname[i], DLNA_DB_MEDIA_TITLE))
      item->metadata->title = strdup(argv[i]);
    if (!strcmp(colname[i], DLNA_DB_MEDIA_ALBUM))
      item->metadata->album = strdup(argv[i]);
    if (!strcmp(colname[i], DLNA_DB_MEDIA_AUTHOR))
      item->metadata->author = strdup(argv[i]);
    if (!strcmp(colname[i], DLNA_DB_MEDIA_COMMENT))
      item->metadata->comment = strdup(argv[i]);
    if (!strcmp(colname[i], DLNA_DB_MEDIA_GENRE))
      item->metadata->genre = strdup(argv[i]);
    if (!strcmp(colname[i], DLNA_DB_MEDIA_TRACK))
      item->metadata->track = atoi(argv[i]);
  }
  return 0;
}

static int dms_db_properties_callback(void *data, int argc, char **argv, char **colname)
{
  int i;
  dlna_item_t *item = (dlna_item_t *)data;

  item->properties = calloc (1, sizeof (dlna_properties_t));
  for (i = 0; i < argc; i++)
  {
    if (!strcmp(colname[i], DLNA_DB_PROP_DURATION))
      strncpy (item->properties->duration, argv[i], sizeof(item->properties->duration) - 1);
    if (!strcmp(colname[i], DLNA_DB_PROP_BITRATE))
      item->properties->bitrate = strtoul(argv[i], NULL, 10);
    if (!strcmp(colname[i], DLNA_DB_PROP_SAMPLE_FREQUENCY))
      item->properties->sample_frequency = strtoul(argv[i], NULL, 10);
    if (!strcmp(colname[i], DLNA_DB_PROP_BPS))
      item->properties->bps = strtoul(argv[i], NULL, 10);
    if (!strcmp(colname[i], DLNA_DB_PROP_CHANNELS))
      item->properties->channels = strtoul(argv[i], NULL, 10);
    if (!strcmp(colname[i], DLNA_DB_PROP_RESOLUTION))
      strncpy (item->properties->resolution, argv[i], sizeof(item->properties->resolution) - 1);
  }
  return 0;
}

dlna_item_t *
dms_db_get (dlna_t *dlna, uint32_t id)
{
  sqlite3 *db = (sqlite3 *)dlna->db;
  dlna_item_t *item = NULL;
  char* sql = NULL;
  char *errMsg;
  int rc = -1;

  if (!db)
    return NULL;

  item = calloc (1, sizeof (dlna_item_t));

  sql = sqlite3_mprintf("SELECT * FROM "ITEMS_TABLE" WHERE uid=%d ;",id);
  rc = sqlite3_exec(db, sql, dms_db_items_callback, (void*)item, &errMsg);
  if ( rc != SQLITE_OK || !item->filename )
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL, "SQL error: %s\n", errMsg);
    sqlite3_free(errMsg);
    free(item);
    return NULL;
  }
  sql = sqlite3_mprintf("SELECT * FROM "METADATA_TABLE" WHERE uid=%d ;",id);
  rc = sqlite3_exec(db, sql, dms_db_metadata_callback, (void*)item, &errMsg);
  if ( rc != SQLITE_OK )
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL, "SQL error: %s\n", errMsg);
    sqlite3_free(errMsg);
    item->metadata = NULL;
  }
  sql = sqlite3_mprintf("SELECT * FROM "PROPERTIES_TABLE" WHERE uid=%d ;",id);
  rc = sqlite3_exec(db, sql, dms_db_properties_callback, (void*)item, &errMsg);
  if ( rc != SQLITE_OK )
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL, "SQL error: %s\n", errMsg);
    sqlite3_free(errMsg);
    item->properties = NULL;
  }
  item->profile = dlna_get_media_profile(dlna, item->profileid);
    
  return item;
}

int
dms_db_create (dlna_t *dlna)
{
  sqlite3 *db = (sqlite3 *)dlna->db;
  char* sql_items = DLNA_DB_ITEMS_CREATE_TABLE;
  char* sql_media = DLNA_DB_METADATA_CREATE_TABLE;
  char* sql_properties = DLNA_DB_PROPERTIES_CREATE_TABLE;
  char *errMsg;
  int rc;

  if (!db)
    return -1;

  rc = sqlite3_exec(db, sql_items, NULL, NULL, &errMsg);
  if ( rc != SQLITE_OK )
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL, "SQL error: %s\n", errMsg);
    sqlite3_free(errMsg);
    return -1;
  }
  rc = sqlite3_exec(db, sql_media, NULL, NULL, &errMsg);
  if ( rc != SQLITE_OK )
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL, "SQL error: %s\n", errMsg);
    sqlite3_free(errMsg);
    return -1;
  }
  rc = sqlite3_exec(db, sql_properties, NULL, NULL, &errMsg);
  if ( rc != SQLITE_OK )
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL, "SQL error: %s\n", errMsg);
    sqlite3_free(errMsg);
    return -1;
  }
  return 0;
}

int
dms_db_add (dlna_t *dlna, uint32_t id, dlna_item_t *item)
{
  sqlite3 *db = (sqlite3 *)dlna->db;
  char* sql = NULL;
  char *errMsg;
  int rc = -1;

  if (!db)
    return -1;

  sql = sqlite3_mprintf(DLNA_DB_ITEMS_INSERT,id,
            item->filename,
            (uint32_t)item->filesize & 0x00FFFFFFFF,
            item->profile->id);
  rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
  if ( rc != SQLITE_OK )
  {
    dlna_log (dlna, DLNA_MSG_CRITICAL, "SQL error: %s\n", errMsg);
    sqlite3_free(errMsg);
    return -1;
  }

  if (item->metadata)
  {
    sql = sqlite3_mprintf(DLNA_DB_METADATA_INSERT,id,
              item->metadata->title?item->metadata->title:"",
              item->metadata->album?item->metadata->album:"",
              item->metadata->author?item->metadata->author:"",
              item->metadata->comment?item->metadata->comment:"",
              item->metadata->genre?item->metadata->genre:"",
              item->metadata->track);
    rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if ( rc != SQLITE_OK )
    {
      dlna_log (dlna, DLNA_MSG_CRITICAL, "SQL error: %s\n", errMsg);
      sqlite3_free(errMsg);
      return -1;
    }
    sqlite3_free(sql);
  }
  if (item->metadata)
  {
    sql = sqlite3_mprintf(DLNA_DB_PROPERTIES_INSERT,id,
            item->properties->duration?item->properties->duration:"",
            item->properties->bitrate,
            item->properties->sample_frequency,
            item->properties->bps,
            item->properties->channels,
            item->properties->resolution?item->properties->resolution:"");
    rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if ( rc != SQLITE_OK )
    {
      dlna_log (dlna, DLNA_MSG_CRITICAL, "SQL error: %s\n", errMsg);
      sqlite3_free(errMsg);
      return -1;
    }
  }
  return 0;
}
