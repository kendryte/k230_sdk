THIRDPARTY_ROOT=$(CDK_SRC_DIR)/user/thirdparty
MW_ROOT=$(CDK_SRC_DIR)/user/middleware

CC=riscv64-unknown-linux-gnu-g++
LIBS=-lpthread
LIBPATH+=-L/usr/local/lib
CFLAGS=-DENABLE_SSL=OFF -DMSGPACK_NO_BOOST -DASIO_STANDALONE -Dunix

RPC_SERVER_BIN=$(CDK_SRC_DIR)/user/out/little/rpc_server

#add common include
INCS=-I./ -I/usr/local/include

#add 3rdparty common include
INCS+=-I$(THIRDPARTY_ROOT)/asio
INCS+=-I$(THIRDPARTY_ROOT)/msgpack/include
INCS+=-I$(THIRDPARTY_ROOT)/rpc


# rpc_server
RPC_SERVER_SRCS= ./sample_rpc_server.cpp ./comm_rpc_server.cpp
SERVER_COBJS:=$(RPC_SERVER_SRCS:.cpp=.o)
SERVER_BIN=$(CDK_SRC_DIR)/user/out/little/rpc_server

#rpc client
RPC_CLIENT_SRCS= ./sample_rpc_client.cpp ./comm_rpc_client.cpp
CLIENT_COBJS:=$(RPC_CLIENT_SRCS:.cpp=.o)
CLIENT_BIN=$(CDK_SRC_DIR)/user/out/little/rpc_client

all:$(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_COBJS) : %.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)

$(SERVER_BIN):$(SERVER_COBJS)
	$(CC) -o $(SERVER_BIN) $(SERVER_COBJS) $(LIBPATH) -Wl,--start-group $(LIBS) -Wl,--end-group
	rm -f  $(SERVER_COBJS)

$(CLIENT_COBJS) : %.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)

$(CLIENT_BIN):$(CLIENT_COBJS)
	$(CC) -o $(CLIENT_BIN) $(CLIENT_COBJS) $(LIBPATH) -Wl,--start-group $(LIBS) -Wl,--end-group
	rm -f  $(CLIENT_COBJS)


clean:
	rm -f $(SERVER_BIN) $(SERVER_COBJS) $(CLIENT_BIN) $(CLIENT_COBJS)
