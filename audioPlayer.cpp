/*
 *  audioPlayer.h
 *
 *  Created by Tom Wypych and Kevin Ponto for audio playback.
 *
 */

#ifdef USE_AUDIO
#include "audioPlayer.h"

#define DEBUG_MESSAGE ;

//code pos was defined in CGLX.  Just zero it for now

#define CODEPOS 0

#ifdef LINUX_BUILD
static pthread_mutex_t audioMutex = PTHREAD_MUTEX_INITIALIZER;
#else
static CRITICAL_SECTION audioMutex;
#endif

static enum
{
  Unintialized,                 /* ALUT has not been initialized yet or has been de-initialised */
  AudioDeviceAndContext,         /* alutInit has been called successfully */
  ExternalDeviceAndContext      /* alutInitWithoutContext has been called */
} initialisationState = Unintialized;

audioPlayer::audioPlayer(void)
{
#ifndef LINUX_BUILD
	InitializeCriticalSection(&audioMutex);
#endif

}

audioPlayer::~audioPlayer(void)
{
	reset();
	alDeleteBuffers( NUM_AUDIO_BUFFERS, uiBuffers );
	alDeleteSources (1, &uiSource);
}


bool audioPlayer::init(unsigned long ulFrequency,
			unsigned int bitsPerChannel,
			unsigned int numChannels)
{
	printf("Creating audio player\n");

	this->ulFrequency = ulFrequency;
	this->ulFormat = AL_FORMAT_STEREO16;
	this->bitsPerChannel = bitsPerChannel;
	this->numChannels = numChannels;

	if (this->ulFrequency == 0)
	{
		printf("*************** WARNING *****************\n");	
		printf("* Detected frequency of %08f        *\n", this->ulFrequency);
		printf("* Assuming this is an error.            *\n");  
		printf("* switching back to 4800 kHz            *\n");	
		printf("*****************************************\n");	
		this->ulFrequency = 48000;

	}
	


	printf("	%d channels - %d bits per channel Audio at %d kHz\n",
		this->numChannels, this->bitsPerChannel, this->ulFrequency);

	//we will probably never have 8 bits per channel
	//but just in case
	if(this->bitsPerChannel == 8)
	{

	    if(this->numChannels == 1) ulFormat = AL_FORMAT_MONO8;
	    else if(this->numChannels == 2) ulFormat = AL_FORMAT_STEREO8;
	    else if(alIsExtensionPresent("AL_EXT_MCFORMATS"))
	    {
	        if(this->numChannels == 4) ulFormat = alGetEnumValue("AL_FORMAT_QUAD8");
	        if(this->numChannels == 6) ulFormat = alGetEnumValue("AL_FORMAT_51CHN8");
	    }
	}
	else if(this->bitsPerChannel == 16)
    	{
        	if(this->numChannels == 1) ulFormat = AL_FORMAT_MONO16;
	        else  if(this->numChannels == 2) ulFormat = AL_FORMAT_STEREO16;
        	else if(alIsExtensionPresent("AL_EXT_MCFORMATS"))
		{
			if(this->numChannels == 4) ulFormat = alGetEnumValue("AL_FORMAT_QUAD16");
			if(this->numChannels == 6) ulFormat = alGetEnumValue("AL_FORMAT_51CHN16");
	        }
		else
		{
			//tell the user the extension is missing
			//but try to do multichannel sound anyway
			printf("alIsExtension AL_EXT_MCFORMATS missing\n");
		
			if(this->numChannels == 4) ulFormat = alGetEnumValue("AL_FORMAT_QUAD16");
            		if(this->numChannels == 6) ulFormat = alGetEnumValue("AL_FORMAT_51CHN16");
			
		}		
	}

	//init everything
	bool HasAudio = AudioInit(0, NULL);
	
	if (!HasAudio)
	{
		printf("No Audio Detected on this computer\n");
		printf("Switching to Video Only Mode\n");
		//change the mode on the player
		//myDecoder->setRunningType(video_only);
		return false;
	}
	
	alGenBuffers( NUM_AUDIO_BUFFERS, uiBuffers );
	alGenSources (1, &uiSource);

	audioBufferObject *myABO = NULL;

	this->reset();

	printf("Audio Inits Finished\n");

	return true;
}	

