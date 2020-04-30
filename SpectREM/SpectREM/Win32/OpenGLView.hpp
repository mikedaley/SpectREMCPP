//
//  OpenGLView.hpp
//  ZXRetroEmulator
//
//  Created by Adrian Brown on 03/09/2016.
//  Copyright © 2016 71Squared Ltd. All rights reserved.
//

#pragma once

//-----------------------------------------------------------------------------------------

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glext.h>

//-----------------------------------------------------------------------------------------

#define WGL_DRAW_TO_WINDOW_ARB				0x2001
#define WGL_ACCELERATION_ARB				0x2003
#define WGL_SWAP_METHOD_ARB					0x2007
#define WGL_SUPPORT_OPENGL_ARB				0x2010
#define WGL_DOUBLE_BUFFER_ARB				0x2011
#define WGL_PIXEL_TYPE_ARB					0x2013
#define WGL_COLOR_BITS_ARB					0x2014
#define WGL_DEPTH_BITS_ARB					0x2022
#define WGL_STENCIL_BITS_ARB				0x2023
#define WGL_FULL_ACCELERATION_ARB			0x2027
#define WGL_SWAP_EXCHANGE_ARB				0x2028
#define WGL_TYPE_RGBA_ARB					0x202B
#define WGL_CONTEXT_MAJOR_VERSION_ARB		0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB		0x2092
#define GL_ARRAY_BUFFER						0x8892
#define GL_STATIC_DRAW						0x88E4
#define GL_FRAGMENT_SHADER					0x8B30
#define GL_VERTEX_SHADER					0x8B31
#define GL_COMPILE_STATUS					0x8B81
#define GL_LINK_STATUS						0x8B82
#define GL_INFO_LOG_LENGTH					0x8B84
#define GL_TEXTURE0							0x84C0
#define GL_TEXTURE1							0x84C1
#define GL_TEXTURE2							0x84C2
#define GL_BGRA								0x80E1
#define GL_ELEMENT_ARRAY_BUFFER				0x8893
#define GL_FRAMEBUFFER_COMPLETE             0x8CD5
#define GL_COLOR_ATTACHMENT0                0x8CE0
#define GL_FRAMEBUFFER                      0x8D40


//-----------------------------------------------------------------------------------------

