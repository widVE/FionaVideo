/*
 *  mediaDecoder.cpp
 *
 *  Created by Jason Kimball.
 *
 *  Rewritten by Tom Wypych for movie playback.
 *
 *  - need error handling, returning arbitrary number is confusing.
 *  Modified by Kevin Ponto and So Yamaoka on 7/20/08.
 *
 */

#include "mediaDecoder.h"
#include <cassert>


#define MESSAGE ;

#define INT64_C int64_t
#define MAX_AUDIO_FRAME_SIZE 192000
#define AVCODEC_MAX_AUDIO_FRAME_SIZE MAX_AUDIO_FRAME_SIZE

#define MAX_PATH_SIZE 2048

char * runningTypeStr[3]={"video_only", "video_audio", "audio_only"};

//pthread_mutex_t m_DecoderMutex = PTHREAD_MUTEX_INITIALIZER;



void *mediaDecoder::objectLauncher (void* object)
{
	mediaDecoder *myObj;
	myObj = (mediaDecoder*)object;

	myObj->decodeVideo();

	return (void*) NULL;
}

mediaDecoder::mediaDecoder(void)
{
	markTime = 0;
	path = (char*)malloc(sizeof(char)*MAX_PATH_SIZE);
	resetState();

	readyForFrame = false;
	finishedFile = false;

	//************************
	// for now, video only
	//************************

	av_register_all();

	running = video_audio;
	//m_DecoderMutex = PTHREAD_MUTEX_INITIALIZER;
	#ifdef WIN32
#else
	pthread_mutex_init(&m_DecoderMutex, NULL);
#endif

	return;
}


mediaDecoder::mediaDecoder(char inPath[])
{
	markTime = 0;
	path = (char*)malloc(sizeof(char)*MAX_PATH_SIZE);

	readyForFrame = true;
	finishedFile = false;

	resetState();
	open(inPath);

	av_register_all();

	return;
}

bool mediaDecoder::hasFinishedFile()
{
	return finishedFile;
}

void mediaDecoder::setNoTiming(void)
{
	timing = NONE;
}

void mediaDecoder::setLocalTiming(void)
{
	timing = LOCAL;
}

void mediaDecoder::setManualTiming(void)
{
	timing = MANUAL;
}

void mediaDecoder::releaseFrame(void)
{
	releaseTokens++;
}

void mediaDecoder::play(void)
{
	moviePlaying = true;

	return;
}

void mediaDecoder::pause(void)
{
	//set mark time to zero
	//so the timing will reset
	//itself

	moviePlaying = false;

	return;
}

void mediaDecoder::resetTiming(void)
{
	markTime = 0;
}


void mediaDecoder::setFrameSize(int w, int h)
{
	width = w;
	height = h;
}

int mediaDecoder::getWidth(void)
{
	return width;
}

int mediaDecoder::getHeight(void)
{
	return height;
}

int mediaDecoder::systemReady(void)
{
	if (status == 2)
		return true;
	else
		return false;
}


void mediaDecoder::setReadyForFrame(void)
{
	//printf("ready\n");
	readyForFrame = true;
}

void mediaDecoder::setFrameComplete()
{
	frameComplete--;
}

int mediaDecoder::getAudioInPipe(void)
{
	return audioInPipe;
}

int mediaDecoder::getNumAudioBuffers(void)
{
	return numAudioBuffers;
}


bool mediaDecoder::isFrameComplete(/*bool setComplete*/)
{
//	if (setComplete)
//		frameComplete--;

	if (frameComplete<=0)
		return false;

	return true;

	//return frameFinished? true:false;
}


void mediaDecoder::setDiscard(AVDiscard d)
{
	if (isDecodingAudio())
		pFormatCtx->streams[audioStream]->discard = d;
}

int mediaDecoder::getBitsPerChannel(void){
//ffmpeg always decodes (even 8bit samples) to
//16 bits
	return 16;//(int)aCodecCtx->bit_rate;
}


int mediaDecoder::getFrequency(void){
	return (int)aCodecCtx->sample_rate;
}

int mediaDecoder::getNumChannels(void){
	return aCodecCtx->channels;
}

double mediaDecoder::getTimeBaseNum(void){
	//return (double)pCodecCtx->time_base.num;
	return (double)time_base_num;
}