void audioPlayer::addTimeOffset(double time)
{
	audioBufferOffsetSec += time;
}

double audioPlayer::getAudioTime(void)
{
	// get the current position in buffer	
	alGetSourcef(uiSource, AL_SEC_OFFSET,  &pos);
	// add that to the previous time
	//audioPlayedSec = audioBufferOffsetSec + pos; 
	
	//add offset time as well
	audioPlayedSec = audioBufferOffsetSec + pos + audioOffsetTime;
	return audioPlayedSec;
}


bool audioPlayer::isPlaying(void)
{
	alGetSourcei(uiSource, AL_SOURCE_STATE, &iState);
	return (iState == AL_PLAYING);
}

bool audioPlayer::bufferFilled()
{
	return freeBufferQueue.size()<3;
}

void audioPlayer::reset()
{
#ifdef LINUX_BUILD
	pthread_mutex_lock( &audioMutex );
#else
	EnterCriticalSection(&audioMutex);
#endif
	//we can refill the buffers from 0
	firstBufferFill=0;
	audioPlayedSec=0;
	audioBufferOffsetSec=0;
	audioOffsetTime =0;

	//clear the buffers
	alSourceStop(uiSource);
	//alSourcei(uiSource, AL_BUFFER, 0);

	//unqueue these buffers
	alGetSourcei(uiSource, AL_BUFFERS_PROCESSED, &iBuffersProcessed);
	for (int i =0; i < iBuffersProcessed; i++)
		alSourceUnqueueBuffers(uiSource, 1, &uiBuffer);


	// cleaning up.
//	alSourceUnqueueBuffers(uiSource, NUM_AUDIO_BUFFERS, uiBuffers);
	while(!freeBufferQueue.empty())
		freeBufferQueue.pop();

	//clear pts buffers
	while(!ptsBufferQueue.empty())
		ptsBufferQueue.pop();

	timePastPerIteration = 0.0;
//	fprintf(stdout, "%s:%d: freeBufferQueue.size() = %d\n",
//		__FILE__, __LINE__, freeBufferQueue.size());

#ifdef LINUX_BUILD
	pthread_mutex_unlock( &audioMutex );
#else
	LeaveCriticalSection(&audioMutex);
#endif
	DEBUG_MESSAGE("done audio reset\n");
}

void audioPlayer::play()
{
	DEBUG_MESSAGE("play with %d buffers\n", NUM_AUDIO_BUFFERS - freeBufferQueue.size());
	 alSourcePlay (uiSource);
}

void audioPlayer::pause()
{
	 alSourcePause (uiSource);
}

ALboolean audioPlayer::SanityCheck (void)
{
	ALCcontext *context;

	if (initialisationState == Unintialized)
	{
		printf("%s [%d]: %s\n",CODEPOS,"Uninitialized AL Context");
		return AL_FALSE;
	}

	context = alcGetCurrentContext ();
	if (context == NULL)
	{
		printf("%s [%d]: %s\n",CODEPOS,"No current AL Context");
		return AL_FALSE;
	}

	if (alGetError () != AL_NO_ERROR)
	{
		printf("%s [%d]: %s\n",CODEPOS,"AL error on entry");
		return AL_FALSE;
	}

	if (alcGetError (alcGetContextsDevice (context)) != ALC_NO_ERROR)
	{
		printf("%s [%d]: %s\n",CODEPOS,"AL error on alcGetContextDevice()");
		return AL_FALSE;
	}

	return AL_TRUE;
}


