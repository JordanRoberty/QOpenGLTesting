#include "Cube.h"

Cube::Cube(QOpenGLFunctions *glFuncs) : m_vbo(QOpenGLBuffer::VertexBuffer), m_ebo(QOpenGLBuffer::IndexBuffer), m_color(1.0f, 1.0f, 1.0f, 1.0f), m_rotation(), m_translation(), m_scale(1.0f, 1.0f, 1.0f), m_modelMatrix(), m_glFuncs(glFuncs)
{
  create();
}


Cube::~Cube()
{
  destroy();
}

void Cube::create()
{
  const GLuint POSITION_ATTRIBUTE = 0;

  // -- set up geometry --
  GLfloat vertices[] = {
    // front face
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    // back face
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f
  };

  // -- set up indices --
  GLuint indices[]= {
    // front face
    0, 1, 2,
    0, 2, 3,
    // back face
    4, 5, 6,
    4, 6, 7,
    // left face
    0, 4, 7,
    0, 7, 3,
    // right face
    1, 5, 6,
    1, 6, 2,
    // top face
    3, 2, 6,
    3, 6, 7,
    // bottom face
    0, 1, 5,
    0, 5, 4
  };

  m_vbo.create();
  m_vbo.bind();
  m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
  m_vbo.allocate(vertices, sizeof(vertices));
  m_vbo.release();

  m_ebo.create();
  m_ebo.bind();
  m_ebo.setUsagePattern(QOpenGLBuffer::StaticDraw);
  m_ebo.allocate(indices, sizeof(indices));
  m_ebo.release();

  m_vao.create();
  m_vao.bind();
  m_ebo.bind();
  m_glFuncs->glEnableVertexAttribArray(POSITION_ATTRIBUTE);

  m_vbo.bind();
  m_glFuncs->glVertexAttribPointer(POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

  m_vbo.release();
  m_vao.release();
}

void Cube::destroy()
{
  if(m_vbo.isCreated())
  {
    m_vbo.destroy();
  }

  if(m_ebo.isCreated())
  {
    m_ebo.destroy();
  }

  if(m_vao.isCreated())
  {
    m_vao.destroy();
  }
}

void Cube::draw()
{
  m_vao.bind();
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
  m_vao.release();
}


void Cube::applyTransformations()
{
  m_modelMatrix.setToIdentity();
  m_modelMatrix.translate(m_translation);
  m_modelMatrix.rotate(m_rotation);
  m_modelMatrix.scale(m_scale);
}





