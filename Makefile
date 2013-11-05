#
# variables
#

# compiler
CC = g++

# compiler options
CFLAGS = -O2 -Wall

# linker options
LFLAGS = -shared -fPIC -lboost_thread-mt -lboost_system-mt -lpthread -lticp

# source files
SOURCES = GroundOcfServer.cpp \
		  GroundPacketServer.cpp \
		  myErrors.cpp \
		  NetProtConf.cpp \
		  OcfServer.cpp \
		  PacketServer.cpp \
		  SpacePacketConf.cpp \
		  TestProtConf.cpp \
		  TmFrameTimestamp.cpp \
		  TmFrameBitrate.cpp \
		  TmMasterChannel.cpp \
		  TmOcf.cpp \
		  TmPhysicalChannel.cpp \
		  TmTransferFrame.cpp \
		  TmVirtualChannel.cpp
# header files
HEADERS = Tmtp.h TmtpOcf.h TmtpPacket.h \
		$(SOURCES:.cpp=.h)

# where to install the library and include files
PREFIX = /usr/local

# executeable
LIB = libtmtp.so

# doxygen definition file
DOXYFILE = Doxyfile

#
# targets
#

all: $(LIB)

install: all
	mkdir -p $(PREFIX)/include/tmtp && \
	cp -f $(LIB) $(PREFIX)/lib/ && \
	cp -f $(HEADERS) $(PREFIX)/include/tmtp/

uninstall:
	rm -rf $(PREFIX)/lib/$(LIB)
	rm -rf $(PREFIX)/include/tmtp

$(LIB): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LFLAGS)

.PHONY: clean
clean:
	rm -rf $(LIB)

.PHONY: remake
remake: clean all

.PHONY: doc
doc:
	doxygen $(DOXYFILE)
