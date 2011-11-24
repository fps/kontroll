COMPILE_FLAGS = -Wall `pkg-config alsa libglade-2.0 gtk+-2.0 liblo --cflags`
#COMPILE_FLAGS += -g -O2 
COMPILE_FLAGS += -O3


LINK_FLAGS    = `pkg-config alsa libglade-2.0 gtk+-2.0 liblo --libs ` -export-dynamic

#if you do not have LASH support, comment out the next two line
COMPILE_FLAGS +=  `pkg-config --cflags lash-1.0` 
LINK_FLAGS += `pkg-config --libs lash-1.0` 


TARGET = kontroll
PREFIX = /usr/local

STUFF = kontroll
OBJECTS = $(STUFF:%=%.o)
SOURCES = $(STUFF:%=%.cc)
HEADERS = $(STUFF:%=%.h)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS) $(LINK_FLAGS)

$(OBJECTS): %.o: %.cc $(SOURCES) $(HEADERS)
	$(CXX) -c $< $(COMPILE_FLAGS) -DPREFIX=\"$(PREFIX)\"

.PHONY: clean
clean:
	rm -f $(TARGET) *~ *.o  core*

.PHONY: install
install: $(TARGET)
	install $(TARGET) $(PREFIX)/bin/
	install -d $(PREFIX)/share/kontroll/
	install $(TARGET).glade $(PREFIX)/share/kontroll
