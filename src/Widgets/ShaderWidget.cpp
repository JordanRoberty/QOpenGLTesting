#include "ShaderWidget.h"
#include <iostream>
#include <fstream>
#include "../Utilitaire/BlinnPhongIlluminationModel.h"
#include "../Utilitaire/DepthPeelingEffect.h"

// ------------------------------------------------------ Constructor ------------------------------------------------------
ShaderWidget::ShaderWidget(QWidget *parent):
  QOpenGLWidget(parent),
  m_maxLayers(6),
  m_viewportWidth(width()),
  m_viewportHeight(height()),
  m_nearPlane(0.01f),
  m_farPlane(100.0f),
  m_cameraType(TRACKBALL),
  m_useDepthPeeling(1)
{
  // -- init light --
  m_light.direction = QVector3D(0.0f, -1.0f, -2.0f);
  m_light.ambient = QVector3D(1.0f, 1.0f, 1.0f);
  m_light.diffuse = QVector3D(0.5f, 0.5f, 0.5f);
  m_light.specular = QVector3D(1.0f, 1.0f, 1.0f);
  m_light.intensity = 1.0f;

  // -- init materials --
  //m_material.ambient = QVector4D(1.0f, 1.0f, 0.0f, 0.5f); Using the shape's color
  m_material.diffuse = QVector3D(0.8f, 0.6f, 0.0f);
  m_material.specular = QVector3D(1.0f, 1.0f, 1.0f);
  m_material.shininess = 32.0f;

  // -- init timers --
  m_fpsTimer.start();
  m_displayTimer = new QTimer(this);
  connect(m_displayTimer, &QTimer::timeout, this, &ShaderWidget::updateFPSDisplay);
  m_displayTimer->start(1000);

  m_displayType = m_useDepthPeeling ? "Depth Peeling" : "Direct";
}

ShaderWidget::~ShaderWidget()
{
    makeCurrent();
    cleanUp();
    doneCurrent();
}

// ------------------------------------------------------ Event ------------------------------------------------------

/*C to switch camera, P to write each colorTexture on Debug, M to enable/disable depth peeling*/
void ShaderWidget::keyPressEvent(QKeyEvent *event)
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
  else if (event->key() == Qt::Key_P)
  {
    TexToPng();
  }
  else if(event->key() == Qt::Key_M)
  {
    m_useDepthPeeling = !m_useDepthPeeling;
    m_displayType = m_useDepthPeeling ? "Depth Peeling" : "Direct";
    cleanUpShaders();
    initShaders();
  }

  update();
}

void ShaderWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
      m_lastMousePosition = QVector2D(event->localPos());
    }
    update();
}

void ShaderWidget::mouseMoveEvent(QMouseEvent *event)
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

void ShaderWidget::wheelEvent(QWheelEvent *event)
{
  const float delta = event->angleDelta().y()/100;
  if(m_cameraType == TRACKBALL)
  {
    m_trackBall.moveFront(delta);
  }
  update();
}

QOpenGLFunctions* ShaderWidget::getOpenGLFunctions(){
  return this;
 }

// ------------------------------------------------------ QOpenGLWidget functions ------------------------------------------------------

void ShaderWidget::initializeGL()
{
  initializeOpenGLFunctions();
  glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

  initShaders();
  createObjects();
  createFullScreenQuad();

  // depth peeling
  initTextures();
  initFramebuffers();

  m_viewMatrix = m_cameraType == TRACKBALL ? m_trackBall.getViewMatrix() : m_freefly.getViewMatrix();

  const float aspect = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);
  m_projectionMatrix.setToIdentity();
  m_projectionMatrix.perspective(45.0f, aspect, m_nearPlane, m_farPlane);
}

void ShaderWidget::resizeGL(int w, int h)
{
  m_viewportWidth = w;
  m_viewportHeight = h;
  glViewport(0, 0, w, h);

  initTextures();
  initFramebuffers();

  const float aspect = static_cast<float>(w) / static_cast<float>(h);
  m_projectionMatrix.setToIdentity();
  m_projectionMatrix.perspective(45.0f, aspect, m_nearPlane, m_farPlane);
}

void ShaderWidget::paintGL() 
{
  m_viewMatrix = m_cameraType == TRACKBALL ? m_trackBall.getViewMatrix() : m_freefly.getViewMatrix();

  glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if(m_useDepthPeeling)
  {
    depthPeeling();
  }
  else
  {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_mainProgram.bind();
    setBlinnPhongUniforms(m_mainProgram);
    setSceneUniforms(m_mainProgram);
    renderScene(m_mainProgram);
    m_mainProgram.release();

    glDisable(GL_BLEND);
  }

  m_frameCount++;
  update();
}

