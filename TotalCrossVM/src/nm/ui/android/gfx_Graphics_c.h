/*********************************************************************************
 *  TotalCross Software Development Kit                                          *
 *  Copyright (C) 2000-2012 SuperWaba Ltda.                                      *
 *  All Rights Reserved                                                          *
 *                                                                               *
 *  This library and virtual machine is distributed in the hope that it will     *
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                         *
 *                                                                               *
 *********************************************************************************/

#include "gfx_ex.h"

#ifdef ANDROID
#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer
#endif
#ifdef darwin
bool isIpad;
#else
bool isIpad = false;
#endif

bool checkGlError(const char* op, int line)
{
   GLint error;
   
   //debug("%s (%d)",op,line);
   
   if (!op)
      return glGetError() != 0;
   else
   for (error = glGetError(); error; error = glGetError())
   {
      char* msg = "???";
      switch (error)
      {
         case GL_INVALID_ENUM     : msg = "INVALID ENUM"; break;
         case GL_INVALID_VALUE    : msg = "INVALID VALUE"; break;
         case GL_INVALID_OPERATION: msg = "INVALID OPERATION"; break;
         case GL_OUT_OF_MEMORY    : msg = "OUT OF MEMORY"; break;
      }
      debug("glError %s at %s (%d)\n", msg, op, line);
      return true;
   }
   return false;
}

#define GL_CHECK_ERROR checkGlError(__FILE__,__LINE__);

#ifdef ANDROID
static void setProjectionMatrix(GLfloat w, GLfloat h);
static ANativeWindow *window,*lastWindow;
static EGLDisplay _display;
static EGLSurface _surface;
static EGLContext _context;
#endif
static void destroyEGL();
static bool surfaceWillChange;

VoidPs* imgTextures;
int32 realAppH,appW,appH,glShiftY;
GLfloat ftransp[16], f255[256];
int32 flen;
GLfloat *glcoords;//[flen*2]; x,y
GLfloat *glcolors;//[flen];   alpha
static GLfloat texcoords[16], lrcoords[8], shcolors[24],shcoords[8];
void glClearClip();
void glSetClip(int32 x1, int32 y1, int32 x2, int32 y2);

// http://www.songho.ca/opengl/gl_projectionmatrix.html
//////////// texture
#define TEXTURE_VERTEX_CODE  \
      "attribute vec4 vertexPoint;" \
      "attribute vec2 aTextureCoord;" \
      "uniform float alpha;" \
      "uniform mat4 projectionMatrix; " \
      "varying vec2 vTextureCoord;" \
      "varying float vAlpha;" \
      "void main()" \
      "{" \
      "    gl_Position = vertexPoint * projectionMatrix;" \
      "    vTextureCoord = aTextureCoord;" \
      "    vAlpha = alpha;" \
      "}"

#define TEXTURE_FRAGMENT_CODE \
      "precision mediump float;" \
      "varying vec2 vTextureCoord;" \
      "uniform sampler2D sTexture;" \
      "varying float vAlpha;" \
      "void main()" \
      "{" \
      "   gl_FragColor = texture2D(sTexture, vTextureCoord);" \
      "   gl_FragColor.a = gl_FragColor.a * vAlpha;" \
      "}"

static GLuint textureProgram;
static GLuint texturePoint;
static GLuint textureCoord,textureS,textureAlpha;

///////

#define TEXT_VERTEX_CODE  \
      "attribute vec4 vertexPoint;" \
      "attribute vec2 aTextureCoord;" \
      "uniform vec3 rgb;" \
      "uniform mat4 projectionMatrix; " \
      "varying vec2 vTextureCoord;" \
      "varying vec3 v_rgb;" \
      "void main()" \
      "{" \
      "    gl_Position = vertexPoint * projectionMatrix;" \
      "    vTextureCoord = aTextureCoord;" \
      "    v_rgb = rgb;" \
      "}"

#define TEXT_FRAGMENT_CODE \
      "precision mediump float;" \
      "varying vec2 vTextureCoord;" \
      "uniform sampler2D sTexture;" \
      "varying vec3 v_rgb;" \
      "void main()" \
      "{" \
      "   gl_FragColor = texture2D(sTexture, vTextureCoord);" \
      "   gl_FragColor.rgb = v_rgb;" \
      "}"

static GLuint textProgram;
static GLuint textPoint;
static GLuint textCoord,textS,textRGB;

//////////// points (text)

#define POINTS_VERTEX_CODE \
      "attribute vec4 a_Position; uniform vec4 a_Color; varying vec4 v_Color; attribute float alpha;" \
      "uniform mat4 projectionMatrix; " \
      "void main() {gl_PointSize = 1.0; v_Color = vec4(a_Color.x,a_Color.y,a_Color.z,alpha); gl_Position = a_Position * projectionMatrix;}"

#define POINTS_FRAGMENT_CODE \
      "precision mediump float;" \
      "varying vec4 v_Color;" \
      "void main() {gl_FragColor = v_Color;}"

static GLuint pointsProgram;
static GLuint pointsPosition;
static GLuint pointsColor;
static GLuint pointsAlpha;

///////////// line, rect, point

#define LRP_VERTEX_CODE \
      "attribute vec4 a_Position;" \
      "uniform mat4 projectionMatrix;" \
      "void main() {gl_PointSize = 1.0; gl_Position = a_Position*projectionMatrix;}"

