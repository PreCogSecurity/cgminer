
ACLOCAL_AMFLAGS = -I m4

JANSSON_INCLUDES= -I$(top_srcdir)/compat/jansson-2.5/src

if WANT_USBUTILS
USBUTILS_INCLUDES = -I$(top_srcdir)/compat/libusb-1.0/libusb
else
USBUTILS_INCLUDES =
endif

EXTRA_DIST	= example.conf m4/gnulib-cache.m4 linux-usb-cgminer \
		  ADL_SDK/readme.txt api-example.php miner.php	\
		  API.class API.java api-example.c windows-build.txt \
		  bitstreams/* API-README FPGA-README SCRYPT-README \
		  bitforce-firmware-flash.c hexdump.c ASIC-README \
		  01-cgminer.rules GPU-README \
		  tests/test.h tests/test_sha2.c tests/test_util.c

SUBDIRS		= lib compat ccan

# -fcommon: the driver structs declared in miner.h are tentative definitions
# that rely on traditional common-symbol merging. GCC >= 10 defaults to
# -fno-common which turns those into multiple-definition link errors, so
# restore the historical behaviour explicitly.
INCLUDES	= $(PTHREAD_FLAGS) -fno-strict-aliasing -fcommon $(JANSSON_INCLUDES) $(USBUTILS_INCLUDES)

bin_PROGRAMS	= cgminer

bin_SCRIPTS	= $(top_srcdir)/*.cl

cgminer_LDFLAGS	= $(PTHREAD_FLAGS)
cgminer_LDADD	= $(DLOPEN_FLAGS) @LIBCURL_LIBS@ @JANSSON_LIBS@ @PTHREAD_LIBS@ \
		  @OPENCL_LIBS@ @NCURSES_LIBS@ @PDCURSES_LIBS@ @WS2_LIBS@ \
		  @LIBUSB_LIBS@ @MM_LIBS@ @RT_LIBS@ \
		  @MATH_LIBS@ lib/libgnu.a ccan/libccan.a

if HAVE_WINDOWS
cgminer_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib @OPENCL_FLAGS@
else
cgminer_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib @OPENCL_FLAGS@ @LIBCURL_CFLAGS@
endif

cgminer_CPPFLAGS += $(ADL_CPPFLAGS)

# common sources
cgminer_SOURCES := cgminer.c

cgminer_SOURCES	+= elist.h miner.h compat.h bench_block.h	\
		   util.c util.h uthash.h logging.h		\
		   sha2.c sha2.h api.c

cgminer_SOURCES	+= logging.c

# GPU sources, TODO: make them selectable
# the GPU portion extracted from original main.c
cgminer_SOURCES += driver-opencl.h driver-opencl.c

# the original GPU related sources, unchanged
cgminer_SOURCES += ocl.c ocl.h findnonce.c findnonce.h
cgminer_SOURCES += adl.c adl.h adl_functions.h
cgminer_SOURCES += *.cl

if HAS_SCRYPT
cgminer_SOURCES += scrypt.c scrypt.h
endif

if HAS_KECCAK
cgminer_SOURCES += keccak.c keccak.h
endif

if NEED_FPGAUTILS
cgminer_SOURCES += fpgautils.c fpgautils.h
endif

if WANT_USBUTILS
cgminer_SOURCES += usbutils.c usbutils.h
endif

if HAS_BFLSC
cgminer_SOURCES += driver-bflsc.c driver-bflsc.h
endif

if HAS_BITFORCE
cgminer_SOURCES += driver-bitforce.c
endif

if HAS_BITFURY
cgminer_SOURCES += driver-bitfury.c driver-bitfury.h
endif

if HAS_ICARUS
cgminer_SOURCES += driver-icarus.c
endif

if HAS_AVALON
cgminer_SOURCES += driver-avalon.c driver-avalon.h
endif

if HAS_KLONDIKE
cgminer_SOURCES += driver-klondike.c
endif

if HAS_MODMINER
cgminer_SOURCES += driver-modminer.c
bitstreamsdir = $(bindir)/bitstreams
dist_bitstreams_DATA = $(top_srcdir)/bitstreams/*
endif

# Unit tests, run with `make check`.
#
# test_util links util.c with function/data sections and lets the linker
# garbage-collect everything the test does not reference, so the pure helper
# functions can be exercised without pulling in the socket/pool/network code.
check_PROGRAMS	= tests/test_sha2 tests/test_util
TESTS		= tests/test_sha2 tests/test_util

tests_test_sha2_SOURCES	= tests/test_sha2.c sha2.c
tests_test_sha2_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir) \
		  -I$(top_builddir)/lib -I$(top_srcdir)/lib @LIBCURL_CFLAGS@

tests_test_util_SOURCES	= tests/test_util.c util.c
tests_test_util_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir) \
		  -I$(top_builddir)/lib -I$(top_srcdir)/lib @LIBCURL_CFLAGS@
tests_test_util_CFLAGS	= -ffunction-sections -fdata-sections
tests_test_util_LDFLAGS	= @TEST_LDFLAGS@
