//
//  OpenGLView.cpp
//  ZXRetroEmulator
//
//  Created by Adrian Brown on 03/09/2016.
//  Copyright © 2016 71Squared Ltd. All rights reserved.
//

#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "OpenGLView.hpp"
#include <unordered_map>

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) do { \
        stmt; \
      } while (0)
#endif

//-----------------------------------------------------------------------------------------

static const GLint textureUnit0 = 0;
static const GLint textureUnit1 = 1;

static const GLuint borderWidth = 32;
static const GLuint screenWidth = borderWidth + 256 + borderWidth;
static const GLuint screenHeight = borderWidth + 192 + borderWidth;

static char const * cS_DISPLAY_TEXTURE = "s_displayTexture";
static char const * cS_CLUT_TEXTURE = "s_clutTexture";
static char const * cS_REFLECTION_TEXTURE = "s_reflectionTexture";
static char const * cU_BORDER_SIZE = "u_borderSize";
static char const * cU_CONTRAST = "u_contrast";
static char const * cU_SATURATION = "u_saturation";
static char const * cU_BRIGHTNESS = "u_brightness";
static char const * cU_PIXEL_FILTER_VALUE = "u_pixelFilterValue";
static char const * cU_SCAN_LINE_SIZE = "u_scanlineSize";
static char const * cU_SCAN_LINES = "u_scanlines";
static char const * cU_SCREEN_CURVE = "u_screenCurve";
static char const * cU_RGB_OFFSET = "u_rgbOffset";
static char const * cU_SHOW_VIGNETTE = "u_showVignette";
static char const * cU_VIGNETTE_X = "u_vignetteX";
static char const * cU_VIGNETTE_Y = "u_vignetteY";
static char const * cU_SHOW_REFLECTION = "u_showReflection";
static char const * cU_TIME = "u_time";
static char const * cU_SCREEN_SIZE = "u_screenSize";

static std::string cDISPLAY_VERT_SHADER = "";
static std::string cDISPLAY_FRAG_SHADER = "";
static std::string cCLUT_VERT_SHADER = "";
static std::string cCLUT_FRAG_SHADER = "";

/**
     3-----2 <--  1
     |\    |
     |  \  | <--  0
     |    \|
     0-----1 <-- -1
     ^  ^  ^
     |  |  |
    -1  0  1
 **/
static const GLfloat quad[] = {
    //X      Y      Z        U      V
    -1.0f,  -1.0f,  0.0f,    0.0f,  0.0f, // 0
     1.0f,  -1.0f,  0.0f,    1.0f,  0.0f, // 1
     1.0f,   1.0f,  0.0f,    1.0f,  1.0f, // 2
    -1.0f,   1.0f,  0.0f,    0.0f,  1.0f  // 3
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
	// check if under VS/Debugger and set up ROM paths accordingly
	// could probably remove these.. Will test further first
	if (IsDebuggerPresent() != 0)
	{
		cDISPLAY_VERT_SHADER = "\\SpectREM\\Win32\\display.vert";
		cDISPLAY_FRAG_SHADER = "\\SpectREM\\Win32\\display.frag";
		cCLUT_VERT_SHADER = "\\SpectREM\\Win32\\clut.vert";
		cCLUT_FRAG_SHADER = "\\SpectREM\\Win32\\clut.frag";
	}
	else
	{
		cDISPLAY_VERT_SHADER = "display.vert";
		cDISPLAY_FRAG_SHADER = "display.frag";
		cCLUT_VERT_SHADER = "clut.vert";
		cCLUT_FRAG_SHADER = "clut.frag";
	}
}