typedef BOOL(WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC(WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int *attribList);
typedef BOOL(WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
typedef void (APIENTRY * PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRY * PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (APIENTRY * PFNGLBUFFERDATAPROC) (GLenum target, ptrdiff_t size, const GLvoid *data, GLenum usage);
typedef void (APIENTRY * PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint(APIENTRY * PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint(APIENTRY * PFNGLCREATESHADERPROC) (GLenum type);
typedef void (APIENTRY * PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void (APIENTRY * PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (APIENTRY * PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);
typedef void (APIENTRY * PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRY * PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRY * PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef GLint(APIENTRY * PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const char *name);
typedef void (APIENTRY * PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, char *infoLog);
typedef void (APIENTRY * PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, char *infoLog);
typedef void (APIENTRY * PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRY * PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const char *const*string, const GLint *length);
typedef void (APIENTRY * PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const char *name);
typedef GLint(APIENTRY * PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const char *name);
typedef void (APIENTRY * PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (APIENTRY * PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1FPROC) (GLuint program, GLint location, GLfloat v0);
typedef void (APIENTRY * PFNGLGENERATEMIPMAPPROC) (GLenum target);
typedef void (APIENTRY * PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRY * PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLGENFRAMEBUFFERSPROC) (GLsizei n, GLuint *framebuffers);
typedef void (APIENTRY * PFNGLBINDFRAMEBUFFERPROC) (GLenum target, GLuint framebuffer);
typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTUREPROC) (GLenum target, GLenum attachment, GLuint texture, GLint level);
typedef void (APIENTRY * PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum *bufs);
typedef GLenum(APIENTRY * PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum target);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1IPROC) (GLuint program, GLint location, GLint v0);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2FPROC) (GLuint program, GLint location, GLfloat v0, GLfloat v1);


//-----------------------------------------------------------------------------------------

class OpenGLView
{
public:
										OpenGLView(std::string bpath);
										~OpenGLView();

public:
	void								Deinit();
	bool								Init(HWND hWnd, int width, int height, const uint16_t idClutVert, uint16_t idClutFrag, uint16_t idDisplayVert, uint16_t idDisplayFrag, LPWSTR idType);

	void								UpdateTextureData(unsigned char *pData, GLint vX, GLint vY);
	void								OpenGLView::Resize(int width, int height);
	void								OpenGLView::Resize(int x, int y, int width, int height);
	void								OpenGLView::ShaderSetScreenCurve(GLfloat curve);
	void								OpenGLView::ShaderSetVignette(bool onoff);
	void								OpenGLView::ShaderSetReflection(bool onoff);
	void								OpenGLView::LoadFileTextures();
	bool								OpenGLView::LoadBitmap(LPTSTR szFileName, GLuint& texid);


private:
	bool								InitialiseExtensions();
	bool								LoadExtensionList();
	void								LoadShaders(uint16_t vertCLUT, uint16_t fragCLUT, uint16_t vertDISPLAY, uint16_t fragDISPLAY, LPWSTR idtype);
	void								SetupTexture();
	void								SetupQuad();
    void                                paintGL();
	GLuint								OpenGLView::prepareShaderProgram(std::string vertexShaderPath, std::string fragmentShaderPath);
    void                                CheckOpenGLError(const char* stmt, const char* fname, int line);
	std::string							GetLastErrorAsString();
	


public:
	std::string							appBasePath;

	PFNGLATTACHSHADERPROC				glAttachShader;
	PFNGLBINDBUFFERPROC					glBindBuffer;
	PFNGLBINDVERTEXARRAYPROC			glBindVertexArray;
	PFNGLBUFFERDATAPROC					glBufferData;
	PFNGLCOMPILESHADERPROC				glCompileShader;
	PFNGLCREATEPROGRAMPROC				glCreateProgram;
	PFNGLCREATESHADERPROC				glCreateShader;
	PFNGLDELETEBUFFERSPROC				glDeleteBuffers;
	PFNGLDELETEPROGRAMPROC				glDeleteProgram;
	PFNGLDELETESHADERPROC				glDeleteShader;
	PFNGLDELETEVERTEXARRAYSPROC			glDeleteVertexArrays;
	PFNGLDETACHSHADERPROC				glDetachShader;
	PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray;
	PFNGLGENBUFFERSPROC					glGenBuffers;
	PFNGLGENVERTEXARRAYSPROC			glGenVertexArrays;
	PFNGLGETATTRIBLOCATIONPROC			glGetAttribLocation;
	PFNGLGETPROGRAMINFOLOGPROC			glGetProgramInfoLog;
	PFNGLGETPROGRAMIVPROC				glGetProgramiv;
	PFNGLGETSHADERINFOLOGPROC			glGetShaderInfoLog;
	PFNGLGETSHADERIVPROC				glGetShaderiv;
	PFNGLLINKPROGRAMPROC				glLinkProgram;
	PFNGLSHADERSOURCEPROC				glShaderSource;
	PFNGLUSEPROGRAMPROC					glUseProgram;
	PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer;
	PFNGLBINDATTRIBLOCATIONPROC			glBindAttribLocation;
	PFNGLGETUNIFORMLOCATIONPROC			glGetUniformLocation;
	PFNGLUNIFORMMATRIX4FVPROC			glUniformMatrix4fv;
	PFNGLACTIVETEXTUREPROC				glActiveTexture;
	PFNGLUNIFORM1IPROC					glUniform1i;
	PFNGLGENERATEMIPMAPPROC				glGenerateMipmap;
	PFNGLDISABLEVERTEXATTRIBARRAYPROC	glDisableVertexAttribArray;
	PFNGLUNIFORM3FVPROC					glUniform3fv;
	PFNGLUNIFORM4FVPROC					glUniform4fv; 
	PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormatARB;
	PFNWGLCREATECONTEXTATTRIBSARBPROC	wglCreateContextAttribsARB;
	PFNWGLSWAPINTERVALEXTPROC			wglSwapIntervalEXT;
	PFNGLPROGRAMUNIFORM1FPROC			glProgramUniform1f;
    PFNGLGENFRAMEBUFFERSPROC            glGenFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC            glBindFramebuffer;
    PFNGLFRAMEBUFFERTEXTUREPROC         glFramebufferTexture;
    PFNGLDRAWBUFFERSPROC                glDrawBuffers;
    PFNGLCHECKFRAMEBUFFERSTATUSPROC     glCheckFramebufferStatus;
    PFNGLPROGRAMUNIFORM1IPROC           glProgramUniform1i;
    PFNGLPROGRAMUNIFORM2FPROC           glProgramUniform2f;

private:
	HWND								m_hWnd;
	HDC									m_HDC;
	HGLRC								m_GLRC;
	PIXELFORMATDESCRIPTOR				m_PFD;
/*
	GLuint								vertexBuffer;
	GLuint								vertexArray;
	GLuint								shaderProgName;
	GLuint								textureName;
	GLuint								clutTextureName;

	GLuint								textureID;
	GLuint								s_clutTexture;
	GLuint								u_borderSize;
	GLuint								u_contrast;
	GLuint								u_saturation;
	GLuint								u_brightness;
	GLuint								u_scanlineSize;
	GLuint								u_scanlines;
	GLuint								u_screenCurve;
*/

    GLuint          _viewWidth = 320;
	GLuint          _viewHeight = 256;
	GLuint			_viewTop = 0;
	GLuint			_viewLeft = 0;

    GLuint          _vertexBuffer;
    GLuint          _vertexArray;

    GLuint          _clutShaderProg;
    GLuint          _displayShaderProg;

    GLuint          _clutFrameBuffer;
    GLuint          _clutInputTexture;
    GLuint          _clutTexture;
    GLuint          _clutOutputTexture;
	GLuint          _reflectionTexture;


    // Display shader uniforms/samplers
    GLuint          displayDepthBuffer;

    GLint           s_displayTexture;
    GLint           s_texture;
    GLint           s_reflectionTexture;
    GLint           s_clutTexture;
    GLint           u_borderSize;
    GLint           u_contrast;
    GLint           u_saturation;
    GLint           u_brightness;
    GLint           u_scanlineSize;
    GLint           u_scanlines;
    GLint           u_screenCurve;
    GLint           u_pixelFilterValue;
    GLint           u_rgbOffset;
    GLint           u_showVignette;
    GLint           u_vignetteX;
    GLint           u_vignetteY;
    GLint           u_showReflection;
    GLint           u_time;
    GLint           u_screenSize;

	bool			u_showVignetteValue;
	GLfloat			u_screenCurveValue;
	bool			u_showReflectionValue;




};

//-----------------------------------------------------------------------------------------