#define LRP_FRAGMENT_CODE \
      "precision mediump float;" \
      "uniform vec4 a_Color;" \
      "void main() {gl_FragColor = a_Color;}"

static GLuint lrpProgram;
static GLuint lrpPosition;
static GLuint lrpColor;
static GLubyte rectOrder[] = { 0, 1, 2, 0, 2, 3 }; // order to draw vertices

///////////// line, rect, point

#define DOT_VERTEX_CODE \
      "attribute vec4 a_Position;" \
      "uniform mat4 projectionMatrix;" \
      "varying vec2 v_xy;" \
      "void main() {gl_PointSize = 1.0; gl_Position = a_Position*projectionMatrix; v_xy = a_Position.xy;}"

#define DOT_FRAGMENT_CODE \
      "precision mediump float;" \
      "varying vec2 v_xy;" \
      "uniform float isVert;" \
      "uniform vec4 color1;" \
      "uniform vec4 color2;" \
      "void main() {gl_FragColor = mod(isVert > 0.0 ? v_xy.y : v_xy.x, 2.0) >= 1.0 ? color1 : color2;}"

static GLuint dotProgram;
static GLuint dotPosition,dotIsVert;
static GLuint dotColor1,dotColor2;

///////////// shaded rect

#define SHADE_VERTEX_CODE \
      "attribute vec4 a_Position; attribute vec4 a_Color; varying vec4 v_Color;" \
      "uniform mat4 projectionMatrix;" \
      "void main() {gl_PointSize = 1.0; v_Color = a_Color; gl_Position = a_Position*projectionMatrix;}"

#define SHADE_FRAGMENT_CODE \
      "precision mediump float;" \
      "varying vec4 v_Color;" \
      "void main() {gl_FragColor = v_Color;}"

static GLuint shadeProgram;
static GLuint shadePosition;
static GLuint shadeColor;

GLuint loadShader(GLenum shaderType, const char* pSource)
{
   GLint ret=1;               
   GLuint shader = glCreateShader(shaderType); GL_CHECK_ERROR
   glShaderSource(shader, 1, &pSource, NULL); GL_CHECK_ERROR
   glCompileShader(shader); GL_CHECK_ERROR

   glGetShaderiv(shader, GL_COMPILE_STATUS, &ret); GL_CHECK_ERROR
   if(!ret)
   {
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &ret); GL_CHECK_ERROR
      GLchar buffer[ret];
      glGetShaderInfoLog(shader, ret, &ret, buffer); GL_CHECK_ERROR
      debug("Shader compiler error: %s",buffer);
   }
   return shader;
}

static GLint lastProg=-1;
static Pixel lrpLastRGB = -2, lastTextRGB, lastTextureId, lastTextId;
static float lastAlphaMask = -1;
static void setCurrentProgram(GLint prog)
{
   if (prog != lastProg)
   {
      glUseProgram(lastProg = prog); GL_CHECK_ERROR
      if (lastProg != textureProgram && lastProg != textProgram) lrpLastRGB = -2;
   }
}
static void resetGlobals()
{
   lastAlphaMask = lastTextRGB = lastTextureId = lastTextId = -1;
}

static GLuint createProgram(char* vertexCode, char* fragmentCode)
{
   GLint ret;
   GLuint p = glCreateProgram();
   glAttachShader(p, loadShader(GL_VERTEX_SHADER, vertexCode)); GL_CHECK_ERROR
   glAttachShader(p, loadShader(GL_FRAGMENT_SHADER, fragmentCode)); GL_CHECK_ERROR
   glLinkProgram(p); GL_CHECK_ERROR
   //glValidateProgram(p);
   glGetProgramiv(p, GL_LINK_STATUS, &ret); GL_CHECK_ERROR
   if (!ret)
   {
      glGetProgramiv(p, GL_INFO_LOG_LENGTH, &ret); GL_CHECK_ERROR
      GLchar buffer[ret];
      glGetProgramInfoLog(p, ret, &ret, buffer); GL_CHECK_ERROR
      debug("Link error: %s",buffer);
   }
   return p;
}

bool initGLES(ScreenSurface screen); // in iOS, implemented in mainview.m
void recreateTextures(bool delTex); // imagePrimitives_c.h

void setTimerInterval(int32 t);  
int32 desiredglShiftY;
bool setShiftYonNextUpdateScreen;
#ifdef ANDROID           
void JNICALL Java_totalcross_Launcher4A_nativeInitSize(JNIEnv *env, jobject this, jobject surface, jint width, jint height) // called only once
{                    
   if (!screen.extension)           
      screen.extension = newX(ScreenSurfaceEx);

   if (surface == null) // passed null when the surface is destroyed
   {       
      if (width == -999)
      {                
         if (needsPaint != null)
         {
            desiredglShiftY = height == 0 ? 0 : appH - height; // change only after the next screen update, since here we are running in a different thread
            setShiftYonNextUpdateScreen = true;
            *needsPaint = true; // schedule a screen paint to update the shiftY values
            setTimerInterval(1);      
         }
      }
      else
      if (width == -998)
         recreateTextures(true); // first we delete the textures before the gl context is invalid
      else
      if (width == -997) // when the screen is turned off and on again, this ensures that the textures will be recreated
         recreateTextures(false); // now we set the changed flag for all textures
      else
         surfaceWillChange = true; // block all screen updates
      return;
   }  
   desiredglShiftY = glShiftY = 0;         
   setShiftYonNextUpdateScreen = true;
   appW = width;
   appH = height;
   surfaceWillChange = false;
   if (window) // fixed memory leak
      ANativeWindow_release(window);
   window = ANativeWindow_fromSurface(env, surface);
   realAppH = (*env)->CallStaticIntMethod(env, applicationClass, jgetHeight);
   if (lastWindow && lastWindow != window)
   {  
      destroyEGL();
      initGLES(&screen);
      recreateTextures(false); // now we set the changed flag for all textures
   }
   lastWindow = window;
}
#endif

