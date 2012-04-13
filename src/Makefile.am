bin_PROGRAMS = cpcd

INCLUDES = $(PKG_CPPFLAGS)
AM_CFLAGS = @CFLAGS@ -Wall

cpcd_SOURCES = cpcd.c cpcd.h cpcd_common.h cpcd_datastructures.h cpcd_datastructures.c cpcd_config.h cpcd_config.c cpcd_util.c cpcd_util.h cpcd_html.c cpcd_uhc.h cpcd_uhc.c cpcd_html.h datastructures.h datastructures.c cgi.h cgi.c 
cpcd_CFLAGS = $(AM_CFLAGS)

install-exec-local:
	-useradd -c "CryptNET Peer Cache Daemon" -s /bin/bash -d /var/tmp cpcd
	$(mkinstalldirs) -m 755 $(DESTDIR)@DATADIR@
	-chown cpcd $(DESTDIR)@DATADIR@
	$(mkinstalldirs) -m 755 $(DESTDIR)@LOGDIR@
	-chown cpcd $(DESTDIR)@LOGDIR@
	$(mkinstalldirs) -m 755 $(DESTDIR)@PIDDIR@
	-chown cpcd $(DESTDIR)@PIDDIR@
	
uninstall-local:
	-rm -rf $(DESTDIR)@DATADIR@
	-rm -rf $(DESTDIR)@LOGDIR@
	-rm -rf $(DESTDIR)@PIDDIR@
	-userdel cpcd
