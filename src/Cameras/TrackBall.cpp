#include "TrackBall.h"

TrackBall::TrackBall(): m_distance(5), m_angleX(0), m_angleY(0)
{
}

TrackBall::~TrackBall()
{
}

void TrackBall::moveFront(float delta)
{
  m_distance -= delta;
}

void TrackBall::rotateLeft(float degree)
{
  m_angleY += degree;
}

void TrackBall::rotateUp(float degree)
{
  m_angleX += degree;
}

QMatrix4x4 TrackBall::getViewMatrix() const
{
  QMatrix4x4 viewMatrix;
  viewMatrix.translate(0, 0, -m_distance);
  viewMatrix.rotate(m_angleX, 1, 0, 0);
  viewMatrix.rotate(m_angleY, 0, 1, 0);
  return viewMatrix;
}

QVector3D TrackBall::getPosition() {
        // convert angle to radian
        float angleXRad = m_angleX * M_PI / 180.0f;
        float angleYRad = m_angleY * M_PI / 180.0f;
        
        // carthesian coordinates
        // x = r * cos(θ) * cos(phi)
        // y = r * sin(phi)
        // z = r * sin(θ) * cos(phi)
        // with r as distance, θ as angleX, phi as angleY
        float cosY = cos(angleYRad);
        return QVector3D(
            m_distance * cos(angleXRad) * cosY,
            m_distance * sin(angleYRad),
            m_distance * sin(angleXRad) * cosY
        );
    }