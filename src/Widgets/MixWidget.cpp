#include "MixWidget.h"
#include <iostream>

// ------------------------------------------------------ Constructor ------------------------------------------------------

MixWidget::MixWidget(QWidget *parent) : 
                    QOpenGLWidget(parent),
                    m_maxLayers(16),
                    m_viewportWidth(width()),
                    m_viewportHeight(height()),
                    m_nearPlane(0.01f),
                    m_farPlane(10000.0f),
                    m_cameraType(TRACKBALL),
                    m_useDepthPeeling(1),
                    m_gltfLoader(this)
{
  // -- init light --
    m_light.direction = QVector3D(0.0f, -1.0f, -2.0f);
    m_light.ambient = QVector3D(1.0f, 1.0f, 1.0f);
    m_light.diffuse = QVector3D(0.5f, 0.5f, 0.5f);
    m_light.specular = QVector3D(1.0f, 1.0f, 1.0f);
    m_light.intensity = 1.0f;

  // -- init materials --
    //m_material.ambient = QVector4D(1.0f, 1.0f, 0.0f, 0.5f); Using the shape's color
    m_material.diffuse = QVector3D(0.f, 0.0f, 0.0f);
    m_material.specular = QVector3D(1.0f, 1.0f, 1.0f);
    m_material.shininess = 32.0f;

    m_fpsTimer.start();
    m_displayTimer = new QTimer(this);
    connect(m_displayTimer, &QTimer::timeout, this, &MixWidget::updateFPSDisplay);
    m_displayTimer->start(1000);
}

MixWidget::~MixWidget()
{
    makeCurrent();
    cleanUp();
    m_gltfLoader.cleanUp();
    doneCurrent();
}

// ------------------------------------------------------ Event ------------------------------------------------------

/*C to switch camera, P to write each colorTexture on Debug, M to enable/disable depth peeling*/
void MixWidget::keyPressEvent(QKeyEvent *event)
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
  }

  update();
}

void MixWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
      m_lastMousePosition = QVector2D(event->localPos());
    }
    update();
}

void MixWidget::mouseMoveEvent(QMouseEvent *event)
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

void MixWidget::wheelEvent(QWheelEvent *event)
{
  const float delta = event->angleDelta().y()/100;
  if(m_cameraType == TRACKBALL)
  {
    m_trackBall.moveFront(delta);
  }
  update();
}

// ------------------------------------------------------ QOpenGLWidget functions ------------------------------------------------------

void MixWidget::initializeGL()
{
  initializeOpenGLFunctions();
  glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

  // scene
  initShaders();
  m_gltfLoader.loadModel("../res/brain/brain.gltf");
  createFullScreenQuad();

  // depth peeling
  initTextures();
  initFramebuffers();

  m_viewMatrix = m_cameraType == TRACKBALL ? m_trackBall.getViewMatrix() : m_freefly.getViewMatrix();

  const float aspect = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);
  m_projectionMatrix.setToIdentity();
  m_projectionMatrix.perspective(45.0f, aspect, m_nearPlane, m_farPlane);
}

void MixWidget::resizeGL(int w, int h)
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

void MixWidget::paintGL() 
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
    m_mainProgram.bind();
    setDepthPeelingUniforms(m_mainProgram, 0);
    renderGLTF(m_mainProgram);
    m_mainProgram.release();
  }

  m_frameCount++;
  update();
}

// ------------------------------------------------------ Initialize functions ------------------------------------------------------


void MixWidget::createFullScreenQuad()
{
  GLuint displayListId = glGenLists(1);

  glNewList(displayListId, GL_COMPILE);
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

  m_fullScreenQuadList = displayListId;
}

void MixWidget::initShaders()
{
  ShaderManager manager("../shaders/Mix");

  // -- Blinn-Phong + Depth Peeling shaders --
  manager.loadModule("main.vs.glsl");
  QString vertexSource = manager.buildShader("main.vs.glsl");
  if(!m_mainProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSource))
  {
    std::cout << "Main vertex shader error : " << m_mainProgram.log().toStdString() << std::endl;
  }
  
  manager.loadModule("main.fs.glsl");
  QString shaderSource = manager.buildShader("main.fs.glsl");
  if(!m_mainProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, shaderSource))
  {
    std::cout << "Main fragment shader error : " << m_mainProgram.log().toStdString() << std::endl;
  }

  if(!m_mainProgram.link())
  { 
    std::cout << "Main link shader error : " << m_mainProgram.log().toStdString() << std::endl;
  }

  // -- Depth Peeling shaders --
  //manager.loadModule("peeling.vs.glsl");
  //vertexSource = manager.buildShader("peeling.vs.glsl");
  if(!m_depthProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSource)) // same vertex shader as the main program
  {
    std::cout << "Peeling vertex shader error : " << m_depthProgram.log().toStdString() << std::endl;
  }

  manager.loadModule("peeling.fs.glsl");
  shaderSource = manager.buildShader("peeling.fs.glsl");
  if(!m_depthProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, shaderSource))
  {
    std::cout << "Peeling fragment shader error : " << m_depthProgram.log().toStdString() << std::endl;
  }

  if(!m_depthProgram.link())
  {
    std::cout << "Peeling link shader error : " << m_depthProgram.log().toStdString() << std::endl;
  }

  // -- Blending shaders --

  manager.loadModule("blend.vs.glsl");
  vertexSource = manager.buildShader("blend.vs.glsl");
  if(!m_blendProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSource))
  {
    std::cout << "Blend vertex shader error : " << m_blendProgram.log().toStdString() << std::endl;
  }

  manager.loadModule("blend.fs.glsl");
  shaderSource = manager.buildShader("blend.fs.glsl");
  if(!m_blendProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, shaderSource))
  {
    std::cout << "Blend fragment shader error : " << m_blendProgram.log().toStdString() << std::endl;
  }

  if(!m_blendProgram.link())
  {
    std::cout << "Blend link shader error : " << m_blendProgram.log().toStdString() << std::endl;
  }

}