OpenGLView::OpenGLView(std::string bpath)
{
	// check if under VS/Debugger and set up ROM paths accordingly
	// could probably remove these.. Will test further first
	if (IsDebuggerPresent() != 0)
	{
		cDISPLAY_VERT_SHADER.append(bpath);
		cDISPLAY_VERT_SHADER.append("\\display.vert");
		cDISPLAY_FRAG_SHADER.append(bpath);
		cDISPLAY_FRAG_SHADER.append("\\display.frag");
		cCLUT_VERT_SHADER.append(bpath);
		cCLUT_VERT_SHADER.append("\\clut.vert");
		cCLUT_FRAG_SHADER.append(bpath);
		cCLUT_FRAG_SHADER.append("\\clut.frag");
	}
	else
	{
		cDISPLAY_VERT_SHADER = "display.vert";
		cDISPLAY_FRAG_SHADER = "display.frag";
		cCLUT_VERT_SHADER = "clut.vert";
		cCLUT_FRAG_SHADER = "clut.frag";
	}
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

bool OpenGLView::Init(HWND hWnd, int width, int height, uint16_t idClutVert, uint16_t idClutFrag,  uint16_t idDisplayVert, uint16_t idDisplayFrag, LPWSTR idType)
{
	unsigned int formatCount;
	int pixelformats[1];

    _viewWidth = width;
    _viewHeight = height;


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
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 2,
		0
	};

	m_GLRC = wglCreateContextAttribsARB(m_HDC, 0, contextAL);
	if (wglMakeCurrent(m_HDC, m_GLRC) != TRUE)
	{
		return false;
	}
	
	glClearColor(1.0f, 0.0f, 0.4f, 1.0f);

	LoadShaders(idClutVert, idClutFrag, idDisplayVert, idDisplayFrag, idType);
	SetupTexture();
	SetupQuad();

	return true;
}

//-----------------------------------------------------------------------------------------

void OpenGLView::LoadShaders(uint16_t vertCLUT, uint16_t fragCLUT, uint16_t vertDISPLAY, uint16_t fragDISPLAY, LPWSTR idtype)
{
    // CLUT Shader program
	// Get the shaders from resource
	std::string vertCLUTR;
	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(vertCLUT), MAKEINTRESOURCE(idtype));
 	if (NULL != hRes)
	{
		HGLOBAL hData = LoadResource(0, hRes);
		if (NULL != hData)
		{
			DWORD dataSize = SizeofResource(0, hRes);
			char* data = (char*)LockResource(hData);
			vertCLUTR.assign(data, dataSize);
		}
	}
    else
    {
        int meh = GetLastError();
        wchar_t buf[256];
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
        int n = 0;
    }
	std::string fragCLUTR;
	hRes = FindResource(0, MAKEINTRESOURCE(fragCLUT), MAKEINTRESOURCE(idtype));
	if (NULL != hRes)
	{
		HGLOBAL hData = LoadResource(0, hRes);
		if (NULL != hData)
		{
			DWORD dataSize = SizeofResource(0, hRes);
			char* data = (char*)LockResource(hData);
			fragCLUTR.assign(data, dataSize);
		}
	}
    _clutShaderProg = prepareShaderProgram(vertCLUTR, fragCLUTR);
    GL_CHECK(s_displayTexture = glGetUniformLocation(_clutShaderProg, cS_DISPLAY_TEXTURE));
    GL_CHECK(s_clutTexture = glGetUniformLocation(_clutShaderProg, cS_CLUT_TEXTURE));

    // Display Shader program
	std::string vertDisplayR;
	hRes = FindResource(0, MAKEINTRESOURCE(vertDISPLAY), MAKEINTRESOURCE(idtype));
	if (NULL != hRes)
	{
		HGLOBAL hData = LoadResource(0, hRes);
		if (NULL != hData)
		{
			DWORD dataSize = SizeofResource(0, hRes);
			char* data = (char*)LockResource(hData);
			vertDisplayR.assign(data, dataSize);
		}
	}
	std::string fragDisplayR;
	hRes = FindResource(0, MAKEINTRESOURCE(fragDISPLAY), MAKEINTRESOURCE(idtype));
	if (NULL != hRes)
	{
		HGLOBAL hData = LoadResource(0, hRes);
		if (NULL != hData)
		{
			DWORD dataSize = SizeofResource(0, hRes);
			char* data = (char*)LockResource(hData);
			fragDisplayR.assign(data, dataSize);
		}
	}
    _displayShaderProg = prepareShaderProgram(vertDisplayR, fragDisplayR);
    GL_CHECK(s_texture = glGetUniformLocation(_displayShaderProg, cS_DISPLAY_TEXTURE));
    GL_CHECK(s_reflectionTexture = glGetUniformLocation(_displayShaderProg, cS_REFLECTION_TEXTURE));
    GL_CHECK(u_borderSize = glGetUniformLocation(_displayShaderProg, cU_BORDER_SIZE));
    GL_CHECK(u_contrast = glGetUniformLocation(_displayShaderProg, cU_CONTRAST));
    GL_CHECK(u_saturation = glGetUniformLocation(_displayShaderProg, cU_SATURATION));
    GL_CHECK(u_brightness = glGetUniformLocation(_displayShaderProg, cU_BRIGHTNESS));
    GL_CHECK(u_scanlineSize = glGetUniformLocation(_displayShaderProg, cU_SCAN_LINE_SIZE));
    GL_CHECK(u_scanlines = glGetUniformLocation(_displayShaderProg, cU_SCAN_LINES));
    GL_CHECK(u_screenCurve = glGetUniformLocation(_displayShaderProg, cU_SCREEN_CURVE));
    GL_CHECK(u_pixelFilterValue = glGetUniformLocation(_displayShaderProg, cU_PIXEL_FILTER_VALUE));
    GL_CHECK(u_rgbOffset = glGetUniformLocation(_displayShaderProg, cU_RGB_OFFSET));
    GL_CHECK(u_showVignette = glGetUniformLocation(_displayShaderProg, cU_SHOW_VIGNETTE));
    GL_CHECK(u_vignetteX = glGetUniformLocation(_displayShaderProg, cU_VIGNETTE_X));
    GL_CHECK(u_vignetteY = glGetUniformLocation(_displayShaderProg, cU_VIGNETTE_Y));
    GL_CHECK(u_showReflection = glGetUniformLocation(_displayShaderProg, cU_SHOW_REFLECTION));
    GL_CHECK(u_time = glGetUniformLocation(_displayShaderProg, cU_TIME));
    GL_CHECK(u_screenSize = glGetUniformLocation(_displayShaderProg, cU_SCREEN_SIZE));
}