// ------------------------------------------------------ Initialize functions ------------------------------------------------------

void ShaderWidget::createObjects()
{
  createSphere(QVector3D(0.2, -0.2, -0.2), 0.5f, 32, 16, QVector4D(0.0, 1.0, 0.0, 0.5));
  //createSphere(QVector3D(0.0, 0.0, 0.0), 0.5f, 32, 16, QVector4D(0.0, 0.0, 1.0, 0.5));
  createSphere(QVector3D(0.2, 0.0, 0.2), 0.5f, 32, 16, QVector4D(1.0, 0.0, 0.0, 0.5));

  m_displayListScene = glGenLists(1);
  glNewList(m_displayListScene, GL_COMPILE);
  {
    for(auto &sphere : m_displayListsSphere)
    {
      glCallList(sphere);
    }
  }
  glEndList();
}

void ShaderWidget::createSphere(const QVector3D &center, float radius, int slices, int stacks, const QVector4D &color)
{
  GLuint sphereList = glGenLists(1);    
  glNewList(sphereList, GL_COMPILE);
    
  glPushMatrix();    

  glLoadIdentity();
  glTranslatef(center.x(), center.y(), center.z());
  for (int i = 0; i < stacks; ++i) {
    float phi1 = M_PI * static_cast<float>(i) / stacks;
    float phi2 = M_PI * static_cast<float>(i + 1) / stacks;
        
    glBegin(GL_TRIANGLE_STRIP);
  glColor4f(color.x(), color.y(), color.z(), color.w());    
        
    for (int j = 0; j <= slices; ++j) {
      float theta = 2.0f * M_PI * static_cast<float>(j) / slices;
            
      float x1 = radius * std::sin(phi1) * std::cos(theta);
      float y1 = radius * std::sin(phi1) * std::sin(theta);
      float z1 = radius * std::cos(phi1);
      glNormal3f(x1 / radius, y1 / radius, z1 / radius);
      glVertex3f(x1, y1, z1);
      
      float x2 = radius * std::sin(phi2) * std::cos(theta);
      float y2 = radius * std::sin(phi2) * std::sin(theta);
      float z2 = radius * std::cos(phi2);
      glNormal3f(x2 / radius, y2 / radius, z2 / radius);
      glVertex3f(x2, y2, z2);
    }
  
    glEnd();
  }
    
  glPopMatrix();    
  glEndList();

  // GLuint sphereList = glGenLists(1);
  // glNewList(sphereList, GL_COMPILE);

  // glPushMatrix();
  // glLoadIdentity();
  // glTranslatef(center.x(), center.y(), center.z());
  // glColor4f(color.x(), color.y(), color.z(), color.w());
  // glBegin(GL_TRIANGLES);

  // glVertex3f(-0.5f, -0.5f, 0.0f);
  // glVertex3f(-0.5f, 0.5f, 0.0f);
  // glVertex3f(0.5f, 0.5f, 0.0f);

  // glVertex3f(-0.5f, -0.5f, 0.0f);
  // glVertex3f(0.5f, 0.5f, 0.0f);
  // glVertex3f(0.5f, -0.5f, 0.0f);

  // glEnd();
  // glPopMatrix();
  // glEndList();


   m_displayListsSphere.push_back(sphereList);
}

void ShaderWidget::createFullScreenQuad()
{
  m_fullScreenQuadList= glGenLists(1);

  glNewList(m_fullScreenQuadList, GL_COMPILE);
  {
    glBegin(GL_TRIANGLES);

    // front face
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);

    glEnd();
  }
  glEndList();
}

