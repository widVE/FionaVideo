/*
 * VideoObject.cpp
 *
 *  Created on: Apr 2, 2009
 *      Author: kponto
 */

#include <GL/glew.h>
#include <stdio.h>
#include "VideoObject.h"

#define DEBUG_MSG ;//(args ...) ;
//fprintf(stderr, ## args)

#ifdef WIN32
#define usleep Sleep
#endif

#ifdef USE_DECODER_THREAD

static void *decoderFunc (void* object)
{
	VideoObject *v = (VideoObject*) object;
	if (!v)
		return 0;
	mediaDecoder *m =  v->getMediaDecoderPtr();
	while(!v->exit())
	{
		m->decodePacket();
		usleep(10);
	}
	
	return 0;
}
#endif

VideoObject::VideoObject()
{
	m_Decoder = new mediaDecoder();
	m_VideoPlayer = 0;
	
	m_FrameNumber=0;
	m_TimesLooped=0;
	m_TimeMultipler=1;
#ifdef USE_DECODER_THREAD
	m_Exit=false;
	pthread_create(&decodeThread, NULL, &decoderFunc, this);
#endif
}
VideoObject::~VideoObject()
{
#ifdef USE_DECODER_THREAD
	m_Exit=true;
	printf("joining thread\n");
	pthread_join(decodeThread, NULL);
#endif
	printf("removing: %s\n", fileName);
	delete m_Decoder;
	if (m_VideoPlayer)
		delete m_VideoPlayer;
#ifdef USE_AUDIO
	if(m_AudioPlayer)
		delete m_AudioPlayer;
#endif
}


void VideoObject::reset()
{
	m_FrameNumber=0;
	m_TimesLooped=0;
	//m_Decoder->rewind();
	m_Decoder->seek(0);
	if (m_VideoPlayer)
		m_VideoPlayer->reset();
#ifdef USE_AUDIO
	if(m_AudioPlayer)
		m_AudioPlayer->reset();
#endif
}

void VideoObject::seek(const double time)
{
	double dur = m_Decoder->getDuration();
	if (time< 0)
		m_Decoder->seek(0);
	else if (time < dur)
		m_Decoder->seek(time);
	else
	{
		printf("********************  LOOOP ********* \n");
		//we have looped the movie, so we need to account for this
		m_TimesLooped = (int)time / dur;
		m_Decoder->seek(time - (double(m_TimesLooped)*dur));
	}

	if (m_VideoPlayer)
		m_VideoPlayer->reset();



	//numFramesPlayed = 0;
	//setMovieState(BUFFERING);

	//set the status to buffering


	m_Time=time;
}

void VideoObject::play()
{

}

void VideoObject::pause()
{

}


bool VideoObject::init(const char* fileName)
{
	//copy the name
	strcpy(this->fileName, fileName);

	int error =  (m_Decoder->open(fileName));
	
	//can we load this file
	if (error == 1)
	{
		printf("load: %s\n", fileName);
#ifdef USE_AUDIO
		m_AudioPlayer = new audioPlayer();
		m_AudioPlayer->init(m_Decoder->getFrequency(), m_Decoder->getBitsPerChannel(), m_Decoder->getNumChannels());
#endif
		return true;
	}

	printf("error loading: %s\n", fileName);
	//else there is an error
	return false;
}

bool VideoObject::initGL(const int numBuffers)
{

	//make the videoPlayer
	if (numBuffers > 0)
	{
		m_VideoPlayer = new videoPlayer();
		DEBUG_MSG("video size %d %d\n", m_Decoder->getWidth(), m_Decoder->getHeight());
		m_VideoPlayer->init(m_Decoder->getWidth(), m_Decoder->getHeight(), numBuffers, false);
	}
	//m_Decoder->setRunningType(video_only);
	
	m_Width = m_Decoder->getWidth();
	m_Height = m_Decoder->getHeight();

	//ready for the first frame
	m_Decoder->initGL();
	m_Decoder->setReadyForFrame();

	return true;
}

void VideoObject::bindFrame(int textureOffset)
{
	m_VideoPlayer->bind(textureOffset);
}
void VideoObject::unbindFrame(int textureOffset)
{
	m_VideoPlayer->unbind(textureOffset);
}

void VideoObject::Draw(const bool debug)
{
	if (!m_VideoPlayer)
		return;

	glPushMatrix();

	glTranslated(m_OffsetX, m_OffsetY, 0);
	m_VideoPlayer->drawFrame();
	if(debug)
		m_VideoPlayer->drawDebug();

	glPopMatrix();
}

