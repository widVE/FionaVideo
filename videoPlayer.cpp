/*
 *  videoPlayer.cpp
 *
 *  Created by Tom Wypych, So Yamaoka
 *  and Kevin Ponto for video playback.
 *  - RGB image is assembled from YUV with Jason's shader code.
 *
 */



#include <GL/glew.h>
#include "videoPlayer.h"
#include <assert.h>

#define USE_PBO

#define DEBUG_MESSAGE ;
#define MESSAGE printf

const static char* m_fragmentProgram = "!!ARBfp1.0\n"
"PARAM vr = { 1.0000,  0.0000,  1.13983 };\n"
"PARAM vg = { 1.0000, -0.39465, -0.58060 };\n"
"PARAM vb = { 1.0000,  2.03211,  0.0000 };\n"
"TEMP yuv;\n"
"TEMP rgb;\n"
"TEMP tc;\n"
"TEX yuv.r, fragment.texcoord[0], texture[0], RECT;\n"
"MUL tc, fragment.texcoord[0], {0.5, 0.5, 0.5, 1.0};\n"
"TEX yuv.g, tc, texture[1], RECT;\n"
"TEX yuv.b, tc, texture[2], RECT;\n"
"ADD yuv.r, yuv.r, -0.0625;\n"
"MUL yuv.r, yuv.r, 1.1643;\n"
"ADD yuv.g, yuv.g, -0.5;\n"
"ADD yuv.b, yuv.b, -0.5;\n"

"MUL rgb.r, 1.5958, yuv.b;\n"
"ADD rgb.r, rgb.r, yuv.r;\n"

"MUL tc.r, 0.39173, yuv.g;\n"
"MUL tc.g, 0.81290, yuv.b;\n"
"ADD rgb.g, yuv.r, -tc.r;\n"
"ADD rgb.g, rgb.g, -tc.g;\n"

"MUL rgb.b, 2.017, yuv.g;\n"
"ADD rgb.b, rgb.b, yuv.r;\n"

"MOV result.color.rgb, rgb;\n"
"MOV result.color.a, 1;\n"
"END";

videoPlayer::~videoPlayer(void)
{
	glDeleteTextures(NUM_TEXTURES, m_texture);
	glDeleteProgramsARB(1, &m_frag);
	glDeleteBuffersARB(NUM_PBOS, ioBuf);
	delete[] metaData;
	delete[] m_texture;
	delete[] ioBuf;
	return;
}


videoPlayer::videoPlayer(void)
{
	MESSAGE("Creating video player\n");

	//this->reset();
	currentWriteBuffer = currentReadBuffer=0;
	return;
}

/*! resets the variables */
void videoPlayer::reset(void)
{
	//reset the variables
	currentReadBuffer = 0;
	//currentWriteBuffer = 0;
	currentWriteBuffer = currentReadBuffer;
	currentWriteBuffer = currentWriteBuffer >=0 ? currentWriteBuffer : NUM_BUFFERS-1;
	imagesInPipe = 0;
	imagesPlayed = 0;
	
	//reset all pts values
	for (int i=0; i < NUM_BUFFERS; i++)
		metaData[i].pts = 0;

	return;
}

/*! get the PTS value for the next video frame */
int64_t videoPlayer::getNextPtsValue(void)
{
	if (imagesInPipe <= 0)
		return 0;

	int r = (currentReadBuffer+1) % NUM_BUFFERS;
	//printf("pts sent %d \n", metaData[r].pts);
	return metaData[r].pts;

}

int64_t videoPlayer::getCurrentPtsValue(void)
{
	if (imagesInPipe <= 0)
		return 0;
	
	int r = (currentReadBuffer+1) % NUM_BUFFERS;
	//printf("pts sent %d \n", metaData[r].pts);
	return metaData[r].pts;

}



