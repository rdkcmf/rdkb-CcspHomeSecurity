AM_CFLAGS = -D_ANSC_LINUX
AM_CFLAGS += -D_ANSC_USER
AM_CFLAGS += -D_ANSC_LITTLE_ENDIAN_
AM_CFLAGS += -D_COSA_SIM_
AM_LDFLAGS = -L$(top_builddir)/../CcspCommonLibrary/lib
AM_LDFLAGS += -lccsp_common
AM_LDFLAGS += -lsyscfg
AM_LDFLAGS += -lcurl
AM_LDFLAGS += -lxml2
AM_LDFLAGS += -lulog

AM_CPPFLAGS = -Wall -Werror
ACLOCAL_AMFLAGS = -I m4
hardware_platform = i686-linux-gnu
bin_PROGRAMS = CcspHomeSecurity
CcspHomeSecurity_CPPFLAGS = -I/var/tmp/pc-rdkb/include -I/var/tmp/pc-rdkb/include/dbus-1.0 -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/custom -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/include -I$(top_srcdir)/../CcspCommonLibrary/source/debug_api/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/include/linux -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/components/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/package/slap/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/ansc/include -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/components/common/MessageBusHelper/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/http/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/ansc/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/tls/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/web/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/stun/include -I/var/tmp/pc-rdkb/include/fcgi-2.4.0/include -I/var/tmp/pc-rdkb/include/libxml2-2.7.7 -I/var/tmp/pc-rdkb/include/curl-7.28.1 -I/var/tmp/pc-rdkb/include/openssl $(CPPFLAGS)
CcspHomeSecurity_SOURCES = hdk_context.c hdk_methods.c hdk_adi.c hdk.c crc32.c hdk_device.c hdk_ccsp_mbus.c hdk_encode.c hdk_data.c hnapd.c
CcspHomeSecurity_LDFLAGS =  -L$(top_builddir)/../CcspCommonLibrary/lib -lccsp_common -lsyscfg -lcurl -lxml2 -lulog