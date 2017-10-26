//
//  OpenGLView.cpp
//  ZXRetroEmulator
//
//  Created by Adrian Brown on 03/09/2016.
//  Copyright © 2016 71Squared Ltd. All rights reserved.
//

#include <string>
#include <fstream>
#include <vector>
#include "OpenGLView.hpp"

//-----------------------------------------------------------------------------------------

const GLfloat quad[] = {
	//X      Y      Z        U      V
	-1.0f,   1.0f,  0.0f,    0.0f,  1.0f,
	-1.0f,  -1.0f,  0.0f,    0.0f,  0.0f,
	1.0f,  -1.0f,  0.0f,    1.0f,  0.0f,
	1.0f,  -1.0f,  0.0f,    1.0f,  0.0f,
	-1.0f,   1.0f,  0.0f,    0.0f,  1.0f,
	1.0f,   1.0f,  0.0f,    1.0f,  1.0f
};

typedef struct
{
	float r;
	float g;
	float b;
	float a;
} Color;

// Constants for the colour lookup table
const float normalColor = 208.0f / 255.0f;
const float brightColor = 1.0f;
const Color CLUT[] = {
	// Non-bright colours
	{ 0.0, 0.0, 0.0, 1.0 },
	{ 0.0, 0.0, normalColor, 1.0 },
	{ normalColor, 0.0, 0.0, 1.0 },
	{ normalColor, 0.0, normalColor, 1.0 },
	{ 0.0, normalColor, 0.0, 1.0 },
	{ 0.0, normalColor, normalColor, 1.0 },
	{ normalColor, normalColor, 0.0, 1.0 },
	{ normalColor, normalColor, normalColor, 1.0 },

	// Bright colours
	{ 0.0, 0.0, 0.0, 1.0 },
	{ 0.0, 0.0, brightColor, 1.0 },
	{ brightColor, 0.0, 0.0, 1.0 },
	{ brightColor, 0.0, brightColor, 1.0 },
	{ 0.0, brightColor, 0.0, 1.0 },
	{ 0.0, brightColor, brightColor, 1.0 },
	{ brightColor, brightColor, 0.0, 1.0 },
	{ brightColor, brightColor, brightColor, 1.0 }
};

//-----------------------------------------------------------------------------------------

OpenGLView::OpenGLView()
{
}

//-----------------------------------------------------------------------------------------

OpenGLView::~OpenGLView()
{
	Deinit();
}

//-----------------------------------------------------------------------------------------

void OpenGLView::Deinit()
{
	wglMakeCurrent(NULL, NULL);
	ReleaseDC(m_hWnd, m_HDC);
	wglDeleteContext(m_GLRC);
}

//-----------------------------------------------------------------------------------------

bool OpenGLView::Init(HWND hWnd)
{
	unsigned int formatCount;
	int pixelformats[1];

	// Remember the hwnd
	m_hWnd = hWnd;

	// Initialise the extensions
	if (!InitialiseExtensions())
	{
		return false;
	}

	// Get the DC
	m_HDC = GetDC(m_hWnd);

	// Setup the attribute list
	int attributeListInt[] = {
		WGL_SUPPORT_OPENGL_ARB, TRUE,
		WGL_DRAW_TO_WINDOW_ARB, TRUE,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB, 24,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_DOUBLE_BUFFER_ARB, TRUE,
		WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		0
	};

	// Find a format
	if ( wglChoosePixelFormatARB(m_HDC, attributeListInt, NULL, 1, pixelformats, &formatCount) != TRUE )
	{
		return false;
	}
	
	if (SetPixelFormat(m_HDC, pixelformats[0], &m_PFD) != TRUE )
	{
		return false;
	}

	// Set the 4.0 version of OpenGL in the attribute list.
	int contextAL[] = 
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 0,
		0
	};

	m_GLRC = wglCreateContextAttribsARB(m_HDC, 0, contextAL);
	if (wglMakeCurrent(m_HDC, m_GLRC) != TRUE)
	{
		return false;
	}
	
	glClearColor(1.0f, 0.0f, 0.4f, 1.0f);

	LoadShaders();
	SetupTexture();
	SetupQuad();

	return true;
}

