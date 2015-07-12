///////////////////////////////////////////////////////////////////////////////
// ModelGL.cpp
// ===========
// Model component of OpenGL
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2008-09-15
// UPDATED: 2011-03-24
///////////////////////////////////////////////////////////////////////////////

#if _WIN32
    #include <windows.h>    // include windows.h to avoid thousands of compile errors even though this class is not depending on Windows
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include "ModelGL.h"
#include "teapot.h"
#include "cameraSimple.h"
#include "wcharUtil.h"


// constants
const float DEG2RAD = 3.141593f / 180;
const float FOV_Y = 60.0f;              // vertical FOV in degree
const float NEAR_PLANE = 1.0f;
const float FAR_PLANE = 100.0f;
const float CAMERA_ANGLE_X = 45.0f;     // pitch in degree
const float CAMERA_ANGLE_Y = -45.0f;    // heading in degree
const float CAMERA_DISTANCE = 25.0f;    // camera distance



///////////////////////////////////////////////////////////////////////////////
// default ctor
///////////////////////////////////////////////////////////////////////////////
ModelGL::ModelGL() : windowWidth(0), windowHeight(0), mouseLeftDown(false),
                     mouseRightDown(false), drawModeChanged(false), drawMode(0),
                     cameraAngleX(CAMERA_ANGLE_X), cameraAngleY(CAMERA_ANGLE_Y),
                     cameraDistance(CAMERA_DISTANCE), windowSizeChanged(false)
{
    cameraPosition[0] = cameraPosition[1] = cameraPosition[2] = 0;
    cameraAngle[0] = cameraAngle[1] = cameraAngle[2] = 0;
    modelPosition[0] = modelPosition[1] = modelPosition[2] = 0;
    modelAngle[0] = modelAngle[1] = modelAngle[2] = 0;
    bgColor[0] = bgColor[1] = bgColor[2] = bgColor[3] = 0;

    for(int i = 0; i < 16; ++i)
    {
        matrixView[i] = matrixModel[i] = matrixModelView[i] = 0;
    }
}



///////////////////////////////////////////////////////////////////////////////
// destructor
///////////////////////////////////////////////////////////////////////////////
ModelGL::~ModelGL()
{
}



///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL states and scene
///////////////////////////////////////////////////////////////////////////////
void ModelGL::init()
{
    glShadeModel(GL_SMOOTH);                        // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);          // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_SCISSOR_TEST);

     // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);   // background color
    glClearStencil(0);                              // clear stencil buffer
    glClearDepth(1.0f);                             // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initLights();
}



///////////////////////////////////////////////////////////////////////////////
// initialize lights
///////////////////////////////////////////////////////////////////////////////
void ModelGL::initLights()
{
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {.0f, .0f, .0f, 1.0f};      // ambient light
    GLfloat lightKd[] = {.9f, .9f, .9f, 1.0f};      // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};               // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = {0, 5, 5, 0};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                            // MUST enable each light source after configuration
}



///////////////////////////////////////////////////////////////////////////////
// set camera position and lookat direction
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0); // eye(x,y,z), focal(x,y,z), up(x,y,z)
}



///////////////////////////////////////////////////////////////////////////////
// set rendering window size
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setWindowSize(int width, int height)
{
    // assign the width/height of viewport
    windowWidth = width;
    windowHeight = height;
    windowSizeChanged = true;
}



///////////////////////////////////////////////////////////////////////////////
// configure projection and viewport
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setViewport(int x, int y, int w, int h)
{
    // set viewport to be the entire window
    glViewport((GLsizei)x, (GLsizei)y, (GLsizei)w, (GLsizei)h);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV_Y, (float)(w)/h, NEAR_PLANE, FAR_PLANE); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



///////////////////////////////////////////////////////////////////////////////
// configure projection and viewport of sub window
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setViewportSub(int x, int y, int width, int height, float nearPlane, float farPlane)
{
    // set viewport
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV_Y, (float)(width)/height, nearPlane, farPlane); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



///////////////////////////////////////////////////////////////////////////////
// draw 2D/3D scene
///////////////////////////////////////////////////////////////////////////////
void ModelGL::draw()
{
    drawSub1();
    drawSub2();

    if(windowSizeChanged)
    {
        setViewport(0, 0, windowWidth, windowHeight);
        windowSizeChanged = false;
    }

    if(drawModeChanged)
    {
        if(drawMode == 0)           // fill mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
        else if(drawMode == 1)      // wireframe mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        else if(drawMode == 2)      // point mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        drawModeChanged = false;
    }
}