void MixWidget::initTextures()
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

void MixWidget::initFramebuffers()
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

void MixWidget::renderGLTF(QOpenGLShaderProgram &shaderProgram)
{
  if(m_gltfLoader.m_meshes.empty())
  {
    return;
  }

  shaderProgram.setUniformValue("u_projection", m_projectionMatrix);
  shaderProgram.setUniformValue("u_view", m_viewMatrix);

  for(const auto& mesh : m_gltfLoader.m_meshes)
  {
    if(!mesh.textureInfos.empty())
    {
      shaderProgram.setUniformValue("u_hasTexture", true);
     
      for(size_t i = 0; i < mesh.textureInfos.size(); i++)
      {
        const auto& textureInfo = mesh.textureInfos[i];
        if(textureInfo.type == GLTFLoader::TextureType::Texture1D)
        { 
          shaderProgram.setUniformValue("u_textureType", 0);
          glActiveTexture(GL_TEXTURE0);
          textureInfo.texture->bind();
          shaderProgram.setUniformValue("u_texture1D", 0);


          glActiveTexture(GL_TEXTURE1);
          glBindTexture(GL_TEXTURE_2D, 0);
          shaderProgram.setUniformValue("u_texture2D", 1);
        }
        else
        {
          shaderProgram.setUniformValue("u_textureType", 1);
          glActiveTexture(GL_TEXTURE0);
          textureInfo.texture->bind();
          shaderProgram.setUniformValue("u_texture2D", 0);

          glActiveTexture(GL_TEXTURE1);
          glBindTexture(GL_TEXTURE_1D, 0);
          shaderProgram.setUniformValue("u_texture1D", 1); 
        }
      }
    }
    else
    {
      shaderProgram.setUniformValue("u_hasTexture", false);
    }

    shaderProgram.setUniformValue("u_model", mesh.modelMatrix);

                
    glCallList(mesh.displayListId);

    if(!mesh.textureInfos.empty())
    {
      for(size_t i = 0; i < mesh.textureInfos.size(); i++)
      {
        mesh.textureInfos[i].texture->release();
      }
    }
  }


}


void MixWidget::drawFullScreenQuad()
{
  glCallList(m_fullScreenQuadList);
}

// Apply the depth peeling algorithm with the Blinn-Phong shading
void MixWidget::depthPeeling()
{
  glEnable(GL_DEPTH_TEST);
  initDepthPeeling();
  depthPeelingPass();
  glDisable(GL_DEPTH_TEST);
  blendPass();
}

// ------------------------------------------------------ Uniforms functions ------------------------------------------------------

// set the specific uniforms in shaders/Mix/main.vs and shaders/Mix/main.fs
void MixWidget::setSceneUniforms(QOpenGLShaderProgram &program)
{
  program.setUniformValue("u_MVMatrix", m_viewMatrix);
  program.setUniformValue("u_ProjectionMatrix", m_projectionMatrix);
}

// set the specific uniforms in shaders/Mix/peeling.frag
void MixWidget::setDepthPeelingUniforms(QOpenGLShaderProgram &program, const int layer)
{
  program.setUniformValue("u_useDepthPeeling", m_useDepthPeeling);
  program.setUniformValue("u_layer", layer);  
}

// set the specific uniforms in shaders/Mix/BlinnPhong.frag
void MixWidget::setBlinnPhongUniforms(QOpenGLShaderProgram &program)
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
  
  // Position de la caméra pour le calcul spéculaire
  QVector3D cameraPos;
  if (m_cameraType == TRACKBALL) {
      cameraPos = m_trackBall.getPosition();
  } else {
      cameraPos = m_freefly.getPosition();
  }
  program.setUniformValue("u_viewPosition", cameraPos);
}
// ------------------------------------------------------ Depth peeling functions ------------------------------------------------------