//-----------------------------------------------------------------------------------------

void OpenGLView::SetupQuad()
{

	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	glUseProgram(shaderProgName);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureName);
	glUniform1i(textureID, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, clutTextureName);
	glUniform1i(s_clutTexture, 1);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)12);
}

//-----------------------------------------------------------------------------------------

void OpenGLView::LoadShaders()
{
	std::string vertex_file_path = "SpectREM\\display.vsh";
	std::string fragment_file_path = "SpectREM\\display.fsh";

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open())
	{
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
		{
			VertexShaderCode += "\n" + Line;
		}
		VertexShaderStream.close();
	}
	else
	{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path.c_str());
		getchar();
		return;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) 
	{
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
		{
			FragmentShaderCode += "\n" + Line;
		}
		FragmentShaderStream.close();
	}
	else
	{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", fragment_file_path.c_str());
		getchar();
		return;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path.c_str());
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path.c_str());
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	shaderProgName = glCreateProgram();
	glAttachShader(shaderProgName, VertexShaderID);
	glAttachShader(shaderProgName, FragmentShaderID);
	glLinkProgram(shaderProgName);

	// Check the program
	glGetProgramiv(shaderProgName, GL_LINK_STATUS, &Result);
	glGetProgramiv(shaderProgName, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(shaderProgName, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(shaderProgName, VertexShaderID);
	glDetachShader(shaderProgName, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	textureID = glGetUniformLocation(shaderProgName, "displayTexture");
	s_clutTexture = glGetUniformLocation(shaderProgName, "clutTexture");
	u_borderSize = glGetUniformLocation(shaderProgName, "u_borderSize");
	u_contrast = glGetUniformLocation(shaderProgName, "u_contrast");
	u_saturation = glGetUniformLocation(shaderProgName, "u_saturation");
	u_brightness = glGetUniformLocation(shaderProgName, "u_brightness");
	u_scanlineSize = glGetUniformLocation(shaderProgName, "u_scanlineSize");
	u_scanlines = glGetUniformLocation(shaderProgName, "u_scanlines");
	u_screenCurve = glGetUniformLocation(shaderProgName, "u_screenCurve");
}

//-----------------------------------------------------------------------------------------

void OpenGLView::SetupTexture()
{
	glGenTextures(1, &textureName);
	glBindTexture(GL_TEXTURE_2D, textureName);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 320, 256, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &clutTextureName);
	glBindTexture(GL_TEXTURE_2D, clutTextureName);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 16, 0, GL_RGBA, GL_FLOAT, CLUT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

}

//-----------------------------------------------------------------------------------------

void OpenGLView::UpdateTextureData(unsigned char *pData)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glProgramUniform1f(shaderProgName, u_borderSize, 32.0f);
 	glProgramUniform1f(shaderProgName, u_contrast, 0.75f);
 	glProgramUniform1f(shaderProgName, u_saturation, 1.0f);
 	glProgramUniform1f(shaderProgName, u_brightness, 1.0f);
 	glProgramUniform1f(shaderProgName, u_scanlineSize, 0.0f);
 	glProgramUniform1f(shaderProgName, u_scanlines, 960.0f);
 	glProgramUniform1f(shaderProgName, u_screenCurve, 0.0f);

	glBindTexture(GL_TEXTURE_2D, textureName);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 320, 256, GL_RED, GL_UNSIGNED_BYTE, pData);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glFlush();

	SwapBuffers(m_HDC);
}

//-----------------------------------------------------------------------------------------

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//-----------------------------------------------------------------------------------------