static void initPoints()
{
   pointsProgram = createProgram(POINTS_VERTEX_CODE, POINTS_FRAGMENT_CODE);
   setCurrentProgram(pointsProgram);
   pointsColor = glGetUniformLocation(pointsProgram, "a_Color"); GL_CHECK_ERROR
   pointsAlpha = glGetAttribLocation(pointsProgram, "alpha"); GL_CHECK_ERROR
   pointsPosition = glGetAttribLocation(pointsProgram, "a_Position"); GL_CHECK_ERROR // get handle to vertex shader's vPosition member
   glEnableVertexAttribArray(pointsAlpha); GL_CHECK_ERROR // Enable a handle to the colors - since this is the only one used, keep it enabled all the time
   glEnableVertexAttribArray(pointsPosition); GL_CHECK_ERROR // Enable a handle to the vertices - since this is the only one used, keep it enabled all the time
}

static int pixLastRGB = -1;
void glDrawPixels(int32 n, int32 rgb)
{
   setCurrentProgram(pointsProgram);
   if (pixLastRGB != rgb)
   {
      PixelConv pc;
      pc.pixel = pixLastRGB = rgb;
      glUniform4f(pointsColor, f255[pc.r], f255[pc.g], f255[pc.b], 0); GL_CHECK_ERROR
   }
   glVertexAttribPointer(pointsAlpha, 1, GL_FLOAT, GL_FALSE, 0, glcolors); GL_CHECK_ERROR
   glVertexAttribPointer(pointsPosition, 2, GL_FLOAT, GL_FALSE, 0, glcoords); GL_CHECK_ERROR
   glDrawArrays(GL_POINTS, 0,n); GL_CHECK_ERROR
}

void glDrawLines(Context currentContext, TCObject g, int32* x, int32* y, int32 n, int32 tx, int32 ty, Pixel rgb, bool fill)
{
   PixelConv pc;
   ty += glShiftY;
   setCurrentProgram(lrpProgram);
   pc.pixel = rgb;
   pc.a = 255;
   if (lrpLastRGB != pc.pixel) // prevent color change = performance x2 in galaxy tab2
   {          
      lrpLastRGB = pc.pixel;
      glUniform4f(lrpColor, f255[pc.r],f255[pc.g],f255[pc.b],f255[pc.a]); GL_CHECK_ERROR
   }                                
   if (checkGLfloatBuffer(currentContext, n))
   {
      int32 i,nn=n;
      float *glV = glcoords;            
      int32 cx1 = Graphics_clipX1(g), cy1 = Graphics_clipY1(g), cx2 = Graphics_clipX2(g), cy2 = Graphics_clipY2(g), xx,yy;
      bool doClip = false;    
      for (i = 0; i < n; i++)
      {
         *glV++ = xx = (float)(*x++ + tx);
         *glV++ = yy = (float)(*y++ + ty);
         if (!doClip && (yy < cy1 || yy > cy2 || xx < cx1 || xx > cx2))
            doClip = true;
      }           
      if (doClip) glSetClip(cx1,cy1,cx2,cy2);
      glVertexAttribPointer(lrpPosition, 2, GL_FLOAT, GL_FALSE, 0, glcoords); GL_CHECK_ERROR
      glDrawArrays(fill ? GL_TRIANGLE_FAN : GL_LINES, 0,nn); GL_CHECK_ERROR
      if (doClip) glClearClip();
   }
}

static void initShade()
{         
   shadeProgram = createProgram(SHADE_VERTEX_CODE, SHADE_FRAGMENT_CODE);
   setCurrentProgram(shadeProgram);
   shadeColor = glGetAttribLocation(shadeProgram, "a_Color"); GL_CHECK_ERROR
   shadePosition = glGetAttribLocation(shadeProgram, "a_Position"); GL_CHECK_ERROR // get handle to vertex shader's vPosition member
   glEnableVertexAttribArray(shadeColor); GL_CHECK_ERROR // Enable a handle to the colors - since this is the only one used, keep it enabled all the time
   glEnableVertexAttribArray(shadePosition); GL_CHECK_ERROR // Enable a handle to the vertices - since this is the only one used, keep it enabled all the time
   shcolors[3] = shcolors[7] = shcolors[11] = shcolors[15] = shcolors[19] = shcolors[23] = 1; // note: last 2 colors are not used by opengl
}

