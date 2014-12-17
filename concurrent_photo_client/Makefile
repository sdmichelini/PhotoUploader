CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		concurrent_photo_client.o tcp_client.o data_link_layer.o

LIBS =

TARGET =	concurrent_photo_client

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
