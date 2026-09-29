#ifndef QLITE3_H
#define QLITE3_H

#include <sqlite3.h>

void create_tables(void);
int is_valid_id(int id);
int user_exists(sqlite3 *db, int id);
int exist_callback(void *data, int argc, char **argv, char **cols);
int db_register(int id, const char *password);
int db_delete_user(int id);
int db_login(int id, const char *password);
int db_query(int user_id, const char *english, char *chinese_out);
#endif