void glFillShadedRect(TCObject g, int32 x, int32 y, int32 w, int32 h, PixelConv c1, PixelConv c2, bool horiz)
{
   int32 cx1 = Graphics_clipX1(g), cy1 = Graphics_clipY1(g), cx2 = Graphics_clipX2(g), cy2 = Graphics_clipY2(g), xx,yy;
   bool doClip = y < cy1 || y+h > cy2 || x < cx1 || x+w > cx2;
   
   if (doClip) glSetClip(Graphics_clipX1(g), Graphics_clipY1(g), Graphics_clipX2(g), Graphics_clipY2(g));
   setCurrentProgram(shadeProgram);
   glVertexAttribPointer(shadeColor, 4, GL_FLOAT, GL_FALSE, 0, shcolors); GL_CHECK_ERROR
   glVertexAttribPointer(shadePosition, 2, GL_FLOAT, GL_FALSE, 0, shcoords); GL_CHECK_ERROR
   
   y += glShiftY;
   
   shcoords[0] = shcoords[2] = x;
   shcoords[1] = shcoords[7] = y;
   shcoords[3] = shcoords[5] = y+h;
   shcoords[4] = shcoords[6] = x+w;

   if (!horiz)
   {
      shcolors[0] = shcolors[12] = f255[c2.r]; // upper left + upper right
      shcolors[1] = shcolors[13] = f255[c2.g];
      shcolors[2] = shcolors[14] = f255[c2.b];
      
      shcolors[4] = shcolors[8]  = f255[c1.r]; // lower left + lower right
      shcolors[5] = shcolors[9]  = f255[c1.g];
      shcolors[6] = shcolors[10] = f255[c1.b];
   }
   else
   {
      shcolors[0] = shcolors[4] = f255[c2.r];  // upper left + lower left
      shcolors[1] = shcolors[5] = f255[c2.g];
      shcolors[2] = shcolors[6] = f255[c2.b];
      
      shcolors[8]  = shcolors[12] = f255[c1.r]; // lower right + upper right
      shcolors[9]  = shcolors[13] = f255[c1.g];
      shcolors[10] = shcolors[14] = f255[c1.b];
   }
    
   glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, rectOrder); GL_CHECK_ERROR
   if (doClip) glClearClip();
}

void initTexture()
{         
   // images
   textureProgram = createProgram(TEXTURE_VERTEX_CODE, TEXTURE_FRAGMENT_CODE);
   setCurrentProgram(textureProgram);
   textureS     = glGetUniformLocation(textureProgram, "sTexture"); GL_CHECK_ERROR
   texturePoint = glGetAttribLocation(textureProgram, "vertexPoint"); GL_CHECK_ERROR
   textureCoord = glGetAttribLocation(textureProgram, "aTextureCoord"); GL_CHECK_ERROR
   textureAlpha = glGetUniformLocation(textureProgram, "alpha"); GL_CHECK_ERROR

   glEnableVertexAttribArray(textureCoord); GL_CHECK_ERROR
   glEnableVertexAttribArray(texturePoint); GL_CHECK_ERROR

   // text char
   textProgram = createProgram(TEXT_VERTEX_CODE, TEXT_FRAGMENT_CODE);
   setCurrentProgram(textProgram);
   textS     = glGetUniformLocation(textProgram, "sTexture"); GL_CHECK_ERROR
   textPoint = glGetAttribLocation(textProgram, "vertexPoint"); GL_CHECK_ERROR
   textCoord = glGetAttribLocation(textProgram, "aTextureCoord"); GL_CHECK_ERROR
   textRGB   = glGetUniformLocation(textProgram, "rgb"); GL_CHECK_ERROR

   glEnableVertexAttribArray(textCoord); GL_CHECK_ERROR
   glEnableVertexAttribArray(textPoint); GL_CHECK_ERROR
}

void glLoadTexture(Context currentContext, TCObject img, int32* textureId, Pixel *pixels, int32 width, int32 height, bool updateList, bool onlyAlpha)
{
   int32 i;
   PixelConv* pf = (PixelConv*)pixels, *pt, *pt0;
   bool textureAlreadyCreated = *textureId != 0;
   bool err;
   if (onlyAlpha)
      pt = pt0 = (PixelConv*)pixels;
   else
   {
      pt0 = pt = (PixelConv*)xmalloc(width*height*4);
      if (!pt)
      {
         throwException(currentContext, OutOfMemoryError, "Out of bitmap memory for image with %dx%d",width,height);
         return;
      }
   }
   if (!textureAlreadyCreated) 
   {
      glGenTextures(1, (GLuint*)textureId); err = GL_CHECK_ERROR              
      if (err) 
      {
         throwException(currentContext, OutOfMemoryError, "Cannot bind texture for image with %dx%d",width,height);
         return;
      }
   }
   // OpenGL ES provides support for non-power-of-two textures, provided that the s and t wrap modes are both GL_CLAMP_TO_EDGE.
   glBindTexture(GL_TEXTURE_2D, *textureId); GL_CHECK_ERROR
   if (!textureAlreadyCreated)
   {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); GL_CHECK_ERROR
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); GL_CHECK_ERROR
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); GL_CHECK_ERROR
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); GL_CHECK_ERROR
   }
   // must invert the pixels from ARGB to RGBA
   if (!onlyAlpha)
      for (i = width*height; --i >= 0;pt++,pf++) {pt->a = pf->r; pt->b = pf->g; pt->g = pf->b; pt->r = pf->a;}
   if (textureAlreadyCreated)
   {
      glTexSubImage2D(GL_TEXTURE_2D, 0,0,0,width,height, onlyAlpha ? GL_ALPHA : GL_RGBA,GL_UNSIGNED_BYTE, pt0); GL_CHECK_ERROR
      glBindTexture(GL_TEXTURE_2D, 0); GL_CHECK_ERROR
   }
   else
   {
      glTexImage2D(GL_TEXTURE_2D, 0, onlyAlpha ? GL_ALPHA : GL_RGBA, width, height, 0, onlyAlpha ? GL_ALPHA : GL_RGBA,GL_UNSIGNED_BYTE, pt0); err = GL_CHECK_ERROR
      if (err)
         throwException(currentContext, OutOfMemoryError, "Out of texture memory for image with %dx%d",width,height);
      else
      {   
         if (updateList && !VoidPsContains(imgTextures, img)) // dont add duplicate
            imgTextures = VoidPsAdd(imgTextures, img, null);
         glBindTexture(GL_TEXTURE_2D, 0); GL_CHECK_ERROR
      }
   }
   if (!onlyAlpha) xfree(pt0);
}

