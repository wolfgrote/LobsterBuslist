CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		buslist.o

LIBS =

TARGET =	buslist

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
