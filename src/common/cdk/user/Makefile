all:
	@mkdir -p out/big/
	@mkdir -p out/little/
	@cd component/ipcmsg; make
	@cd component/datafifo; make
	@cd mapi; make
	@cd thirdparty; make
	@cd samples; make

clean:
	@cd mapi; make clean
	@cd component/ipcmsg; make clean
	@cd component/datafifo; make clean
	@cd thirdparty; make clean
	@cd samples; make clean
