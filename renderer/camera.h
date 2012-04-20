#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>

#include "inputstate.h"

class Camera
{
public:
  Camera(float moveRate = 1.0, float zoomRate = 0.001, InputState::MouseButton button = InputState::MouseMiddle);

  void setAxis(const QVector3D& u, const QVector3D& v, const QVector3D& w);

  bool update(const InputState &state, float delta);

  void reset();

  inline QVector3D translation() const { return _translation; }

protected:
  InputState::MouseButton _button;
  bool _pressed;

  float _moveRate;
  float _zoomRate;

  QPointF _lastPoint;

  QVector3D _u, _v, _w;

  QVector3D _translation;

  void press(const QPointF& point);
  void release(const QPointF& point);

  void move(const QPointF& point);
};

#endif // CAMERA_H
