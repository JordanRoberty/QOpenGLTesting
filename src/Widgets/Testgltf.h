#ifndef TESTGLTF_H
#define TESTGLTF_H

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <vector>


#include "../Utilitaire/gltfLoader.h"
#include "../Cameras/TrackBall.h"
#include "../Cameras/Freefly.h"
#include "../Widgets/CameraType.h"

class Testgltf : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

  public:
    Testgltf(QWidget *parent = nullptr);
    ~Testgltf();
  
  protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
  
  private:
    GLTFLoader m_gltfLoader;
    QOpenGLShaderProgram* m_shaderProgram;

    void switchCamera();
    TrackBall m_trackBall;
    Freefly m_freefly;
    float m_nearPlane;
    float m_farPlane;
    QVector2D m_lastMousePosition;
    int m_cameraType;

    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;
};

#endif // TESTGLTF_H