// Initialize the first depth peeling pass to render the scene in the first framebuffer
void MixWidget::initDepthPeeling()
{
  m_peelingFbo[0]->bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_mainProgram.bind();
  //setSceneUniforms(m_mainProgram);
  setBlinnPhongUniforms(m_mainProgram);
  setDepthPeelingUniforms(m_mainProgram, 0);

  glFinish();

  renderGLTF(m_mainProgram);


  //m_gltfLoader.render(&m_mainProgram, m_projectionMatrix, m_viewMatrix);
  m_mainProgram.release();
  m_peelingFbo[0]->release();
}

// Render the scene in the i-th framebuffer and perform the depth peeling pass
void MixWidget::depthPeelingPass()
{
  m_mainProgram.bind();
  for(int i = 1; i<m_maxLayers; ++i)
  {
    m_peelingFbo[i]->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE3);
    m_depthTextures[i-1]->bind();
    m_mainProgram.setUniformValue("u_previousDepthTexture", 3);


    //setSceneUniforms(m_mainProgram);
    setBlinnPhongUniforms(m_mainProgram);
    setDepthPeelingUniforms(m_mainProgram, i);
        
    renderGLTF(m_mainProgram);




    m_peelingFbo[i]->release();
  }

  m_mainProgram.release();
}

// Blend the color textures of the i-th framebuffer to the default framebuffer
void MixWidget::blendPass()
{
  glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  m_blendProgram.bind();

  for(int i=0; i<m_maxLayers; ++i)
  {
    glActiveTexture(GL_TEXTURE4+i);
    m_colorTextures[i]->bind();
    m_blendProgram.setUniformValue(QString("u_layerTexture[%1]").arg(i).toStdString().c_str(), 4+i);
  }

  m_blendProgram.setUniformValue("u_numLayers", m_maxLayers);
  m_blendProgram.setUniformValue("u_useDepthPeeling", m_useDepthPeeling);

  drawFullScreenQuad();

  m_blendProgram.release();

  glDisable(GL_BLEND);
}


// ------------------------------------------------------ Clean up functions ------------------------------------------------------

// Call the clean up functions to delete the objects, textures, shaders and framebuffers
void MixWidget::cleanUp()
{
  cleanupObjects();
  cleanupTextures();
  cleanupShaders();
  cleanupFramebuffers();
}

void MixWidget::cleanupObjects()
{
  glDeleteLists(m_fullScreenQuadList, 1);
}

void MixWidget::cleanupTextures()
{
  for(auto texture : m_depthTextures)
  {
    if(texture->isCreated())
    {
      texture->destroy();
      delete texture;
    }
  }

  for(auto texture : m_colorTextures)
  {
    if(texture->isCreated())
    {
      texture->destroy();
      delete texture;
    }
  }
}

void MixWidget::cleanupShaders()
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

void MixWidget::cleanupFramebuffers()
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

void MixWidget::switchCamera()
{
  m_cameraType = m_cameraType == TRACKBALL ? FREEFLY : TRACKBALL;
}

// Render each color textures in PNG files to debug the depth peeling algorithm
void MixWidget::TexToPng()
{
  for(int i=0; i<m_maxLayers; ++i)
  {
    GLuint textureID = m_colorTextures[i]->textureId();
    int width = m_viewportWidth;
    int height = m_viewportHeight;

    std::vector<GLubyte> pixels(4 * width * height);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    QImage image(reinterpret_cast<const uchar*>(pixels.data()), width, height, QImage::Format_RGBA8888);
    image = image.mirrored();

    QString filename = QString("../Debug/texture_output_%1.png").arg(i);
    image.save(filename, "PNG");
  }

  GLuint textureID = m_gltfLoader.m_meshes[0].textureInfos[0].texture->textureId();
  int width = m_gltfLoader.m_meshes[0].textureInfos[0].texture->width();
  int height = m_gltfLoader.m_meshes[0].textureInfos[0].texture->height();

  std::vector<GLubyte> pixels(4 * width * height);
  glBindTexture(GL_TEXTURE_1D, textureID);
  glGetTexImage(GL_TEXTURE_1D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
  glBindTexture(GL_TEXTURE_1D, 0);

  QImage image(reinterpret_cast<const uchar*>(pixels.data()), width, height, QImage::Format_RGBA8888);
  image = image.mirrored();

  QString filename = QString("../Debug/texture_after.png");
  image.save(filename, "PNG");
}



void MixWidget::updateFPSDisplay()
{
  qint64 elapsed = m_fpsTimer.elapsed();
  m_fps = m_frameCount * 1000.0 / elapsed;
  
  // Display the FPS in the window title
  setWindowTitle(QString("OpenGL - FPS: %1").arg(m_fps, 0, 'f', 1));

  m_frameCount = 0;
  m_fpsTimer.restart();

}
