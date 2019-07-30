/*
 *  mediaDecoder.h
 *
 *  Created by Jason Kimball.
 *
 *  Rewritten by Tom Wypych for movie playback.
 *
 *  Modified by Kevin Ponto and So Yamaoka on 7/20/08.
 *
 */

#ifndef MEDIA_DECODER
#define MEDIA_DECODER

#define NUM_BUFFERS 100
#define NUM_AUDIO_BUFFERS 20

//#include "cglX.h"
//#include "eventDefinitions.h"
#include "GlobalDefines.h"
//#include "VideoTransportReceiver.h"
#ifdef LINUX_BUILD
extern "C"{

#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif

}
#endif
extern "C"
{
// has changed with new ffmpeg -> was <ffmpeg/*.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#ifdef LINUX_BUILD
//#include <libswresample/swresample.h>
//#include <libswscale/swscale.h>
#endif
}

#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#ifdef WIN32
#include <io.h>
#include <Windows.h>
#include "gettimeofday.h"
#else
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#endif

enum	timingType	{NONE, LOCAL, NET, MANUAL, AUDIO};
enum	runningType	{video_only, video_audio, audio_only};



struct audioBufferObject
{
	int8_t*	buffer;
	int		size;
	long int pts;
};

struct seekRequestObject
{
	bool needToSeek;
	double timestep;
};

class mediaDecoder
{
public:

	mediaDecoder(void);
	mediaDecoder(char inPath[]);
	~mediaDecoder(void);

	void setNoTiming(void);
	void setLocalTiming(void);
	void setNetTiming(int port);
	void setManualTiming(void);

	void releaseFrame(void);

	int open(const char *inPath);
	int close(void);

	void play(void);
	void pause(void);

	void resetTiming(void);

	int getWidth();
	int getHeight();

	int systemReady(void);

	void setFrameComplete();
	bool isFrameComplete();

	AVFrame* getAVFrame();

	void setReadyForFrame(void);
	int timedFrameAvailable(void);

	//audio
	int audioAvailable(void);
	audioBufferObject* getAudio(const bool decode=true);
	runningType getRunning(void);
	void  setRunningType(runningType rType);
	bool isDecodingAudio();

	void decodePacket(void);

	double getTimeBaseNum(void);
	double getTimeBaseDen(void);
	int getFrameDelay(void);

	int getFrequency(void);
	int getNumChannels(void);
	int getBitsPerChannel(void);

	int getFrameNumber(void);
	int getAudioInPipe(void);
	int getNumAudioBuffers(void);

	void rewind(void);
	int reopenFile(void);
	bool hasFinishedFile();

	void setFrameRate(double d);
	void setFrameSize(int w, int h);

	int64_t getPts();
	int64_t getDts();

	int64_t getAudioPts();
	int64_t getAudioDts();

	void initGL();

	//#ifdef ALLOW_HURRY_UP
		void setHurryUp(int i);	
	//#endif

	void setDiscard(AVDiscard d);
	void seek(const double timestep=0);
	double getStartTime();
	double getDuration();
	double getFrameRate();

	void setFormat(const char *fmt);
//===================================
//
// VideoReceiver stuff 
//
//===================================
//	int init(MediaReceiverNetworkVideoIntraFrame *networkFrame);
//	bool decodeFrame(MediaReceiverNetworkFrame *networkFrame);
//===================================

private:

	int64_t pts, dts, global_video_pkt_pts;
	int64_t audio_pts, audio_dts, global_audio_pkt_pts;

	bool finishedFile;
	int		audioInPipe;
	int numAudioBuffers;

#ifdef WIN32

#else

    pthread_mutex_t mutex1;
	pthread_mutex_t m_DecoderMutex;
#endif
	int ffmpegInit(void);
	void decodeVideo(void);
	void resetState(void);
	static void *objectLauncher (void* object);

	void recordFrameOut(void);

	int time_base_den, time_base_num;
	double startTime;

	int numBuffers;
	char *path;
	uint8_t**	buffer;

	AVFormatContext *pFormatCtx;
	AVCodecContext  *pCodecCtx;
	AVCodec         *pCodec;
	AVFrame			*pFrameDecoder;
//	AVFrame         **pFrame;//, *pFrameRGB;
	AVPacket        packet;


	//to try to guess the input format
	AVInputFormat	*pInputFormat;


	//audio

	audioBufferObject*	audioBuffer;
	AVCodecContext  *aCodecCtx;
	AVCodec         *aCodec;
	audioBufferObject	sharingABO;

	int		currentAudioWriteBuffer;
	int		currentAudioReadBuffer;

//	SwsContext *img_convert_ctx;

	int             frameFinished;
	int 		frameComplete;
	int		videoStream;
	int             numBytes;
	int		status;
	int		frameDelay;

	int		height;
	int		width;

	int		moviePlaying;
	int		audioStream;

	int		shutDownFlag;
	int		releaseTokens;

	timingType	timing;
	runningType	running;
	unsigned long	markTime;
	bool		firstRun;
	bool 		readyForFrame;
	int		interlaced;

	bool needToRewind;
	seekRequestObject seekRequest;
#ifdef WIN32
#else
	pthread_t thread;
#endif

};

#endif
