BIN:=mqtt_sub

.PHONY: all clean

CFLAGS += -Wall -std=gnu99
LDFLAGS := -luci -lmosquitto -lsqlite3

all: $(BIN)

$(BIN): mqtt_sub.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(BIN)