void glDeleteTexture(TCObject img, int32* textureId, bool updateList)
{
   if (textureId != null && textureId[0] != 0)        
   {
      glDeleteTextures(1,(GLuint*)textureId); GL_CHECK_ERROR
      *textureId = 0;                               
   }
   if (updateList && VoidPsContains(imgTextures, img))
      imgTextures = VoidPsRemove(imgTextures, img, null);
}
void glClearClip()
{            
   glDisable(GL_SCISSOR_TEST); GL_CHECK_ERROR
}
// note2: 777e4e85d26ddff1bb1d211c161bebc626d69636 - removed glClearClip and glSetClip. Some Motorola devices were clipping out the whole screen when the keyboard was visible and the screen was shifted. prior: 4d329c97ef58a42f365a2d48b70f0d9126869355
void glSetClip(int32 x1, int32 y1, int32 x2, int32 y2) 
{
   y1 += glShiftY;
   y2 += glShiftY;
   glEnable(GL_SCISSOR_TEST); GL_CHECK_ERROR
   if (x1 < 0) x1 = 0; else if (x1 > appW) x1 = appW;
   if (x2 < 0) x2 = 0; else if (x2 > appW) x2 = appW;
   if (y1 < 0) y1 = 0; else if (y1 > appH) y1 = appH;
   if (y2 < 0) y2 = 0; else if (y2 > appH) y2 = appH;
   int32 h = y2-y1, w = x2-x1;
   if (h < 0) h = 0;
   if (w < 0) w = 0;
   glScissor(x1,appH - y2,w,h); GL_CHECK_ERROR
}
                                                           
void glDrawTexture(int32* textureId, int32 x, int32 y, int32 w, int32 h, int32 dstX, int32 dstY, int32 imgW, int32 imgH, PixelConv* color, int32* clip, int32 alphaMask)
{                    
   if (textureId[0] == 0) return;
      
   GLfloat* coords = texcoords;
   
   setCurrentProgram(color ? textProgram : textureProgram);
   if (lastTextId != *textureId)
   {
      lastTextId = *textureId;
      glBindTexture(GL_TEXTURE_2D, *textureId); GL_CHECK_ERROR
   }

   dstY += glShiftY;

   // destination coordinates
   coords[0] = coords[6] = dstX;
   coords[1] = coords[3] = (dstY+h);
   coords[2] = coords[4] = (dstX+w);
   coords[5] = coords[7] = dstY;

   glVertexAttribPointer(color ? textPoint : texturePoint, 2, GL_FLOAT, false, 0, coords); GL_CHECK_ERROR

   // source coordinates                  
   GLfloat left = (float)x/(float)imgW,top=(float)y/(float)imgH,right=(float)(x+w)/(float)imgW,bottom=(float)(y+h)/(float)imgH; // 0,0,1,1
   coords[ 8] = coords[14] = left;
   coords[ 9] = coords[11] = bottom;
   coords[10] = coords[12] = right;
   coords[13] = coords[15] = top;
   glVertexAttribPointer(color ? textCoord : textureCoord, 2, GL_FLOAT, false, 0, &coords[8]); GL_CHECK_ERROR

   bool doClip = false;
   if (clip != null) 
   {          
      int32 cx1 = clip[0], cy1 = clip[1], cx2 = clip[2], cy2 = clip[3];
      doClip = dstY < cy1 || dstY+h > cy2 || dstX < cx1 || dstX+w > cx2;
   }

   if (doClip) glSetClip(clip[0],clip[1],clip[2],clip[3]);

   if (!color && lastAlphaMask != alphaMask) // prevent color change = performance x2 in galaxy tab2
   {
      lastAlphaMask = alphaMask;
      glUniform1f(textureAlpha, f255[alphaMask]);
   }
   if (color && lastTextRGB != color->pixel) // prevent color change = performance x2 in galaxy tab2
   {
      lastTextRGB = color->pixel;
      glUniform3f(textRGB, f255[color->r],f255[color->g],f255[color->b]); GL_CHECK_ERROR
   }
   glDrawArrays(GL_TRIANGLE_FAN, 0, 4); GL_CHECK_ERROR
   //glBindTexture(GL_TEXTURE_2D, 0); GL_CHECK_ERROR - 3% gain
   if (doClip) glClearClip();
}