//-----------------------------------------------------------------------------------------

void OpenGLView::SetupQuad()
{
    // Generate the Vertex array and bind the vertex buffer to it.
    GL_CHECK(glGenVertexArrays(1, &_vertexArray));
    GL_CHECK(glBindVertexArray(_vertexArray));
    GL_CHECK(glGenBuffers(1, &_vertexBuffer));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), static_cast<void *>(nullptr)));
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void *>(12)));
    GL_CHECK(glEnableVertexAttribArray(0));
}

//-----------------------------------------------------------------------------------------

void OpenGLView::SetupTexture()
{
    // Setup the OpenGL texture data storage for the emulator output data. This is stored as a 1 byte per pixel array
    GL_CHECK(glGenTextures(1, &_clutInputTexture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, _clutInputTexture));
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

    // Setup the OpenGL texture data storage for the CLUT data. This provides the colour to be used based on the value stored
    // in the emulator image output data and is used within the fragment shader to pick the right colour to display
    GL_CHECK(glGenTextures(1, &_clutTexture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, _clutTexture));
    GL_CHECK(glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 16, 0, GL_RGBA, GL_FLOAT, CLUT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

    // Setup frame buffer to render into. The texture will be rendered into the frame buffer using LINEAR filtering and this
    // texture will be rendered to the screen using custom filtering inside the fragment shader
    GL_CHECK(glGenFramebuffers(1, &_clutFrameBuffer));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _clutFrameBuffer));
    GL_CHECK(glGenTextures(1, &_clutOutputTexture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, _clutOutputTexture));
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screenWidth, screenHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _clutOutputTexture, 0));
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    GL_CHECK(glDrawBuffers(1, drawBuffers));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        exit(EXIT_FAILURE);
    }
}

//-----------------------------------------------------------------------------------------

