DEPS = $(wildcard *.h)
SRC = $(wildcard src/*.c)
OBJ = $(patsubst %.c, %.o, $(SRC))
TARGET = mqtt_sub
LDFLAGS+=-luci -lmosquitto -lsqlite3 -lcurl 
RM = rm -f

all: $(TARGET)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	$(RM) $(OBJ) $(TARGET)
