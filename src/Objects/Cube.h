#ifndef CUBE_H
#define CUBE_H

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QVector4D>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector3D>
#include <QOpenGLFunctions>

class Cube 
{
  public:
    Cube(QOpenGLFunctions *glFuncs);
    ~Cube();

    void create();
    void destroy();
    void draw();

    void applyTransformations(); // Apply the transformations to the model matrix

    void setColor(const QVector4D &color){m_color = color;};
    void setRotation(const QQuaternion &rotation){m_rotation = rotation;};
    void setTranslation(const QVector3D &translation){m_translation = translation;};
    void setScale(const QVector3D &scale){m_scale = scale;};

    inline QVector4D getColor() const { return m_color; }
    inline QMatrix4x4 getModelMatrix() const { return m_modelMatrix; }


  private:
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_ebo;
    QOpenGLVertexArrayObject m_vao;

    QQuaternion m_rotation;
    QVector3D m_translation;
    QVector3D m_scale;
    QMatrix4x4 m_modelMatrix;

    QVector4D m_color;

    QOpenGLFunctions *m_glFuncs;


};

#endif // CUBE_H