double mediaDecoder::getTimeBaseDen(void){
	//return (double)pCodecCtx->time_base.den;
	return (double)time_base_den;
}

int  mediaDecoder::getFrameDelay(void){
	return frameDelay;
}

int mediaDecoder::getFrameNumber(void){
	return pCodecCtx->frame_number;
}

AVFrame* mediaDecoder::getAVFrame(void)
{

	//int r = (global_video_pkt_pts)%INT_MAX;
	//pFrameDecoder->pts  = (r< 0) ? INT_MAX + r: r;
	//pFrameDecoder->pts = global_video_pkt_pts;
	 //fprintf(stderr, "PTS: %Ld %d\n", global_video_pkt_pts, pFrameDecoder->pts);
	return pFrameDecoder;
}

int64_t mediaDecoder::getPts()
{
	return global_video_pkt_pts;
}

int64_t mediaDecoder::getDts()
{
	return global_video_pkt_pts;
}

int64_t mediaDecoder::getAudioPts()
{
	return global_audio_pkt_pts;
}

int64_t mediaDecoder::getAudioDts()
{
	return 0;
}

void mediaDecoder::rewind(void)
{
	//pthread_mutex_lock( &m_DecoderMutex );
	//wait to finish
	//needToRewind=true;
	frameComplete=0;
	readyForFrame=true;
	finishedFile=false;
	markTime=0;
	audioInPipe = 0;

	//pCodecCtx->frame_number=0;
	currentAudioWriteBuffer= currentAudioReadBuffer=0;
	this->seek(0);
	//pthread_mutex_unlock( &m_DecoderMutex );
}

void mediaDecoder::resetState(void)
{
	numBuffers = NUM_BUFFERS;
	firstRun = true;
	buffer = new uint8_t* [numBuffers];


	numAudioBuffers = NUM_AUDIO_BUFFERS;
	audioBuffer = new audioBufferObject [numAudioBuffers];
	audioInPipe = 0;
	currentAudioWriteBuffer = currentAudioReadBuffer=0;

	pInputFormat = NULL;
	pFormatCtx = NULL;
	pCodecCtx = NULL;
	pCodec = NULL;
	pFrameDecoder = 0;
//	pFrameRGB = NULL;
//	img_convert_ctx = NULL;


	status = 0;
	moviePlaying = 0;

	shutDownFlag = 0;
	frameDelay = 0;
	releaseTokens = 0;

	timing = NONE;
	frameComplete=0;
	readyForFrame=true;
	finishedFile=false;

	seekRequest.needToSeek=false;

	//fprintf(stdout, "setup frame complete %d\n", frameComplete);


	return;
}


int mediaDecoder::open(const char *inPath)
{
	// changed to strncpy for safety.
	strncpy(path, inPath, MAX_PATH_SIZE);
	if(!ffmpegInit())
		status = 1;

	return status;
}

int mediaDecoder::reopenFile(void)
{
	if(avformat_open_input(&pFormatCtx, path, NULL, 0)!=0)
		return -2;

	//if(av_find_stream_info(pFormatCtx)<0)
	//	return -4;

}

void mediaDecoder::setFormat(const char *fmt)
{
	
	pInputFormat =  av_find_input_format(fmt);   	

	if (!pInputFormat)
	{
		printf("%s format not found!\n", fmt);
	}
}

