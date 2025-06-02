#include "chunk.h"
#include "noiseutils.h"
#include <QDebug>

chunk::chunk() :
    m_chunkGridX(0),
    m_chunkGridZ(0),
    // m_vao, m_vbo, m_ebo sao inicializados por seus proprios construtores padrao
    m_indexCount(0),
    m_vertexCount(0),
    //m_modelmatrix e minicializada como identidade por padrao
    m_currentResolution(0),
    m_currentLOD(-1)
{}

chunk::~chunk() {

}

chunk::chunk(chunk&& other) noexcept
    : m_chunkGridX(other.m_chunkGridX),
    m_chunkGridZ(other.m_chunkGridZ),
    m_vao(std::move(other.m_vao)),         // Transfere a propriedade do VAO
    m_vbo(std::move(other.m_vbo)),         // Transfere a propriedade do VBO
    m_ebo(std::move(other.m_ebo)),         // Transfere a propriedade do EBO
    m_indexCount(other.m_indexCount),
    m_vertexCount(other.m_vertexCount),
    m_modelMatrix(std::move(other.m_modelMatrix)), // QMatrix4x4 também suporta movimento
    m_currentResolution(other.m_currentResolution),
    m_currentLOD(other.m_currentLOD)
{
    // Deixa o objeto 'other' em um estado válido, mas "vazio" ou resetado,
    // para que seu destrutor não tente liberar recursos que foram movidos.
    other.m_chunkGridX = 0; // Opcional para tipos simples, mas boa prática
    other.m_chunkGridZ = 0; // Opcional
    other.m_indexCount = 0;
    other.m_vertexCount = 0;
    other.m_currentResolution = 0;
    other.m_currentLOD = -1;
    // other.m_modelMatrix.setToIdentity(); // Opcional, já que foi movida

    // Os objetos m_vao, m_vbo, m_ebo em 'other' agora estão em um estado "movido de"
    // (geralmente inválido para uso, mas seguro para destruição).
    // qInfo() << "Chunk Move Constructed";
}

// Operador de Atribuição por Movimento
chunk& chunk::operator=(chunk&& other) noexcept {
    if (this != &other) { // Proteção contra auto-atribuição (ex: c = std::move(c);)
        // Liberar recursos existentes deste objeto (this)
        // Se m_vao, m_vbo, m_ebo estivessem ativos, seus destrutores seriam chamados
        // quando são sobrescritos pelas operações de movimento abaixo.
        // Se você quiser ser explícito sobre a liberação antes da atribuição:
        // if (m_vao.isCreated()) m_vao.destroy();
        // if (m_vbo.isCreated()) m_vbo.destroy();
        // if (m_ebo.isCreated()) m_ebo.destroy();

        // Mover os dados de 'other' para 'this'
        m_chunkGridX = other.m_chunkGridX;
        m_chunkGridZ = other.m_chunkGridZ;
        m_vao = std::move(other.m_vao);
        m_vbo = std::move(other.m_vbo);
        m_ebo = std::move(other.m_ebo);
        m_indexCount = other.m_indexCount;
        m_vertexCount = other.m_vertexCount;
        m_modelMatrix = std::move(other.m_modelMatrix);
        m_currentResolution = other.m_currentResolution;
        m_currentLOD = other.m_currentLOD;

        // Resetar o objeto 'other'
        other.m_chunkGridX = 0;
        other.m_chunkGridZ = 0;
        other.m_indexCount = 0;
        other.m_vertexCount = 0;
        other.m_currentResolution = 0;
        other.m_currentLOD = -1;
        // other.m_modelMatrix.setToIdentity();
    }
    // qInfo() << "Chunk Move Assigned";
    return *this;
}

void chunk::init(int cX, int cZ, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLExtraFunctions *glFuncs) {
    m_chunkGridX = cX;
    m_chunkGridZ = cZ;
    float worldX = static_cast<float>(m_chunkGridX * CHUNK_SIZE);
    float worldZ = static_cast<float>(m_chunkGridZ * CHUNK_SIZE);
    m_modelMatrix.setToIdentity();
    m_modelMatrix.translate(worldX, 0.0f, worldZ);
    setLOD(0);
    generateMesh(m_currentResolution, terrainShaderProgram, glFuncs);
}

void chunk::generateMesh(int resolution, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLExtraFunctions *glFuncs) {
    if (resolution <= 1) {
        qWarning() << "Chunk resolution too low or invalid:" << resolution;
        m_indexCount = 0;
        m_vertexCount = 0;
        //Certifique=se de que od buffers antigos sejam destruidos de existirem
        if (m_vao.isCreated()) m_vao.destroy();
        if (m_vbo.isCreated()) m_vbo.destroy();
        if (m_ebo.isCreated()) m_ebo.destroy();
        return;
    }
    m_currentResolution = resolution;

    std::vector<Vertex> vertices;
    vertices.reserve(resolution * resolution);
    std::vector<GLuint> indices;
    indices.reserve((resolution - 1) * (resolution - 1) * 6);
    float step = static_cast<float>(CHUNK_SIZE) / (resolution - 1);

    for (int r = 0; r < resolution; ++r) {
        for (int c = 0; c < resolution; ++c) {
            Vertex v;
            float localX = c * step;
            float localZ = r * step;
            float noise_coord_x = (m_chunkGridX * CHUNK_SIZE) + localX;
            float noise_coord_z = (m_chunkGridZ * CHUNK_SIZE) + localZ;
            v.position = QVector3D(localX, NoiseUtils::getHeight(noise_coord_x, noise_coord_z), localZ);
            float offset_norm = 0.1f;
            float hL = NoiseUtils::getHeight(noise_coord_x - offset_norm, noise_coord_z);
            float hR = NoiseUtils::getHeight(noise_coord_x + offset_norm, noise_coord_z);
            float hD = NoiseUtils::getHeight(noise_coord_x, noise_coord_z - offset_norm);
            float hU = NoiseUtils::getHeight(noise_coord_x, noise_coord_z + offset_norm);
            v.normal = QVector3D(hL - hR, 2.0f * offset_norm, hD - hU).normalized();
            vertices.push_back(v);
        }
    }

    for (int r = 0; r < resolution - 1; ++r) {
        for (int c = 0; c < resolution - 1; ++c) {
            GLuint topLeft = r * resolution + c;
            GLuint topRight = topLeft + 1;
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

    m_indexCount = static_cast<int>(indices.size());
    m_vertexCount = static_cast<int>(vertices.size());
    if (m_indexCount == 0 || m_vertexCount == 0) {
        qWarning() << "Generated mesh os empty for chunk (" << m_chunkGridX << "," << m_chunkGridZ << ") with resolution" << resolution;

        if (m_vao.isCreated()) m_vao.destroy();
        if (m_vbo.isCreated()) m_vbo.destroy();
        if (m_ebo.isCreated()) m_ebo.destroy();
        return;
    }

    if (m_vao.isCreated()) m_vao.destroy();
    if (m_vbo.isCreated()) m_vbo.destroy();
    if (m_ebo.isCreated()) m_ebo.destroy();

    m_vao.create();
    m_vao.bind();
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertices.data(), m_vertexCount * sizeof(Vertex));
    m_ebo.create();
    m_ebo.bind(QOpenGLBuffer::IndexBuffer);
    m_ebo.allocate(indices.data(), m_indexCount * sizeof(GLuint));

    terrainShaderProgram->enableAttributeArray(0);
    terrainShaderProgram->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));
    terrainShaderProgram->enableAttributeArray(1);
    terrainShaderProgram->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, normal), 3, sizeof(Vertex));

    m_vao.release();
    //m_vbo.release();
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
