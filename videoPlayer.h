/*
 *  videoPlayer.h
 *
 *  Created by Tom Wypych, So Yamaoka
 *  and Kevin Ponto for video playback.
 *  - RGB image is assembled from YUV with Jason's shader code.
 *
 */


#include <GL/glew.h>

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

extern "C"{
#include <libavformat/avformat.h>
}
#include "GlobalDefines.h"

#ifndef VIDEO_PLAYER
#define VIDEO_PLAYER


//this is kind of dirty, but we
//will define the number of buffers
//as the value passed to this class
#define NUM_BUFFERS numBuffers
#define NUM_YUV_TEXTURES 3
#define NUM_TEXTURES NUM_YUV_TEXTURES*NUM_BUFFERS
#define NUM_PBOS NUM_TEXTURES

struct MetaData
{
	int64_t dts;
	int64_t pts;

};

class videoPlayer
{
private:

	//texture index to display
	int currentReadBuffer;
	//texture index to fill
	int currentWriteBuffer;
	//number of images buffered
	int imagesInPipe;
	//number of images we have played
	int imagesPlayed;

	//the bounds for clipping
	//the video based on the display
	double gBottomCopySegment;
	double gTopCopySegment;

	//texture array
	GLuint	*m_texture;

	//metaData array
	MetaData *metaData;

	int videoWidth;
	int videoHeight;

	//stores the width and height for
	//each fo the YUV textures
	int tex_width[NUM_YUV_TEXTURES];
	int tex_height[NUM_YUV_TEXTURES];

	//PBOs for faster transfer
	GLuint *ioBuf;
	int ioBufIndex;

	//for shader
	GLuint m_frag;

	//number of video buffers
	int numBuffers;

	//for fading of bounding box
	int numDrawBoundingBoxFrames;

public:
	videoPlayer(void);
	~videoPlayer(void);

	void init(const int Width, const int Height,  const int numBuffers, const bool enableClipping=false);
	void reset();

	bool nextFrame(const int pts);
	int getNumImagesInPipe(void);
	int getImagesPlayed(void);
	bool isReadyForFrame(void);
	void addFrameToBuffer(AVFrame *pFrame);
	void drawFrame(const int frame_offset=0);
	void drawDebug();
	int64_t getNextPtsValue(void);
	int64_t getCurrentPtsValue(void);

	void setDrawBoundingBox(const int numFrames=5);

	void bind(int textureOffset=0);
	void unbind(int textureOffset=0);
};

#endif