int mediaDecoder::ffmpegInit(void)
{
	int i;

	//moved to constructor
//	av_register_all();

	if(avformat_open_input(&pFormatCtx, path, pInputFormat, 0)!=0)
		return -2;

	if(avformat_find_stream_info(pFormatCtx, 0)<0)
		return -4;

	//doesn't seem to work
	//attempt to get pts and dts
	//pFormatCtx->flags |= AVFMT_FLAG_GENPTS;

//#ifdef DEBUG
	av_dump_format(pFormatCtx, 0, path, 0);
//#endif



	videoStream=-1;
	for(i=0; i < (signed int)pFormatCtx->nb_streams; i++)
	if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
	{
		videoStream=i;
		break;
	}

	//if(videoStream==-1)
	//	return -8;

	

	
	if(videoStream!=-1)
	{
		pCodecCtx=pFormatCtx->streams[videoStream]->codec;
		pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
		if(pCodec==NULL)
		{
			fprintf(stderr, "Unsupported codec!\n");
			return -16;
		}

		if(avcodec_open2(pCodecCtx, pCodec, 0)<0)
			return -32;
	}

	audioStream=-1;
	//if (cglx::isHead())
	for(i=0; i < (signed int)pFormatCtx->nb_streams; i++)
	if((pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) && (audioStream < 0))
	{
		audioStream=i;
		running = video_audio;
	}

	

	//see if this computer even has audio
	if (audioStream==-1)
		if (videoStream==-1)
			//no video or audio, this is bad
			return -8;
		else
			running = video_only;


	//if there is no video
	if (videoStream==-1)
	{
		pCodecCtx=pFormatCtx->streams[audioStream]->codec;
		pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	}


//	if(audioStream==-1)
//		return -8;

	if (running == video_audio)
	{
		
		aCodecCtx=pFormatCtx->streams[audioStream]->codec;
		aCodec=avcodec_find_decoder(aCodecCtx->codec_id);

		if(aCodec==NULL)
		{
			fprintf(stderr, "Unsupported audio codec!\n");
			running = video_only;
		}
		/*if (aCodecCtx->channels != 2)
		{
			printf("Only two-channel audio is supported at this time\n");
			printf("Switching to video only mode\n");
			running = video_only;
		}*/
		else
		{
			for (i = 0; i < numAudioBuffers; i++)
			{
				audioBuffer[i].buffer = (int8_t*)av_malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE );
				for (int x = 0; x < AVCODEC_MAX_AUDIO_FRAME_SIZE; x++)
					audioBuffer[i].buffer[x] = 0;

				audioBuffer[i].size = 0;
			}

			sharingABO.buffer = (int8_t*)av_malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE );
		}

	}



	if((running == video_audio) && (avcodec_open2(aCodecCtx, aCodec, 0)<0))
		running = video_only;


	pFrameDecoder = avcodec_alloc_frame();
	assert(pFrameDecoder);
//	pFrameRGB = avcodec_alloc_frame();
//	if((pFrameRGB==NULL) || (pFrame==NULL))
//		return -64;

	if (videoStream==-1)
	{
		//dummy numbers
		width=100; height=100;
	}
	else
	{
		width = pCodecCtx->width;
		height = pCodecCtx->height;
	}

	if ((width<=0) || (height<=0))
	{
		printf("videoStream= %d, Width = %d, this is messed up\n", videoStream, pCodecCtx->width);
	//	return -15;
		width = 1920;
		height = 1080;
	}

//	numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
//	avpicture_fill((AVPicture *)pFrameRGB, buffer[0], PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

	startTime = pFormatCtx->start_time*.000001;

	time_base_num = pCodecCtx->time_base.num;
	time_base_den = pCodecCtx->time_base.den;

	frameDelay = (int)(1000000 * ((double)pCodecCtx->time_base.num)/((double)pCodecCtx->time_base.den));

	//if it's interlaced, half the framerate
	interlaced = pCodecCtx->flags&CODEC_FLAG_INTERLACED_ME ? 1 : 0;
	if (interlaced){
		printf("Video is interlaced, dividing the framerate by 2\n");
		frameDelay*=2;
	}
	if (1000000.0/((double)frameDelay) > 1000)
	{
		printf("*************** WARNING *****************\n");
		printf("* Detected framerate of %08f     *\n", 1000000.0/((double)frameDelay));
		printf("* Assuming this is an error.            *\n");
		printf("* switching back to 30 fps              *\n");
		printf("*****************************************\n");
		frameDelay = 1000000.0 / 30.;
		time_base_num = 1000;
		time_base_den = 1000*30;
	}



	printf("Detected video of size %dx%d\n",width,height);
	printf("Frame rate: %f\n", 1000000.0/((double)frameDelay));

/*
	img_convert_ctx = sws_getContext(width, height, pCodecCtx->pix_fmt, width, height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
	if(img_convert_ctx == NULL)
	{
		fprintf(stderr, "Cannot initialize the conversion context!\n");
		return -128;
	}
*/
	/*if ((width > 2048) || (height > 2048))
		return -1;*/ // removed 22 July 08 KD

	

	av_dump_format(pFormatCtx, 0, path, 0);

	//printf("pthread_create\n");
	return 0;
}

void mediaDecoder::initGL()
{
	firstRun = false;
#ifndef USE_TVM_DECODING
	if (running != audio_only)
		pthread_create(	&thread, NULL, mediaDecoder::objectLauncher, this);
#endif
}