/*! flips to the next video frame */
bool videoPlayer::nextFrame(const int pts)
{
	DEBUG_MESSAGE("pts: %d\n", pts);
	if (imagesInPipe > 0)
	{
		if (pts > 0)
		{
			while(imagesInPipe > 0)
			{
				DEBUG_MESSAGE("%d < %d ?\n",  metaData[currentReadBuffer].pts%INT_MAX, pts);
				//assume ascending order
				if ((metaData[currentReadBuffer].pts) < pts)
				{
					DEBUG_MESSAGE("----> switch from %d ", metaData[currentReadBuffer].pts);
					imagesInPipe--;
					imagesPlayed++;
					currentReadBuffer = (currentReadBuffer+1) % NUM_BUFFERS;
					DEBUG_MESSAGE("to %d at spot %d\n", metaData[currentReadBuffer].pts, currentReadBuffer);
				}else{
					//we are all caught up
					return true;
				}

			}
			return false;
			/*
			//printf("pts %d to %d\n", metaData[currentReadBuffer].pts, pts);

			//tell it we played a frame
			if (imagesInPipe > 0)
				imagesPlayed++;

			//we need to play until we find a frame
			//with this pts value
			while (imagesInPipe > 0)
			{

				//if we found the pts frame
				if ((metaData[currentReadBuffer].pts%INT_MAX) == pts)
					return true;

				//if we are ahead
				if ((metaData[currentReadBuffer].pts%INT_MAX) > pts)
				{
					//if we are way off...don't lock this system
					if ((metaData[currentReadBuffer].pts%INT_MAX) - pts > NUM_BUFFERS)
					{
						imagesInPipe--;
						currentReadBuffer = (currentReadBuffer+1) % NUM_BUFFERS;
					}
					return false;
				}

				imagesInPipe--;
				currentReadBuffer = (currentReadBuffer+1) % NUM_BUFFERS;

			}
			//we didn't find the pts frame
			return false;*/
		}
		else
		{
			//printf("pts < 0 so go to next frame\n");
			imagesInPipe--;
			currentReadBuffer = (currentReadBuffer+1) % NUM_BUFFERS;
			imagesPlayed++;

			return true;
		}
	}
	else
	{
		
	//	imagesPlayed++;
		//we will make this value negative to tell the other end
		//not to upload until we are back on track
	//	imagesInPipe--;
	//	printf("we are trying to play ahead for some reason\n");
		return false;
	}
	return false;
}

/*! returns the number of images currently buffered */
int videoPlayer::getImagesPlayed(void)
{
	return imagesPlayed;

}

/*! returns the number of images currently buffered */
int videoPlayer::getNumImagesInPipe(void)
{
	return imagesInPipe;

}

/*! returns whether there is room in the buffers for another frame */
bool videoPlayer::isReadyForFrame()
{
	return (imagesInPipe < NUM_BUFFERS);
}

