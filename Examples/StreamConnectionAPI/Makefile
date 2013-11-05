#
# variables
#

# compiler
CC = g++
# compiler options
CFLAGS = -c -Wall -Wextra -Werror
# linker options
LFLAGS = 
# build directory
OBJDIR = build
# source files
SERVER_SOURCES = server.cpp StreamConnection.cpp StreamPacket.cpp
CLIENT_SOURCES = client.cpp StreamConnection.cpp StreamPacket.cpp
SERVER_OBJECTS = $(addprefix $(OBJDIR)/, $(SERVER_SOURCES:.cpp=.o))
CLIENT_OBJECTS = $(addprefix $(OBJDIR)/, $(CLIENT_SOURCES:.cpp=.o))
# executeable
SERVER_EX = server
CLIENT_EX = client

#
# targets
#

all: $(SERVER_EX) $(CLIENT_EX)

$(SERVER_EX): $(SERVER_OBJECTS)
	$(CC) $(LFLAGS) $(SERVER_OBJECTS) -o $@

$(CLIENT_EX): $(CLIENT_OBJECTS)
	$(CC) $(LFLAGS) $(CLIENT_OBJECTS) -o $@

$(OBJDIR)/%.o : %.cpp
	$(CC) $(CFLAGS) $< -o $@

$(SERVER_OBJECTS): | $(OBJDIR)
$(CLIENT_OBJECTS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/client.o: StreamConnection.h

$(OBJDIR)/server.o: StreamConnection.h

$(OBJDIR)/StreamConnection.o: StreamConnection.h StreamPacket.h

$(OBDIR)/StreamPacket.o: StreamPacket.h

clean:
	rm -rf $(OBJDIR) $(SERVER_EX) $(CLIENT_EX)