void mediaDecoder::setFrameRate(double d)
{
	time_base_num = 1000;
	time_base_den = 1000*d;

	frameDelay = 1000000./d;
	printf("Frame rate forced to: %f\n", 1000000.0/((double)frameDelay));
}

//#ifdef ALLOW_HURRY_UP
void mediaDecoder::setHurryUp(int i)
{
	//pCodecCtx->hurry_up=i;
}	
//#endif

void mediaDecoder::decodePacket(void)
{
	//pthread_mutex_lock( &m_DecoderMutex );

	//if we have a seek request
	if (seekRequest.needToSeek==true)
	{
		frameComplete=0;
		readyForFrame=true;
		finishedFile=false;
		markTime=0;
		audioInPipe = 0;
		frameFinished=false;

		global_video_pkt_pts=0;
		
		//pCodecCtx->frame_number=0;
		currentAudioWriteBuffer= currentAudioReadBuffer=0;
		


		/*if (running == audio_only)
		{
			int64_t TARGET_PTS = av_rescale_q(seekRequest.timestep, AV_TIME_BASE_Q, pFormatCtx->streams[audioStream]->time_base);

			//TARGET_PTS*= 10000000;//av_q2d(pStream->time_base);
			printf("seek to %lf, pts %ld\n", seekRequest.timestep, TARGET_PTS);
			if (av_seek_frame(pFormatCtx, audioStream, TARGET_PTS, 0) < 0)
			{
				fprintf(stderr, "%s: error while seeking\n", pFormatCtx->filename);
			}

		}
		else*/
		{
			int64_t TARGET_PTS = seekRequest.timestep* AV_TIME_BASE;
			//TARGET_PTS*= 10000000;//av_q2d(pStream->time_base);
			printf("seek to %lf, pts %ld\n", seekRequest.timestep, TARGET_PTS);
			if (av_seek_frame(pFormatCtx, -1, TARGET_PTS, AVSEEK_FLAG_ANY) < 0)
			{
				fprintf(stderr, "%s: error while seeking\n", pFormatCtx->filename);
			}
		}


		//clear our buffers?
		if (audioStream != -1)
		if ((running == video_audio) || (running == audio_only))
		{	
			for (int i = 0; i < numAudioBuffers; i++)
			{
				for (int x = 0; x < AVCODEC_MAX_AUDIO_FRAME_SIZE; x++)
					audioBuffer[i].buffer[x] = 0;

				audioBuffer[i].size = 0;
			}
			avcodec_flush_buffers(aCodecCtx);
		}
		//pFrameDecoder=0;


		avcodec_flush_buffers(pCodecCtx);

		
		//we are done with this request
		seekRequest.needToSeek=false;
	}


//	printf("check: %d %d %d \n", shutDownFlag , readyForFrame , ((running == video_only)||(audioInPipe < numAudioBuffers)));

	int av_read_status;
	if(!shutDownFlag && ((running == audio_only)||readyForFrame) && ((running == video_only)||(audioInPipe < numAudioBuffers)))
	{
		bool stop = false;
		if (!stop && !shutDownFlag)
			av_read_status = av_read_frame(pFormatCtx, &packet);

		//if we are done with a file, set the bool
		if (av_read_status == EOF)
		{
			finishedFile=true;
		}

		while((!stop) && (av_read_status >=0) && !shutDownFlag)
		{

			//fprintf(stdout, "pipe, num, readyforframe = %d, %d, %d\n", imagesInPipe, numBuffers, readyForFrame);
			// Is this a packet from the video stream?
			if(((running == video_audio) || (running == video_only)) && packet.stream_index==videoStream && !shutDownFlag)
			{

				//printf("******************* decode video ******************************");

				pts = 0;

				// Save global pts to be stored in pFrame in first call
				global_video_pkt_pts = packet.pts;
				global_video_pkt_pts = packet.dts;

				// Decode video frame
				if (running == audio_only)
					frameFinished=true;
				else
					avcodec_decode_video2(pCodecCtx, pFrameDecoder, &frameFinished, &packet);


				if(packet.dts == AV_NOPTS_VALUE 
							&& pFrameDecoder->opaque && *(uint64_t*)pFrameDecoder->opaque != AV_NOPTS_VALUE) {
							pts = *(uint64_t *)pFrameDecoder->opaque;
				} 
				else if(packet.dts != AV_NOPTS_VALUE) 
				{
					pts = packet.dts;
				} 
				else 
				{
					pts = 0;
				}

				//what if we just use the pts?
				//pts  = packet.pts;
				
				pts *= av_q2d(pFormatCtx->streams[videoStream]->time_base)*1000;
				pFrameDecoder->pts =pts;
				global_video_pkt_pts = pts;

				// Did we get a video frame?
				if(frameFinished && !shutDownFlag)
				{

					frameComplete++;
					readyForFrame = false;

					//imagesInPipe++;
					stop = true;
					if (status != 2)
						status = 2;
//							fprintf(stdout, "not full %d\n", stop);
				
				
					//wait here if video is full
				//	while(videoBuffersFull()) {  usleep(1); };

				}
				//else
				//	fprintf(stderr,  "frame not completed in one packet\n");
			}

		//	printf("Running is %s\n", runningTypeStr[running]);
			// Is this a packet from the audio stream?
			if(((running == video_audio) || (running == audio_only)) && packet.stream_index==audioStream && !shutDownFlag)
			{


/*						 if(packet.pts != AV_NOPTS_VALUE && !shutDownFlag) {
						global_audio_pkt_pts = av_q2d(aCodecCtx->time_base);//*packet.pts;
					}
*/
				//printf("******************* decode audio *******************************\n");
				//printf("audio in pipe: %d\n", audioInPipe);
				global_audio_pkt_pts = av_q2d(aCodecCtx->time_base); //packet.pts;

				for (int x = 0; x < packet.size; x++)
					audioBuffer[currentAudioWriteBuffer].buffer[x] = packet.data[x];
			
				audioBuffer[currentAudioWriteBuffer].pts = av_q2d(pFormatCtx->streams[audioStream]->time_base)*packet.pts*1000;
				//audioBuffer[currentAudioWriteBuffer].pts = packet.pts;
				audioBuffer[currentAudioWriteBuffer++].size = packet.size;

				audioInPipe++;

				currentAudioWriteBuffer %= numAudioBuffers;

				stop = true;


			}


			// Free the packet that was allocated by av_read_frame
			av_free_packet(&packet);

			//get the next packet
			if (!stop)
				av_read_status = av_read_frame(pFormatCtx, &packet);

			//if we are done with a file, loop back
			if (av_read_status == EOF)
			{
				finishedFile=true;
			}
		}
	}
//	else
//		fprintf(stdout, "not ready for frame\n");

	//pthread_mutex_unlock( &m_DecoderMutex );


}


