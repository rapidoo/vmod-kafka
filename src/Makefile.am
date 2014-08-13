AM_CPPFLAGS = @VMOD_INCLUDES@

vmoddir = @VMOD_DIR@
vmod_LTLIBRARIES = libvmod_kafka.la

libvmod_kafka_la_LDFLAGS = -module -export-dynamic -avoid-version -shared -lrdkafka

libvmod_kafka_la_SOURCES = \
	vcc_if.c \
	vcc_if.h \
	vmod_kafka.c


vcc_if.c vcc_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_kafka.vcc
	@VMODTOOL@ $(top_srcdir)/src/vmod_kafka.vcc

VMOD_TESTS = tests/*.vtc
.PHONY: $(VMOD_TESTS)

tests/*.vtc:
	@VARNISHTEST@ -Dvarnishd=@VARNISHD@ -Dvmod_topbuild=$(abs_top_builddir) $@

check: $(VMOD_TESTS)

EXTRA_DIST = \
	vmod_kafka.vcc \
	$(VMOD_TESTS)

