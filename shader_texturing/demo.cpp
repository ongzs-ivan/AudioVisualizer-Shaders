#define GLFW_INCLUDE_ES2 1
#define GLFW_DLL 1
//#define GLFW_EXPOSE_NATIVE_WIN32 1
//#define GLFW_EXPOSE_NATIVE_EGL 1

// OPENGL libraries
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <GLFW/glfw3.h>
//#include <GLFW/glfw3native.h>

// FMOD libraries

#include <fmod.hpp>
#include <fmod_errors.h>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream> 
#include <ctime>
#include "bitmap.h"

#define WINDOW_WIDTH 800 
#define WINDOW_HEIGHT 600
#define SPECTRUM_SIZE 64
#define TEXTURE_COUNT 2

GLint GprogramID = -1;
GLuint GtextureID[TEXTURE_COUNT];

GLFWwindow* window;

#define _USE_MATH_DEFINES
#include <math.h>

FMOD::System* m_fmodSystem;
FMOD::Sound* m_music;
FMOD::Channel* m_musicChannel;

float m_spectrumLeft[SPECTRUM_SIZE];
float m_spectrumRight[SPECTRUM_SIZE];
float m_spectrumAvg[SPECTRUM_SIZE];

GLuint mTextureID[TEXTURE_COUNT];

static float timer = 0.0f;
float oldTime = 0.0f;
float deltaTime = 0.0f;

void loadTexture(const char* path, GLuint textureID)
{
	CBitmap bitmap(path);

	// Create linear filtered texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //apply texture wrapping along horizontal
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //apply texture wrapping along vertical

	// old school (minecraft) filtering
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // near filtering
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // far filtering
	//

	//bilinear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // near filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // far filtering


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.GetWidth(), bitmap.GetHeight(),
		0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.GetBits());
}

static void error_callback(int error, const char* description)
{
  fputs(description, stderr);
}

GLuint LoadShader(GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   	return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
		 char infoLog[4096];
         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         printf ( "Error compiling shader:\n%s\n", infoLog );            
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;
}

GLuint LoadShaderFromFile(GLenum shaderType, std::string path)
{
    GLuint shaderID = 0;
    std::string shaderString;
    std::ifstream sourceFile( path.c_str() );

    if( sourceFile )
    {
        shaderString.assign( ( std::istreambuf_iterator< char >( sourceFile ) ), std::istreambuf_iterator< char >() );
        const GLchar* shaderSource = shaderString.c_str();

		return LoadShader(shaderType, shaderSource);
    }
    else
        printf( "Unable to open file %s\n", path.c_str() );

    return shaderID;
}

void ERRCHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}
}

void initFmod()
{
	FMOD_RESULT result;
	unsigned int version;

	result = FMOD::System_Create(&m_fmodSystem);
	ERRCHECK(result);

	result = m_fmodSystem->getVersion(&version);
	ERRCHECK(result);

	if (version < FMOD_VERSION)
		printf("FMOD Error! You are using an old version of FMOD.", version, FMOD_VERSION);

	// initialize fmod system
	result = m_fmodSystem->init(32, FMOD_INIT_NORMAL, 0);
	ERRCHECK(result);

	// load and set up music
	result = m_fmodSystem->createStream("../media/RX0.mp3", FMOD_SOFTWARE, 0, &m_music);
	ERRCHECK(result);

	// play the loaded mp3 music
	result = m_fmodSystem->playSound(FMOD_CHANNEL_FREE, m_music, false, &m_musicChannel);
	ERRCHECK(result);

	// set sound channel loop count
	m_musicChannel->setLoopCount(0);
}

int Init ( void )
{
	initFmod();
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;
    
    //load textures
    glGenTextures(TEXTURE_COUNT, GtextureID);
    loadTexture("../media/RX0.bmp", GtextureID[0]);
    //loadTexture("../media/background1.bmp", GtextureID[1]);
    //====
    
    fragmentShader = LoadShaderFromFile(GL_VERTEX_SHADER, "../vertexShader1.vert" );
    vertexShader = LoadShaderFromFile(GL_FRAGMENT_SHADER, "../fragmentShader1.frag" );
    
    // Create the program object
    programObject = glCreateProgram ( );
    
    if ( programObject == 0 )
       return 0;
    
    glAttachShader ( programObject, vertexShader );
    glAttachShader ( programObject, fragmentShader );
    
    glBindAttribLocation ( programObject, 0, "vPosition" );
    glBindAttribLocation ( programObject, 1, "vColor" );
    glBindAttribLocation ( programObject, 2, "vTexCoord" );
    
    // Link the program
    glLinkProgram ( programObject );
    
    // Check the link status
    glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );
    
    if ( !linked ) 
    {
       GLint infoLen = 0;
    
       glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
       
       if ( infoLen > 1 )
       {
	 	 char infoLog[1024];
          glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
          printf ( "Error linking program:\n%s\n", infoLog );            
       }
    
       glDeleteProgram ( programObject );
       return 0;
    }
    
    // Store the program object
    GprogramID = programObject;
    
    glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    return 1;
}

int Deinit(void)
{
	return 0;
}

float hearingDecibel = 0.25f;

