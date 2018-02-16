#include <stdio.h>
#include <string.h>
#include "sqlite3.h"
#include "erl_driver.h"

// Example code 

int foo(int x) {
  return x+1;
}

int bar(int y) {
  return y*2;
}
 
// Sqlite3 code

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int query_function(const char *db_string, const char *query_string){
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open(db_string, &db);
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }

    rc = sqlite3_exec(db, query_string, callback, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sqlite3_close(db);
    return 0;
}

// int main(int argc, char **argv){
//     if( argc!=3 ){
//         fprintf(stderr, "Usage: %s DATABASE SQL-STATEMENT\n", argv[0]);
//         return(1);
//     } else {
//         query_function(argv[1], argv[2]);
//     }
// }

// Erlang Port Driver code

typedef struct {
    ErlDrvPort port;
} example_data;

static ErlDrvData erl_sqlite_start(ErlDrvPort port, char *buff) {
    example_data* d = (example_data*)driver_alloc(sizeof(example_data));
    d->port = port;
    return (ErlDrvData)d;
}

static void erl_sqlite_stop(ErlDrvData handle) {
    driver_free((char*)handle);
}

static void erl_sqlite_output(ErlDrvData handle, char *buff, ErlDrvSizeT bufflen) {
    example_data* d = (example_data*)handle;
    
    char fn = buff[0], arg = buff[1], res;
    if (fn == 1) {
      res = foo(arg);
    } else if (fn == 2) {
      res = bar(arg);
    } else if (fn == 3) {
        //char q = buff[2];
        fprintf(stderr, "ARG 2 : %c\n", buff[2]);
        res = query_function("db", "select * from ti");
    }

    // const char *db_string = buff[0];
    // const char *query_string = buff[1];
    // char res;
    // printf("%s\n", db_string);
    // printf("%s\n", query_string);
    // res = query_function(db_string, query_string);
    driver_output(d->port, &res, 1);
}

ErlDrvEntry example_driver_entry = {
    NULL,/* F_PTR init, called when driver is loaded */
    erl_sqlite_start,/* L_PTR start, called when port is opened */
    erl_sqlite_stop,/* F_PTR stop, called when port is closed */
    erl_sqlite_output,/* F_PTR output, called when erlang has sent */
    NULL,/* F_PTR ready_input, called when input descriptor ready */
    NULL,/* F_PTR ready_output, called when output descriptor ready */
    "erl_sqlite",/* char *driver_name, the argument to open_port */
    NULL,/* F_PTR finish, called when unloaded */
    NULL,/* void *handle, Reserved by VM */
    NULL,/* F_PTR control, port_command callback */
    NULL,/* F_PTR timeout, reserved */
    NULL,/* F_PTR outputv, reserved */
    NULL,/* F_PTR ready_async, only for async drivers */
    NULL,/* F_PTR flush, called when port is about
         to be closed, but there is data in driver
         queue */
    NULL,/* F_PTR call, much like control, sync call
         to driver */
    NULL,/* F_PTR event, called when an event selected
         by driver_event() occurs. */
    ERL_DRV_EXTENDED_MARKER,
        /* int extended marker, Should always be
        set to indicate driver versioning */
    ERL_DRV_EXTENDED_MAJOR_VERSION,
        /* int major_version, should always be
        set to this value */
    ERL_DRV_EXTENDED_MINOR_VERSION, 
        /* int minor_version, should always be
        set to this value */
    0,/* int driver_flags, see documentation */
    NULL,/* void *handle2, reserved for VM use */
    NULL,/* F_PTR process_exit, called when a
         monitored process dies */
    NULL /* F_PTR stop_select, called to close an
         event object */
};

DRIVER_INIT(erl_sqlite) { /* must match name in driver_entry */
    return &example_driver_entry;
}
