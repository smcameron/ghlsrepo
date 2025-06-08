
PREFIX?=/usr/local
DESTDIR?=

CFLAGS=-Wall -Wextra -fsanitize=address   


ghlsrepo:	ghlsrepo.c Makefile
	$(CC) ${CFLAGS} -I /usr/local/include -L /usr/local/lib -o ghlsrepo ghlsrepo.c -l cjson -l curl 

install:	ghlsrepo ghlsrepo.1
	install -m 755 ghlsrepo ${DESTDIR}${PREFIX}/bin
	install -m 644 ghlsrepo.1 ${DESTDIR}${PREFIX}/man/man1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/ghlsrepo
	rm -f ${DESTDIR}${PREFIX}/man/man1/ghlsrepo.1

clean:
	rm -f ghlsrepo

