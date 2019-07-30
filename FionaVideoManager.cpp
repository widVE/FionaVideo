#include "FionaVideoManager.h"
#include "FionaUT.h"
#include "FionaUtil.h"
#include <Kit3D/glslUtils.h>

FionaVideoManager::FionaVideoManager(void)
{
	//shader stuff
	program = 0;

	angSize=0;	//uniform location
	rOff=0;	//uniform location
	lOff=0;	//uniform location
	lTexY=0;	//uniform location
	lTexU=0;
	lTexV=0;
	rTexY=0;	//uniform location
	rTexU=0;
	rTexV=0;
	mode=0;	//uniform location (0 = left, 1 = right, 2 = anaglyph)
	rotateUniform=0;	//uniform location

	gDist=4.f;
	gRotX=90.f;
	gRotY=0.f;
	gImageAngleX=360.f;
	gImageAngleY=180.f;

	rx=0.f;
	ry=0.f;
	lx=0.f;
	ly=0.f;

	anaglyph=false;
	rotateIn=true;

	//time
	videoTime=videoStartTime=videoOffsetTime=0;

	rotX=rotY=rotZ=0;

	timeOffset = -6./24.;
			
}


FionaVideoManager::~FionaVideoManager(void)
{
}

double FionaVideoManager::getTime(bool useFionaTime)
{
	if (useFionaTime)
	{
		return fionaConf.physicsTime;
	}
	else
	{
		clock_t t = clock();
		return (t / double(CLOCKS_PER_SEC));
	}
}

void  FionaVideoManager::initGL()
{
	printf("init GL\n");
	 if(glewInit() != GLEW_OK)
        throw std::runtime_error("glewInit failed");

	for (int i=0; i < videos.size(); i++)
	{
		videos[i]->initGL(10);
	}

	reloadShaders();
}

bool FionaVideoManager::load(char *fname)
{
	VideoObject *v = new VideoObject();
	if (v->init(fname))
	{
		v->setRunningType(video_only);
		videos.push_back(v);
		return true;
	}
	else
	{
		delete v;
		return false;
	}
}

void FionaVideoManager::reloadShaders()
{
	std::string vshader = std::string("spheremap.vert");
	std::string fshader = std::string("spheremap.frag");
	program = loadProgramFiles(vshader,fshader,true);


	lTexY = glGetUniformLocation(program, "leftTextureY");
	lTexU = glGetUniformLocation(program, "leftTextureU");
	lTexV = glGetUniformLocation(program, "leftTextureV");
	rTexY = glGetUniformLocation(program, "rightTextureY");
	rTexU = glGetUniformLocation(program, "rightTextureU");
	rTexV = glGetUniformLocation(program, "rightTextureV");
	mode = glGetUniformLocation(program, "mode");
	angSize = glGetUniformLocation(program, "ImageAngleSize");
	rOff = glGetUniformLocation(program, "roffset");
	lOff = glGetUniformLocation(program, "loffset");
	rotateUniform = glGetUniformLocation(program, "rotateOn");
}

void FionaVideoManager::process()
{
	if (playing)
	{
		videoTime = getTime() - videoStartTime + videoOffsetTime;
	}

	
	//printf("%.2f\n", videoTime);

	for (int i=0; i < videos.size(); i++)
	{



		videos[i]->process();
		if (playing)
		{
			//note, this is assumed to have 0 or 1 videos
			videos[i]->setPlaybackTime(videoTime + double(i)*timeOffset);
		}
	}

}

