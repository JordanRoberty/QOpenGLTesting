#include "Testgltf.h"
#include <iostream>

Testgltf::Testgltf(QWidget *parent) : QOpenGLWidget(parent), m_gltfLoader(this), m_cameraType(CameraType::TRACKBALL), m_nearPlane(0.01f), m_farPlane(10000.0f)
{
}

Testgltf::~Testgltf()
{
    makeCurrent();
    delete m_shaderProgram;
    m_gltfLoader.cleanUp();
    doneCurrent();
}

// ------------------------------------------------------ Event ------------------------------------------------------

/*C to switch camera, P to write each colorTexture on Debug, M to enable/disable depth peeling*/
void Testgltf::keyPressEvent(QKeyEvent *event)
{
  if(CameraType::TRACKBALL == m_cameraType)
  {
    switch (event->key()) 
    {
      case Qt::Key_Up:
          m_trackBall.rotateUp(1);
          break;
      case Qt::Key_Down:
          m_trackBall.rotateUp(-1);
          break;
      case Qt::Key_Left:
          m_trackBall.rotateLeft(-1);
          break;
      case Qt::Key_Right:
          m_trackBall.rotateLeft(1);
          break;
      case Qt::Key_Z:
          m_trackBall.moveFront(0.1);
          break;
      case Qt::Key_S:
          m_trackBall.moveFront(-0.1);
          break;
      case Qt::Key_Escape:
          close();
          break;
      default:
          break;
    }
  }
  else
  {
    switch (event->key()) 
    {
      case Qt::Key_Z:
          m_freefly.moveFront(0.1);
          break;
      case Qt::Key_S:
          m_freefly.moveFront(-0.1);
          break;
      case Qt::Key_Q:
          m_freefly.moveLeft(0.1);
          break;
      case Qt::Key_D:
          m_freefly.moveLeft(-0.1);
          break;
      case Qt::Key_Escape:
          close();
          break;
      default:
          break;
    }
  }

  if(event->key() == Qt::Key_C)
  {
    switchCamera();
  }


  update();

}

void Testgltf::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
      m_lastMousePosition = QVector2D(event->localPos());
    }
    update();
}

void Testgltf::mouseMoveEvent(QMouseEvent *event)
{
  if(event->buttons() & Qt::LeftButton)
  {
    const QVector2D mousePos(event->localPos());
    const QVector2D diff = mousePos - m_lastMousePosition;
    if(m_cameraType == TRACKBALL)
    {
      m_trackBall.rotateLeft(diff.x());
      m_trackBall.rotateUp(diff.y());
    }
    else
    {
      m_freefly.rotateLeft(-diff.x());
      m_freefly.rotateUp(-diff.y());
    }
    m_lastMousePosition = mousePos;
  }
  update();
}

void Testgltf::wheelEvent(QWheelEvent *event)
{
  const float delta = event->angleDelta().y()/100;
  if(m_cameraType == TRACKBALL)
  {
    m_trackBall.moveFront(delta);
  }
  update();
}



void Testgltf::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "../shaders/gltf/simple.vs.glsl");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "../shaders/gltf/simple.fs.glsl");
    m_shaderProgram->link();

    m_gltfLoader.loadModel("../res/brain/brain.gltf");

    m_viewMatrix = m_cameraType == CameraType::TRACKBALL ? m_trackBall.getViewMatrix() : m_freefly.getViewMatrix();

    const float aspect = static_cast<float>(width()) / static_cast<float>(height());
    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(45.0f, aspect, m_nearPlane, m_farPlane);
}

void Testgltf::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    const float aspect = static_cast<float>(w) / static_cast<float>(h);
    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(45.0f, aspect, m_nearPlane, m_farPlane);
}

void Testgltf::paintGL()
{
    m_viewMatrix = m_cameraType == CameraType::TRACKBALL ? m_trackBall.getViewMatrix() : m_freefly.getViewMatrix();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_gltfLoader.render(m_shaderProgram, m_projectionMatrix, m_viewMatrix);

    //update();
}

void Testgltf::switchCamera()
{
    m_cameraType = m_cameraType == CameraType::TRACKBALL ? CameraType::FREEFLY : CameraType::TRACKBALL;
}