#include "httpd.h"
#include <sys/stat.h>

#define CHUNK_SIZE 1024 // read 1024 bytes at a time

// Public directory settings
char *PUBLIC_DIR;
char PRIVATE_DIR[CHUNK_SIZE];
#define INDEX_HTML "/index.html"
#define NOT_FOUND_HTML "/404.html"
#define NOT_FORBIDEEN_HTML "/403.html"

int main(int c, char **v) {
  char *port = c == 1 ? "8000" : v[1];
  PUBLIC_DIR = c <= 2 ? "./webroot" : v[2];
  sprintf(PRIVATE_DIR, "%s/private", PUBLIC_DIR);
  //printf("PUBLIC_DIR %s\t PRIVATE_DIR %s\n", PUBLIC_DIR, PRIVATE_DIR);
  serve_forever(port, PUBLIC_DIR);
  return 0;
}

int file_exists(const char *file_name) {
  struct stat buffer;
  int exists;

  exists = (stat(file_name, &buffer) == 0);

  return exists;
}

int read_file(const char *file_name) {
  char buf[CHUNK_SIZE];
  FILE *file;
  size_t nread;
  int err = 1;

  file = fopen(file_name, "r");

  if (file) {
    while ((nread = fread(buf, 1, sizeof buf, file)) > 0)
      fwrite(buf, 1, nread, stdout);

    err = ferror(file);
    fclose(file);
  }
  return err;
}

void route() {
  ROUTE_START()

  GET("/") {
    char index_html[20];
    sprintf(index_html, "%s%s", PUBLIC_DIR, INDEX_HTML);

    HTTP_200;
    if (file_exists(index_html)) {
      read_file(index_html);
    } else {
      printf("Hello! You are using %s\n\n", request_header("User-Agent"));
    }
  }

  GET("/test") {
    HTTP_200;
    printf("List of request headers:\n\n");

    header_t *h = request_headers();

    while (h->name) {
      printf("%s: %s\n", h->name, h->value);
      h++;
    }
  }

  POST("/") {
    HTTP_201;
    printf("Wow, seems that you POSTed %d bytes.\n", payload_size);
    printf("Fetch the data using `payload` variable.\n");
    if (payload_size > 0)
      printf("Request body: %s", payload);
  }

  GET(uri) {
    char file_name[255];
    sprintf(file_name, "%s%s", PUBLIC_DIR, uri);

    if (file_exists(file_name)) {
    //printf("[H] Auth1 %d\t%d\n", isAuthor(), isAccessLog());
      //проверка зашол ли пользованель в приватную папку
      char *istr = strstr(uri, ".png");
      if (strncmp(file_name, PRIVATE_DIR, strlen(PRIVATE_DIR))==0  && !isAccessLog() && istr == NULL) {
        //если он ещё не аунтифицирован то ничего не делать
     	if (!isAuthor()){
     	  return;
     	}
     	HTTP_403;
     	//отправка страници с кодом ошибки 403
	sprintf(file_name, "%s%s", PUBLIC_DIR, NOT_FORBIDEEN_HTML);
      } else 	
      	HTTP_200;
      	
      //printf("[H] file_name %s\n", file_name);
      read_file(file_name);
    } else {
      HTTP_404;
      sprintf(file_name, "%s%s", PUBLIC_DIR, NOT_FOUND_HTML);
      if (file_exists(file_name))
        read_file(file_name);
    }
  }

  ROUTE_END()
}
