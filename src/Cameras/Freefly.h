#ifndef FREEFLY_H
#define FREEFLY_H

#include <QQuaternion>
#include <QVector3D>
#include <QMatrix4x4>
#include <QPoint>
#include <QtMath>

class Freefly
{
  public:
    Freefly();
    ~Freefly();

    QMatrix4x4 getViewMatrix() const;
    void computeDirectionVectors();
    void moveLeft(float t);
    void moveFront(float t);
    void rotateLeft(float degrees);
    void rotateUp(float degrees);
    QVector3D getPosition() const { return m_position; }


  private: 

    QVector3D m_position;
    QVector3D m_frontVector;
    QVector3D m_leftVector;
    QVector3D m_upVector;
    float m_theta;
    float m_phi;
};

#endif // FREEFLY_H