void OpenGLView::UpdateTextureData(unsigned char *pData)
{
    glClearColor(0.0f, 1.0f, 1.0f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render the output to a texture which has the default dimentions of the output image
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _clutFrameBuffer));
    GL_CHECK(glViewport(0, 0, screenWidth, screenHeight));
    GL_CHECK(glUseProgram(_clutShaderProg));
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, _clutInputTexture));
    GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screenWidth, screenHeight, GL_RED, GL_UNSIGNED_BYTE, pData));
    GL_CHECK(glUniform1i(s_displayTexture, textureUnit0));

    GL_CHECK(glActiveTexture(GL_TEXTURE1));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, _clutTexture));
    GL_CHECK(glUniform1i(s_clutTexture, textureUnit1));

    GL_CHECK(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
    paintGL();
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
	wcex.lpszClassName = TEXT("OpenGL Extension Checking");
	RegisterClassEx(&wcex);

	// Create a window to use 
	HWND hwnd = CreateWindowEx(WS_EX_APPWINDOW, TEXT("OpenGL Extension Checking"), TEXT("OpenGL Extension Checking"), WS_POPUP, 0, 0, 640, 480, NULL, NULL, GetModuleHandle(NULL), NULL);
	
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
    std::unordered_map<const char*, PROC*> glBindings
    {
        {"wglChoosePixelFormatARB", reinterpret_cast<PROC*>(&wglChoosePixelFormatARB)},
        {"wglCreateContextAttribsARB", reinterpret_cast<PROC*>(&wglCreateContextAttribsARB)},
        {"wglSwapIntervalEXT", reinterpret_cast<PROC*>(&wglSwapIntervalEXT)},
        {"glAttachShader", reinterpret_cast<PROC*>(&glAttachShader)},
        {"glBindBuffer", reinterpret_cast<PROC*>(&glBindBuffer)},
        {"glBindVertexArray", reinterpret_cast<PROC*>(&glBindVertexArray)},
        {"glBufferData", reinterpret_cast<PROC*>(&glBufferData)},
        {"glCompileShader", reinterpret_cast<PROC*>(&glCompileShader)},
        {"glCreateProgram", reinterpret_cast<PROC*>(&glCreateProgram)},
        {"glCreateShader", reinterpret_cast<PROC*>(&glCreateShader)},
        {"glDeleteBuffers", reinterpret_cast<PROC*>(&glDeleteBuffers)},
        {"glDeleteProgram", reinterpret_cast<PROC*>(&glDeleteProgram)},
        {"glDeleteShader", reinterpret_cast<PROC*>(&glDeleteShader)},
        {"glDeleteVertexArrays", reinterpret_cast<PROC*>(&glDeleteVertexArrays)},
        {"glDetachShader", reinterpret_cast<PROC*>(&glDetachShader)},
        {"glEnableVertexAttribArray", reinterpret_cast<PROC*>(&glEnableVertexAttribArray)},
        {"glGenBuffers", reinterpret_cast<PROC*>(&glGenBuffers)},
        {"glGenVertexArrays", reinterpret_cast<PROC*>(&glGenVertexArrays)},
        {"glGetAttribLocation", reinterpret_cast<PROC*>(&glGetAttribLocation)},
        {"glGetProgramInfoLog", reinterpret_cast<PROC*>(&glGetProgramInfoLog)},
        {"glGetProgramiv", reinterpret_cast<PROC*>(&glGetProgramiv)},
        {"glGetShaderInfoLog", reinterpret_cast<PROC*>(&glGetShaderInfoLog)},
        {"glGetShaderiv", reinterpret_cast<PROC*>(&glGetShaderiv)},
        {"glLinkProgram", reinterpret_cast<PROC*>(&glLinkProgram)},
        {"glShaderSource", reinterpret_cast<PROC*>(&glShaderSource)},
        {"glUseProgram", reinterpret_cast<PROC*>(&glUseProgram)},
        {"glVertexAttribPointer", reinterpret_cast<PROC*>(&glVertexAttribPointer)},
        {"glBindAttribLocation", reinterpret_cast<PROC*>(&glBindAttribLocation)},
        {"glGetUniformLocation", reinterpret_cast<PROC*>(&glGetUniformLocation)},
        {"glUniformMatrix4fv", reinterpret_cast<PROC*>(&glUniformMatrix4fv)},
        {"glActiveTexture", reinterpret_cast<PROC*>(&glActiveTexture)},
        {"glUniform1i", reinterpret_cast<PROC*>(&glUniform1i)},
        {"glProgramUniform1f", reinterpret_cast<PROC*>(&glProgramUniform1f)},
        {"glGenerateMipmap", reinterpret_cast<PROC*>(&glGenerateMipmap)},
        {"glDisableVertexAttribArray", reinterpret_cast<PROC*>(&glDisableVertexAttribArray)},
        {"glUniform3fv", reinterpret_cast<PROC*>(&glUniform3fv)},
        {"glUniform4fv", reinterpret_cast<PROC*>(&glUniform4fv)},
        {"glGenFramebuffers", reinterpret_cast<PROC*>(&glGenFramebuffers)},
        {"glBindFramebuffer", reinterpret_cast<PROC*>(&glBindFramebuffer)},
        {"glFramebufferTexture", reinterpret_cast<PROC*>(&glFramebufferTexture)},
        {"glDrawBuffers", reinterpret_cast<PROC*>(&glDrawBuffers)},
        {"glCheckFramebufferStatus", reinterpret_cast<PROC*>(&glCheckFramebufferStatus)},
        {"glProgramUniform1i", reinterpret_cast<PROC*>(&glProgramUniform1i)},
        {"glProgramUniform2f", reinterpret_cast<PROC*>(&glProgramUniform2f)}
    };

    for (auto& glBinding : glBindings)
    {
        *glBinding.second = wglGetProcAddress(glBinding.first);
        if (*glBinding.second == nullptr)
        {
            return false;
        }
    }

    return true;
}

