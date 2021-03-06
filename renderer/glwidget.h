#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QMouseEvent>
#include <QTimer>
#include <QGLWidget>

#include "inputstate.h"
#include "timer.h"

class Scene;

class GLWidget : public QGLWidget {
  Q_OBJECT

public:
  enum RenderMode {
    Points,
    Wireframe,
    Solid
  };

  explicit GLWidget(QWidget *parent = 0);
  virtual ~GLWidget();

  void loadScene(const char* filename);

  void renderMode(RenderMode mode);

  void centerView();

public slots:
  void animate();

protected:
  Timer _timer;
  QTimer _qtimer;

  InputState _inputState;

  Scene *_scene;
  Scene *_nextScene;

  void initializeGL();
  void paintGL();
  void resizeGL(int width, int height);

  void mousePressEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void wheelEvent(QWheelEvent* event);

  void rebuildProjection();

  QPointF normalizePoint(const QPointF& point);
};

#endif // GLWIDGET_H
