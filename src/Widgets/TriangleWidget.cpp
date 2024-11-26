#include "TriangleWidget.h"

TriangleWidget::TriangleWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_program(nullptr),
      m_displayListId(0)
{
}

TriangleWidget::~TriangleWidget()
{
    makeCurrent();
    delete m_program;
    if(m_displayListId != 0)
    {
        glDeleteLists(m_displayListId, 1);
    }
    doneCurrent();
}

void TriangleWidget::initializeGL()
{
    initializeOpenGLFunctions();

    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "../shaders/triangle/triangle.vs.glsl");
    m_program->addShaderFromSourceFile(QOpenGLShader::Geometry, "../shaders/triangle/triangle.gs.glsl");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "../shaders/triangle/triangle.fs.glsl");
    m_program->link();

    createList();
}

void TriangleWidget::resizeGL(int w, int h)
{
    h = (h == 0) ? 1 : h;
    glViewport(0, 0, w, h);
}

void TriangleWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program->bind();

    glCallList(m_displayListId);

    m_program->release();

    update();
}

void TriangleWidget::createList()
{
    m_displayListId = glGenLists(1);
    glNewList(m_displayListId, GL_COMPILE);

    glBegin(GL_TRIANGLES);

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(-1.0f, -1.0f);

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2f(0.0f, 1.0f);

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex2f(1.0f, -1.0f);

    glEnd();

    glEndList();
}