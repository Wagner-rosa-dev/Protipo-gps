#include "myglwidget.h"
#include <QOpenGLContext>
#include <QDebug>
#include <QOpenGLShaderProgram> // Incluído para QOpenGLShaderProgram

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !! VERIFIQUE SE ESTA É A VERSÃO DO ARQUIVO myglwidget.cpp QUE ESTÁ SENDO  !!
// !! EFETIVAMENTE COMPILADA E EXECUTADA NA SUA PLACA TORADEX.               !!
// !! Data/Hora da Modificação: 04/06/2025                                   !!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//Constantes com o código GLSL dos shaders
const char* terrainVertexShaderSource = R"(#version 300 es

// Terrain Vertex Shader - TESTE_VERSAO_NOVA_SHADER_04_06_2025

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

layout (std140) uniform SceneMatrices {
    mat4 projectionMatrix;
    mat4 viewMatrix;
};

uniform mat4 modelMatrix;

out vec3 v_worldPos;
out vec3 v_normal;

void main() {
    vec4 worldPos4 = modelMatrix * vec4(a_position, 1.0);
    gl_Position = projectionMatrix * viewMatrix * worldPos4;
    v_worldPos = worldPos4.xyz;
    v_normal = normalize(mat3(modelMatrix) * a_normal);
}
)";
const char* terrainFragmentShaderSource = R"(#version 300 es

// Terrain Fragment Shader - TESTE_VERSAO_NOVA_SHADER_04_06_2025

precision mediump float;

in vec3 v_worldPos;
in vec3 v_normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectBaseColor;

void main() {
    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(lightPos - v_worldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;
    vec3 resultColor = (ambient + diffuse) * objectBaseColor;

    float heightFactor = clamp(v_worldPos.y / 20.0, 0.0, 1.0);
    resultColor = mix(resultColor, vec3(0.6, 0.5, 0.3), heightFactor * 0.5);

    FragColor = vec4(resultColor, 1.0);
}
)";

const char* lineVertexShaderSource = R"(#version 300 es

// Line Vertex Shader - TESTE_VERSAO_NOVA_SHADER_04_06_2025

layout (location = 0) in vec3 a_position;

layout (std140) uniform SceneMatrices {
    mat4 projectionMatrix;
    mat4 viewMatrix;
};

uniform mat4 modelMatrix;

void main() {
    vec3 elevated_position = a_position + vec3(0.0, 0.1, 0.0);
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(elevated_position, 1.0);
}
)";

const char* lineFragmentShaderSource = R"(#version 300 es

// Line Fragment Shader - TESTE_VERSAO_NOVA_SHADER_04_06_2025

precision mediump float; // Define a precisão padrão para floats

out vec4 FragColor;

void main() {
    FragColor = vec4(0.5, 0.5, 0.5, 1.0); // Cor cinza para as bordas
}
)";

MyGLWidget::MyGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), m_extraFunction(nullptr), m_uboBindingPointSceneMatrices(0){
    connect(&m_timer, &QTimer::timeout, this, QOverload<>::of(&MyGLWidget::update));
    m_timer.start(16); // Aproximadamente 60 FPS
}

MyGLWidget::~MyGLWidget() {
    makeCurrent(); // Garante que o contexto OpenGL está ativo para limpeza
    // Objetos QOpenGL* (shaders, buffers, vao) são limpos por seus destrutores.
    doneCurrent();
}

void MyGLWidget::initializeGL() {
    initializeOpenGLFunctions();
    qInfo() << "MYGLWIDGET_CPP EXECUTANDO - VERSAO SUPER NOVA 04_06_2025_1530";
    m_extraFunction = QOpenGLContext::currentContext()->extraFunctions();
    if (!m_extraFunction) {
        qWarning("QOpenGLExtraFunctions not available. UBOs and other advanced features might not work.");
    }

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f); // Cor de fundo azul escura
    glEnable(GL_DEPTH_TEST); // Habilita teste de profundidade

    qInfo() << "Compilando Terrain Shaders (Versão de Teste 04/06/2025)...";
    if (!m_terrainShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, terrainVertexShaderSource)) {
        qWarning() << "Terrain Vertex Shader Compilation Error:" << m_terrainShaderProgram.log();
    }
    if (!m_terrainShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, terrainFragmentShaderSource)) {
        qWarning() << "Terrain Fragment Shader Compilation Error:" << m_terrainShaderProgram.log();
    }
    if (!m_terrainShaderProgram.link()) {
        qWarning() << "Terrain Shader Linker Error:" << m_terrainShaderProgram.log();
    } else {
        qInfo() << "Terrain Shaders linked successfully.";
    }

    qInfo() << "Compilando Line Shaders (Versão de Teste 04/06/2025)...";
    if (!m_lineShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, lineVertexShaderSource)) {
        qWarning() << "Line Vertex Shader Compilation Error:" << m_lineShaderProgram.log();
    }
    if (!m_lineShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, lineFragmentShaderSource)) {
        qWarning() << "Line Fragment Shader Compilation Error:" << m_lineShaderProgram.log();
    }
    if (!m_lineShaderProgram.link()) {
        qWarning() << "Line Shader Linker Error:" << m_lineShaderProgram.log();
    } else {
        qInfo() << "Line Shaders linked successfully.";
    }

    setupUBO();
    setupLineQuadVAO();
    m_terrainManager.init(2, &m_terrainShaderProgram, &m_lineShaderProgram, &m_lineQuadVao, &m_lineQuadVbo, m_extraFunction);

    m_camera.setPosition(QVector3D(CHUNK_SIZE * 0.5f, 35.0f, CHUNK_SIZE * 2.5f));
    m_camera.lookAt(QVector3D(CHUNK_SIZE * 0.5f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));
}