/*! inits all texture buffers */
void videoPlayer::init(const int Width, const int Height, const int numBuffers, const bool enableClipping)
{
	//glewInit();

	//set the values
	this->videoWidth = Width;
	this->videoHeight = Height;
	this->numBuffers = numBuffers;

	//generate the texture array
	MESSAGE("	generate %d texture buffers (%dx%d) for %d buffered frames\n", NUM_TEXTURES, Width, Height, NUM_BUFFERS);
	m_texture = new GLuint[NUM_TEXTURES];
	glGenTextures(NUM_TEXTURES, m_texture);

	//generate the metaData array
	metaData = new MetaData[numBuffers];
	/*
	 glEnable(GL_FRAGMENT_PROGRAM_ARB);


	// generate the fragment program
	MESSAGE("	generate the fragment program\n");
	glGenProgramsARB(1, &m_frag);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_frag);
	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
					(GLsizei)strlen(m_fragmentProgram), m_fragmentProgram);

	//check for errors
	int error;
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error);
	if (error != -1)
	{
    	printf("Error position: %d\n", error);
        printf("%s error output:\n%s\n", "fragment program",
        	glGetString(GL_PROGRAM_ERROR_STRING_ARB));
	}
	*/
	// set Y texture size
	// getting rid of extra 32 pixels
	tex_width[0] = this->videoWidth+32;

	// set UV textures are half size of Y.
	tex_width[1] = tex_width[2] = tex_width[0]>>1;

	tex_height[0] = Height;
	tex_height[1] = tex_height[2] = tex_height[0]>>1;

	//generate the textures
	MESSAGE("	generate the textures\n");
	int key;
	for (int j = 0; j < NUM_BUFFERS; j++)
	for(int i=0; i<NUM_YUV_TEXTURES; i++)
	{
		key = i + NUM_YUV_TEXTURES*j;
		glBindTexture(GL_TEXTURE_2D, m_texture[key]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

		//tell OpenGL to ignore padding at ends of rows
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, tex_width[i], tex_height[i], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
	}


	//generate the PBO buffer
	ioBuf = new GLuint[NUM_PBOS];
	glGenBuffersARB(NUM_PBOS, ioBuf);
	ioBufIndex = 0;

	//*******************************************/
	/*
	/* now find what we will cull.  We will only
	/* cull based on rows to make things simpler
	/*
	//*******************************************/

	//find the dimensions of the wall and the video
	//float dWall = cglx::getWallHeight() / (float) cglx::getWallWidth();
	float dWall = 1;
	float dVid = tex_height[0]  / (float) tex_width[0];

	//quickly find the geometry
	//int c = cglx::getColPos (0);
	//int r = cglx::getRowPos (0);
	int c=1;
	int r=1;
	int tr=1, tc=1, spanX=1, spanY=1;

	//for for all of the monitors
/*
	for (int numDisp = 1; numDisp < cglx::getNumDisplays (); numDisp++)
	{
		//tc = cglx::getColPos (numDisp);
		//tr = cglx::getRowPos (numDisp);
		

		spanX = abs(tc-c) +1 > spanX? abs(tc-c) +1 : spanX;
		spanY = abs(tr-r) +1 > spanY? abs(tr-r) +1 : spanY;

		c = tc<c? tc : c;
		r = tr<r? tr : r;
	}
*/
	//find the number of rows, for the headnode it
	//will always be 1
//	int numRows = cglx::isHead()?1:cglx::getRowDim();
	int numRows = 1;

	if (!enableClipping)
	{
		gBottomCopySegment = 0.0;
		gTopCopySegment = 1.0;

	}
	else
	{
		if (dWall <= dVid)
		{
			//if the wall is wider than tall
			//set the top and bottom copy regions
			gBottomCopySegment = r / (float) numRows;
			gTopCopySegment = gBottomCopySegment+(spanY / (float) numRows);
			//fprintf(stdout, "gBottomCopySegment, gTopCopySegment = %f, %f (%d, %d)\n",
			//	gBottomCopySegment, gTopCopySegment, cglx::getColPos(), cglx::getRowPos());


		}
		else
		{
			//the wall is taller than wide

			//find the new ratio
			double ratioSize = dVid /dWall;
			double ymax = .5*(1+ratioSize);
			double ymin = .5*(1-ratioSize);
			double yheight = ymax-ymin;

			gBottomCopySegment = r / (double) numRows;
			gTopCopySegment = gBottomCopySegment+(spanY / (double) numRows);

			gBottomCopySegment = (gBottomCopySegment - ymin) / yheight;
			gTopCopySegment = (gTopCopySegment - ymin) / yheight;

			if (gBottomCopySegment < 0)
				gBottomCopySegment=0;

			if (gTopCopySegment > 1)
				gTopCopySegment=1;

			if ((gBottomCopySegment > 1) || (gTopCopySegment < 0))
			{
				//this means this row see's nothing
				gTopCopySegment=0;
				gBottomCopySegment=0;

			}

			//fprintf(stdout, "gBottomCopySegment, gTopCopySegment = %f, %f (%d, %d)\n",
			//	gBottomCopySegment, gTopCopySegment, cglx::getColPos(), cglx::getRowPos());
			//printf("xmin %f, xmax %f \n", ymin, ymax);
		}
	}

	this->reset();
}

/*! takes a new video frame and adds it to texture memory */
void videoPlayer::addFrameToBuffer(AVFrame *pFrame)
{
	//printf("upload frame\n");

	//only upload if we aren't behind
	//this will hopefully allow videos
	//which are behind to catch up
	if (pFrame)
	{
		//copy out metadata
		metaData[currentWriteBuffer].pts = pFrame->pts;
		DEBUG_MESSAGE("upload frame %d to spot%d\n", pFrame->pts, currentWriteBuffer);

		//to make sure only data for this monitor is uploaded
		if ((gTopCopySegment > 0) && (pFrame > 0))
		for(int i=0; i<NUM_YUV_TEXTURES; i++)
		{
			glBindTexture(GL_TEXTURE_2D, m_texture[i+currentWriteBuffer*NUM_YUV_TEXTURES]);

#ifdef USE_PBO
			//copy only part of the data
			double height = tex_height[i];//tex_height[i]*(gTopCopySegment - gBottomCopySegment);
			int size = ceil(pFrame->linesize[i]*height);

			if (size <= 0)
			{
				fprintf(stderr, "Decoded frame size = %d\n, this is an ffmpeg issue\n", size);
				return;
			}

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, ioBuf[ioBufIndex]);
			glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, size, NULL, GL_STREAM_DRAW);
			void* ioMem = glMapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY);
			assert(ioMem);
			int offset_height = int((1.-gTopCopySegment)*tex_height[i]);
			memcpy(ioMem, pFrame->data[i]+offset_height*pFrame->linesize[i], size);
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB);

			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, offset_height, pFrame->linesize[i], height, GL_LUMINANCE,
					GL_UNSIGNED_BYTE, 0);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
			
			ioBufIndex++;
			ioBufIndex = ioBufIndex >= NUM_PBOS ? 0 : ioBufIndex;
