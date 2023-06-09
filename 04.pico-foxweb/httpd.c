#include "httpd.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>
#include <syslog.h>

#define MAX_CONNECTIONS 1000
#define BUF_SIZE 65535
#define QUEUE_SIZE 1000000

static int listenfd;
int *clients;
static void start_server(const char *);
static void respond(int);

// Client request
char *method, // "GET" or "POST"
    *uri,     // "/index.html" things before '?'
    *qs,      // "a=1&b=2" things after  '?'
    *prot,    // "HTTP/1.1"
    *payload, // for POST
    *logMessage,
    *user;
    
static char *buf;



FILE * logFile;
int payload_size;
struct sockaddr_in clientaddr;
FILE *autFile;
int attemp;

bool author = false;
bool accessLog = false;

bool isAuthor(){return author;}
bool isAccessLog(){return accessLog;}

void serve_forever(const char *PORT, const char *ROOT) {
  socklen_t addrlen;

  int slot = 0;
  
  logFile = fopen("/var/log/picofoxweb/log.txt", "w");// /var/log/picofoxweb/
  autFile = fopen("/var/log/picofoxweb/aut.txt", "w+");//w+ a+

  //printf("Server started %shttp://127.0.0.1:%s%s\n", "\033[92m", PORT,
  //      "\033[0m");

  // create shared memory for client slot array
  clients = mmap(NULL, sizeof(*clients) * MAX_CONNECTIONS,
                 PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

  // Setting all elements to -1: signifies there is no client connected
  int i;
  for (i = 0; i < MAX_CONNECTIONS; i++)
    clients[i] = -1;
  start_server(PORT);
  
  logMessage = malloc(1024);
  user = malloc(1024);
  sprintf(logMessage, "Server started at port no. %s with root directory as %s\n", PORT, ROOT);
  //Журнализация в системный журнал сообщений о запуске службы  с указанием, прослушиваемого порта и рабочего каталога с ресурсами.
  //syslog(LOG_INFO, logMessage);
  
  // Ignore SIGCHLD to avoid zombie threads
  signal(SIGCHLD, SIG_IGN);
	
  // ACCEPT connections
  while (1) {
    addrlen = sizeof(clientaddr);
    clients[slot] = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

    if (clients[slot] < 0) {
      perror("accept() error");
      exit(1);
    } else {
      if (fork() == 0) {
        close(listenfd);
        respond(slot);
        close(clients[slot]);
        clients[slot] = -1;
        exit(0);
      } else {
        close(clients[slot]);
      }
    }

    while (clients[slot] != -1)
      slot = (slot + 1) % MAX_CONNECTIONS;
  }
  free(logMessage);
  fclose(logFile);
  fclose(autFile);
}

// start server
void start_server(const char *port) {
  struct addrinfo hints, *res, *p;

  // getaddrinfo for host
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if (getaddrinfo(NULL, port, &hints, &res) != 0) {
    perror("getaddrinfo() error");
    exit(1);
  }
  // socket and bind
  for (p = res; p != NULL; p = p->ai_next) {
    int option = 1;
    listenfd = socket(p->ai_family, p->ai_socktype, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (listenfd == -1)
      continue;
    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
      break;
  }
  if (p == NULL) {
    perror("socket() or bind()");
    exit(1);
  }

  freeaddrinfo(res);

  // listen for incoming connections
  if (listen(listenfd, QUEUE_SIZE) != 0) {
    perror("listen() error");
    exit(1);
  }
}

// get request header by name
char *request_header(const char *name) {
  header_t *h = reqhdr;
  while (h->name) {
    if (strcmp(h->name, name) == 0)
      return h->value;
    h++;
  }
  return NULL;
}

// get all request headers
header_t *request_headers(void) { return reqhdr; }

// Handle escape characters (%xx)
static void uri_unescape(char *uri) {
  char chr = 0;
  char *src = uri;
  char *dst = uri;

  // Skip inital non encoded character
  while (*src && !isspace((int)(*src)) && (*src != '%'))
    src++;

  // Replace encoded characters with corresponding code.
  dst = src;
  while (*src && !isspace((int)(*src))) {
    if (*src == '+')
      chr = ' ';
    else if ((*src == '%') && src[1] && src[2]) {
      src++;
      chr = ((*src & 0x0F) + 9 * (*src > '9')) * 16;
      src++;
      chr += ((*src & 0x0F) + 9 * (*src > '9'));
    } else
      chr = *src;
    *dst++ = chr;
    src++;
  }
  *dst = '\0';
}

void get_req_resource(const char* path, int client, char* auth_data) 
{
	char data_to_send[BYTES];
	int fd, bytes_read;
	//fprintf(stderr, "2 %s %d %d %d\n", user, attemp, author, accessLog);
	if (strncmp(path, PRIVATE, strlen(PRIVATE))==0  && !isAuthor())
	{
		//printf("[H] Auth needed...\n");
		if (!isAuthor()) {
			if (auth_data == NULL)
			{
				send(client, "HTTP/1.0 401 Unauthorized\n", 26, 0);
				send(client, "WWW-Authenticate: Digest realm=\"Realm\"\n",38,0);
				return;
			} else {
				//printf("[H] Got Auth...%s\n", auth_data);				
	    			loginVerification(auth_data, strlen(auth_data), &author, &accessLog);
	    			char *newUser = scane(auth_data, strlen(auth_data), "username=\"");
				if (strcmp(user, newUser) == 0) {
					printf("new %s %d %d %d\n", user, attemp, author, accessLog);
					if (attemp < 5) {
						if (!isAuthor())
							attemp += 1;
					} else {
					
						//fprintf(stderr, "401 Unauthorized\n");
						send(client, "HTTP/1.0 401 Unauthorized\n", 26, 0);
						return;
					}
					//fprintf(stderr, "new %s %d %d %d\n", user, attemp, author, accessLog);
					fprintf(autFile, "%s %d %d %d", user, author, accessLog, attemp);
				} else {
					attemp = 0;
					if (!author)
						fprintf(autFile, "%s %d %d %d", newUser, false, false, attemp);
					else 
						fprintf(autFile, "%s %d %d %d", newUser, author, accessLog, attemp);
				}
				fseek(autFile, 0, SEEK_SET);
			}
			if (!author) {
				send(client, "HTTP/1.0 401 Unauthorized\n", 26, 0);
				send(client, "WWW-Authenticate: Digest realm=\"Realm\"\n",38,0);
				return;
			}
		}
		
	}	
}

// client connection
void respond(int slot) {
  int rcvd;
  char date[70];
  char *auth_data = NULL, *auth;

  buf = malloc(BUF_SIZE);
  rcvd = recv(clients[slot], buf, BUF_SIZE, 0);
  if (rcvd < 0) // receive error
    fprintf(stderr, ("recv() error\n"));
  else if (rcvd == 0) // receive socket closed
    fprintf(stderr, "Client disconnected upexpectedly.\n");
  else // message received
  {
    buf[rcvd] = '\0';    
    method = strtok(buf, " \t\r\n");
    uri = strtok(NULL, " \t");
    prot = strtok(NULL, " \t\r\n");

    uri_unescape(uri);
    
    //fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", method, uri);
    
    char *clientIP = inet_ntoa(clientaddr.sin_addr);
    
    time_t curTime = time(NULL);
    struct tm tm = *localtime(&curTime);
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec); 
    
    //!!!!
    
    //Запись в формате NCSA Common log format
    //'-' означает пропущенное поле
    sprintf(logMessage, "%s - - [%s] \"%s %s %s\"", clientIP, date, method, uri, prot);
    //sprintf(logMessage, "%s - - \"%s %s %s\"", clientIP, method, uri, prot);
    //Журнализация в собственный текстовой журнал HTTP-запросов
    fprintf(logFile, "%s\n", logMessage);

    //fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", method, uri);   
     

    qs = strchr(uri, '?');

    if (qs)
      *qs++ = '\0'; // split URI
    else
      qs = uri - 1; // use an empty string

    header_t *h = reqhdr;
    char *t, *t2;
    while (h < reqhdr + 16) {
      char *key, *val;

      key = strtok(NULL, "\r\n: \t");
      if (!key)
        break;

      val = strtok(NULL, "\r\n");
      while (*val && *val == ' ')
        val++;

      h->name = key;
      h->value = val;
      h++;
      //fprintf(stderr, "[H] %s: %s\n", key, val);
      // Поиск данных аинтификации
      auth = strstr(key, "Authorization");
      if (auth != NULL) {
      	auth_data = malloc(strlen(val));
      	sprintf(auth_data, "%s", val);     	
      }
      

      t = val + 1 + strlen(val);
      if (t[1] == '\r' && t[2] == '\n')
        break;
    }
    	
    fscanf(autFile, "%s %d %d %d", user, &author, &accessLog, &attemp);
    fseek(autFile, 0, SEEK_SET); 	
  	
  	
    //Если зашли в папку private и пользователь ещё не аутифицирован
    //отправка запроса на аунтификацию или анализ данных аинтификации
    if (strncmp(uri, PRIVATE, strlen(PRIVATE))==0  && !isAuthor()) {
    	get_req_resource(uri, clients[slot], auth_data);
    }
    t = strtok(NULL, "\r\n");
    t2 = request_header("Content-Length"); // and the related header if there is
    payload = t;
    payload_size = t2 ? atol(t2) : (rcvd - (t - buf));

    // bind clientfd to stdout, making it easier to write
    int clientfd = clients[slot];
    dup2(clientfd, STDOUT_FILENO);
    close(clientfd);
    //!!!!
    //Журнализация в собственный текстовой журнал HTTP-запросов
    //sprintf(logMessage, "%s %d %d \"%s %s %s\"", clientIP, clientfd, clientfd, method, uri, prot);
    sprintf(logMessage, "%s %d %d [%s] \"%s %s %s\"", clientIP, clientfd, clientfd, date, method, uri, prot);
        
    
    
    // call router
    route();
    //!!!!
    sprintf(logMessage, "%s %d", logMessage, payload_size);
    fprintf(logFile, "%s\n", logMessage);

    // tidy up
    fflush(stdout);
    shutdown(STDOUT_FILENO, SHUT_WR);
    close(STDOUT_FILENO);
  }

  free(buf);
}
