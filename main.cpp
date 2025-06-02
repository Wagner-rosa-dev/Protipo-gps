#include "myglwidget.h"
#include <QSurfaceFormat>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);

#ifdef QT_OPENGL_ES_3
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setVersion(3, 0);// OpenGL ES 3.0
#else
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3); // OpenGL 3.3 Core (minimo para UBOSs de forma robusta)
#endif
    QSurfaceFormat::setDefaultFormat(format);

    MyGLWidget w;
    w.resize(800, 600);
    w.show();

    return a.exec();
}