bool VideoObject::intersects(float minX, float minY, float maxX, float maxY)
{
	//printf("is x => %f %f and %lf %lf\n", minX, maxX, m_OffsetX, m_OffsetX+m_Width);
	if ((maxX >= m_OffsetX) && (minX <= (m_OffsetX+m_Width)) &&
	    (maxY >= m_OffsetY) && (minY <= (m_OffsetY+m_Height))
	)
		return true;
	return false;
}

bool VideoObject::intersects(float x, float y)
{
	//printf("is x => %f %f and %lf %lf\n", minX, maxX, m_OffsetX, m_OffsetX+m_Width);
	if ((x >= m_OffsetX) && (x <= (m_OffsetX+m_Width)) &&
	    (y >= m_OffsetY) && (y <= (m_OffsetY+m_Height))
	)
		return true;
	return false;
}

void VideoObject::nextFrame(const int time)
{
//	if (m_VideoPlayer)
//				printf("*******************  SOMETHING IS WRONG...STILL HAVE VIDEO PLAYER %d ************************\n", m_VideoPlayer);

	
	double dTime = time*.001*m_TimeMultipler; 
	double duration = m_Decoder->getDuration();
	int n = int(dTime /m_Decoder->getDuration());

	//printf("time = %d\n", time);

	//pseudo mod
	dTime -= duration*n;

	if (m_VideoPlayer)
	{
		int dur = int(duration*1000);
		if (dur > 0)
			m_VideoPlayer->nextFrame(int(time*m_TimeMultipler) % dur);
		else
		//lets assume we are streaming, so just go to the next frame
			m_VideoPlayer->nextFrame(-1);
	//m_VideoPlayer->nextFrame((time +m_Decoder->getStartTime()) 
		//if (abs(m_VideoPlayer->getCurrentPtsValue() - (time*1000)) > 2000)
		//	this->seek(time*.001);
	}

	m_FrameNumber++;

#ifdef SEEK_WHEN_OFF
	if (m_VideoPlayer)
	{
		//try doing once per second??
		
		if (int(dTime) != int(m_Time))
		{
			//try to see if we are way off
			double videoTime = m_VideoPlayer->getCurrentPtsValue()*.001;
			
			//if we are off by more than three seconds
			if (videoTime)
			if (fabs(videoTime - dTime) > SEEK_WHEN_OFF_BY_SEC)
			{
				//to prevent this from happening all of the time

				printf("video off: %f != %f\n", videoTime, dTime);
				//seek time + .25 second
				this->seek(dTime+.25);
			}
		}
	}
#endif

	//try to auto loop
	/*if ((m_Time <= duration)
		&& (dTime > duration)
		)*/
	//if (dTime < m_Time - 1)
	//	this->seek(0);

	if (m_Decoder->hasFinishedFile())
		this->seek(0);

	//printf("%f %f %f\n", dTime, m_Time, duration);

	m_Time=dTime;

	DEBUG_MSG("Frame %d\n", m_FrameNumber);
}


bool VideoObject::readyToUpload(const bool uploadHead)
{

	if (m_Decoder->isFrameComplete() && m_VideoPlayer && m_VideoPlayer->isReadyForFrame())
		return true;
	else
		return false;
}

audioBufferObject* VideoObject::getNewAudio(const bool decode)
{

	if (m_Decoder->getAudioInPipe())
		return m_Decoder->getAudio(decode);
#ifdef USE_AUDIO
	if (!m_AudioPlayer) 
		return false;

	//printf("audio time = %f\n", m_AudioPlayer->getAudioTime());

	//fprintf(stdout, "%s:%d: audio in pipe = %d\n",
	//			__FILE__, __LINE__, m_Decoder->getAudioInPipe());

	//printf("audio in pipe %d ready? %d \n", m_Decoder->getAudioInPipe(), m_AudioPlayer->isReadyForAudio());
	
	if (m_Decoder->getAudioInPipe() && m_AudioPlayer->isReadyForAudio())
	{
			fprintf(stdout, "%s:%d: audio in pipe = %d\n",
				__FILE__, __LINE__, m_Decoder->getAudioInPipe());
			m_AudioPlayer->addAudioToBuffer(m_Decoder->getAudio());
	}
		
	//if (m_AudioPlayer->removeAudioFromBuffer())
			//ave differences
	//	gPtsDiff = (gPtsDiff*.2) + .8*(m_AudioPlayer->getPtsValue() - m_VideoPlayer->getCurrentPtsValue());
#endif
	
	return 0;
}

