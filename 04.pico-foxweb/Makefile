all: PICOFoxweb

clean:
	@rm -rf *.o
	@rm -rf PICOFoxweb

PICOFoxweb: httpd.o main.o verification.o 
	gcc -o PICOFoxweb $^ -I/usr/include/postgresql -L/usr/lib/postgresql/14/lib -lpq -lcrypto

install: PICOFoxweb                     
	mkdir -p /var/www/picofoxweb
	cp -r webroot /var/www/picofoxweb/
	useradd -c "PICOFoxweb user" -r -s /sbin/nologin -d /var/www/picofoxweb picofoxweb
	chown -R picofoxweb:picofoxweb /var/www/picofoxweb
	install -o root -g root -m 0755 PICOFoxweb /usr/local/sbin  
	install -o root -g root -m 0644 picofoxweb.service /etc/systemd/system/
	systemctl daemon-reload
	systemctl restart picofoxweb.service
	mkdir -p /var/log/picofoxweb
	chown -R picofoxweb:picofoxweb /var/log/picofoxweb
	
uninstall:
	systemctl stop picofoxweb
	rm -rf /var/www/picofoxweb
	#rm -rf /var/log/picofoxweb
	rm -f /usr/local/sbin/PICOFoxweb
	rm -f /etc/systemd/system/picofoxweb.service
	systemctl daemon-reload
	userdel -f picofoxweb
	
main.o: main.c httpd.h
	gcc -I/usr/include/postgresql -L/usr/lib/postgresql/14/lib -lpq -lcrypto -c -o main.o main.c

httpd.o: httpd.c httpd.h
	gcc -I/usr/include/postgresql -L/usr/lib/postgresql/14/lib -lpq -lcrypto -c -o httpd.o httpd.c
	
verification.o: verification.c verification.h
	gcc -I/usr/include/postgresql -L/usr/lib/postgresql/14/lib -lpq -lcrypto -c -o verification.o verification.c
