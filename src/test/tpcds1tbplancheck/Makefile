subdir = src/test/tpcds1tbplancheck
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

# where to find psql for testing an existing installation
PSQLDIR = $(bindir)
DBNAME = "tpcds1tb"

pg_regress.o:
	$(MAKE) -C $(top_builddir)/src/test/regress

tpcds1tb: pg_regress.o
	$(top_builddir)/src/test/regress/pg_regress --init-file=$(top_builddir)/src/test/regress/init_file --psqldir='$(PSQLDIR)' --dbname=$(DBNAME) --inputdir=$(srcdir) --schedule=$(srcdir)/perfsuites_load_schedule

