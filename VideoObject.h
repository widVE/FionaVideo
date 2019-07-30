/*
 * VideoObject.h
 *
 *  Created on: Apr 2, 2009
 *      Author: kponto
 */

#ifndef VIDEOOBJECT_H_
#define VIDEOOBJECT_H_

/*
 *  audioPlayer.h
 *
 *  Created by Tom Wypych and Kevin Ponto for audio playback.
 *
 */

#include <queue>

#include "mediaDecoder.h"
#include "videoPlayer.h"

//#define USE_AUDIO
#ifdef USE_AUDIO
#include "audioPlayer.h"
#endif

#define MAX_FILENAME 1024

//#define USE_DECODER_THREAD


class VideoObject{
private:
	unsigned int m_Width, m_Height;
	double m_OffsetX, m_OffsetY;
	char fileName[MAX_FILENAME];

	//decoders et al
	
	mediaDecoder *m_Decoder;
	videoPlayer	 *m_VideoPlayer;
#ifdef USE_AUDIO
	audioPlayer	 *m_AudioPlayer;
#endif
	long unsigned int m_FrameNumber;
	unsigned int m_TimesLooped;

	bool m_Culled;
	
	double m_Time;

	float m_TimeMultipler;
#ifdef USE_DECODER_THREAD

#ifdef WIN32

#else
	//thread stuff
	pthread_t decodeThread;
#endif

#endif

public:
	VideoObject();
	~VideoObject();

	bool init(const char* fileName);
	bool initGL(const int numBuffers=25);

	void setOffsetX(const double d);
	void setOffsetY(const double d);

	void Draw(const bool debug=true);
	void nextFrame(const int time=-1);
	void bindFrame(int textureOffset=0);
	void unbindFrame(int textureOffset=0);

	bool intersects(float minX, float minY, float maxX, float maxY);
	bool intersects(float x, float y);

	void seek(const double time);

	void reset();
	void play();
	void pause();

	void process();
	void setPlaybackTime(double time=0);

	bool uploadFrame(const bool uploadHead=true);
	bool readyToUpload(const bool uploadHead=true);
	
	//for audio
	bool hasAudio();
	audioBufferObject* getNewAudio(const bool decode=true);

	//

	//setters
	void  setRunningType(runningType rType);
	void setCulled(bool b);
	void setFormat(const char *fmt);

	//getters
	int getWidth();
	int getHeight();
	double getFrameRate();
	bool isCulled();
	mediaDecoder * getMediaDecoderPtr();
	double getVideoTime();
	double getLoopTime();

	double getFPS();

#ifdef USE_DECODER_THREAD
	//thread stuff
	bool m_Exit;
	bool exit() {return m_Exit;}
#endif
};


#endif /* VIDEOOBJECT_H_ */
