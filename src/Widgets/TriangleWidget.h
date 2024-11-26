#ifndef TRIANGLEWIDGET_H
#define TRIANGLEWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

class TriangleWidget : public QOpenGLWidget, protected QOpenGLFunctions
{

    Q_OBJECT

    public:
        TriangleWidget(QWidget *parent = nullptr);
        ~TriangleWidget();

    protected:
        void initializeGL() override;
        void resizeGL(int w, int h) override;
        void paintGL() override;

    private:
        // -- Display list --
        void createList();

        // -- OpenGL State Information --
        QOpenGLShaderProgram *m_program = nullptr;
        GLuint m_displayListId;
};

#endif // TRIANGLEWIDGET_H