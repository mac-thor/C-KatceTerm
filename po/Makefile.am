
MV = mv -f
XGETTEXT = xgettext
MSGFMT = msgfmt

dist_pkgdata_DATA = de.po de.mo
pkgdata_DATA = de.po
pkgdatadir = $(predix)/share/locale

DEPENDON=../asczukat.c ../katceterm.c ../katzuasc.c

SUFFIXES = .po .mo

.po:	$(DEPENDON)
	$(MV) $< katceterm.po
	$(XGETTEXT) -ns -d katceterm ../*.c
	$(MV) katceterm.po $*.po

.po.mo:
	$(MSGFMT) -o $*.mo $<

# run solaris make not parallel mode
.NO_PARALLEL:

# run gnu make not parallel mode
.NOTPARALLEL:

