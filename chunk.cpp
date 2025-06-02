#include "chunk.h"
#include "noiseutils.h"
#include <QDebug>

chunk::chunk() :
    m_indexCount(0),
    m_vertexCount(0),
    m_currentResolution(0),
    m_currentLOD(-1)
{}

chunk::~chunk() {

}

void chunk::init(int cX, int cZ, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLExtraFunctions *glFuncs) {
    m_chunkGridX = cX;
    m_chunkGridZ = cZ;
    float worldX = m_chunkGridX * CHUNK_SIZE;
    float worldZ = m_chunkGridZ * CHUNK_SIZE;
    m_modelMatrix.setToIdentity();
    m_modelMatrix.translate(worldX, 0, worldZ);
    setLOD(0);
    generateMesh(m_currentResolution, terrainShaderProgram, glFuncs);
}

void chunk::generateMesh(int resolution, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLExtraFunctions *glFuncs) {
    if (resolution == 0) return;
    m_currentResolution = resolution;

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    float step = static_cast<float>(CHUNK_SIZE) / (resolution - 1);

    for (int r = 0; r < resolution; ++r) {
        for (int c = 0; c < resolution; ++c) {
            Vertex v;
            float localX = c * step;
            float localZ = r * step;
            float worldX = (m_chunkGridX * CHUNK_SIZE) + localX;
            float worldZ = (m_chunkGridZ * CHUNK_SIZE) + localZ;
            v.position = QVector3D(localX, NoiseUtils::getHeight(worldX, worldZ), localZ);
            float hL = NoiseUtils::getHeight(worldX - 0.1f, worldZ);
            float hR = NoiseUtils::getHeight(worldX + 0.1f, worldZ);
            float hD = NoiseUtils::getHeight(worldX, worldZ - 0.1f);
            float hU = NoiseUtils::getHeight(worldX, worldZ + 0.1f);
            v.normal = QVector3D(hL - hR, 2.0f, hD - hU).normalized();
            vertices.push_back(v);
        }
    }

    for (int r = 0; r < resolution - 1; ++r) {
        for (int c = 0; c < resolution - 1; ++c) {
            GLuint topLeft = r * resolution + c;
            GLuint topRight = topLeft - 1;
            GLuint bottomLeft = (r + 1) * resolution + c;
            GLuint bottomRight = bottomLeft + 1;
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    m_indexCount = indices.size();
    m_vertexCount = vertices.size();
    if (m_indexCount == 0 || m_vertexCount == 0) return;

    if (m_vao.isCreated()) m_vao.destroy();
    if (m_vbo.isCreated()) m_vbo.destroy();
    if (m_ebo.isCreated()) m_ebo.destroy();

    m_vao.create();
    m_vao.bind();
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertices.data(), m_vertexCount * sizeof(Vertex));
    m_ebo.create();
    m_ebo.bind();//(QOpenGLBuffer::IndexBuffer);
    m_ebo.allocate(indices.data(), m_indexCount * sizeof(GLuint));

    terrainShaderProgram->enableAttributeArray(0);
    terrainShaderProgram->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));
    terrainShaderProgram->enableAttributeArray(1);
    terrainShaderProgram->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, normal), 3, sizeof(Vertex));

    m_vbo.release();
    m_vbo.release();
    m_ebo.release(QOpenGLBuffer::IndexBuffer);
}


void chunk::setLOD(int lodLevel) {
    m_currentLOD = lodLevel;
    if (lodLevel == 0) {
        m_currentResolution = HIGH_RES;
    } else {
        m_currentResolution = LOW_RES;
    }
}

QVector3D chunk::getCenterPosition() const {
    float worldX = (m_chunkGridX + 0.5f) * CHUNK_SIZE;
    float worldZ = (m_chunkGridZ + 0.5f) * CHUNK_SIZE;
    return QVector3D(worldX, NoiseUtils::getHeight(worldX, worldZ), worldZ);
}

void chunk::render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLExtraFunctions *glFuncs) {
    if (m_indexCount == 0 || !m_vao.isCreated()) return;
    terrainShaderProgram->setUniformValue("modelMatrix", m_modelMatrix);
    m_vao.bind();
    glFuncs->glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    m_vao.release();
}

void chunk::renderBorders(QOpenGLShaderProgram* lineShaderProgram, QOpenGLExtraFunctions* glFuncs, QOpenGLVertexArrayObject* lineQuadVao, QOpenGLBuffer* lineQuadVbo) {
    if (!lineQuadVao || !lineQuadVao->isCreated()) return;
    lineShaderProgram->setUniformValue("modelMatrix", m_modelMatrix);
    lineQuadVao->bind();
    glFuncs->glDrawArrays(GL_LINE_LOOP, 0, 4);
    lineQuadVao->release();
}
