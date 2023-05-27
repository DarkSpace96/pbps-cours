#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

void do_exit(PGconn *conn, PGresult *res) {
    
    fprintf(stderr, "%s\n", PQerrorMessage(conn));    

    PQclear(res);
    PQfinish(conn);    
    
    exit(1);
}

int main() {
    
    PGconn *conn = PQconnectdb("user=postgres dbname=autdb");

    if (PQstatus(conn) == CONNECTION_BAD) {
        
        fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
            
        PQfinish(conn);
        exit(1);
    }
        
    PGresult *res = PQexec(conn, "CREATE TABLE usertable(Id SERIAL PRIMARY KEY," \
        "Name VARCHAR(20), Password VARCHAR(20), Private BOOL)");
        
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        do_exit(conn, res); 
    }
    
    PQclear(res);
    
    PQfinish(conn);

    return 0;
}
