#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "sandboxed_api/lenval_core.h"

extern "C" {
/* All global state is held in this structure */
    static struct Global {
        unsigned int x, y;         /* Pseudo-random number generator state */
    } g;


/* Return a pseudo-random unsigned integer */
    unsigned int speedtest1_random(void){
        g.x = (g.x>>1) ^ ((1+~(g.x&1)) & 0xd0000001);
        g.y = g.y*1103515245 + 12345;
        return g.x ^ g.y;
    }


/* Substitute random() function that gives the same random
** sequence on each run, for repeatability. */
    static void randomFunc(
        sqlite3_context *context,
        int NotUsed,
        sqlite3_value **NotUsed2
        ){
        sqlite3_result_int64(context, (sqlite3_int64)speedtest1_random());
    }

    void reset_random_s() {
        g.x = 0xad131d0b;
        g.y = 0x44f9eac8;
    }

    void setup(sqlite3* db) {
        sqlite3_create_function(db, "random", 0, SQLITE_UTF8, 0, randomFunc, 0, 0);
    }

    int sqlite3_open_s(
        sapi::LenValStruct* filename,   /* Database filename (UTF-8) */
        sqlite3 **ppDb          /* OUT: SQLite db handle */
        ){
        int err = sqlite3_open(static_cast<const char*>(filename->data), ppDb);
        if (err != 0) {
            return err;
        }
        setup(*ppDb);
        // printf("db: %p\n", *ppDb);
        return err;
    }


    int sqlite3_exec_s(
        sqlite3**db, /* An open database */
        sapi::LenValStruct*sql /* SQL to be evaluated */
        ){
        // printf("db: %p\n", *db);
        // puts(static_cast<const char*>(sql->data));
        return sqlite3_exec(*db, static_cast<const char*>(sql->data), NULL, NULL, NULL);
    }

    int sqlite3_prepare_v2_s(
        sqlite3 **db,            /* Database handle */
        sapi::LenValStruct*zSql,       /* SQL statement, UTF-8 encoded */
        int nByte,              /* Maximum length of zSql in bytes. */
        sqlite3_stmt **ppStmt  /* OUT: Statement handle */
        ){
        return sqlite3_prepare_v2(*db, static_cast<const char*>(zSql->data), nByte, ppStmt, NULL);

    }

    int sqlite3_step_s(
        sqlite3_stmt**stmt
        ){
        // printf("step: %p\n", *stmt);
        return sqlite3_step(*stmt);
    }

    int sqlite3_column_count_s(
        sqlite3_stmt**stmt
        ){
        return sqlite3_column_count(*stmt);
    }

    // TODO: find a nice way to return the const char* here
    // const char * sqlite3_column_text_s(
    void sqlite3_column_text_s(
        sqlite3_stmt**stmt,
        int iCol
        ){
        auto text = (const char *)sqlite3_column_text(*stmt, iCol);
        // printf("untrusted text: %s\n", text);
        // return text;
        (void) text;
    }

    int sqlite3_bind_int_s(sqlite3_stmt** stmt, int col, int val){
        return sqlite3_bind_int(*stmt, col, val);
    }
    int sqlite3_bind_int64_s(sqlite3_stmt**stmt, int col, sqlite3_int64 val){
        return sqlite3_bind_int64(*stmt, col, val);
    }
    int sqlite3_bind_text_s(sqlite3_stmt**stmt,int arg1,sapi::LenValStruct* text, int n){
        return sqlite3_bind_text(*stmt, arg1, static_cast<const char*>(text->data), n, SQLITE_TRANSIENT);
    }

    int sqlite3_reset_s(
        sqlite3_stmt**stmt
        ){
        // printf("reset: %p\n", *stmt);
        return sqlite3_reset(*stmt);
    }

    int sqlite3_finalize_s(
        sqlite3_stmt**stmt
        ){
        // puts("finalize");
        return sqlite3_finalize(*stmt);
    }

    int sqlite3_close_s(
        sqlite3**db /* An open database */
        ){
        return sqlite3_close(*db);
    }

    void nop() {}

    void hello_world() {
        puts("Hello World!");
    }

}
