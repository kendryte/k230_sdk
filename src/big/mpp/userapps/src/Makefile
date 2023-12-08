all:
	@cd sensor; make || exit 1
	@cd connector; make || exit 1
	@cd fft; make || exit 1
	@cd cipher; make || exit 1
	@cd pm; make || exit 1
clean:
	@cd sensor; make clean
	@cd fft; make clean
	@cd cipher; make clean
	@cd connector; make clean
	@cd pm; make clean