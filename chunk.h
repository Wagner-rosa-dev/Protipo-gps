#ifndef CHUNK_H
#define CHUNK_H

#include <QVector3D>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>
#include <QMatrix4x4>
#include <vector>
#include <utility>

struct Vertex {
    QVector3D position;
    QVector3D normal;
};

const int CHUNK_SIZE = 32;

class chunk {
public:
    chunk();
    ~chunk();

    // ----- semantica de movimento e copia ------
    //construtor de movimento
    chunk(chunk&& other) noexcept;
    //Operador de atribuição por movimento
    chunk& operator=(chunk&& other) noexcept;

    //Impedir copias(importante apra classes com rescursos opengl gerenciados)
    chunk(const chunk& other) = delete;
    chunk& operator=(const chunk& other) = delete;




    void init(int cX, int cZ, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLExtraFunctions *glFuncs);
    void generateMesh(int resolution, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLExtraFunctions *glFuncs);
    void render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLExtraFunctions *glFuncs);
    void renderBorders(QOpenGLShaderProgram* lineShaderProgram, QOpenGLExtraFunctions* glFuncs, QOpenGLVertexArrayObject* lineQuadVao, QOpenGLBuffer* lineQuadVbo);

    void setLOD(int lodLevel);
    int getLOD() const {return m_currentLOD; }
    QVector3D getCenterPosition() const;
    QMatrix4x4 modelMatrix() const { return m_modelMatrix; }

private:
    int m_chunkGridX;
    int m_chunkGridZ;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_ebo;
    int m_indexCount;
    int m_vertexCount;

    QMatrix4x4 m_modelMatrix;
    int m_currentResolution;
    int m_currentLOD;

public:
    static const int HIGH_RES = 16;
    static const int LOW_RES = 8;
};

#endif // CHUNK_H