///////////////////////////////////////////////////////////////////////////////
// draw upper window (view from the camera)
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawSub1()
{
    // set upper viewport
    setViewportSub(0, windowHeight/2, windowWidth, windowHeight/2, 1, 10);

    // clear buffer
    glClearColor(0.1f, 0.1f, 0.1f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // initialze ModelView matrix
    glPushMatrix();
    glLoadIdentity();

    // ModelView matrix is product of viewing matrix and modeling matrix
    // ModelView_M = View_M * Model_M
    // First, transform the camera (viewing matrix) from world space to eye space
    // Notice all values are negated, because we move the whole scene with the
    // inverse of camera transform
    glRotatef(-cameraAngle[2], 0, 0, 1); // roll
    glRotatef(-cameraAngle[1], 0, 1, 0); // heading
    glRotatef(-cameraAngle[0], 1, 0, 0); // pitch
    glTranslatef(-cameraPosition[0], -cameraPosition[1], -cameraPosition[2]);

    // we have set viewing matrix upto this point. 
    //(Matrix from world space to eye space)
    // save the view matrix only
    glGetFloatv(GL_MODELVIEW_MATRIX, matrixView); // save viewing matrix
    //=========================================================================


    // always draw the grid at the origin (before any modeling transform)
    drawGrid(10, 1);

    // In order to get the modeling matrix only, reset GL_MODELVIEW matrix
    glLoadIdentity();

    // transform the object
    // From now, all transform will be for modeling matrix only. (transform from object space to world space)
    glTranslatef(modelPosition[0], modelPosition[1], modelPosition[2]);
    glRotatef(modelAngle[0], 1, 0, 0);
    glRotatef(modelAngle[1], 0, 1, 0);
    glRotatef(modelAngle[2], 0, 0, 1);

    // save modeling matrix
    glGetFloatv(GL_MODELVIEW_MATRIX, matrixModel);
    //=========================================================================


    // re-strore GL_MODELVIEW matrix by multiplying matrixView and matrixModel before drawing the object
    // ModelView_M = View_M * Model_M
    glLoadMatrixf(matrixView);              // Mmv = Mv
    glMultMatrixf(matrixModel);             // Mmv *= Mm

    // save ModelView matrix
    glGetFloatv(GL_MODELVIEW_MATRIX, matrixModelView);
    //=========================================================================


    // draw a teapot after ModelView transform
    // v' = Mmv * v
    drawAxis(4);
    drawTeapot();

    glPopMatrix();
}



///////////////////////////////////////////////////////////////////////////////
// draw bottom window (3rd person view)
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawSub2()
{
    // set bottom viewport
    setViewportSub(0, 0, windowWidth, windowHeight/2, NEAR_PLANE, FAR_PLANE);

    // clear buffer
    glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);   // background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glPushMatrix();

    // First, transform the camera (viewing matrix) from world space to eye space
    glTranslatef(0, 0, -cameraDistance);
    glRotatef(cameraAngleX, 1, 0, 0); // pitch
    glRotatef(cameraAngleY, 0, 1, 0); // heading

    // draw grid
    drawGrid(10, 1);

    // draw a teapot
    glPushMatrix();
    glTranslatef(modelPosition[0], modelPosition[1], modelPosition[2]);
    glRotatef(modelAngle[0], 1, 0, 0);
    glRotatef(modelAngle[1], 0, 1, 0);
    glRotatef(modelAngle[2], 0, 0, 1);
    drawAxis(4);
    drawTeapot();
    glPopMatrix();

    // draw the camera
    glPushMatrix();
    glTranslatef(cameraPosition[0], cameraPosition[1], cameraPosition[2]);
    glRotatef(cameraAngle[0], 1, 0, 0);
    glRotatef(cameraAngle[1], 0, 1, 0);
    glRotatef(cameraAngle[2], 0, 0, 1);
    drawCamera();
    drawFrustum(FOV_Y, 1, 1, 10);
    glPopMatrix();

    glPopMatrix();
}



