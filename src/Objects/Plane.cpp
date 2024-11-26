#include "Plane.h"

Plane::Plane(QOpenGLFunctions *glFuncs) : m_vbo(QOpenGLBuffer::VertexBuffer), m_ebo(QOpenGLBuffer::IndexBuffer), m_color(1.0f, 1.0f, 1.0f, 1.0f), m_rotation(), m_translation(), m_scale(1.0f, 1.0f, 1.0f), m_modelMatrix(), m_glFuncs(glFuncs)
{
  create();
}

Plane::~Plane()
{
  destroy();
}


void Plane::create()
{
  const GLuint POSITION_ATTRIBUTE = 0;

  // -- set up geometry --
  GLfloat vertices[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f
  };

  // -- set up indices --
  GLuint indices[]= {
    0, 1, 2,
    0, 2, 3
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

void Plane::destroy()
{
  if(m_vao.isCreated())
  {
    m_vao.release();
    m_vao.destroy();
  }

  if(m_vbo.isCreated())
  {
    m_vbo.release();
    m_vbo.destroy();
  }

  if(m_ebo.isCreated())
  {
    m_ebo.release();
    m_ebo.destroy();
  }
}

void Plane::draw()
{
  m_vao.bind();
  m_glFuncs->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  m_vao.release();
}

void Plane::applyTransformations()
{
  m_modelMatrix.setToIdentity();
  m_modelMatrix.translate(m_translation);
  m_modelMatrix.rotate(m_rotation);
  m_modelMatrix.scale(m_scale);
}