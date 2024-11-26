#ifndef TRACKBALL_H
#define TRACKBALL_H

#include <QQuaternion>
#include <QVector3D>
#include <QMatrix4x4>
#include <QPoint>
#include <cmath>

class TrackBall
{
  public:
    TrackBall();
    ~TrackBall();

    void moveFront(float delta);
    void rotateLeft(float degree);
    void rotateUp(float degree);

    QMatrix4x4 getViewMatrix() const;
    QVector3D getPosition();


  private :
    float m_distance;
    float m_angleX;
    float m_angleY;
};

#endif // TRACKBALL_H