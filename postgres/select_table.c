#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libpq-fe.h>

void do_exit(PGconn *conn, PGresult *res) {
    
    fprintf(stderr, "%s\n", PQerrorMessage(conn));    

    PQclear(res);
    PQfinish(conn);    
    
    exit(1);
}

int main(int argc, char* argv[]) {
    
    PGconn *conn = PQconnectdb("user=postgres dbname=autdb");
    char *user, *password, *private, sql[126];
    char c; 
    extern char *optarg;

    if (PQstatus(conn) == CONNECTION_BAD) {
        
        fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
            
        PQfinish(conn);
        exit(1);
    }
        
    while ((c = getopt (argc, argv, "u:p:a:")) != -1)
		switch (c)
		{
			case 'u':
				user = malloc(strlen(optarg));
				strcpy(user,optarg);
				break;
			case 'p':
				password = malloc(strlen(optarg));
				strcpy(password,optarg);
				break;
			case 'a':
				private = malloc(strlen(optarg));
				strcpy(private,optarg);
				break;
			case '?':
				fprintf(stderr,"Wrong arguments given!!!\n");
				exit(1);
			default:
				exit(1);
		}
    //printf ("%s\t%s\t%s\n", user, password, private);
    
    sprintf(sql, "INSERT INTO usertable (Name, Password, Private) VALUES('%s','%s', %s)", user, password, private);
    
    //printf ("%s\n", sql);
    
    PGresult *res = PQexec(conn, sql);
        
    if (PQresultStatus(res) != PGRES_COMMAND_OK) 
        do_exit(conn, res);     
    
    PQclear(res); 
    
    PQfinish(conn);

    return 0;
}
