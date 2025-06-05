#include "terrainmanager.h"
#include <QDebug>

terrainmanager::terrainmanager() : m_gridSize(0), m_lineQuadVaoRef(nullptr), m_lineQuadVboRef(nullptr)
{}

void terrainmanager::init(int gridSize, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLVertexArrayObject* lineQuadVao, QOpenGLBuffer* lineQuadVbo, QOpenGLExtraFunctions *glFuncs) {
    m_gridSize = gridSize;
    m_chunks.resize(m_gridSize);
    m_lineQuadVaoRef = lineQuadVao;
    m_lineQuadVboRef = lineQuadVbo;

    for (int i = 0; i < m_gridSize; ++i) {
        m_chunks[i].resize(m_gridSize);
        for (int j = 0; j < m_gridSize; ++j) {
            int chunkX = i - m_gridSize / 2;
            int chunkZ = j - m_gridSize / 2;
            m_chunks[i][j].init(chunkX, chunkZ, terrainShaderProgram, glFuncs);
        }
    }
}

void terrainmanager::update(const QVector3D& cameraPos, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLExtraFunctions *glFuncs) {
    for (int i = 0; i < m_gridSize; ++i) {
        for (int j = 0; j < m_gridSize; ++j) {
            chunk& chunk = m_chunks[i][j];
            float distanceToChunk = cameraPos.distanceToPoint(chunk.getCenterPosition());
            int desiredLOD = (distanceToChunk > LOD_DISTANCE_THRESHOLD) ? 1 : 0;
            if (chunk.getLOD() != desiredLOD) {
                int oldRes = (chunk.getLOD() == 0) ? chunk::HIGH_RES : chunk::LOW_RES;
                chunk.setLOD(desiredLOD);
                int newRes = (chunk.getLOD() == 0) ? chunk::HIGH_RES : chunk::LOW_RES;
                if(oldRes != newRes) {
                    // qDebug() << "Chunk" << i << j << "changed LOD to" << desiredLOD << "res" << newRes << "dist" << distanceToChunk;
                    chunk.generateMesh(newRes, terrainShaderProgram, glFuncs);
                }
            }
        }
    }
}

void terrainmanager::render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLExtraFunctions *glFuncs) {

    for (int i = 0; i < m_gridSize; ++i) {
        for (int j = 0; j < m_gridSize; ++j) {
            m_chunks[i][j].render(terrainShaderProgram, glFuncs);
        }
    }

    lineShaderProgram->bind();
    for (int i = 0; i < m_gridSize; ++i) {
        for (int j = 0; j < m_gridSize; ++j) {
            m_chunks[i][j].renderBorders(lineShaderProgram, glFuncs, m_lineQuadVaoRef, m_lineQuadVboRef);
        }
    }
}
