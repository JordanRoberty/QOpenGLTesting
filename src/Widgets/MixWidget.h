#ifndef MIXWIDGET_H
#define MIXWIDGET_H

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <vector>

    #include <QElapsedTimer>
    #include <QTimer>
    #include <QThread>


#include "../Cameras/TrackBall.h" 
#include "../Cameras/Freefly.h"
#include "../Widgets/CameraType.h"
#include "../Utilitaire/ShaderManager.h"
#include "../Utilitaire/gltfLoader.h"

class MixWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

  public:
    MixWidget(QWidget *parent = nullptr);
    ~MixWidget();

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
    void createFullScreenQuad();                                                                                                  // Create a full screen quad
    void initShaders();
    void initTextures();
    void initFramebuffers();

    // -- Drawing functions --
    void renderGLTF(QOpenGLShaderProgram &shaderProgram);
    void drawFullScreenQuad();
    void depthPeeling(); // Perform the depth peeling algorithm

    // -- Uniforms functions --
    void setSceneUniforms(QOpenGLShaderProgram &program); // Set the specific uniforms in shaders/Mix/main.vs and shaders/Mix/main.fs
    void setBlinnPhongUniforms(QOpenGLShaderProgram &program);  // Set the specific uniforms in shaders/Mix/blinnPhong.frag
    void setDepthPeelingUniforms(QOpenGLShaderProgram &program, const int layer); // Set the specific uniforms in shaders/Mix/peeling.frag

    // -- Depth Peeling functions --
    void initDepthPeeling(); // Fill the depth peeling FBOs with the scene
    void depthPeelingPass(); // Perform the depth peeling pass
    void blendPass(); // Blend the color of each layer

    // -- Clean up functions --
    void cleanUp();
    void cleanupObjects();
    void cleanupTextures();
    void cleanupShaders();
    void cleanupFramebuffers();

    // -- utility functions --
    void switchCamera();
    void TexToPng();

    // -- Blinn-Phong parameters --
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

    // -- Blinn-Phong variables --
    Material m_material;
    Light m_light;
    int m_useDepthPeeling; // Activate or deactivate the depth peeling algorithm

    // -- Camera --
    TrackBall m_trackBall;
    Freefly m_freefly;
    float m_nearPlane;
    float m_farPlane;
    QVector2D m_lastMousePosition;
    int m_cameraType;

    // -- Shaders --
    QOpenGLShaderProgram m_mainProgram; // perform color computation and depth peeling
    QOpenGLShaderProgram m_blendProgram; // blend the layers

    // -- Objects --
    GLTFLoader m_gltfLoader;
    GLuint m_fullScreenQuadList;
    GLuint m_transparentObjects;

    // -- FBOs --
    std::vector<QOpenGLFramebufferObject *> m_peelingFbo;

    // -- Textures --
    std::vector<QOpenGLTexture *> m_colorTextures;
    std::vector<QOpenGLTexture *> m_depthTextures;

    // -- Transformation matrix --
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;

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

    private slots:
      void updateFPSDisplay();
};

#endif // MIXWIDGET_H