AUTOMAKE_OPTIONS        = foreign
ACLOCAL_AMFLAGS         = -I autotools
AM_MAKEFLAGS            = -s

pkglib_LTLIBRARIES	= warserv.la
warserv_la_SOURCES	= warserv.c warserv_help.c play.c events.c
noinst_HEADERS		= warserv.h

warserv_la_LDFLAGS	= -module -avoid-version

EXTRA_DIST		= autotools/ccdv.c autotools/shtool RELNOTES

dist_doc_DATA		= README.WarServ


include $(top_srcdir)/autotools/rules.mk
