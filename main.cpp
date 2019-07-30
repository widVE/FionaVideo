//
//  main.cpp
//  Base Skeleton framework for running our own C++ framework in the CAVE / Dev Lab / Oculus / etc.
//  Adapted from Hyun Joon Shin and Ross Tredinnick's FionaUT project


#define WITH_FIONA
#ifdef WIN32
#include "gl/glew.h"
#else
#include "GL/glew.h"
#endif

#include <cmath>
#include <math.h>

#include "FionaUT.h"

#ifdef WIN32
#include "FionaUtil.h"
#endif
#include <Kit3D/glslUtils.h>
#include <Kit3D/glUtils.h>

#ifdef LINUX_BUILD
#define FALSE 0
#endif

#include "FionaVideoManager.h"

FionaVideoManager videoManager;
/*
//Video stuff
#include "videoObject.h"
VideoObject videoObject;
//number of buffered frames
//TODO  make this a command line options
int gNumBuffers = 100;
GLuint program = 0;

GLuint angSize=0;	//uniform location
GLuint rOff=0;	//uniform location
GLuint lOff=0;	//uniform location
GLuint lTexY=0;	//uniform location
GLuint lTexU=0;
GLuint lTexV=0;
GLuint rTexY=0;	//uniform location
GLuint rTexU=0;
GLuint rTexV=0;
GLuint mode=0;	//uniform location (0 = left, 1 = right, 2 = anaglyph)
GLuint rotateUniform=0;	//uniform location

float gDist=4.f;
float gRotX=90.f;
float gRotY=0.f;
float gImageAngleX=360.f;
float gImageAngleY=180.f;

float rx=0.f;
float ry=0.f;
float lx=0.f;
float ly=0.f;

bool anaglyph=false;
bool rotateIn=true;

void drawCube(GLfloat s);

#include "GLUT/glut.h"

#include "fmod/inc/fmod.hpp"
*/
class FionaScene;
FionaScene* scene = NULL;
/*
extern bool cmp(const std::string& a, const std::string& b);
extern std::string getst(char* argv[], int& i, int argc);
/*static bool cmp(const char* a, const char* b)
{
	if( strlen(a)!=strlen(b) ) return 0;
	size_t len = MIN(strlen(a),strlen(b));
	for(size_t i=0; i<len; i++)
		if(toupper(a[i]) != toupper(b[i])) return 0;
	return 1;
}

static bool cmpExt(const std::string& fn, const std::string& ext)
{
	std::string extt = fn.substr(fn.rfind('.')+1,100);
	std::cout<<"The extension: "<<extt<<std::endl;
	return cmp(ext.c_str(),extt.c_str());
}
*/

jvec3 curJoy(0.f,0.f,0.f);
jvec3 secondCurJoy(0.f, 0.f, 0.f);
jvec3 pos(0.f,0.f,0.f);
jvec3 secondPos(0.f, 0.f, 0.f);
quat ori(1.f,0.f,0.f,0.f);
quat secondOri(1.f, 0.f, 0.f, 0.f);

//int		calibMode = 0;

//video
bool firstRun=true;
float gStartTime = 0.f;
/*
void initGL()
{
	
	videoObject.initGL(gNumBuffers);
	//for now
	videoObject.setRunningType(video_only);
	//start over
	videoObject.reset();
	//will this buffer some frames for us?
	for (int i=0; i<gNumBuffers*.75; i++)
		videoObject.process();

	//gStartTime = FionaUTTime();
	gStartTime = fionaConf.physicsTime;
}

void drawCubeforVideo(int w, int h)
{
glColor3f(1,1,1);

glBegin(GL_QUADS);
    // Front Face
/*
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
    glTexCoord2f(w*1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(w*1.0f, h*1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, h*1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
*/
/*
    // Back Face
    glTexCoord2f(0.0f, h*1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(0.0f, 0); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(w*1.0f, 0.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
    glTexCoord2f(w*1.0f, h*1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
    // Top Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, h*1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
    glTexCoord2f(w*1.0f, h*1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(w*1.0f, 0.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
    // Bottom Face
    glTexCoord2f(0.0f, h*1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(w*1.0f, h*1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Top Left Of The Texture and Quad
    glTexCoord2f(w*1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
    // Right face
    glTexCoord2f(w*1.0f, h*1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(w*1.0f, 0.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, h*1.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
    // Left Face
    glTexCoord2f(0.0f, h*1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
    glTexCoord2f(w*1.0f, h*1.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(w*1.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
glEnd();
}

void drawVideoCube()
{
	//size of the cube in meters
	static bool cube = false;

	glPushMatrix();
	//glEnable(GL_DEPTH_TEST);
	if(cube)
	{
		float s=3;
		glTranslatef(0,0,-2);
		glScalef(s,s,s);
		glUseProgram(program);
		videoObject.bindFrame();
		drawCubeforVideo(videoObject.getWidth(), videoObject.getHeight());
		videoObject.unbindFrame();
		glUseProgram(0);
	}
	else
	{
		float s=3;
		glRotatef(-90.f, 0.f, 0.f, 1.f);	//Ross 4.8.2016 this needed for 360 example
		glRotatef(90.f, 0.f, 1.f, 0.f);
		//glTranslatef(0,0,-2.f);
		/*glScalef(s,s,s);*/
		//sphere
