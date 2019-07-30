#pragma once

#include "VideoObject.h"
#include <vector>

class FionaVideoManager
{
private:
	bool playing;
	double videoTime, videoStartTime, videoOffsetTime;

	std::vector<VideoObject*> videos;

	double getTime(bool useFionaTime=true);

	//for shaders
	GLuint program;

	GLuint angSize;	//uniform location
	GLuint rOff;	//uniform location
	GLuint lOff;	//uniform location
	GLuint lTexY;	//uniform location
	GLuint lTexU;
	GLuint lTexV;
	GLuint rTexY;	//uniform location
	GLuint rTexU;
	GLuint rTexV;
	GLuint mode;	//uniform location (0 = left, 1 = right, 2 = anaglyph)
	GLuint rotateUniform;	//uniform location

	float gDist;
	float gRotX;
	float gRotY;
	float gImageAngleX;
	float gImageAngleY;

	float rx;
	float ry;
	float lx;
	float ly;

	bool anaglyph;
	bool rotateIn;

	//to align videos
	//we need to figure out how to automate this
	float rotX, rotY, rotZ;
	double timeOffset;

public:
	FionaVideoManager(void);
	~FionaVideoManager(void);

	bool load(char *fname);

	void initGL();
	void reloadShaders();

	void drawSphere();

	void drawDebug();

	void process();

	void nextFrame();
	void togglePlayback();
	void reset();

	void rotateX(float f){rotX+=f;}
	void rotateY(float f){rotY+=f;}
	void rotateZ(float f){rotZ+=f;}
	void increaseTimeOffset(double d){timeOffset+=d;}

	int numLoadedVideos(){return videos.size();}
};

