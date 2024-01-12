all:
	@cd rtsp_demo; make
	@cd rtsp_server; make
	@cd backchannel_client; make
	# @cd mp4_demuxer; make
	# @cd mp4_muxer; make
	#@cd audio_mix_demo; make
	@cd kplayer_demo; make
	@cd ws_discovery; make
	@cd rest_rpc; make
	@cd comm; make
	@ if [ -d tuning-server ]; then cd tuning-server && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release &&\
	cmake --build build -j &&\
	cp build/t_frameworks/t_server_c/t_server_c-6.1.0 ../../out/little/; fi

clean:
	@cd rtsp_demo; make clean
	@cd rtsp_server; make clean
	@cd backchannel_client; make clean
	# @cd mp4_demuxer; make clean
	# @cd mp4_muxer; make clean
	#@cd audio_mix_demo; make clean
	@cd kplayer_demo; make clean
	@cd ws_discovery; make clean
	@cd rest_rpc; make clean
	@cd comm; make clean
	@cd tuning-server && rm -rf build
