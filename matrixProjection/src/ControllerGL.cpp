///////////////////////////////////////////////////////////////////////////////
// ControllerGL.cpp
// ================
// Derived Controller class for OpenGL window
// It is the controller of OpenGL rendering window. It initializes DC and RC,
// when WM_CREATE called, then, start new thread for OpenGL rendering loop.
//
// When this class is constructed, it gets the pointers to model and view
// components.
//
//  AUTHOR: Song Ho Ahn (song.ahn@gamil.com)
// CREATED: 2008-09-15
// UPDATED: 2008-10-07
///////////////////////////////////////////////////////////////////////////////

#include <process.h>                                // for _beginthreadex()
#include "ControllerGL.h"
#include "Log.h"
using namespace Win;



///////////////////////////////////////////////////////////////////////////////
// default contructor
///////////////////////////////////////////////////////////////////////////////
ControllerGL::ControllerGL(ModelGL* model, ViewGL* view) : model(model), view(view),
                                                           threadHandle(0), threadId(0),
                                                           loopFlag(false)
{
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_CLOSE
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::close()
{
    loopFlag = false;
    ::WaitForSingleObject(threadHandle, INFINITE);   // wait for rendering thread is terminated

    // close OpenGL Rendering context
    view->closeContext(handle);

    ::DestroyWindow(handle);
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_CREATE
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::create()
{
    // create a OpenGL rendering context
    if(!view->createContext(handle, 32, 24, 8))
    {
        Win::log(L"[ERROR] Failed to create OpenGL rendering context from ControllerGL::create().");
        return -1;
    }

    // create a thread for OpenGL rendering
    // The params of _beginthreadex() are security, stackSize, functionPtr, argPtr, initFlag, threadId.
    threadHandle = (HANDLE)_beginthreadex(0, 0, (unsigned (__stdcall *)(void *))threadFunction, this, 0, &threadId);
    if(threadHandle)
    {
        loopFlag = true;
        Win::log(L"Created a rendering thread for OpenGL.");
    }
    else
    {
        Win::log(L"[ERROR] Failed to create rendering thread from ControllerGL::create().");
    }

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_PAINT
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::paint()
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// handle WM_COMMAND
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::command(int id, int cmd, LPARAM msg)
{
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// route to worker thread
// The function prototype is:
// unsigned int (__stdcall *)(void *)
///////////////////////////////////////////////////////////////////////////////
void ControllerGL::threadFunction(void* param)
{
    ((ControllerGL*)param)->runThread();
}



///////////////////////////////////////////////////////////////////////////////
// rendering thread
// initialize OpenGL states and start rendering loop
///////////////////////////////////////////////////////////////////////////////
void ControllerGL::runThread()
{
    // set the current RC in this thread
    ::wglMakeCurrent(view->getDC(), view->getRC());

    // initialize OpenGL states
    model->init();
    Win::log(L"Initialized OpenGL states.");

    // rendering loop
    Win::log(L"Entering OpenGL rendering thread...");
    while(loopFlag)
    {
        ::Sleep(1);                     // yield to other processes or threads
        model->draw();
        view->swapBuffers();
    }

    // clean up OpenGL stuff
    model->quit();

    // terminate rendering thread
    ::wglMakeCurrent(0, 0);             // unset RC
    ::CloseHandle(threadHandle);
    Win::log(L"Exit OpenGL rendering thread.");
}



///////////////////////////////////////////////////////////////////////////////
// handle Left mouse down
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::lButtonDown(WPARAM state, int x, int y)
{
    // update mouse position
    model->setMousePosition(x, y);

    if(state == MK_LBUTTON)
    {
        model->setMouseLeft(true);
    }

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle Left mouse up
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::lButtonUp(WPARAM state, int x, int y)
{
    // update mouse position
    model->setMousePosition(x, y);

    model->setMouseLeft(false);

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle reft mouse down
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::rButtonDown(WPARAM state, int x, int y)
{
    // update mouse position
    model->setMousePosition(x, y);

    if(state == MK_RBUTTON)
    {
        model->setMouseRight(true);
    }

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle reft mouse up
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::rButtonUp(WPARAM state, int x, int y)
{
    // update mouse position
    model->setMousePosition(x, y);

    model->setMouseRight(false);

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_MOUSEMOVE
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::mouseMove(WPARAM state, int x, int y)
{
    if(state == MK_LBUTTON)
    {
        model->rotateCamera(x, y);
    }
    if(state == MK_RBUTTON)
    {
        model->zoomCamera(y);
    }

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_SIZE
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::size(int w, int h, WPARAM wParam)
{
    model->setWindowSize(w, h);
    Win::log(L"Changed OpenGL rendering window size: %dx%d.", w, h);
    return 0;
}