void updateFmod()
{
	m_fmodSystem->update();
	// Get spectrum for left and right stereo channels
	m_musicChannel->getSpectrum(m_spectrumLeft, SPECTRUM_SIZE, 0, FMOD_DSP_FFT_WINDOW_RECT);
	m_musicChannel->getSpectrum(m_spectrumRight, SPECTRUM_SIZE, 1, FMOD_DSP_FFT_WINDOW_RECT);

	//std::cout << m_spectrumLeft[0] << "," << m_spectrumRight[0] << std::endl;

	// average the left and right spectrum
	for (int i = 0; i < 6; i++)
	{
		m_spectrumAvg[i] = (m_spectrumLeft[i] + m_spectrumRight[i]) * 0.5f * 10;
	}

	//std::cout << (m_spectrumLeft[0] + m_spectrumRight[0]) * 0.5f << std::endl;
	//m_spectrumAvg[0] = (m_spectrumLeft[0] + m_spectrumRight[0]) * 0.5f;
	//std::cout << m_spectrumAvg << std::endl;

	//if (m_spectrumAvg[0] > hearingDecibel)
	//{
	//	std::cout << "Lows: " << m_spectrumAvg[0] << std::endl;
	//}
	//if (m_spectrumAvg[1] >= hearingDecibel)
	//{
	//	std::cout << "Mids1: " << m_spectrumAvg[1] << std::endl;
	//}
	//if (m_spectrumAvg[2] >= hearingDecibel)
	//{
	//	std::cout << "Mids2: " << m_spectrumAvg[2] << std::endl;
	//}
	//if (m_spectrumAvg[3] >= hearingDecibel)
	//{
	//	std::cout << "Mids3: " << m_spectrumAvg[3] << std::endl;
	//}
	//if (m_spectrumAvg[4] >= hearingDecibel)
	//{
	//	std::cout << "Mids4: " << m_spectrumAvg[4] << std::endl;
	//}
	//if (m_spectrumAvg[5] >= hearingDecibel)
	//{
	//	std::cout << "High: " << m_spectrumAvg[5] << std::endl;
	//}
}

void Draw(void)
{
	updateFmod();
	// set the sampler2D varying variable to the first texture unit(index 0)
	glUniform1i(glGetUniformLocation(GprogramID, "sampler2d"), 0);
	
	// uniform variables declaration //
	static float factor1 = 0.0f;

	static float low;
	low = m_spectrumAvg[0];

	static float mids[4];
	mids[0] = m_spectrumAvg[1];
	mids[1] = m_spectrumAvg[2];
	mids[2] = m_spectrumAvg[3];
	mids[3] = m_spectrumAvg[4];

	static float high;
	high = m_spectrumAvg[5];

	static int highCount = 0;
	if (m_spectrumAvg[5] >= 1.5)
	{
		highCount++;
	}

	factor1 += 0.05f;
	
	GLint timeLoc = glGetUniformLocation(GprogramID, "Time");
	if (timeLoc != -1)
	{
		glUniform1f(timeLoc, deltaTime);
	}

	GLint lowsLoc = glGetUniformLocation(GprogramID, "Low");
	if (lowsLoc != -1)
	{
		glUniform1f(lowsLoc, low);
	}

	GLint midsLoc = glGetUniformLocation(GprogramID, "Mids");
	if (midsLoc != -1)
	{
		glUniform1fv(midsLoc, 4, mids);
	}

	GLint factor1Loc = glGetUniformLocation(GprogramID, "Factor1");
	if(factor1Loc != -1)
	{
	   glUniform1f(factor1Loc, factor1);
	}

	GLint highsLoc = glGetUniformLocation(GprogramID, "High");
	if (highsLoc != -1)
	{
		glUniform1f(highsLoc, high);
	}

	GLint highCountLoc = glGetUniformLocation(GprogramID, "HighCount");
	if (highCountLoc != -1)
	{
		glUniform1i(highCountLoc, highCount);
	}
	//===================

   static GLfloat vVertices[] = {-1.0,  1.0, 0.0f,
								  -1.0, -1.0, 0.0f,
                                  1.0, -1.0,  0.0f,
                                  1.0,  -1.0, 0.0f,
                                  1.0, 1.0, 0.0f,
								  -1.0, 1.0,  0.0f};
					  
   static GLfloat vColors[] = {1.0f,  0.0f, 0.0f, 1.0f,
								0.0f, 1.0f, 0.0f, 1.0f,
								0.0f, 0.0f,  1.0f, 1.0f,
								0.0f,  0.0f, 1.0f, 1.0f,
								1.0f, 1.0f, 0.0f, 1.0f,
								1.0f, 0.0f,  0.0f, 1.0f};

   static GLfloat vTexCoords[] = {0.0f,  1.0f,
									0.0f, 0.0f,
									1.0f, 0.0f,
									1.0f,  0.0f,
									1.0f, 1.0f,
									0.0f, 1.0f};

   glBindTexture(GL_TEXTURE_2D, GtextureID[0]);

   // Set the viewport
   glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

   // Clear the color buffer
   glClear(GL_COLOR_BUFFER_BIT);

   // Use the program object
   glUseProgram(GprogramID);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
   glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, vColors);
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, vTexCoords);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);
   glEnableVertexAttribArray(2);

   glDrawArrays(GL_TRIANGLES, 0, 6);

   glDisableVertexAttribArray(0);
   glDisableVertexAttribArray(1);
   glDisableVertexAttribArray(2);
}

int main(void)
{
	srand(time(NULL));
	glfwSetErrorCallback(error_callback);

	// Initialize GLFW library
	if (!glfwInit())
	return -1;

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create and open a window
	window = glfwCreateWindow(WINDOW_WIDTH,
							WINDOW_HEIGHT,
							"Audio Visualizer - by Ivan Ong",
							NULL,
							NULL);

	if (!window)
	{
	glfwTerminate();
	printf("glfwCreateWindow Error\n");
	exit(1);
	}

	glfwMakeContextCurrent(window);

	Init();

	// Repeat
	while (!glfwWindowShouldClose(window)) 
	{
		deltaTime = clock() - oldTime;
		oldTime = clock();

	Draw();
	glfwSwapBuffers(window);
	glfwPollEvents();
	}

	Deinit();

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
