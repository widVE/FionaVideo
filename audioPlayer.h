/*
 *  audioPlayer.h
 *
 *  Created by Tom Wypych and Kevin Ponto for audio playback.
 *
 */

#ifdef USE_AUDIO

#ifndef AUDIO_PLAYER
#define AUDIO_PLAYER

//#include <cglX.h>
#include <queue>

#include "AL/al.h"
#include "AL/alc.h"
#include "mediaDecoder.h"
//#include "eventDefinitions.h"

#define NUM_AUDIO_BUFFERS              (20)
#define	SERVICE_UPDATE_PERIOD	(5000)


class audioPlayer 
{
private:
	ALCcontext *AudioContext;
	unsigned long	ulFrequency;
	unsigned long	ulFormat;
	unsigned int bitsPerChannel;
	unsigned int numChannels;

	ALint		iBuffersProcessed;
	ALint		iState;
	ALint		iQueuedBuffers;

	ALuint		uiBuffer;
	ALuint		uiBuffers[NUM_AUDIO_BUFFERS];
	ALuint		uiSource;
	ALboolean AudioInit (int *argcp, char **argv);
	
	//used to enqueue the first x buffers
	ALuint firstBufferFill;
	ALfloat pos;
	double audioPlayedSec;
	double audioBufferOffsetSec;
	double timePerBuffer[NUM_AUDIO_BUFFERS];

	std::queue<ALuint> freeBufferQueue;
	std::queue<int64_t> ptsBufferQueue;
	double timePastPerIteration;

	int64_t m_pts;

	double audioOffsetTime;


public:
	audioPlayer(void);
	~audioPlayer(void);

	ALboolean SanityCheck (void);
	ALboolean AudioExit (void);
	
	void reset(void);
	void play(void);
	void pause(void);

	bool init(unsigned long	ulFrequency, unsigned int bitsPerChannel, unsigned int numChannels);
	bool isPlaying(void);
	bool isReadyForAudio();
	bool bufferFilled();

	void addAudioToBuffer(audioBufferObject *myABO);
	bool removeAudioFromBuffer();

	int64_t getPtsValue();
	
	double getAudioTime();
	void addTimeOffset(double time);
	void printPtsBufferQueue();

	void setVolume(float f);
	float getVolume();
};

#endif
#endif