CC=g++
LD = g++

SRCDIR = src
BINDIR = bin
OBJDIR = obj

LIBS   = `pkg-config opencv4 --cflags --libs` -lpthread
CFLAGS  += -Wall -Wextra -pedantic -Wno-unused-parameter -c $(LIBS)
LDFLAGS += $(LIBS)

ifeq ($(DEBUG),1)
CFLAGS += -O0 -no-pie -g -pg -DDEBUG
LDFLAGS += -O0 -no-pie -g -pg -DDEBUG
else
CFLAGS += -O3 -flto -DRELEASE
LDFLAGS += -O3 -flto -DRELEASE
endif

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
HEADERS = $(wildcard $(SRCDIR)/*.h)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

EXECUTABLE = take_photo
TARGET = $(BINDIR)/$(EXECUTABLE)

EXECUTABLE2 = equirectangularConversionTop
TARGET2 = $(BINDIR)/$(EXECUTABLE2)

EXECUTABLE3 = equirectangularConversion
TARGET3 = $(BINDIR)/$(EXECUTABLE3)

EXECUTABLE4 = equiCam
TARGET4 = $(BINDIR)/$(EXECUTABLE4)

all: $(SOURCES) $(TARGET) $(TARGET2) $(TARGET3) $(TARGET4)

$(TARGET): $(filter-out obj/$(EXECUTABLE4).o obj/$(EXECUTABLE2).o obj/$(EXECUTABLE3).o,$(OBJECTS)) | $(BINDIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(TARGET2): obj/$(EXECUTABLE2).o | $(BINDIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(TARGET3): $(filter-out obj/$(EXECUTABLE4).o obj/$(EXECUTABLE2).o obj/$(EXECUTABLE).o,$(OBJECTS)) | $(BINDIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(TARGET4): $(filter-out obj/$(EXECUTABLE).o obj/$(EXECUTABLE2).o obj/$(EXECUTABLE3).o,$(OBJECTS)) | $(BINDIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ $<

$(BINDIR) $(OBJDIR):
	mkdir -p $@

module = uvcvideo
module:
	lsmod | grep $(module) &>/dev/null || sudo modprobe $(module)

run: module $(TARGET)
	$(TARGET) ${ARGS}

clean:
	rm -rf $(BINDIR) $(OBJDIR)

.PHONY:
	clean run all

