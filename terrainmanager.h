#ifndef TERRAINMANAGER_H
#define TERRAINMANAGER_H

#include "chunk.h"
#include <vector>
#include <QVector3D>
#include <QOpenGLShader>
#include <QOpenGLExtraFunctions>

class terrainmanager{
public:
    terrainmanager();

    void init(int gridSize, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLVertexArrayObject* lineQuadVao, QOpenGLBuffer* lineQuadVbo, QOpenGLExtraFunctions *glFuncs);
    void update(const QVector3D& cameraPos, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLExtraFunctions *glFuncs);
    void render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLExtraFunctions *glFuncs);

private:
    std::vector<std::vector<chunk>> m_chunks;
    int m_gridSize;

    QOpenGLVertexArrayObject* m_lineQuadVaoRef;
    QOpenGLBuffer* m_lineQuadVboRef;

    const float LOD_DISTANCE_THRESHOLD = CHUNK_SIZE * 2.5f;
};

#endif // TERRAINMANAGER_H
