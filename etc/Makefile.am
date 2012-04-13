
install-exec-local:
	$(mkinstalldirs) -m 755 $(DESTDIR)@CONFDIR@
	install -b -o cpcd -m 744 cpcd.conf $(DESTDIR)@CONFDIR@/cpcd.conf
	install -m 700 cpcd.init $(DESTDIR)/etc/init.d/cpcd
	-chmod 755 $(DESTDIR)/etc/init.d/cpcd
	
uninstall-local:
	-rm -rf $(DESTDIR)@CONFDIR@/cpcd.conf
	-rm -rf $(DESTDIR)/etc/init.d/cpcd