void FionaVideoManager::drawSphere()
{
	
	glPushMatrix();
	//glEnable(GL_DEPTH_TEST);

	float s=3;
	//glRotatef(-90.f, 0.f, 0.f, 1.f);	//Ross 4.8.2016 this needed for 360 example
	glRotatef(90.f, 0.f, 1.f, 0.f);
	//glTranslatef(0,0,-2.f);
	/*glScalef(s,s,s);*/
	//sphere
	glUseProgram(program);

	//get the current frame
	for (int i=0; i < videos.size(); i++)
	{
		VideoObject *v = videos[i];
		v->bindFrame(3*i);
	}
	/*
	if(fionaConf.stereo && !fionaConf.splitStereo)
	{
		if(fionaRenderCycleLeft)
		{
			glUniform1i(lTexY, 0);
			glUniform1i(lTexU, 1);
			glUniform1i(lTexV, 2);
		}
		else if(fionaRenderCycleRight)
		{
			glUniform1i(rTexY, 0);
			glUniform1i(rTexU, 1);
			glUniform1i(rTexV, 2);
		}
	}
	else
	{
		glUniform1i(lTexY, 0);
		glUniform1i(lTexU, 1);
		glUniform1i(lTexV, 2);
		glUniform1i(rTexY, 3);
		glUniform1i(rTexU, 4);
		glUniform1i(rTexV, 5);
	}
	*/

	glUniform1i(lTexY, 0);
	glUniform1i(lTexU, 1);
	glUniform1i(lTexV, 2);

	if (videos.size() > 1)
	{
		glUniform1i(rTexY, 3);
		glUniform1i(rTexU, 4);
		glUniform1i(rTexV, 5);
	}
	else
	{
		glUniform1i(rTexY, 0);
		glUniform1i(rTexU, 1);
		glUniform1i(rTexV, 2);
	}

	glUniform2f(angSize, gImageAngleX, gImageAngleY);
	glUniform2f(rOff, rx, ry);
	glUniform2f(lOff, lx, ly);
	//glUniform1i(rTex, 1);
	if(rotateIn)
	{
		glUniform1i(rotateUniform, 1);
	}
	else	
	{
		glUniform1i(rotateUniform, 0);
	}

	static GLUquadric *shape = 0;
	if(shape == 0)
	{
		shape = gluNewQuadric();
		gluQuadricDrawStyle(shape, GLU_FILL);
		gluQuadricNormals(shape, GLU_SMOOTH);
	}

	anaglyph=true;

	if(!anaglyph)
	{
		if(FionaRenderCycleLeft())
		{
			glUniform1i(mode, 0);
			gluSphere(shape, 10,128,128);
		}
		else if(FionaRenderCycleRight())
		{

			glRotatef(rotZ, 1.f, 0.f, 0.f);
			glRotatef(rotX, 0.f, 1.f, 0.f);
			glRotatef(rotY, 0.f, 0.f, 1.f);

			glUniform1i(mode, 1);
			gluSphere(shape, 10,128,128);
		}
		else
		{
			//glUniform1i(mode, 2);
			//gluSphere(shape, 10,128,128);

			//lets do two draws
			for (int e=0; e<2; e++)
			{
				if (e==0)
					glColorMask(1,0,0, 1);
				else
				{
					glRotatef(rotZ, 1.f, 0.f, 0.f);
					glRotatef(rotX, 0.f, 1.f, 0.f);
					glRotatef(rotY, 0.f, 0.f, 1.f);

					glColorMask(0,1,1, 1);
				}

				glUniform1i(mode, e);
				gluSphere(shape, 10,128,128);
			}
		}
	}
	else
	{
		//lets do two draws
			for (int e=0; e<2; e++)
			{
				if (e==0)
					glColorMask(1,0,0, 1);
				else
				{
					glRotatef(rotZ, 1.f, 0.f, 0.f);
					glRotatef(rotX, 0.f, 1.f, 0.f);
					glRotatef(rotY, 0.f, 0.f, 1.f);

					glColorMask(0,1,1, 1);
				}

				glUniform1i(mode, e);
				gluSphere(shape, 10,128,128);
			}
		//glUniform1i(mode, 2);
		//gluSphere(shape, 10,128,128);
	}
//#ifdef LINUX_BUILD
	//using gluSphere on linux because otherwise we have a freeglut conflict with needing to call glutInit
	


	//(fionaConf.farClip/2.f)-25.f,512,512);
/*#else
	glutSolidSphere((fionaConf.farClip/2.f)-5.f,128,128);
#endif*/

	for (int i=0; i < videos.size(); i++)
	{
		VideoObject *v = videos[i];
		v->unbindFrame(3*i);
	}
	glUseProgram(0);
	

	//glDisable(GL_DEPTH_TEST);
	glPopMatrix();
}

void FionaVideoManager::nextFrame()
{
	for (int i=0; i < videos.size(); i++)
	{
		videos[i]->nextFrame();
	}
}

void FionaVideoManager::reset()
{
	for (int i=0; i < videos.size(); i++)
	{
		videos[i]->reset();
	}

	videoTime=0;
	videoOffsetTime=0;
	playing=false;
}

void FionaVideoManager::togglePlayback() 
{
	playing=!playing;
	if (playing)
	{
		videoStartTime = getTime();
	}
	else
	{
		videoOffsetTime = videoTime;
	}
}

