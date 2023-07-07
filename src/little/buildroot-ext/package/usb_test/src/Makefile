
OBJ ?= usb_test

$(OBJ): testusb.c
	$(CC) $(CFLAGS) $^ -L ./ -lpthread -Wl,-Bdynamic -g -o $@

	$(CC) $(CFLAGS) test_keyboard.c -Wl,-Bdynamic -g -o test_keyboard

	$(CC) $(CFLAGS) test_mouse.c -Wl,-Bdynamic -g -o test_mouse

	$(CC) $(CFLAGS) hid_gadget_test.c -Wl,-Bdynamic -g -o hid_gadget_test
clean:
	rm -f *.o
	rm -f *.d
	rm -f $(OBJ)

