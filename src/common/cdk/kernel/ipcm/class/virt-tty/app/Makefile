TARGET := virt-tty

LOCAL_CFLAGS := -fno-aggressive-loop-optimizations \
	-ffunction-sections \
	-fdata-sections \
	-Wall \
	-fno-common \
	-fno-builtin \
	-O2 \
	-Wno-implicit-function-declaration \
	-Wno-unused-variable \
	-Wno-unused-function \
	-fstack-protector \
	-Wl,-z,relro -Wl,-z,noexecstack

all:
	$(CROSS_COMPILE)gcc $(LOCAL_CFLAGS) -o $(TARGET) virt-tty_linux_app.c -lpthread
	mv $(TARGET) ${MODULE_DIR}/out/node_$(LOCAL_ID)
clean:
	-rm $(TARGET)
.PHONY: all clean