void initLineRectPoint()
{         
   lrpProgram = createProgram(LRP_VERTEX_CODE, LRP_FRAGMENT_CODE);
   setCurrentProgram(lrpProgram);
   lrpColor = glGetUniformLocation(lrpProgram, "a_Color"); GL_CHECK_ERROR
   lrpPosition = glGetAttribLocation(lrpProgram, "a_Position"); GL_CHECK_ERROR
   glEnableVertexAttribArray(lrpPosition); GL_CHECK_ERROR

   dotProgram = createProgram(DOT_VERTEX_CODE, DOT_FRAGMENT_CODE);
   setCurrentProgram(dotProgram);
   dotColor1 = glGetUniformLocation(dotProgram, "color1"); GL_CHECK_ERROR
   dotColor2 = glGetUniformLocation(dotProgram, "color2"); GL_CHECK_ERROR
   dotPosition = glGetAttribLocation(dotProgram, "a_Position"); GL_CHECK_ERROR
   dotIsVert = glGetUniformLocation(dotProgram, "isVert"); GL_CHECK_ERROR
   glEnableVertexAttribArray(dotPosition); GL_CHECK_ERROR
}

void glSetLineWidth(int32 w)
{         
   setCurrentProgram(lrpProgram);
   glLineWidth(w); GL_CHECK_ERROR
}

typedef enum
{
   SIMPLE,
   DOTS,
   DIAGONAL
}  LRPType;

void drawLRP(int32 x, int32 y, int32 w, int32 h, int32 rgb, int32 rgb2, int32 a, LRPType type)
{           
   float* coords = lrcoords;
   setCurrentProgram(type == DOTS ? dotProgram : lrpProgram);
   glVertexAttribPointer(type == DOTS ? dotPosition : lrpPosition, 2, GL_FLOAT, GL_FALSE, 0, coords); GL_CHECK_ERROR
   int32 ty = glShiftY;
   PixelConv pc;
   pc.pixel = rgb;
   pc.a = a;
   if (type == DOTS)
   {                 
      glUniform1f(dotIsVert, x == w ? 1.0 : 0.0); GL_CHECK_ERROR
      glUniform4f(dotColor1, f255[pc.r],f255[pc.g],f255[pc.b],f255[pc.a]); GL_CHECK_ERROR
      pc.pixel = rgb2;
      pc.a = a;
      glUniform4f(dotColor2, f255[pc.r],f255[pc.g],f255[pc.b],f255[pc.a]); GL_CHECK_ERROR
   }
   else
   if (lrpLastRGB != pc.pixel) // prevent color change = performance x2 in galaxy tab2
   {
      lrpLastRGB = pc.pixel;
      glUniform4f(lrpColor, f255[pc.r],f255[pc.g],f255[pc.b],f255[pc.a]); GL_CHECK_ERROR
   }
   y += ty;
   if (type == DIAGONAL || type == DOTS)
   {                    
      coords[0] = x;
      coords[1] = y;
      coords[2] = w;    // x2
      coords[3] = h+ty; // y2
      glDrawArrays(GL_LINES, 0,2); GL_CHECK_ERROR
   }
   else
   {
      coords[0] = coords[2] = x;
      coords[1] = coords[7] = y;
      coords[3] = coords[5] = y+h;
      coords[4] = coords[6] = x+w;
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, rectOrder); GL_CHECK_ERROR
   }
}

void glDrawPixel(int32 x, int32 y, int32 rgb, int32 a)
{   
   drawLRP(x,y,1,1,rgb,-1,a, SIMPLE);
}

void glDrawThickLine(int32 x1, int32 y1, int32 x2, int32 y2, int32 rgb, int32 a)
{
   drawLRP(x1,y1,x2,y2,rgb,-1,a, DIAGONAL);
}

void glDrawDots(int32 x1, int32 y1, int32 x2, int32 y2, int32 rgb1, int32 rgb2)
{
   drawLRP(x1,y1,x2,y2, rgb1, rgb2, 255, DOTS);
}
 
void glDrawLine(int32 x1, int32 y1, int32 x2, int32 y2, int32 rgb, int32 a)
{
   // The Samsung Galaxy Tab 2 (4.0.4) has a bug in opengl for drawing horizontal/vertical lines: it draws at wrong coordinates, and incomplete sometimes. so we use fillrect, which always work
   if (x1 == x2)
      drawLRP(min32(x1,x2),min32(y1,y2),1,abs32(y2-y1), rgb,-1,a, SIMPLE);
   else
   if (y1 == y2) 
      drawLRP(min32(x1,x2),min32(y1,y2),abs32(x2-x1),1, rgb,-1,a, SIMPLE);
   else              
      drawLRP(x1,y1,x2,y2,rgb,-1,a, DIAGONAL);
}

void glFillRect(int32 x, int32 y, int32 w, int32 h, int32 rgb, int32 a)
{
   drawLRP(x,y,w,h,rgb,-1,a, SIMPLE);
}

typedef union
{
   struct{ GLubyte r, g, b, a; };
   Pixel pixel;
} glpixel;