bool OpenGLView::InitialiseExtensions()
{
	bool extLoaded = false; 

	// Create our window class
	WNDCLASSEX wcex;

	memset(&wcex, 0, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hCursor = NULL;
	wcex.hbrBackground = NULL;
	wcex.lpszClassName = "OpenGL Extension Checking";
	RegisterClassEx(&wcex);

	// Create a window to use 
	HWND hwnd = CreateWindowEx(WS_EX_APPWINDOW, "OpenGL Extension Checking", "OpenGL Extension Checking", WS_POPUP, 0, 0, 640, 480, NULL, NULL, GetModuleHandle(NULL), NULL);
	
	if (hwnd == NULL)
	{
		int res = GetLastError();
		return false;
	}

	// Make sure we dont see this window
	ShowWindow(hwnd, SW_HIDE);

	// Get the DC
	m_HDC = GetDC(hwnd);

	// Select the pixel format
	if (SetPixelFormat(m_HDC, 1, &m_PFD) == TRUE)
	{
		m_GLRC = wglCreateContext(m_HDC);
		wglMakeCurrent(m_HDC, m_GLRC);

		// Lets see what extensions we have
		extLoaded = LoadExtensionList();

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_GLRC);
		ReleaseDC(hwnd, m_HDC);
		m_GLRC = NULL;
		m_HDC = NULL;
	}

	// Destroy the window
	DestroyWindow(hwnd);

	return extLoaded;
}

//-----------------------------------------------------------------------------------------

bool OpenGLView::LoadExtensionList()
{
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if (!wglChoosePixelFormatARB)
	{
		return false;
	}

	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	if (!wglCreateContextAttribsARB)
	{
		return false;
	}

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if (!wglSwapIntervalEXT)
	{
		return false;
	}

	glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
	if (!glAttachShader)
	{
		return false;
	}

	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	if (!glBindBuffer)
	{
		return false;
	}

	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
	if (!glBindVertexArray)
	{
		return false;
	}

	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	if (!glBufferData)
	{
		return false;
	}

	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
	if (!glCompileShader)
	{
		return false;
	}

	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
	if (!glCreateProgram)
	{
		return false;
	}

	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
	if (!glCreateShader)
	{
		return false;
	}

	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
	if (!glDeleteBuffers)
	{
		return false;
	}

	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
	if (!glDeleteProgram)
	{
		return false;
	}

	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
	if (!glDeleteShader)
	{
		return false;
	}

	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
	if (!glDeleteVertexArrays)
	{
		return false;
	}

	glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
	if (!glDetachShader)
	{
		return false;
	}

	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
	if (!glEnableVertexAttribArray)
	{
		return false;
	}

	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
	if (!glGenBuffers)
	{
		return false;
	}

	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
	if (!glGenVertexArrays)
	{
		return false;
	}

	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
	if (!glGetAttribLocation)
	{
		return false;
	}

	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
	if (!glGetProgramInfoLog)
	{
		return false;
	}

	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
	if (!glGetProgramiv)
	{
		return false;
	}

	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
	if (!glGetShaderInfoLog)
	{
		return false;
	}

	glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
	if (!glGetShaderiv)
	{
		return false;
	}

	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
	if (!glLinkProgram)
	{
		return false;
	}

	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
	if (!glShaderSource)
	{
		return false;
	}

	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
	if (!glUseProgram)
	{
		return false;
	}

	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
	if (!glVertexAttribPointer)
	{
		return false;
	}

	glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)wglGetProcAddress("glBindAttribLocation");
	if (!glBindAttribLocation)
	{
		return false;
	}

	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
	if (!glGetUniformLocation)
	{
		return false;
	}

	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
	if (!glUniformMatrix4fv)
	{
		return false;
	}

	glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
	if (!glActiveTexture)
	{
		return false;
	}

	glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
	if (!glUniform1i)
	{
		return false;
	}

	glProgramUniform1f = (PFNGLPROGRAMUNIFORM1FPROC)wglGetProcAddress("glProgramUniform1f");
	if (!glProgramUniform1f)
	{
		return false;
	}

	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
	if (!glGenerateMipmap)
	{
		return false;
	}

	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
	if (!glDisableVertexAttribArray)
	{
		return false;
	}

	glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3fv");
	if (!glUniform3fv)
	{
		return false;
	}

	glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
	if (!glUniform4fv)
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------