void MyGLWidget::setupUBO() {
    if (!m_extraFunction) {
        qWarning("Cannot setup UBO: QOpenGLExtraFunctions not available.");
        return;
    }
    m_uboBindingPointSceneMatrices = 0;
    m_sceneMatricesUBO.create();
    m_sceneMatricesUBO.bind();
    m_sceneMatricesUBO.allocate(sizeof(SceneMatrices));
    m_sceneMatricesUBO.release();

    m_extraFunction->glBindBufferBase(GL_UNIFORM_BUFFER, m_uboBindingPointSceneMatrices, m_sceneMatricesUBO.bufferId());

    if (m_terrainShaderProgram.isLinked()) {
        GLuint terrainBlockIndex = m_extraFunction->glGetUniformBlockIndex(m_terrainShaderProgram.programId(), "SceneMatrices");
        if (terrainBlockIndex != GL_INVALID_INDEX) {
            m_extraFunction->glUniformBlockBinding(m_terrainShaderProgram.programId(), terrainBlockIndex, m_uboBindingPointSceneMatrices);
        } else {
            qWarning("Uniform block 'SceneMatrices' not found in terrain shader.");
        }
    } else {
        qWarning("Terrain shader program not linked, cannot bind UBO for terrain shader.");
    }

    if (m_lineShaderProgram.isLinked()) {
        GLuint lineBlockIndex = m_extraFunction->glGetUniformBlockIndex(m_lineShaderProgram.programId(), "SceneMatrices");
        if (lineBlockIndex != GL_INVALID_INDEX) {
            m_extraFunction->glUniformBlockBinding(m_lineShaderProgram.programId(), lineBlockIndex, m_uboBindingPointSceneMatrices);
        } else {
            qWarning("Uniform block 'SceneMatrices' not found in line shader.");
        }
    } else {
        qWarning("Line shader program not linked, cannot bind UBO for line shader.");
    }
}

void MyGLWidget::setupLineQuadVAO() {
    GLfloat lineQuadVertices[] = {
        0.0f, 0.0f, 0.0f,
        CHUNK_SIZE, 0.0f, 0.0f,
        CHUNK_SIZE, 0.0f, CHUNK_SIZE,
        0.0f, 0.0f, CHUNK_SIZE
    };

    m_lineQuadVao.create();
    m_lineQuadVao.bind();

    m_lineQuadVbo.create();
    m_lineQuadVbo.bind();
    m_lineQuadVbo.allocate(lineQuadVertices, sizeof(lineQuadVertices));

    if (m_lineShaderProgram.isLinked()) {
        m_lineShaderProgram.enableAttributeArray(0);
        m_lineShaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);
    } else {
        qWarning("Line shader program not linked, cannot set attribute buffers for line quad VAO.");
    }

    m_lineQuadVao.release();
    m_lineQuadVbo.release();
}

void MyGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bool terrainShaderOk = m_terrainShaderProgram.isLinked();
    bool lineShaderOk = m_lineShaderProgram.isLinked();

    if (!terrainShaderOk && !lineShaderOk) {
        // qWarning("Neither terrain nor line shaders are linked. Skipping rendering.");
        // A mensagem de erro já foi emitida durante initializeGL
        return;
    }

    SceneMatrices matrices;
    matrices.projectionMatrix = m_camera.projectionMatrix();
    matrices.viewMatrix = m_camera.viewMatrix();

    if (m_sceneMatricesUBO.isCreated()) {
        m_sceneMatricesUBO.bind();
        void *ptr = m_sceneMatricesUBO.mapRange(0, sizeof(SceneMatrices), QOpenGLBuffer::RangeWrite | QOpenGLBuffer::RangeInvalidateBuffer);
        if (ptr) {
            memcpy(ptr, &matrices, sizeof(SceneMatrices));
            m_sceneMatricesUBO.unmap();
        } else {
            qWarning("Failed to map UBO for writing SceneMatrices.");
        }
        m_sceneMatricesUBO.release();
    }

    // Passa os ponteiros dos shaders para o terrainManager.
    // O terrainManager internamente (ou as classes chunk) devem verificar
    // se o shader está linkado antes de usá-lo.
    m_terrainManager.update(m_camera.position(),
                            terrainShaderOk ? &m_terrainShaderProgram : nullptr,
                            m_extraFunction);

    if (terrainShaderOk) {
        m_terrainShaderProgram.bind();
        m_terrainShaderProgram.setUniformValue("lightPos", QVector3D(50.0f, 100.0f, 50.0f));
        m_terrainShaderProgram.setUniformValue("lightColor", QVector3D(1.0f, 1.0f, 1.0f));
        m_terrainShaderProgram.setUniformValue("objectBaseColor", QVector3D(0.4f, 0.6f, 0.2f));
    }


    m_terrainManager.render(terrainShaderOk ? &m_terrainShaderProgram : nullptr,
                            lineShaderOk ? &m_lineShaderProgram : nullptr,
                            m_extraFunction);

    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        qWarning() << "Erro no OpenGl em tempo de execução" << err;
    }



}

void MyGLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    m_camera.setPerspective(45.0f, static_cast<float>(w) / static_cast<float>(h > 0 ? h : 1), 0.1f, 1000.0f);
}

// Slot update chamado pelo m_timer
void MyGLWidget::update() {
    // Lógica de atualização de estado (ex: input do teclado/mouse para câmera)
    // Por enquanto, apenas agenda um repaint.
    QOpenGLWidget::update(); // Reagenda paintGL()
}