//********************************
//
//  If a frame is ready to upload
//  and there is space, upload it
//
//
//********************************
bool VideoObject::uploadFrame(const bool uploadHead)
{
	if (!m_VideoPlayer)
		return false;

	DEBUG_MSG("Frame Number: %d %d %d \n", m_FrameNumber, m_Decoder->isFrameComplete(), m_VideoPlayer->isReadyForFrame());
	if (m_Decoder->isFrameComplete() && m_VideoPlayer->isReadyForFrame())
	{
/*
		if (cglx::isHead() && !uploadHead)
		{
			
			//we don't want to upload video on the head
			m_VideoPlayer->addFrameToBuffer(0);
		}

		else
*/
		{
			//if we are trying to catch up, don't uploadfr
		//	if (myVideoPlayer->getImagesPlayed() < numFramesPlayed-1)
		//		m_VideoPlayer->addFrameToBuffer(0);
		//	else

		/*	AVFrame *pFrame = m_Decoder->getAVFrame();
		//	if (pFrame->pts > m_Time*1000)
				m_VideoPlayer->addFrameToBuffer(pFrame);
			

			m_Decoder->setHurryUp(pFrame->pts > (m_Time*1000) ? 1 : 0);
			*/
			m_VideoPlayer->addFrameToBuffer(m_Decoder->getAVFrame());
		}

		//force upload to finish
		#ifdef FORCE_UPLOAD_TO_FINISH
		//glFinish();
		#endif

		//done with frame
		m_Decoder->setFrameComplete();
		m_Decoder->setReadyForFrame();

		//if we are behind, also skip to the next frame
	//	if (myVideoPlayer->getImagesPlayed() < numFramesPlayed)
	///			myVideoPlayer->nextFrame();

		
		DEBUG_MSG("Frame Number: %d\n", m_FrameNumber);

		return true;

	}

	//if we are behind
	//burn frames
	/*if  ((m_VideoPlayer->getNumImagesInPipe() > 0) && (m_VideoPlayer->getImagesPlayed() < m_FrameNumber))
	{
		m_VideoPlayer->nextFrame();

	}*/

	#ifdef ALLOW_HURRY_UP
	//	m_Decoder->setHurryUp(((m_VideoPlayer->getImagesPlayed() < m_FrameNumber) ? 1 : 0));
	#endif


	return false;
}

double VideoObject::getVideoTime()
{
	if (m_VideoPlayer)
		return .001*m_VideoPlayer->getCurrentPtsValue();
	else
		return 0;
};

void VideoObject::setFormat(const char *fmt)
{
	m_Decoder->setFormat(fmt);
	printf("set format to %s\n",fmt);
}

void VideoObject::process()
{
#ifdef USE_DECODER_THREAD
	//only decode if the thread isn't already doing it
#else
	m_Decoder->decodePacket();
#endif
	if (this->readyToUpload())
	{
		
		this->uploadFrame();
	}
#ifdef USE_AUDIO
	if(m_AudioPlayer)
	{
		if(m_AudioPlayer->isReadyForAudio() && m_Decoder->audioAvailable())
		{
			m_AudioPlayer->addAudioToBuffer(m_Decoder->getAudio());
		}
	}
#endif

}

void VideoObject::setPlaybackTime(double time)
{
	//printf("%f\n", time);

	if (m_Decoder->isDecodingAudio())
	{
		printf("here\n");
	}
	else
	{
		this->nextFrame(time*1000);
	}
}


//getters
int VideoObject::getWidth(){return m_Decoder->getWidth();};
int VideoObject::getHeight(){return m_Decoder->getHeight();};
bool VideoObject::isCulled(){return m_Culled;};
//this may be the lazy method
mediaDecoder * VideoObject::getMediaDecoderPtr(){return m_Decoder;}
bool VideoObject::hasAudio(){return m_Decoder->isDecodingAudio();}
double VideoObject::getLoopTime(){ return m_TimesLooped*m_Decoder->getDuration();}
double VideoObject::getFrameRate() { return m_Decoder->getFrameRate();};


//setters, 
void VideoObject::setOffsetX(const double d){m_OffsetX=d; };
void VideoObject::setOffsetY(const double d){m_OffsetY=d; };
void VideoObject::setRunningType(runningType rType){m_Decoder->setRunningType(rType);};
void VideoObject::setCulled(bool b){ m_Culled=b;}
