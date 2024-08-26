#pragma once
#include <pthread.h>
#include <semaphore.h>
//
class AVStream;
class AVFormatContext;
class AVCodecContext;
class RTSPPusherImpl {
public:
	RTSPPusherImpl();

	~RTSPPusherImpl();

	int  init(const char* url, int video_width, int video_height);
	int  pushVideo(char* pBuf, int nLen, bool bKey, unsigned long long nTimeStamp);
	int  open();
	void close();

	void setSPSPPS(char* pSPSBuf, int nSPSLen, char* pPPSBuf, int nPPSLen);
	void setSPSPPS_EX(char* pSPSPPSBuf,int nLen);
protected:
	static void* ReconnectThread(void* pParam);
	void _DoReconnect();

private:
	int  _reInit();
	int  _openVideoOutput();
	int  _ConnectRtspServer();
	int  _OpenRtspStreams();
	void _CloseRtspStreams();
	void _CloseRtspPusher();

private:
	char m_sUrl[256];
	int m_nVideoWidth;
	int m_nVideoHeight;
	AVFormatContext* outputContext;
	AVCodecContext * videoCodecContext;
	AVStream* videoStream = nullptr;
	AVStream* audioStream = nullptr;

	unsigned long long m_lVTimeStamp;
	unsigned long long m_lATimeStamp;

	char m_pSPSPPS[1024];
	int m_nSPSPPSLen;
	bool m_bInited;
	bool m_bNeedIframe;
	int m_nfps;

	int m_nPushFrameFailCnt;

	sem_t m_sConnect;
	pthread_mutex_t m_Lock;
	pthread_t m_hConnectThread;
	static bool  m_bFfmpegInit;
};
