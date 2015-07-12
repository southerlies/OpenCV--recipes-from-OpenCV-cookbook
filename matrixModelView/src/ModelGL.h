///////////////////////////////////////////////////////////////////////////////
// ModelGL.h
// =========
// Model component of OpenGL
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2008-09-15
// UPDATED: 2008-09-26
///////////////////////////////////////////////////////////////////////////////

#ifndef MODEL_GL_H
#define MODEL_GL_H

class ModelGL
{
public:
    ModelGL();
    ~ModelGL();

    void init();                                    // initialize OpenGL states
    void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
    void draw();

    void setMouseLeft(bool flag) { mouseLeftDown = flag; };
    void setMouseRight(bool flag) { mouseRightDown = flag; };
    void setMousePosition(int x, int y) { mouseX = x; mouseY = y; };
    void setDrawMode(int mode);
    void setWindowSize(int width, int height);
    void setViewMatrix(float x, float y, float z, float pitch, float heading, float roll);
    void setModelMatrix(float x, float y, float z, float rx, float ry, float rz);

    void setCameraX(float x)        { cameraPosition[0] = x; }
    void setCameraY(float y)        { cameraPosition[1] = y; }
    void setCameraZ(float z)        { cameraPosition[2] = z; }
    void setCameraAngleX(float p)   { cameraAngle[0] = p; }
    void setCameraAngleY(float h)   { cameraAngle[1] = h; }
    void setCameraAngleZ(float r)   { cameraAngle[2] = r; }
    float getCameraX()              { return cameraPosition[0]; }
    float getCameraY()              { return cameraPosition[1]; }
    float getCameraZ()              { return cameraPosition[2]; }
    float getCameraAngleX()         { return cameraAngle[0]; }
    float getCameraAngleY()         { return cameraAngle[1]; }
    float getCameraAngleZ()         { return cameraAngle[2]; }

    void setModelX(float x)         { modelPosition[0] = x; }
    void setModelY(float y)         { modelPosition[1] = y; }
    void setModelZ(float z)         { modelPosition[2] = z; }
    void setModelAngleX(float a)    { modelAngle[0] = a; }
    void setModelAngleY(float a)    { modelAngle[1] = a; }
    void setModelAngleZ(float a)    { modelAngle[2] = a; }
    float getModelX()               { return modelPosition[0]; }
    float getModelY()               { return modelPosition[1]; }
    float getModelZ()               { return modelPosition[2]; }
    float getModelAngleX()          { return modelAngle[0]; }
    float getModelAngleY()          { return modelAngle[1]; }
    float getModelAngleZ()          { return modelAngle[2]; }


    void getViewMatrix(float* m);       // should have 16 elements
    void getModelMatrix(float* m);      // should have 16 elements
    void getModelViewMatrix(float* m);  // should have 16 elements

    void rotateCamera(int x, int y);
    void zoomCamera(int dist);


protected:

private:
    // member functions
    void initLights();                              // add a white light ti scene
    void setViewport(int x, int y, int width, int height);
    void setViewportSub(int left, int bottom, int width, int height, float nearPlane, float farPlane);
    void drawGrid(float size, float step);          // draw a grid on XZ plane
    void drawAxis(float size);
    void drawSub1();                                // draw upper window
    void drawSub2();                                // draw bottom window
    void drawFrustum(float fovy, float aspect, float near, float far);

    // members
    int windowWidth;
    int windowHeight;
    bool windowSizeChanged;
    bool drawModeChanged;
    int drawMode;
    bool mouseLeftDown;
    bool mouseRightDown;
    int mouseX;
    int mouseY;
    float cameraPosition[3];
    float cameraAngle[3];
    float modelPosition[3];
    float modelAngle[3];

    // these are for 3rd person view
    float cameraAngleX;
    float cameraAngleY;
    float cameraDistance;
    float bgColor[4];
    float matrixView[16];
    float matrixModel[16];
    float matrixModelView[16];
};
#endif
