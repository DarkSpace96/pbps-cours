#include "verification.h"

void do_exit(PGconn *conn) {
    
    PQfinish(conn);
    //exit(1);
}

char *scane(char *in, int size, char *data) {
	int ch = '"';
	int pos;
	char *istr;
      	//printf("scane %s\n",data);
	istr = strstr(in, data);
   	pos = istr-in;
   	//printf("in %s\n", in);
	/*if (istr == NULL)
      		printf("Строка не найдена\n");
   	else {
      		printf("Искомая строка начинается с символа %d вырезать %ld\n", pos, strlen(data));
      	}*/
	char *buf = malloc(size);
	sprintf(buf, "%s", in + pos + strlen(data));
	strncpy(buf, in + pos + strlen(data), size);
	//printf ("buf # %s\n", buf);
	istr=strchr(buf,ch);
   	pos = istr-buf;
   	/*if (istr==NULL)
      		printf("Символ в строке не найден\n");
   	else
      		printf("Искомый символ в строке на позиции # %d\n", pos);*/
	char *out = malloc(pos);
	strncpy(out, buf, pos); 
	out[pos] = '\0';
	
	//printf ("out # %s\n", out);
	return out;
}

char *md5(char *in) {
	char *out = malloc(33);
	uint8_t digest[MD5_DIGEST_LENGTH];
	//struct MD5Context context;
	MD5_CTX context;
	MD5_Init(&context);
	MD5_Update(&context, in, strlen(in));
	MD5_Final(digest, &context);
	for(int i = 0; i < 16; ++i)
    		sprintf(&out[i*2], "%02x", (unsigned int)digest[i]);
    	return out;	
}

void loginVerification(char *auth_data, int size, bool *author, bool *accessLog) {
char *user = scane(auth_data, size, "username=\"");
	user[strlen(user)] = '\0';	
	//printf ("user # %s\n",user);
	PGconn *conn = PQconnectdb("user=postgres dbname=autdb");
	if (PQstatus(conn) == CONNECTION_BAD) {	
		fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
		do_exit(conn);
		return;
	}
	const char *paramValues[1]; 
	paramValues[0] = user;
	char *stm = "SELECT * FROM users WHERE Name=$1;";
	PGresult *res = PQexecParams(conn, stm, 1, NULL, paramValues,  NULL, NULL, 0);
	if (PQresultStatus(res) != PGRES_TUPLES_OK || PQgetvalue(res, 0, 0) == NULL) {
		printf("No data retrieved\n");
		PQclear(res);
		do_exit(conn);
		*author = false;
		*accessLog = false; 
		return;
	}
	char *pusword = malloc(strlen(PQgetvalue(res, 0, 2)));	
	sprintf(pusword, "%s", PQgetvalue(res, 0, 2));
	//printf ("pusword # %s\n", pusword);
	char *realm = scane(auth_data, size, "realm=\""); 
	//printf ("realm # %s\n", realm);
	char *nonce = scane(auth_data, size, "nonce=\""); 
	//printf ("nonce # %s\n", nonce); 
	char *uri = scane(auth_data, size, "uri=\""); 
	//printf ("uri # %s\n", uri); 
	char *response = scane(auth_data, size, "response=\""); 
	//printf ("response # %s\n", response); 
	char *buff = malloc(500);
	sprintf(buff, "%s:%s:%s", user, realm, pusword);
	//printf ("buff HA1 # %s\n", buff); 
	char *HA1 = md5(buff);
	//printf ("HA1 # %s\n", HA1); 
	sprintf(buff, "GET:%s", uri);
	//printf ("buff HA2 # %s\n", buff); 
	char *HA2 = md5(buff);
	//printf ("HA2 # %s\n", HA2);
	sprintf(buff, "%s:%s:%s", HA1, nonce, HA2);
	//printf("buff response2 # %s\n", buff); 
	char *response2 = md5(buff);	
	//printf ("response2 # %s\n", response2);
	//printf ("strcmp # %d\n", strcmp(response, response2));
	if (strcmp(response, response2) == 0) {
		//printf("OK\n");
		*author = true;
		if (strcmp("t", PQgetvalue(res, 0, 3)) == 0)
			*accessLog = true; 
		else 				
			*accessLog = false;			
		//printf("loginVerification ok %d\t%d\n", *author, *accessLog);
	} else {
		*author = false;
		*accessLog = false;
		//printf("loginVerification not ok %d\t%d\n", *author, *accessLog);
	}
	//printf("loginVerification %d\t%d\n", *author, *accessLog);
}