int32 glGetPixel(int32 x, int32 y)
{                
   glpixel gp;
   glReadPixels(x, appH-y-1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &gp); GL_CHECK_ERROR
   return (((int32)gp.r) << 16) | (((int32)gp.g) << 8) | (int32)gp.b;
}

void glGetPixels(Pixel* dstPixels,int32 srcX,int32 srcY,int32 width,int32 height,int32 pitch)
{          
   #define GL_BGRA 0x80E1 // BGRA is 20x faster than RGBA on devices that supports it
   PixelConv* p;
   glpixel gp;
   int32 i;
   GLint ext_format, ext_type;
   glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &ext_format);
   glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &ext_type);
   if (ext_format == GL_BGRA && ext_type == GL_UNSIGNED_BYTE) 
      for (; height-- > 0; srcY++,dstPixels += pitch)
      {
         glReadPixels(srcX, appH-srcY-1, width, 1, GL_BGRA, GL_UNSIGNED_BYTE, dstPixels); GL_CHECK_ERROR
         p = (PixelConv*)dstPixels;
         for (i = 0; i < width; i++,p++)
         {
            gp.pixel = p->pixel;
            p->a = 255;//gp.a; - with this, the transition effect causes a fade-out when finished in UIGadgets
            p->r = gp.b;
            p->g = gp.g;
            p->b = gp.r;
         }
      }   
   else
      for (; height-- > 0; srcY++,dstPixels += pitch)
      {
         glReadPixels(srcX, appH-srcY-1, width, 1, GL_RGBA, GL_UNSIGNED_BYTE, dstPixels); GL_CHECK_ERROR
         p = (PixelConv*)dstPixels;
         for (i = 0; i < width; i++,p++)
         {
            gp.pixel = p->pixel;
            p->a = 255;//gp.a; - with this, the transition effect causes a fade-out when finished in UIGadgets
            p->r = gp.r;
            p->g = gp.g;
            p->b = gp.b;
         }
      }   
}

void flushAll()
{
   glFlush(); GL_CHECK_ERROR
}

static void setProjectionMatrix(GLfloat w, GLfloat h)
{                              
   GLfloat mat[] =
   {
      2.0/w, 0.0, 0.0, -1.0,
      0.0, -2.0/h, 0.0, 1.0,
      0.0, 0.0, -1.0, 0.0,
      0.0, 0.0, 0.0, 1.0
   };
   setCurrentProgram(textProgram);    glUniformMatrix4fv(glGetUniformLocation(textProgram,    "projectionMatrix"), 1, 0, mat); GL_CHECK_ERROR
   setCurrentProgram(textureProgram); glUniformMatrix4fv(glGetUniformLocation(textureProgram, "projectionMatrix"), 1, 0, mat); GL_CHECK_ERROR
   setCurrentProgram(lrpProgram);     glUniformMatrix4fv(glGetUniformLocation(lrpProgram    , "projectionMatrix"), 1, 0, mat); GL_CHECK_ERROR
   setCurrentProgram(dotProgram);     glUniformMatrix4fv(glGetUniformLocation(dotProgram    , "projectionMatrix"), 1, 0, mat); GL_CHECK_ERROR
   setCurrentProgram(pointsProgram);  glUniformMatrix4fv(glGetUniformLocation(pointsProgram , "projectionMatrix"), 1, 0, mat); GL_CHECK_ERROR
   setCurrentProgram(shadeProgram);   glUniformMatrix4fv(glGetUniformLocation(shadeProgram  , "projectionMatrix"), 1, 0, mat); GL_CHECK_ERROR
#ifdef darwin
    int fw,fh;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &fw);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &fh);
    glViewport(0, 0, fw, fh); GL_CHECK_ERROR
#else
    glViewport(0, 0, w, h); GL_CHECK_ERROR
#endif
}

/////////////////////////////////////////////////////////////////////////
bool checkGLfloatBuffer(Context c, int32 n)
{
   if (n > flen)
   {
      xfree(glcoords);
      xfree(glcolors);
      flen = n*3/2;
      int len = flen*2;
      glcoords = (GLfloat*)xmalloc(sizeof(GLfloat)*len); 
      glcolors = (GLfloat*)xmalloc(sizeof(GLfloat)*flen); 
      if (!glcoords || !glcolors)
      {
         throwException(c, OutOfMemoryError, "Cannot allocate buffer for drawPixels");
         xfree(glcoords);
         xfree(glcolors);
         flen = 0;
         return false;
      }
   }
   return true;
}

bool setupGL(int width, int height)
{
    int i;
    pixLastRGB = -1;
    appW = width;
    appH = height;

    initTexture();
    initLineRectPoint();
    initPoints();
    initShade();
    setProjectionMatrix(appW,appH);

    glPixelStorei(GL_PACK_ALIGNMENT, 1); GL_CHECK_ERROR
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); GL_CHECK_ERROR

    glDisable(GL_CULL_FACE); GL_CHECK_ERROR
    glDisable(GL_DEPTH_TEST); GL_CHECK_ERROR
    glEnable(GL_BLEND); GL_CHECK_ERROR // enable color alpha channel
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); GL_CHECK_ERROR

    for (i = 0; i < 14; i++)
        ftransp[i+1] = (GLfloat)(i<<4) / (GLfloat)255; // make it lighter. since ftransp[0] is never used, shift it to [1]
    ftransp[15] = 1;
    for (i = 0; i <= 255; i++)
        f255[i] = (GLfloat)i/(GLfloat)255;
    return checkGLfloatBuffer(mainContext,1000);
}