///////////////////////////////////////////////////////////////////////////////
// draw a grid on the xz plane
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawGrid(float size, float step)
{
    // disable lighting
    glDisable(GL_LIGHTING);

    glBegin(GL_LINES);

    glColor3f(0.3f, 0.3f, 0.3f);
    for(float i=step; i <= size; i+= step)
    {
        glVertex3f(-size, 0,  i);   // lines parallel to X-axis
        glVertex3f( size, 0,  i);
        glVertex3f(-size, 0, -i);   // lines parallel to X-axis
        glVertex3f( size, 0, -i);

        glVertex3f( i, 0, -size);   // lines parallel to Z-axis
        glVertex3f( i, 0,  size);
        glVertex3f(-i, 0, -size);   // lines parallel to Z-axis
        glVertex3f(-i, 0,  size);
    }

    // x-axis
    glColor3f(0.5f, 0, 0);
    glVertex3f(-size, 0, 0);
    glVertex3f( size, 0, 0);

    // z-axis
    glColor3f(0,0,0.5f);
    glVertex3f(0, 0, -size);
    glVertex3f(0, 0,  size);

    glEnd();

    // enable lighting back
    glEnable(GL_LIGHTING);
}



///////////////////////////////////////////////////////////////////////////////
// draw the local axis of an object
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawAxis(float size)
{
    glDepthFunc(GL_ALWAYS);     // to avoid visual artifacts with grid lines
    glDisable(GL_LIGHTING);

    // draw axis
    glLineWidth(3);
    glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(size, 0, 0);
        glColor3f(0, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, size, 0);
        glColor3f(0, 0, 1);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, size);
    glEnd();
    glLineWidth(1);

    // draw arrows(actually big square dots)
    glPointSize(5);
    glBegin(GL_POINTS);
        glColor3f(1, 0, 0);
        glVertex3f(size, 0, 0);
        glColor3f(0, 1, 0);
        glVertex3f(0, size, 0);
        glColor3f(0, 0, 1);
        glVertex3f(0, 0, size);
    glEnd();
    glPointSize(1);

    // restore default settings
    glEnable(GL_LIGHTING);
    glDepthFunc(GL_LEQUAL);
}



///////////////////////////////////////////////////////////////////////////////
// draw frustum
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawFrustum(float fovY, float aspectRatio, float nearPlane, float farPlane)
{
    float tangent = tanf(fovY/2 * DEG2RAD);
    float nearHeight = nearPlane * tangent;
    float nearWidth = nearHeight * aspectRatio;
    float farHeight = farPlane * tangent;
    float farWidth = farHeight * aspectRatio;

    // compute 8 vertices of the frustum
    float vertices[8][3];
    // near top right
    vertices[0][0] = nearWidth;     vertices[0][1] = nearHeight;    vertices[0][2] = -nearPlane;
    // near top left
    vertices[1][0] = -nearWidth;    vertices[1][1] = nearHeight;    vertices[1][2] = -nearPlane;
    // near bottom left
    vertices[2][0] = -nearWidth;    vertices[2][1] = -nearHeight;   vertices[2][2] = -nearPlane;
    // near bottom right
    vertices[3][0] = nearWidth;     vertices[3][1] = -nearHeight;   vertices[3][2] = -nearPlane;
    // far top right
    vertices[4][0] = farWidth;      vertices[4][1] = farHeight;     vertices[4][2] = -farPlane;
    // far top left
    vertices[5][0] = -farWidth;     vertices[5][1] = farHeight;     vertices[5][2] = -farPlane;
    // far bottom left
    vertices[6][0] = -farWidth;     vertices[6][1] = -farHeight;    vertices[6][2] = -farPlane;
    // far bottom right
    vertices[7][0] = farWidth;      vertices[7][1] = -farHeight;    vertices[7][2] = -farPlane;

    float colorLine1[4] = { 0.7f, 0.7f, 0.7f, 0.7f };
    float colorLine2[4] = { 0.2f, 0.2f, 0.2f, 0.7f };
    float colorPlane[4] = { 0.5f, 0.5f, 0.5f, 0.5f };

    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_LINES);
    glColor4fv(colorLine2);
    glVertex3f(0, 0, 0);
    glColor4fv(colorLine1);
    glVertex3fv(vertices[4]);

    glColor4fv(colorLine2);
    glVertex3f(0, 0, 0);
    glColor4fv(colorLine1);
    glVertex3fv(vertices[5]);

    glColor4fv(colorLine2);
    glVertex3f(0, 0, 0);
    glColor4fv(colorLine1);
    glVertex3fv(vertices[6]);

    glColor4fv(colorLine2);
    glVertex3f(0, 0, 0);
    glColor4fv(colorLine1);
    glVertex3fv(vertices[7]);
    glEnd();

    glColor4fv(colorLine1);
    glBegin(GL_LINE_LOOP);
    glVertex3fv(vertices[4]);
    glVertex3fv(vertices[5]);
    glVertex3fv(vertices[6]);
    glVertex3fv(vertices[7]);
    glEnd();

    glColor4fv(colorLine1);
    glBegin(GL_LINE_LOOP);
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[2]);
    glVertex3fv(vertices[3]);
    glEnd();

    glColor4fv(colorPlane);
    glBegin(GL_QUADS);
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[2]);
    glVertex3fv(vertices[3]);
    glVertex3fv(vertices[4]);
    glVertex3fv(vertices[5]);
    glVertex3fv(vertices[6]);
    glVertex3fv(vertices[7]);
    glEnd();

    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
}


