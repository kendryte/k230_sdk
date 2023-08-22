
OBJ ?= ota

$(OBJ): ota.c rsa_sign.c
	$(CC) $(CFLAGS) $^ -L ./ -lpthread -lcrypto -Wl,-Bdynamic -g -o $@

clean:
	rm -f *.o
	rm -f *.d
	rm -f $(OBJ)