#else
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pFrame->linesize[i], tex_height[i], GL_LUMINANCE,
					GL_UNSIGNED_BYTE, pFrame->data[i]);
#endif

		}
	}
	//add image to pipe
	currentWriteBuffer = (currentWriteBuffer+1) % NUM_BUFFERS;
	imagesInPipe++;

}


void videoPlayer::drawDebug()
{
	//video buffers
	
	glLineWidth(10.0);
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(10.0, 0.0, 1.0);
	glColor3f(0.0, (imagesInPipe/float(NUM_BUFFERS)), 0.0);
	glVertex3f(10.0, this->videoHeight*(imagesInPipe/float(NUM_BUFFERS)), 1.0);
	glEnd();
}

/*! draws the current frame */
void videoPlayer::drawFrame(const int frame_offset)
{
	// do not show the texture unless the movie frame is ready.
	//if(imagesInPipe<1) return;

	int r = frame_offset < imagesInPipe ? frame_offset : imagesInPipe;
	int showTexture = (r+ currentReadBuffer)%NUM_BUFFERS;
	
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_frag);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	//if(imagesInPipe>0)
	for(int i=0; i<3; i++)
	{
		glActiveTexture(GL_TEXTURE0+i);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_texture[i+showTexture*NUM_YUV_TEXTURES]);
	}

	int w = this->videoWidth;
	int h = this->videoHeight;


	glColor3f(1.0, 1.0, 1.0);
	// TODO: texcoord h-4 to get rid of green line.
	// left side start at 1 and not 0 to fix green line...why?
	// super hack.
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 1.f);
		glVertex2f(0.0, 0.0);
		glTexCoord2f(1.f, 1.f);
		glVertex2f(w, 0.0);
		glTexCoord2f(1.f, 0.0);
		glVertex2f(w, h);
		glTexCoord2f(0.0, 0.0);
		glVertex2f(0.0, h);
	glEnd();

	//if(imagesInPipe>0)
	for(int i=0; i<3; i++)
	{
		glActiveTexture(GL_TEXTURE0+i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}


	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);

}

void videoPlayer::bind(int textureOffset)
{
	int frame_offset=0;
	int r = frame_offset < imagesInPipe ? frame_offset : imagesInPipe;
	int showTexture = (r+ currentReadBuffer)%NUM_BUFFERS;
/*	
	if(bindShader)
	{
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_frag);
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
	}
	*/
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	//if(imagesInPipe>0)
	for(int i=0; i<3; i++)
	{
		glActiveTexture(GL_TEXTURE0+textureOffset+i);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_texture[i+showTexture*NUM_YUV_TEXTURES]);
	}

}

void videoPlayer::unbind(int textureOffset)
{
	for(int i=0; i<3; i++)
	{
		glActiveTexture(GL_TEXTURE0+textureOffset+i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
/*
	if(unbindShader)
	{
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}
	*/
}