///////////////////////////////////////////////////////////////////////////////
// set the camera position and rotation
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setViewMatrix(float x, float y, float z, float pitch, float heading, float roll)
{
    cameraPosition[0] = x;
    cameraPosition[1] = y;
    cameraPosition[2] = z;
    cameraAngle[0] = pitch;
    cameraAngle[1] = heading;
    cameraAngle[2] = roll;
}



///////////////////////////////////////////////////////////////////////////////
// set the object position and rotation
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setModelMatrix(float x, float y, float z, float rx, float ry, float rz)
{
    modelPosition[0] = x;
    modelPosition[1] = y;
    modelPosition[2] = z;
    modelAngle[0] = rx;
    modelAngle[1] = ry;
    modelAngle[2] = rz;
}



///////////////////////////////////////////////////////////////////////////////
// rotate the camera
///////////////////////////////////////////////////////////////////////////////
void ModelGL::rotateCamera(int x, int y)
{
    if(mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
}



///////////////////////////////////////////////////////////////////////////////
// zoom the camera
///////////////////////////////////////////////////////////////////////////////
void ModelGL::zoomCamera(int delta)
{
    if(mouseRightDown)
    {
        cameraDistance += (delta - mouseY) * 0.05f;
        mouseY = delta;
    }
}



///////////////////////////////////////////////////////////////////////////////
// change drawing mode
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setDrawMode(int mode)
{
    if(drawMode != mode)
    {
        drawModeChanged = true;
        drawMode = mode;
    }
}



///////////////////////////////////////////////////////////////////////////////
// transpose then return matrix values
///////////////////////////////////////////////////////////////////////////////
void ModelGL::getViewMatrix(float* m)
{
    m[0]  = matrixView[0];
    m[1]  = matrixView[4];
    m[2]  = matrixView[8];
    m[3]  = matrixView[12];
    m[4]  = matrixView[1];
    m[5]  = matrixView[5];
    m[6]  = matrixView[9];
    m[7]  = matrixView[13];
    m[8]  = matrixView[2];
    m[9]  = matrixView[6];
    m[10] = matrixView[10];
    m[11] = matrixView[14];
    m[12] = matrixView[3];
    m[13] = matrixView[7];
    m[14] = matrixView[11];
    m[15] = matrixView[15];
}
void ModelGL::getModelMatrix(float* m)
{
    m[0]  = matrixModel[0];
    m[1]  = matrixModel[4];
    m[2]  = matrixModel[8];
    m[3]  = matrixModel[12];
    m[4]  = matrixModel[1];
    m[5]  = matrixModel[5];
    m[6]  = matrixModel[9];
    m[7]  = matrixModel[13];
    m[8]  = matrixModel[2];
    m[9]  = matrixModel[6];
    m[10] = matrixModel[10];
    m[11] = matrixModel[14];
    m[12] = matrixModel[3];
    m[13] = matrixModel[7];
    m[14] = matrixModel[11];
    m[15] = matrixModel[15];
}
void ModelGL::getModelViewMatrix(float* m)
{
    m[0]  = matrixModelView[0];
    m[1]  = matrixModelView[4];
    m[2]  = matrixModelView[8];
    m[3]  = matrixModelView[12];
    m[4]  = matrixModelView[1];
    m[5]  = matrixModelView[5];
    m[6]  = matrixModelView[9];
    m[7]  = matrixModelView[13];
    m[8]  = matrixModelView[2];
    m[9]  = matrixModelView[6];
    m[10] = matrixModelView[10];
    m[11] = matrixModelView[14];
    m[12] = matrixModelView[3];
    m[13] = matrixModelView[7];
    m[14] = matrixModelView[11];
    m[15] = matrixModelView[15];
}