void ShaderWidget::initShaders()
{
  m_mainProgram.create();
  DynamicShaderBuilder shaderBuilder;

  shaderBuilder.setVersion(330);
  std::string baseTemplate = shaderBuilder.readShaderFile("../shaders/dynamic/main.vs.glsl");
  shaderBuilder.setBaseTemplate(baseTemplate);
  std::string vertexSource = shaderBuilder.generateShaderSource().c_str();
  m_mainProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, QString::fromStdString(vertexSource));

  baseTemplate = shaderBuilder.readShaderFile("../shaders/dynamic/main.fs.glsl");
  shaderBuilder.setBaseTemplate(baseTemplate);
 
  shaderBuilder.setIlluminationModel(std::make_unique<BlinnPhongIlluminationModel>());

  if(m_useDepthPeeling)
  {
      shaderBuilder.addEffect(std::make_unique<DepthPeelingEffect>());
      m_blendProgram.create();
      DynamicShaderBuilder blendShaderBuilder;
      blendShaderBuilder.setVersion(330);
      std::string blendBaseTemplate = blendShaderBuilder.readShaderFile("../shaders/dynamic/blend.vs.glsl");
      blendShaderBuilder.setBaseTemplate(blendBaseTemplate);
      std::string blendVertexSource = blendShaderBuilder.generateShaderSource().c_str();
      m_blendProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, QString::fromStdString(blendVertexSource));

      blendBaseTemplate = blendShaderBuilder.readShaderFile("../shaders/dynamic/blend.fs.glsl");
      blendShaderBuilder.setBaseTemplate(blendBaseTemplate);
      std::string blendFragmentSource = blendShaderBuilder.generateShaderSource().c_str();
      m_blendProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, QString::fromStdString(blendFragmentSource));
      m_blendProgram.link();
  }

  std::string fragmentSource = shaderBuilder.generateShaderSource().c_str();
  m_mainProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, QString::fromStdString(fragmentSource));
  m_mainProgram.link();


  // --- DEBUG ---
  // std::ofstream shaderFile;
  // shaderFile.open("../Debug/shader.fs.glsl");

  // if (shaderFile.is_open())
  // {
  //   shaderFile << fragmentSource;
  //   shaderFile.close();
  // }
  // else
  // {
  //   std::cout << "Unable to open file" << std::endl;
  // }

  // --- DEBUG ---


}

void ShaderWidget::initTextures()
{
  m_depthTextures.clear();
  m_colorTextures.clear();
  m_depthTextures.reserve(m_maxLayers);
  m_colorTextures.reserve(m_maxLayers);
  for(int i = 0; i<m_maxLayers+1; ++i)
  {
    //Depth texture
    QOpenGLTexture *depthTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    depthTexture->create();
    depthTexture->setSize(m_viewportWidth, m_viewportHeight);
    depthTexture->setFormat(QOpenGLTexture::D32F);
    depthTexture->allocateStorage();
    depthTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    depthTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
    depthTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_depthTextures.push_back(depthTexture);

    //Color texture
    QOpenGLTexture *colorTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    colorTexture->create();
    colorTexture->setSize(m_viewportWidth, m_viewportHeight);
    colorTexture->setFormat(QOpenGLTexture::RGBA32F);
    colorTexture->allocateStorage();
    colorTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    colorTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
    colorTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_colorTextures.push_back(colorTexture);
  }
}

void ShaderWidget::initFramebuffers()
{
  m_peelingFbo.clear();
  m_peelingFbo.reserve(m_maxLayers);
  for(int i = 0; i<m_maxLayers+1; ++i)
  {
    QOpenGLFramebufferObject *fbo = new QOpenGLFramebufferObject(m_viewportWidth, m_viewportHeight);
    if(!fbo->isValid())
    {
      std::cout << "Error: Framebuffer is not valid" << std::endl;
      delete fbo;
      continue;
    }
    fbo->bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTextures[i]->textureId(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextures[i]->textureId(), 0);

     if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      std::cout << "Error: Framebuffer is not complete" << std::endl;
      fbo->release();
      delete fbo;
      continue;
    }
    fbo->release();
    m_peelingFbo.push_back(fbo);
  }
}

// ------------------------------------------------------ Drawing functions ------------------------------------------------------

void ShaderWidget::renderScene(QOpenGLShaderProgram &program)
{
  glCallList(m_displayListScene);
}

void ShaderWidget::drawFullScreenQuad()
{
  glCallList(m_fullScreenQuadList);
}

void ShaderWidget::depthPeeling()
{
  glEnable(GL_DEPTH_TEST);
  initDepthPeeling();
  depthPeelingPass();
  glDisable(GL_DEPTH_TEST);
  blendPass();
}

// ------------------------------------------------------ Uniforms functions ------------------------------------------------------

void ShaderWidget::setSceneUniforms(QOpenGLShaderProgram &program)
{
  program.setUniformValue("u_ViewMatrix", m_viewMatrix);
  program.setUniformValue("u_ProjectionMatrix", m_projectionMatrix);
  program.setUniformValue("u_hasTexture", false);
  program.setUniformValue("u_textureType", 4);
}

void ShaderWidget::setDepthPeelingUniforms(QOpenGLShaderProgram &program, const int layer)
{
  program.setUniformValue("u_layer", layer);  
}