ALboolean audioPlayer::AudioInit (int *argcp, char **argv)
{
	ALCdevice *device;
	ALCcontext *context;

	if (initialisationState != Unintialized)
	{
	    printf("%s [%d]: %s\n",CODEPOS,"AL Error on Initialization (initialisationState != Unintialized)");
	    return AL_FALSE;
	}

	if ((argcp == NULL) != (argv == NULL))
	{
	    printf("%s [%d]: %s\n",CODEPOS,"ERROR_INVALID_VALUE");
	    return AL_FALSE;
	}

	device = alcOpenDevice (NULL);
	if (device == NULL)
	{
	    printf("%s [%d]: %s\n",CODEPOS,"ERROR_OPEN_DEVICE");
	    return AL_FALSE;
	}

	context = alcCreateContext (device, NULL);
	if (context == NULL)
	{
	    alcCloseDevice (device);
	    printf("%s [%d]: %s\n",CODEPOS,"ERROR_CREATE_CONTEXT");
	    return AL_FALSE;
	}

	if (!alcMakeContextCurrent (context))
	{
	    alcDestroyContext (context);
	    alcCloseDevice (device);
	    printf("%s [%d]: %s\n",CODEPOS,"ERROR_MAKE_CONTEXT_CURRENT");
	    return AL_FALSE;
	}

	initialisationState = AudioDeviceAndContext;
	AudioContext = context;
	return AL_TRUE;
}


ALboolean audioPlayer::AudioExit (void)
{
	ALCdevice *device;

	if (initialisationState == Unintialized)
	{
		printf("%s [%d]: %s\n",CODEPOS,"ERROR_INVALID_OPERATION");
		return AL_FALSE;
	}

	if (initialisationState == ExternalDeviceAndContext)
	{
		initialisationState = Unintialized;
		return AL_TRUE;
	}

	if (!SanityCheck ())
	{
		return AL_FALSE;
	}

	if (!alcMakeContextCurrent (NULL))
	{
		printf("%s [%d]: %s\n",CODEPOS,"ERROR_MAKE_CONTEXT_CURRENT");
		return AL_FALSE;
	}

	device = alcGetContextsDevice (AudioContext);
	alcDestroyContext (AudioContext);
	if (alcGetError (device) != ALC_NO_ERROR)
	{
		printf("%s [%d]: %s\n",CODEPOS,"ERROR_DESTROY_CONTEXT");
		return AL_FALSE;
	}

	if (!alcCloseDevice (device))
	{
		printf("%s [%d]: %s\n",CODEPOS,"ERROR_CLOSE_DEVICE");
		return AL_FALSE;
	}

	initialisationState = Unintialized;
	return AL_TRUE;
}


bool audioPlayer::isReadyForAudio()
{

	if(firstBufferFill<NUM_AUDIO_BUFFERS)
	{
#ifdef LINUX_BUILD
	pthread_mutex_lock( &audioMutex );
#else
	EnterCriticalSection(&audioMutex);
#endif
		freeBufferQueue.push(uiBuffers[firstBufferFill]);
#ifdef LINUX_BUILD
	pthread_mutex_unlock( &audioMutex );
#else
	LeaveCriticalSection(&audioMutex);
#endif
		firstBufferFill++;
		//printf("check buffer Queue %d\n", freeBufferQueue.size());
		return false;
	}

	DEBUG_MESSAGE("check buffer Queue %d\n", freeBufferQueue.size());
	//fprintf(stdout, "%s:%d: freeBufferQueue.empty() = %d\n",
	//	__FILE__, __LINE__, freeBufferQueue.empty());
#ifdef LINUX_BUILD
	pthread_mutex_lock( &audioMutex );
#else
	EnterCriticalSection(&audioMutex);
#endif
	bool returnVal = !freeBufferQueue.empty();
	
	//bool returnVal = freeBufferQueue.size();

#ifdef LINUX_BUILD
	pthread_mutex_unlock( &audioMutex );
#else
	LeaveCriticalSection(&audioMutex);
#endif
	
	return returnVal;
}

