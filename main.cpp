#include "myglwidget.h"
#include <QSurfaceFormat>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setVersion(3, 0);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);


    QSurfaceFormat::setDefaultFormat(format);

    qInfo() << "Configurando QSurfaceFomat para OPENGL ES" << format.majorVersion() << "." << format.minorVersion();
    if (format.renderableType() == QSurfaceFormat::OpenGLES) {
        qInfo() << "Tipo de renderização OpenGLES";
    } else {
        qWarning() << "Tipo de renderização NÂO é OpenGLE. Verifique a Configurção";
    }

    MyGLWidget w;
    w.resize(800, 600);
    w.show();

    return a.exec();
}