/*		glUseProgram(program);
		
		videoObject.bindFrame(false);

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

		if(!anaglyph)
		{
			if(FionaRenderCycleLeft())
			{
				glUniform1i(mode, 0);
			}
			else if(FionaRenderCycleRight())
			{
				glUniform1i(mode, 1);
			}
			else
			{
				glUniform1i(mode, 2);
			}
		}
		else
		{
			glUniform1i(mode, 2);
		}
//#ifdef LINUX_BUILD
		//using gluSphere on linux because otherwise we have a freeglut conflict with needing to call glutInit
		static GLUquadric *shape = 0;
		if(shape == 0)
		{
			shape = gluNewQuadric();
			gluQuadricDrawStyle(shape, GLU_FILL);
			gluQuadricNormals(shape, GLU_SMOOTH);
		}
		gluSphere(shape, 10,128,128);//(fionaConf.farClip/2.f)-25.f,512,512);
/*#else
		glutSolidSphere((fionaConf.farClip/2.f)-5.f,128,128);
#endif*/
/*
		videoObject.unbindFrame(false);
		glUseProgram(0);
	}

	//glDisable(GL_DEPTH_TEST);
	glPopMatrix();
}
*/
void wandBtns(int button, int state, int idx)
{/*
	if(!fionaConf.dualView || _FionaUTIsSingleViewMachine())
	{
		if(scene)
		{
			scene->buttons(button, state);
		}
	}
	else if(_FionaUTIsDualViewMachine())
	{
		if(idx == 1)
		{
			if(scene)
			{
				scene->buttons(button, state);
			}
		}
	}
	*/
}

void keyboard(unsigned int key, int x, int y)
{
	

	printf("key = %d\n", key);

	float rSpeed=1.0f;

	if (key == 'R')
	{
		videoManager.reset();
//		channel->setPosition(0, FMOD_TIMEUNIT_MS);
	}
	else if (key ==' ')
	{
		videoManager.togglePlayback();
	}
	else if (key =='A')
	{
		videoManager.rotateX(-rSpeed);
	}
	else if (key =='D')
	{
		videoManager.rotateX(rSpeed);
	}
	else if (key =='W')
	{
		videoManager.rotateY(-rSpeed);
	}
	else if (key =='S')
	{
		videoManager.rotateY(rSpeed);
	}
	else if (key =='Q')
	{
		videoManager.rotateZ(-rSpeed);
	}
	else if (key =='E')
	{
		videoManager.rotateZ(rSpeed);
	}
	else if (key =='P')
	{
		videoManager.increaseTimeOffset(-.016);
	}
	else if (key =='O')
	{
		videoManager.increaseTimeOffset(.016);
	}
}

void joystick(int w, const jvec3& v)
{
	/*
	if(!fionaConf.dualView || _FionaUTIsSingleViewMachine())
	{
		curJoy = v;
		if(scene) 
		{
			scene->updateJoystick(v);
		}
	}
	else if(_FionaUTIsDualViewMachine())
	{
		//need this sort of construct because we don't want to 
		//update the first viewer's scene if the second joystick is moved...
		if(w == 1)
		{
			secondCurJoy = v;
			if(scene) 
			{
				scene->updateJoystick(v);
			}
		}
	}
	*/
}

void mouseBtns(int button, int state, int x, int y)
{

}

void mouseMove(int x, int y) 
{

}

void tracker(int s,const jvec3& p, const quat& q)
{ 
	/*
	if(s==1)
	{
		fionaConf.wandPos = p;
		fionaConf.wandRot = q;
	}
	else if(s == 3)
	{
		fionaConf.secondWandPos = p;
		fionaConf.secondWandRot = q;
	}

	if((s==1 || s==3) && scene)
	{
		scene->updateWand(p,q); 
	}
	*/
}

void preDisplay(float value)
{
	
	if (firstRun)
	{
		videoManager.initGL();
		firstRun=false;
	}
	videoManager.process();

	/*
	if(scene != 0)
	{
		scene->preRender(value);
	}
	



	//float videoTime = FionaUTTime() - gStartTime;
	float videoTime = fionaConf.physicsTime - gStartTime;	//physicsTime should be synced in the cave.  gStartTime should be very close...
	printf("mytime: %f \t videotime: %f\n", videoTime, videoObject.getVideoTime());
	//moved to fmod section
	videoObject.process();
	videoObject.setPlaybackTime(videoTime);
	//FionaUTSleep(0.01f);
	*/
}

void postDisplay(void)
{
	/*
	if(scene != 0)
	{
		scene->postRender();
	}
	*/

}

