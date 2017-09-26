#include <epoxy/gl.h>
#include <epoxy/glx.h>
#include <fx.h>
#include <fx3d.h>

#include "qtvr_scene.h"


class TNGView : public FXGLCanvas {
FXDECLARE(TNGView);
protected:
  qtvr_panorama_scene * tng = nullptr;
  bool rotating = false;
public:
  long onConfigure(FXObject*,FXSelector,void*);
  long onRender(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onLeftButtonPress(FXObject*,FXSelector,void*);
  long onLeftButtonRelease(FXObject*,FXSelector,void*);
  long onRightButtonPress(FXObject*,FXSelector,void*);

protected:
  TNGView(){}
public:
  TNGView(FXComposite* p,FXGLContext *ctx,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);

  void setScene(qtvr_panorama_scene * s);
  };


FXDEFMAP(TNGView) TNGViewMap[]={
  FXMAPFUNC(SEL_PAINT, 0, TNGView::onRender),
  FXMAPFUNC(SEL_MOTION, 0, TNGView::onMotion),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS, 0, TNGView::onRightButtonPress),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS, 0, TNGView::onLeftButtonPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE, 0, TNGView::onLeftButtonRelease),
  FXMAPFUNC(SEL_CONFIGURE, 0, TNGView::onConfigure)
  };

FXIMPLEMENT(TNGView,FXGLCanvas,TNGViewMap,ARRAYNUMBER(TNGViewMap));

TNGView::TNGView(FXComposite* p, FXGLContext *ctx, FXuint opts, FXint x, FXint y, FXint w, FXint h) : FXGLCanvas(p, ctx, nullptr, 0, opts, x, y, w, h){
  }


void TNGView::setScene(qtvr_panorama_scene * s) {
  if (tng) delete tng;
  tng = s;
  }



long TNGView::onRender(FXObject*, FXSelector, void*) {
  if (makeCurrent()) {
    if (tng) {
      tng->render(width, height);
      }
    else {
      FXVec4f background = colorToVec4f(backColor);
      glClearColor(background.x, background.y, background.z, background.w);
      glClear(GL_COLOR_BUFFER_BIT);
      }
    if(getContext()->isDoubleBuffer()) swapBuffers();
    makeNonCurrent();
    }
  return 1;
  }


long TNGView::onConfigure(FXObject*, FXSelector, void*) {
  if (makeCurrent()) {
    glViewport(0, 0, width ,height);
    makeNonCurrent();
    }
  return 1;
  }

long TNGView::onMotion(FXObject*, FXSelector, void* ptr) {
  FXEvent * event = reinterpret_cast<FXEvent*>(ptr);
  if (rotating) {
    tng->panview(100.0 * ((event->last_x - event->win_x) / (double)width));
    update();
    }
  else {
    if (makeCurrent()) {
      if (tng->pick(event->win_x,event->win_y,width, height))
        setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
      else
        setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
      makeNonCurrent();
      }
    }
  return 1;
  }

long TNGView::onRightButtonPress(FXObject*, FXSelector, void* ptr) {
  FXEvent * event = reinterpret_cast<FXEvent*>(ptr);
  handle(this,FXSEL(SEL_FOCUS_SELF,0),ptr);
  if (isEnabled()) {
    if (makeCurrent()) {
      tng->select(event->win_x,event->win_y,width,height);
      makeNonCurrent();
      update();
      }
    }
  return 1;
  }


long TNGView::onLeftButtonPress(FXObject*, FXSelector, void* ptr) {
  handle(this,FXSEL(SEL_FOCUS_SELF,0),ptr);
  if (isEnabled()) {
    grab();
    flags&=~FLAG_UPDATE;
    rotating = true;
    }
  return 1;
  }

long TNGView::onLeftButtonRelease(FXObject*, FXSelector, void*) {
  if(isEnabled()){
    ungrab();
    flags|=FLAG_UPDATE;
    rotating = false;
    }
  return 1;
  }



// Event Handler Object
class TNGWindow : public FXMainWindow {
  FXDECLARE(TNGWindow)
private:
  FXGLVisual *glvisual = nullptr;
  FXGLContext *glcontext = nullptr;
  TNGView * glview = nullptr;
protected:
  TNGWindow(){}
public:
  enum{
    ID_CANVAS=FXMainWindow::ID_LAST,
    };
public:
  long onGLPaint(FXObject*, FXSelector, void*);
public:
  TNGWindow(FXApp* a);
  void create();
  virtual ~TNGWindow();
  };

// Message Map
FXDEFMAP(TNGWindow) TNGWindowMap[]={};


FXIMPLEMENT(TNGWindow,FXMainWindow,TNGWindowMap,ARRAYNUMBER(TNGWindowMap))

TNGWindow::TNGWindow(FXApp * app) : FXMainWindow(app, "Star Trek The Next Generation - Interactive Technical Manual", nullptr, nullptr, DECOR_ALL, 0, 0, 640, 480){
  glvisual = new FXGLVisual(app,VISUAL_DOUBLE_BUFFER);
  glcontext = new FXGLContext(app, glvisual);
  glview = new TNGView(this, glcontext, LAYOUT_FILL);
  }

void TNGWindow::create(){
  FXMainWindow::create();
  show(PLACEMENT_SCREEN);


  qtvr_panorama_scene * tng = new qtvr_panorama_scene();
  tng->load(getApp()->getArgv()[1]);
  glview->setScene(tng);

  }

TNGWindow::~TNGWindow() {
  }

int main(int argc, char* argv[]){
  // Make application
  FXApp application("STTNG","STTNG");

  // Open the display
  application.init(argc,argv);

  // Make window
  new TNGWindow(&application);

  // Create the application's windows
  application.create();

  // Run the application
  return application.run();
  }