void mediaDecoder::decodeVideo(void)
{
	

	while(!shutDownFlag)
	{


	//fprintf(stdout, "made frame complete %d\n", frameComplete);

//			usleep(1000);
			
		decodePacket();
#ifdef WIN32
		Sleep(1);
#else
		usleep(1);
#endif

	}
}

int mediaDecoder::close()
{
	moviePlaying = false;
	shutDownFlag = true;

	printf("closing thread\n");
	#ifndef USE_TVM_DECODING
	pthread_join(thread, NULL);
	#endif
	free(path);

	// Free the RGB image
//	av_free(pFrameRGB);

	// Free the YUV frame
//	av_free(pFrame);

	// Close the codec
	avcodec_close(pCodecCtx);

	// Close the video file
	avformat_close_input(&pFormatCtx);

	//resetState();

	delete[] buffer;

	delete [] audioBuffer;

	return 0;
}

mediaDecoder::~mediaDecoder()
{
	//pthread_mutex_unlock( &m_DecoderMutex );
	
	if (!firstRun)
		close();
	return;
}

int mediaDecoder::timedFrameAvailable(void)
{
	if (firstRun)
	{
		firstRun = false;
		return true;
	}
//return true;
	unsigned long localTime;
	struct timeval tx;

	gettimeofday(&tx, NULL);

	localTime = 1000000 * tx.tv_sec;
	localTime += tx.tv_usec;

	if (localTime >= markTime)
	{
		if ((markTime ==0))
			markTime = localTime + frameDelay;
		else
			markTime += frameDelay;
		return true;
	}
	else
	{
		//printf("localTime : %ul marktime: %ul\n",localTime,markTime);
		return false;
	}
}


