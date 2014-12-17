CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		concurrent_photo_server.o tcp_server.o tcp_socket.o data_link_layer.o

LIBS =

TARGET =	concurrent_photo_server

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
