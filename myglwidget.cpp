#include "myglwidget.h"
#include <QOpenGLContext>
#include <QDebug>

//Constantes com o código GLSL dos shaders
const char* terrainVertexShaderSource = R"(
    #version 300es

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

const char* terrainFragmentShaderSource = R"(
    #version 300 es

    precision mediump float;
    // se der errado trocar por highp

    in vec3 v_worldPos;
    in vec3 v_normal;

    out vec4 FragColor;

    uniform vec3 lightPos = vec3(50.0, 100.0, 50.0);
    uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);
    uniform vec3 objectBaseColor = vec3(0.4, 0.6, 0.2);

    void main() {
        vec3 norm = normalize(v_normal);
        vec3 lightDir = normalize(lightPos - v_worldPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        float ambientStrength = 0.2;
        vec3 ambient = ambientStrength * lightColor;
        vec3 resultColor = (ambient + diffuse) * objectBaseColor;
        float heightFactor = clamp(v_worldPos.y / 20.0, 0.0, 1.0);
        resultColor = mix(resultColor, vec3(0.6, 0.5, 0.3), heightFacotr * 0.5);
        FragColor = vec4(resultColor, 1.0);
    }
)";

const char* lineVertexShaderSource = R"(
    #version 300 es

    layout (location = 0) in vec3 a_position;

    layout (std140) uniform SceneMatrices {
        mat4 projectionMatrix;
        mat4 viewMatrix;
    };

    uniform mat4 modelMatrix;

    void main() {
        vec3 elevated_position = a_position + vec3(0.0, 0.1, 0.0);
        gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(elevated_position, 1,0);
    }
)";

const char* lineFragmentShaderSource = R"(
    #version 300 es

    precision mediump float;
    //se nao der certo mudar para highp

    outr vec4 FragColor;

    void main() {
        FragColor = vec4(0.5, 0.5, 0.5, 1.0); //Cor cinza
    }
)";

MyGLWidget::MyGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), m_extraFunction(nullptr), m_uboBindingPointSceneMatrices(0){
    connect(&m_timer, &QTimer::timeout, this, QOverload<>::of(&MyGLWidget::update));
    m_timer.start(16);
}

MyGLWidget::~MyGLWidget() {
    makeCurrent();
    doneCurrent();
}

void MyGLWidget::initializeGL() {
    initializeOpenGLFunctions();
    m_extraFunction = QOpenGLContext::currentContext()->extraFunctions();
    if (!m_extraFunction) {
        qWarning("QOpenGLExtraFunctions not available");
    }

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    // Compilar Shaders a partir do código fonte embutido
    if (!m_terrainShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, terrainVertexShaderSource))
        qWarning() << "Terrain Vertex Shader Error:" << m_terrainShaderProgram.log();
    if (!m_terrainShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, terrainFragmentShaderSource))
        qWarning() << "Terrain Fragment Shader Error:" << m_terrainShaderProgram.log();
    if (!m_terrainShaderProgram.link())
        qWarning() << "Terrain Shader Linker Error:" << m_terrainShaderProgram.log();

    if (!m_lineShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, lineVertexShaderSource))
        qWarning() << "Line Vertex Shader Error:" << m_lineShaderProgram.log();
    if (!m_lineShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, lineFragmentShaderSource))
        qWarning() << "Line Fragment Shader Error:" << m_lineShaderProgram.log();
    if (!m_lineShaderProgram.link())
        qWarning() << "Line Shader Linker Error:" << m_lineShaderProgram.log();

    setupUBO();
    setupLineQuadVAO();
    m_terrainManager.init(2, &m_terrainShaderProgram, &m_lineShaderProgram, &m_lineQuadVao, &m_lineQuadVbo, m_extraFunction);

    //camera inicial
    m_camera.setPosition(QVector3D(CHUNK_SIZE * 0.5f, 35.0f, CHUNK_SIZE * 2.5));
    m_camera.lookAt(QVector3D(CHUNK_SIZE * 0.5f, 0, 0), QVector3D(0,1,0));
    m_camera.pitch(-15.0f);
}

void MyGLWidget::setupUBO() {
    if (!m_extraFunction) return;
    m_uboBindingPointSceneMatrices = 0;
    m_sceneMatricesUBO.create();
    m_sceneMatricesUBO.bind();
    m_sceneMatricesUBO.allocate(sizeof(SceneMatrices));
    m_sceneMatricesUBO.release();

    m_extraFunction->glBindBufferBase(GL_UNIFORM_BUFFER, m_uboBindingPointSceneMatrices, m_sceneMatricesUBO.bufferId());

    GLuint terrainBlockIndex = m_extraFunction->glGetUniformBlockIndex(m_terrainShaderProgram.programId(), "SceneMatrices");
    if (terrainBlockIndex != GL_INVALID_INDEX) {
        m_extraFunction->glUniformBlockBinding(m_terrainShaderProgram.programId(), terrainBlockIndex, m_uboBindingPointSceneMatrices);
    } else {
        qWarning("Uniform block 'ScenceMatrices' not found in line shader");
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
    m_lineShaderProgram.enableAttributeArray(0);
    m_lineShaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);
    m_lineQuadVao.release();
    m_lineQuadVbo.release();
}

void MyGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
            qWarning("Failed to map UBO for writing");
        }
        m_sceneMatricesUBO.release();
    }

    m_terrainManager.update(m_camera.position(), &m_terrainShaderProgram, m_extraFunction);
    m_terrainManager.render(&m_terrainShaderProgram, &m_lineShaderProgram, m_extraFunction);
}

void MyGLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    m_camera.setPerspective(45.0, static_cast<float>(w) / static_cast<float>(h ? h : 1), 0.1f, 1000.0f);
}