void render(void)//////////////////////////////////////////////////////////////////////////////////
{

	if (firstRun)
	{
		
		/*
		initGL();
		firstRun=false;
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
		*/
	}

	videoManager.drawSphere();

	/*
	//allow rotations
	if( FionaIsFirstOfCycle() )
	{
		if(!fionaConf.dualView || _FionaUTIsSingleViewMachine())
		{
			pos+=jvec3(-curJoy.y*0.01f,0,curJoy.z*0.01f);
			ori =exp(YAXIS*curJoy.x*0.01f)*ori;
		}
		else if(_FionaUTIsDualViewMachine())
		{
			secondPos+=jvec3(0,0,secondCurJoy.z*0.01f);
			secondOri =exp(YAXIS*secondCurJoy.x*0.01f)*secondOri;
		}
	}

	if(!fionaConf.dualView || _FionaUTIsSingleViewMachine())
	{
		glRotate(ori);
		glTranslate(pos);
		
	}
	else if(_FionaUTIsDualViewMachine())
	{
		glTranslate(-secondPos);
		glRotate(secondOri);
	}

	//video
	if(scene!=NULL)
	{
		scene->render();
	}
	else 
	{
		drawVideoCube();
	}
	*/
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/*
void drawCube(GLfloat s)
{
	glBegin(GL_QUADS);
	glVertex3f(-s, s, s);	//front face
	glVertex3f(-s, -s, s);
	glVertex3f(s, -s, s);
	glVertex3f(s, s, s);
	glVertex3f(s, s, s);	//right face
	glVertex3f(s, -s, s);
	glVertex3f(s, -s, -s);
	glVertex3f(s, s, -s);
	glVertex3f(s, s, -s);	//back face
	glVertex3f(s, -s, -s);
	glVertex3f(-s, -s, -s);
	glVertex3f(-s, s, -s);
	glVertex3f(-s, s, -s);	//left face
	glVertex3f(-s, -s, -s);
	glVertex3f(-s, -s, s);
	glVertex3f(-s, s, s);
	glVertex3f(-s, -s, s);	//bottom face
	glVertex3f(-s, -s, -s);
	glVertex3f(s, -s, -s);
	glVertex3f(s, -s, s);
	glVertex3f(-s, s, s);	//front face
	glVertex3f(-s, s, -s);
	glVertex3f(s, s, -s);
	glVertex3f(s, s, s);
	glEnd();
}
*/

void cleanup(int errorCode)
{

}

int main(int argc, char *argv[])
{

////FMOD stuff/////////////////////////////////////////////////////////////////////////////////////

    int               key;
    unsigned int      version;

    
      //  Create a fmodsystem object and initialize.
    
   /* result = FMOD::System_Create(&fmodsystem);
    ERRCHECK(result);

    result = fmodsystem->getVersion(&version);
    ERRCHECK(result);

    if (version < FMOD_VERSION)
    {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
//        getch();
        return 0;
    }

    result = fmodsystem->init(1, FMOD_INIT_NORMAL, 0);
    ERRCHECK(result);

    result = fmodsystem->createSound("/mnt/dscvr/apps/MusicTeam/song.wav", FMOD_SOFTWARE | FMOD_2D | FMOD_CREATESTREAM, 0, &sound);
    ERRCHECK(result);

    printf("====================================================================\n");
    printf("PlayStream Example.  Copyright (c) Firelight Technologies 2004-2014.\n");
    printf("====================================================================\n");
    printf("\n");
    printf("Press space to pause, Esc to quit\n");
    printf("\n");
	*/
/*
        Play the sound.
    

    result = fmodsystem->playSound(FMOD_CHANNEL_FREE, sound, false, &channel);
	channel->setLoopCount(-1);
channel->setMode(FMOD_LOOP_NORMAL);
channel->setPosition(0, FMOD_TIMEUNIT_MS); // this flushes the buffer to ensure the loop mode takes effect
    ERRCHECK(result);
///////////////////////////////////////////////////////////////////////////////////////////////////
*/

	glutInit(&argc, argv);
	glewInit();

	float measuredIPD=63.5;
	int userID=0;
	bool writeFull=false;
	std::string fn;

	char *filename = argv[1];

	if(filename != 0)
	{
		

		if (!videoManager.load(filename))
		{
			//we didn't find the video file, lets try to see if this a stereo video
			std::string fname = argv[1];
			videoManager.load((char*)(fname + std::string("-left.mp4")).c_str());
			videoManager.load((char*)(fname + std::string("-right.mp4")).c_str());
		}

		if(videoManager.numLoadedVideos() > 0)
		{

			

			glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
	
			glutCreateWindow	("Window");
			glutDisplayFunc		(render);
			glutJoystickFunc	(joystick);
			glutMouseFunc		(mouseBtns);
			glutMotionFunc		(mouseMove);
			glutWandButtonFunc	(wandBtns);
			glutTrackerFunc		(tracker);
			glutKeyboardFunc	(keyboard);
			glutCleanupFunc		(cleanup);
			glutFrameFunc		(preDisplay);
			glutPostRender		(postDisplay);


			

			glutMainLoop();

		//	delete scene;

		 /*FMOD
				Shut down
    
			result = sound->release();
			ERRCHECK(result);
			result = fmodsystem->close();
			ERRCHECK(result);
			result = fmodsystem->release();
			ERRCHECK(result);
			*/
		}
	}
	else
	{
		printf("Please supply file or link...\n");
	}

	return 0;
}