void mediaDecoder::seek(const double timestep)
{
	//try to make a request to the decoding thread
	//pthread_mutex_lock( &m_DecoderMutex );
	seekRequest.needToSeek=true;
	seekRequest.timestep=timestep;
	//pthread_mutex_unlock( &m_DecoderMutex );
}

void mediaDecoder::recordFrameOut(void)
{
	/*struct timeval tx;

	gettimeofday(&tx, NULL);

	markTime = 1000000 * tx.tv_sec;
	markTime += tx.tv_usec;

	markTime += frameDelay;*/
	//printf("recordFrameOut\n");
	return;
}

int mediaDecoder::audioAvailable(void)
{
	if (needToRewind){
		audioInPipe = 0;
		currentAudioWriteBuffer= currentAudioReadBuffer=0;
		needToRewind=false;
	}
	return audioInPipe;
}

double mediaDecoder::getDuration()
{

	//printf("dur %ld\n", pFormatCtx->duration);

	//double r = pFormatCtx->streams[videoStream]->duration*av_q2d(pStream->time_base);
		
	double r  = pFormatCtx->duration/double(AV_TIME_BASE);
	if (r > 0)
		return  r;
	//else
	r = pFormatCtx->streams[videoStream]->duration*av_q2d(pFormatCtx->streams[videoStream]->time_base);
	
	if (r > 0)
		return  r;

	//printf("num frames %ld rate %f\n", pFormatCtx->streams[videoStream]->nb_frames, av_q2d(pFormatCtx->streams[videoStream]->r_frame_rate));

	r = pFormatCtx->streams[videoStream]->nb_frames * av_q2d(pFormatCtx->streams[videoStream]->r_frame_rate);
}


audioBufferObject* mediaDecoder::getAudio(const bool decode)
{
	if ((audioInPipe <= 0) || (seekRequest.needToSeek==true))
		return NULL;

	if (decode)
	{
		//printf("got here in %s %d\n", __FILE__, __LINE__);
		
		uint8_t *audio_pkt_data = (uint8_t*) audioBuffer[currentAudioReadBuffer].buffer;
		int audio_pkt_size = audioBuffer[currentAudioReadBuffer].size;
		int data_size = 0;
		int length;


		//printf("samples in queue: %d\n", audioInPipe);
		while ((audio_pkt_size > 0) && (data_size <= 0))
		{
		//	printf("audio packed size = %d\n", audio_pkt_size);
			data_size = MAX_AUDIO_FRAME_SIZE;
			// Do something with audio data
			length = avcodec_decode_audio4(aCodecCtx, (AVFrame *)sharingABO.buffer, &data_size, (AVPacket *)audio_pkt_data);

			int avcodec_decode_audio4(AVCodecContext *avctx, AVFrame *frame,
                          int *got_frame_ptr, const AVPacket *avpkt);
			if (length < 0)
			{
			//this is bad 
				audioInPipe--;
				currentAudioReadBuffer++;
				currentAudioReadBuffer %= numAudioBuffers;
				return NULL;

			}

			audio_pkt_data += length;
			audio_pkt_size -= length;
		}

		//printf("got here in %s %d\n", __FILE__, __LINE__);

		sharingABO.pts = audioBuffer[currentAudioReadBuffer].pts;
		sharingABO.size = data_size;

		if (data_size == 0)
			printf("audio packet has no data size!\n");
	}
	//else
	//	printf("audio packet data size %d\n", data_size);

	//printf("reading: %d\nwriting %d\n", currentAudioReadBuffer, currentAudioWriteBuffer);
	//printf("samples in queue: %d\n", audioInPipe);

	audioInPipe--;
	currentAudioReadBuffer++;

	currentAudioReadBuffer %= numAudioBuffers;

	return &sharingABO;
}

runningType mediaDecoder::getRunning(void)
{
	return running;
}
bool mediaDecoder::isDecodingAudio()
{
	return ((running== audio_only) || (running == video_audio));
}

void mediaDecoder::setRunningType(runningType rType)
{
	printf("Running type changed to %s\n", runningTypeStr[rType]);
	running = rType;
//	printf("running type %d\n", running);
}

double mediaDecoder::getFrameRate() 
{ 
	if (videoStream != -1)
		return av_q2d(pFormatCtx->streams[videoStream]->r_frame_rate);
	else
		return 1;
};

double mediaDecoder::getStartTime(){ return startTime; };

