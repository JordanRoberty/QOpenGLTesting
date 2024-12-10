#ifndef SHADERWIDGET_H
#define SHADERWIDGET_H

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

#include <QElapsedTimer>
#include <QTimer>
#include <QThread>

#include "../Cameras/TrackBall.h"
#include "../Cameras/Freefly.h"
#include "../Widgets/CameraType.h"
#include "../Utilitaire/DynamicShaderBuilder.h"

class ShaderWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

  public:
    ShaderWidget(QWidget *parent = nullptr);
    ~ShaderWidget();
    QOpenGLFunctions *getOpenGLFunctions();
  
  protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
  
  private:
  // -- initialize functions --
  void createFullScreenQuad();
  void initShaders();
  void initTextures();
  void initFramebuffers();
  void createObjects();
  void createSphere(const QVector3D &center, float radius, int slices, int stacks, const QVector4D &color);

  // -- Drawing functions --
  void drawFullScreenQuad();
  void renderScene(QOpenGLShaderProgram &shaderProgram);
  void depthPeeling();

  // -- Uniforms functions --
  void setSceneUniforms(QOpenGLShaderProgram &program);
  void setBlinnPhongUniforms(QOpenGLShaderProgram &program);
  void setDepthPeelingUniforms(QOpenGLShaderProgram &program, const int layer);

  // -- Depth peeling functions --
  void initDepthPeeling();
  void depthPeelingPass();
  void blendPass();

  // -- Clean-up functions --
  void cleanUp();
  void cleanUpObjects();
  void cleanUpTextures();
  void cleanUpShaders();
  void cleanUpFramebuffers();

  // -- Utility functions --
  void switchCamera();
  void TexToPng();

  // -- Variables --
  struct Material {
    QVector4D ambient;
    QVector3D diffuse;
    QVector3D specular;
    float shininess;
  };

  struct Light {
    QVector3D direction;
    QVector3D ambient;
    QVector3D diffuse;
    QVector3D specular;
    float intensity;
  };

  Material m_material;
  Light m_light;
  int m_useDepthPeeling;

  // -- Camera --
  TrackBall m_trackBall;
  Freefly m_freefly;
  float m_nearPlane;
  float m_farPlane;
  QVector2D m_lastMousePosition;
  int m_cameraType;

  // -- Shaders --
  QOpenGLShaderProgram m_mainProgram;
  QOpenGLShaderProgram m_blendProgram;

  // -- Objects --
  std::vector<GLuint> m_displayListsSphere;
  GLuint m_displayListScene;
  GLuint m_fullScreenQuadList;

  // -- FBOs --
  std::vector<QOpenGLFramebufferObject *> m_peelingFbo;

  // -- Textures --
  std::vector<QOpenGLTexture *> m_colorTextures;
  std::vector<QOpenGLTexture *> m_depthTextures;

  // -- Transformation Matrix --
  QMatrix4x4 m_viewMatrix;
  QMatrix4x4 m_projectionMatrix;

  // -- Frame count --
  QElapsedTimer m_fpsTimer;
  int m_frameCount = 0;
  qreal m_fps = 0.0;
  QTimer *m_displayTimer;

  // -- Variables --
  int m_maxLayers;
  int m_viewportWidth;
  int m_viewportHeight;
  int m_vertexCount;
  std::string m_displayType;


  DynamicShaderBuilder m_shaderBuilder;

  private slots:
    void updateFPSDisplay();
};

#endif // SHADERWIDGET_H