void OpenGLView::paintGL()
{
    // Render the texture to the actual screen, this time using the size of the screen as the viewport
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GL_CHECK(glViewport(0, 0, static_cast<GLint>(_viewWidth), static_cast<GLint>(_viewHeight)));
    GL_CHECK(glUseProgram(_displayShaderProg));
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, _clutOutputTexture));
    GL_CHECK(glUniform1i(s_texture, textureUnit0));

    // Update uniforms in the shader
    GL_CHECK(glProgramUniform1i(_displayShaderProg, u_borderSize, 32));
    GL_CHECK(glProgramUniform1f(_displayShaderProg, u_contrast, 0.75f));
    GL_CHECK(glProgramUniform1f(_displayShaderProg, u_saturation, 1.0f));
    GL_CHECK(glProgramUniform1f(_displayShaderProg, u_brightness, 1.0f));
    GL_CHECK(glProgramUniform1f(_displayShaderProg, u_scanlineSize, 960));
    GL_CHECK(glProgramUniform1f(_displayShaderProg, u_scanlines, 0));
    GL_CHECK(glProgramUniform1f(_displayShaderProg, u_screenCurve, 0.3f));
    GL_CHECK(glProgramUniform1f(_displayShaderProg, u_pixelFilterValue, 0.15f));
    GL_CHECK(glProgramUniform1f(_displayShaderProg, u_rgbOffset, 0));
    GL_CHECK(glProgramUniform1i(_displayShaderProg, u_showVignette, true));
    GL_CHECK(glProgramUniform1f(_displayShaderProg, u_vignetteX, 0.31f));
    GL_CHECK(glProgramUniform1f(_displayShaderProg, u_vignetteY, 6.53f));
    GL_CHECK(glProgramUniform1i(_displayShaderProg, u_showReflection, false));
    //GL_CHECK(glProgramUniform1f(_displayShaderProg, u_time, static_cast<int>(QDateTime::currentMSecsSinceEpoch())));
    GL_CHECK(glProgramUniform2f(_displayShaderProg, u_screenSize, static_cast<GLfloat>(_viewWidth), static_cast<GLfloat>(_viewHeight)));

    GL_CHECK(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
}

GLuint OpenGLView::prepareShaderProgram(std::string vertexShaderPath, std::string fragmentShaderPath)
{
    struct Shader {
        const std::string&  filename;
        GLenum              type;
        int                 padding;
        std::string         source;
    }   shaders[2] = {
        { "vertShader", GL_VERTEX_SHADER, 0, vertexShaderPath },
        { "fragShader", GL_FRAGMENT_SHADER, 0, fragmentShaderPath }
    };

    GLuint program = glCreateProgram();

    for (int i = 0; i < 2; ++i) {
        Shader &s = shaders[i];

        /*std::ifstream file(s.filename, std::ios::in);
        if (file.is_open())
        {
            std::string Line = "";
            while (std::getline(file, Line))
            {
                s.source += "\n" + Line;
            }
            file.close();
        }
        else
        {
            std::cerr << "Can't open " << s.filename << ". Are you in the right directory ? Don't forget to read the FAQ !\n";
            getchar();
            continue;
        }*/

        GLuint shader = glCreateShader(s.type);
        char const * pSource = s.source.c_str();
        glShaderSource(shader, 1, &pSource, nullptr);

        glCompileShader(shader);

        GLint  compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            std::cerr << s.filename << " failed to compile:";
            GLint  logSize;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
            auto logMsg = std::unique_ptr<char>(new char[logSize]);
            glGetShaderInfoLog(shader, logSize, nullptr, logMsg.get());
            std::cerr << logMsg.get();

            exit(EXIT_FAILURE);
        }

        glAttachShader(program, shader);
    }

    // link  and error check
    glLinkProgram(program);

    GLint  linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        std::cerr << "Shader program failed to link";
        GLint  logSize;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
        auto logMsg = std::unique_ptr<char>(new char[logSize]);
        glGetProgramInfoLog(program, logSize, nullptr, logMsg.get());
        std::cerr << logMsg.get();

        exit(EXIT_FAILURE);
    }
    return program;
}

void OpenGLView::CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error " << err << ", at " << fname << ":" << line << "- for " << stmt << "\n";
        exit(EXIT_FAILURE);
    }
}


//-----------------------------------------------------------------------------------------
