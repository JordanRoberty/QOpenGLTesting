#include "Freefly.h"

Freefly::Freefly(): m_position(0, 0, 5), m_theta(0), m_phi(M_PI)
{
  computeDirectionVectors();
}

Freefly::~Freefly()
{
}

QMatrix4x4 Freefly::getViewMatrix() const
{
  QMatrix4x4 viewMatrix;
  viewMatrix.lookAt(m_position, m_position + m_frontVector, m_upVector);
  return viewMatrix;
}

void Freefly::computeDirectionVectors()
{
  m_frontVector.setX(qCos(m_theta) * qSin(m_phi));
  m_frontVector.setY(qSin(m_theta));
  m_frontVector.setZ(qCos(m_theta) * qCos(m_phi));

  m_frontVector.normalize();

  m_leftVector.setX(qSin(m_phi + M_PI/2));
  m_leftVector.setY(0);
  m_leftVector.setZ(qCos(m_phi + M_PI/2));
  m_leftVector.normalize();

  m_upVector = QVector3D::crossProduct(m_frontVector, m_leftVector);
  m_upVector.normalize();
}

void Freefly::moveLeft(float t)
{
  m_position += t * m_leftVector;
}

void Freefly::moveFront(float t)
{
  m_position += t * m_frontVector;
}

void Freefly::rotateLeft(float degrees)
{
  m_phi+= qDegreesToRadians(degrees);
  computeDirectionVectors();
}

void Freefly::rotateUp(float degrees)
{
  m_theta += qDegreesToRadians(degrees);
  computeDirectionVectors();
}