void ShaderWidget::setBlinnPhongUniforms(QOpenGLShaderProgram &program)
{
  program.setUniformValue("u_lightDirection", m_light.direction);
  program.setUniformValue("u_lightAmbient", m_light.ambient);
  program.setUniformValue("u_lightDiffuse", m_light.diffuse);
  program.setUniformValue("u_lightSpecular", m_light.specular);
  program.setUniformValue("u_lightIntensity", m_light.intensity);
  
  //program.setUniformValue("u_materialAmbient", m_material.ambient); // unused because we use the color of the object
  program.setUniformValue("u_materialDiffuse", m_material.diffuse);
  program.setUniformValue("u_materialSpecular", m_material.specular);
  program.setUniformValue("u_materialShininess", m_material.shininess);

  QVector3D cameraPos;
  if (m_cameraType == TRACKBALL) {
    cameraPos = m_trackBall.getPosition();
  } else {
    cameraPos = m_freefly.getPosition();
  }
  program.setUniformValue("u_viewPosition", cameraPos);
}

// ------------------------------------------------------ Depth Peeling functions ------------------------------------------------------

void ShaderWidget::initDepthPeeling()
{
  m_peelingFbo[0]->bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_mainProgram.bind();
  setBlinnPhongUniforms(m_mainProgram);
  setDepthPeelingUniforms(m_mainProgram, 0);
  setSceneUniforms(m_mainProgram);
  renderScene(m_mainProgram);
  m_mainProgram.release();
  m_peelingFbo[0]->release();
}

void ShaderWidget::depthPeelingPass()
{
  m_mainProgram.bind();
  for(int i = 1; i<m_maxLayers; ++i)
  {
    m_peelingFbo[i]->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    m_depthTextures[i-1]->bind();
    m_mainProgram.setUniformValue("u_previousDepthTexture", 0);

    setBlinnPhongUniforms(m_mainProgram);
    setDepthPeelingUniforms(m_mainProgram, i);
    setSceneUniforms(m_mainProgram);
    renderScene(m_mainProgram);

    m_peelingFbo[i]->release();
  }

  m_mainProgram.release();
}

void ShaderWidget::blendPass()
{
  glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  m_blendProgram.bind();

  for(int i=0; i<m_maxLayers; ++i)
  {
    glActiveTexture(GL_TEXTURE0+i);
    m_colorTextures[i]->bind();
    m_blendProgram.setUniformValue(QString("u_layerTexture[%1]").arg(i).toStdString().c_str(), i);
  }

  m_blendProgram.setUniformValue("u_numLayers", m_maxLayers);

  drawFullScreenQuad();

  m_blendProgram.release();

  glDisable(GL_BLEND);
}

// ------------------------------------------------------ Clean-up functions ------------------------------------------------------

void ShaderWidget::cleanUp()
{
  cleanUpObjects();
  cleanUpTextures();
  cleanUpShaders();
  cleanUpFramebuffers();
}

void ShaderWidget::cleanUpObjects()
{
  for(auto sphere : m_displayListsSphere)
  {
    glDeleteLists(sphere, 1);
  }
  glDeleteLists(m_displayListScene, 1);
}

void ShaderWidget::cleanUpTextures()
{
  for(auto texture : m_depthTextures)
  {
    delete texture;
  }
  for(auto texture : m_colorTextures)
  {
    delete texture;
  }
}

void ShaderWidget::cleanUpShaders()
{
  if(m_mainProgram.isLinked())
  {
    m_mainProgram.removeAllShaders();
    m_mainProgram.release();
  }

  if(m_blendProgram.isLinked())
  {
    m_blendProgram.removeAllShaders();
    m_blendProgram.release();
  }
}

void ShaderWidget::cleanUpFramebuffers()
{
  for(auto fbo : m_peelingFbo)
  {
    if(fbo->isBound())
    {
      fbo->release();
    }
    delete fbo;
  }
}

// ------------------------------------------------------ Utility functions ------------------------------------------------------

void ShaderWidget::switchCamera()
{
  m_cameraType = m_cameraType == TRACKBALL ? FREEFLY : TRACKBALL;
}

void ShaderWidget::TexToPng()
{
  int width = m_viewportWidth;
  int height = m_viewportHeight;
  for(int i=0; i<m_maxLayers; ++i)
  {
    GLuint textureID = m_colorTextures[i]->textureId();

    std::vector<GLubyte> pixels(4 * width * height);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    QImage image(reinterpret_cast<const uchar*>(pixels.data()), width, height, QImage::Format_RGBA8888);
    image = image.mirrored();

    QString filename = QString("../Debug/texture_output_%1.png").arg(i);
    image.save(filename, "PNG");
  }
}

void ShaderWidget::updateFPSDisplay()
{
  qint64 elapsed = m_fpsTimer.elapsed();
  m_fps = m_frameCount * 1000.0 / elapsed;

  setWindowTitle(QString("OpenGL - FPS: %1 in %2").arg(m_fps, 0, 'f', 1).arg(QString::fromStdString(m_displayType)));

  m_frameCount = 0;
  m_fpsTimer.restart();
}