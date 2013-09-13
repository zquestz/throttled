# throttled Makefile
# (C) 2010 - quest and lws

ifeq ($(shell uname -s),FreeBSD)
MAKE = gmake
else
MAKE = make
endif

APPNAME = throttled

all:
	cd src; ${MAKE} ${APPNAME}; mv ${APPNAME} ..; cd ..

install: all
	install -d /usr/local/sbin
	install -c ${APPNAME} /usr/local/sbin/throttled
	install -c throttled-startup /usr/local/sbin/throttled-startup
clean:
	cd src; ${MAKE} clean

uninstall:
	rm /usr/local/sbin/throttled
	rm /usr/local/sbin/throttled-startup