#ifdef ANDROID
bool initGLES(ScreenSurface /*screen*/unused)
{
   int32 i;
   const EGLint attribs[] =
   {
       EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
       EGL_BLUE_SIZE, 8,
       EGL_GREEN_SIZE, 8,
       EGL_RED_SIZE, 8,
       EGL_ALPHA_SIZE, 8,
       EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
       EGL_NONE
   };
   EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
   EGLDisplay display;
   EGLConfig config;
   EGLint numConfigs;
   EGLint format;
   EGLSurface surface;
   EGLContext context;
   EGLint width;
   EGLint height;
   
   if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY)    {debug("eglGetDisplay() returned error %d", eglGetError()); return false;}
   if (!eglInitialize(display, 0, 0))                                       {debug("eglInitialize() returned error %d", eglGetError()); return false;}
   if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs))         {debug("eglChooseConfig() returned error %d", eglGetError()); destroyEGL(); return false;}
   if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {debug("eglGetConfigAttrib() returned error %d", eglGetError()); destroyEGL(); return false;}

   ANativeWindow_setBuffersGeometry(window, 0, 0, format);

   if (!(surface = eglCreateWindowSurface(display, config, window, 0)))     {debug("eglCreateWindowSurface() returned error %d", eglGetError()); destroyEGL(); return false;}
   if (!(context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs))) {debug("eglCreateContext() returned error %d", eglGetError()); destroyEGL(); return false;}
   if (!eglMakeCurrent(display, surface, surface, context))                 {debug("eglMakeCurrent() returned error %d", eglGetError()); destroyEGL(); return false;}
   if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) || !eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {debug("eglQuerySurface() returned error %d", eglGetError()); destroyEGL(); return false;}

   _display = display;
   _surface = surface;
   _context = context;
   return setupGL(width,height);
}

static void destroyEGL()
{         
   eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroyContext(_display, _context);
   eglDestroySurface(_display, _surface);
   eglTerminate(_display);

   _display = EGL_NO_DISPLAY;
   _surface = EGL_NO_SURFACE;
   _context = EGL_NO_CONTEXT;
}
#endif

void graphicsIOSdoRotate();
void privateScreenChange(int32 w, int32 h)
{
#ifdef darwin
   graphicsIOSdoRotate();
#else
    appW = w;
    appH = h;
#endif
   setProjectionMatrix(w,h); 
}

bool graphicsStartup(ScreenSurface screen, int16 appTczAttr)
{
   screen->bpp = 32;
   screen->screenX = screen->screenY = 0;
   screen->screenW = lastW;
   screen->screenH = lastH;
   screen->hRes = ascrHRes;
   screen->vRes = ascrVRes;
   return initGLES(screen);
}

bool graphicsCreateScreenSurface(ScreenSurface screen)
{
#ifndef ANDROID
   screen->extension = deviceCtx;
#endif
   screen->pitch = screen->screenW * screen->bpp / 8;
   screen->pixels = (uint8*)1;
   return screen->pixels != null;
}

void graphicsDestroy(ScreenSurface screen, bool isScreenChange)
{
#ifdef ANDROID
   if (!isScreenChange)
   {
      destroyEGL();
      xfree(screen->extension);
      xfree(glcoords);
      xfree(glcolors);
   }
#else
   if (isScreenChange)
       screen->extension = NULL;
   else
   {
      if (screen->extension)
         free(screen->extension);
      deviceCtx = screen->extension = NULL;
      xfree(glcoords);
      xfree(glcolors);
   }
#endif
}

void setTimerInterval(int32 t);
void setShiftYgl()
{                   
#ifdef ANDROID           
   if (setShiftYonNextUpdateScreen && needsPaint != null)
   {       
      setShiftYonNextUpdateScreen = false;
      glShiftY = desiredglShiftY - desiredScreenShiftY;     // set both at once
      screen.shiftY = desiredScreenShiftY;
      *needsPaint = true; // now that the shifts has been set, schedule another window update to paint at the given location
      setTimerInterval(1);      
   }                                     
   if (glShiftY < 0) // guich: occurs sometimes when the keyboard is closed and the desired shift y is 0. it was resulting in a negative value.
      glShiftY = 0;
#else
    glShiftY = -desiredScreenShiftY;
#endif    
}
extern int32 desiredScreenShiftY;
void graphicsUpdateScreenIOS();
void graphicsUpdateScreen(Context currentContext, ScreenSurface screen)
{ 
   if (surfaceWillChange) return;
#ifdef ANDROID
   eglSwapBuffers(_display, _surface); // requires API LEVEL 9 (2.3 and up)
#else
   graphicsUpdateScreenIOS();
#endif
   // erase buffer with keyboard's background color
   PixelConv gray;
   gray.pixel = shiftScreenColorP ? *shiftScreenColorP : 0xFFFFFF;
   glClearColor(f255[gray.r],f255[gray.g],f255[gray.b],1); GL_CHECK_ERROR
   glClear(GL_COLOR_BUFFER_BIT); GL_CHECK_ERROR
   resetGlobals();
}