bool audioPlayer::removeAudioFromBuffer()
{
	bool retValue = false;
#ifdef LINUX_BUILD
	pthread_mutex_lock( &audioMutex );
#else
	EnterCriticalSection(&audioMutex);
#endif
	// iBuffersProcessed cleared, query again.
	alGetSourcei(uiSource, AL_BUFFERS_PROCESSED, &iBuffersProcessed);

	if(iBuffersProcessed>0)
	{
		ALuint aid;
		alSourceUnqueueBuffers(uiSource, 1, &aid);
		freeBufferQueue.push(aid);

		DEBUG_MESSAGE("remove buffer Queue %d\n", freeBufferQueue.size());

		ptsBufferQueue.pop();
		m_pts = ptsBufferQueue.front();
		
		retValue = true;

		audioBufferOffsetSec += timePastPerIteration;

		double audioDiff = ((this->getPtsValue()*.001)-audioBufferOffsetSec);
		//if (fabs(audioDiff) > .0001)
		//this->addTimeOffset(audioDiff);
		audioOffsetTime=audioDiff;

		//printf("    diff time %f - %f  = %f\n", audioBufferOffsetSec, this->getPtsValue()*.001, audioBufferOffsetSec -this->getPtsValue()*.001);
		//printf("  audiotime =  %f = %f + %f + %f\n", this->getAudioTime(), audioBufferOffsetSec, audioOffsetTime , 
		//				this->getAudioTime() - audioBufferOffsetSec - audioOffsetTime);
		

//		fprintf(stdout, "%s:%d: freeBufferQueue.size(), audioBufferOffsetSec = %d, %f\n",
//			__FILE__, __LINE__, freeBufferQueue.size(), audioBufferOffsetSec);
	}
#ifdef LINUX_BUILD
	pthread_mutex_unlock( &audioMutex );
#else
	LeaveCriticalSection(&audioMutex);
#endif
	return retValue;
}


void audioPlayer::addAudioToBuffer(audioBufferObject *myABO)
{
	if (myABO == NULL)
		return;

#ifdef LINUX_BUILD
	pthread_mutex_lock( &audioMutex );
#else
	EnterCriticalSection(&audioMutex);
#endif
	if(!freeBufferQueue.empty())
	{
		uiBuffer = freeBufferQueue.front();
		freeBufferQueue.pop();
		DEBUG_MESSAGE("add buffer Queue %d\n", freeBufferQueue.size());
			
		ptsBufferQueue.push(myABO->pts);

		alBufferData(uiBuffer, ulFormat, myABO->buffer, myABO->size, ulFrequency);
		alSourceQueueBuffers(uiSource, 1, &uiBuffer);

		static double BPS = ulFrequency * numChannels * 2.0;
		timePastPerIteration = myABO->size / BPS;

		DEBUG_MESSAGE("buffer Queue %d\n", freeBufferQueue.size());
		DEBUG_MESSAGE("    diff time %f - %f  = %f\n", audioBufferOffsetSec, this->getPtsValue()*.001, audioBufferOffsetSec -this->getPtsValue()*.001);
		DEBUG_MESSAGE("  audiotime =  %f = %f + %f + %f\n", this->getAudioTime(), audioBufferOffsetSec, audioOffsetTime , 
						this->getAudioTime() - audioBufferOffsetSec - audioOffsetTime);
		
//		fprintf(stdout, "myABO->size = %d, BPS = %f\n", myABO->size, BPS);
//		fprintf(stdout, "%s:%d: freeBufferQueue.size() queued = %d, timePastPerIteration = %f\n",
//			__FILE__, __LINE__, freeBufferQueue.size(), timePastPerIteration);
	}
	else
	{
		fprintf(stdout, "%s:%d: freeBufferQueue.size() = %d\n",
			__FILE__, __LINE__, freeBufferQueue.size());
	}
#ifdef LINUX_BUILD
	pthread_mutex_unlock( &audioMutex );
#else
	LeaveCriticalSection(&audioMutex);
#endif
}

int64_t audioPlayer::getPtsValue()
{
	//fprintf(stderr, "PTS: %Ld %d\n", m_pts, INT_MAX +int (m_pts%(long int)INT_MAX));
	//int r = (m_pts%INT_MAX);
	//int r = (m_pts & 0x0000ffff);
	//return (r< 0) ? INT_MAX + r: r;
	return m_pts;
}

void audioPlayer::printPtsBufferQueue()
{
	for (int i=0; i < ptsBufferQueue.size(); i++)
	{
		DEBUG_MESSAGE("%d: audio pts %f\n", i, ptsBufferQueue.front()*.001);
		ptsBufferQueue.push(ptsBufferQueue.front());
		ptsBufferQueue.pop();
	}
}

void audioPlayer::setVolume(float f)
{
	alSourcef(uiSource, AL_GAIN, f);
}

float audioPlayer::getVolume()
{
	float f;
	alGetSourcef(uiSource, AL_GAIN, &f);
	return f;
}

#endif