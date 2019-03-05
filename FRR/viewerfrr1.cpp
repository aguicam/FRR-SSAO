#include "viewerfrr1.h"
#include "Helpers/mesh_importer.h"
#include <QOpenGLFunctions>
#include <QtOpenGL>
#include <vector>
#include <Eigen>
#include <glm/glm.hpp>
#include <math.h>


#define GL_GLEXT_PROTOTYPES 1

//shader ID
enum {GeometryPass,SSAO,LightSSAO,BlurSSAO,SepSSAO,Blur1,Blur2};

ViewerFRR1::ViewerFRR1(const QGLFormat &glf, QWidget *parent) : HW(glf, parent) , mesh_importer(this)
{
    // init vars
    setFocusPolicy(Qt::StrongFocus);
}
void ViewerFRR1::initializeGL()
{
    // initialize GL function resolution for current context
    initializeGLFunctions();

    // init vertex and fragment shaders
    initShaders();

    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);


    // initialize vertex buffer and write positions to vertex shader
    initVertexBuffer();

    // init state variables
    glClearColor(1.0, 1.0, 1.0, 1.0);	// set background color
    glColor3f   (1.0, 1.0, 0.0);		// set foreground color

    if(!timerG.isCreated()){
        timerG.create();
    }

}

void ViewerFRR1::resizeGL(int w, int h)
{

    if (h == 0) h = 1;
    width_ = w;
    height_ = h;

    camera_.SetViewport(0, 0, w, h);
    camera_.SetProjection(kFieldOfView, kZNear, kZFar);

    m_winW = w;
    m_winH = h;
    initVertexBuffer();
    return;
}

void ViewerFRR1::paintGL()
{
     if(timerG.isCreated()) timerG.begin();

    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (mesh_ == nullptr) return;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera_.SetViewport();
    Eigen::Matrix4f projection =camera_.SetProjection();
    Eigen::Matrix4f view = camera_.SetView();
    Eigen::Matrix4f model = camera_.SetModel();
    const Eigen::Affine3f piRotation(Eigen::AngleAxisf(M_PI, Eigen::Vector3f(0.0, 1.0, 0.0)));
    model = piRotation* model ;
    Eigen::Matrix4f t = view * model;
    Eigen::Matrix3f normal;

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) normal(i, j) = t(i, j);

    normal = normal.inverse().transpose();

    glUseProgram(m_program[GeometryPass].programId());
    glUniformMatrix4fv(const_uniform[GeometryPass][CONST_UNIFORM_PROJ], 1, GL_FALSE, projection.data());
    glUniformMatrix4fv(const_uniform[GeometryPass][CONST_UNIFORM_MODEL], 1, GL_FALSE, model.data());
    glUniformMatrix4fv(const_uniform[GeometryPass][CONST_UNIFORM_VIEW], 1, GL_FALSE, view.data());
    glUniformMatrix3fv(const_uniform[GeometryPass][CONST_UNIFORM_NORMAL], 1, GL_FALSE, normal.data());

    drawVAO();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    if(showBlur || showSepBlur ||showLight){ glBindFramebuffer(GL_FRAMEBUFFER, fboSSAO);}//
    glClear(GL_COLOR_BUFFER_BIT);


    if(!useSepSSAO){
        glUseProgram(m_program[SSAO].programId());
        glUniform1f(glGetUniformLocation(m_program[SSAO].programId(), "height"), height_);
        glUniform1f(glGetUniformLocation(m_program[SSAO].programId(), "width"), width_);
        glUniform1f(glGetUniformLocation(m_program[SSAO].programId(), "radius"), radi);
        glUniform1f(glGetUniformLocation(m_program[SSAO].programId(), "bias"), bias);

        // Send kernel + rotation
        for (unsigned int i = 0; i < 64; ++i){
            std::string str = "samples[" + std::to_string(i) + "]";
            const char *ch=str.c_str();
            glUniform3fv(glGetUniformLocation(m_program[SSAO].programId(),ch),1,&ssaoKernel[i][0]);//,ssaoKernel[i][1],ssaoKernel[i][2]);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texturePosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureNoise);
        renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }else{
        glUseProgram(m_program[SepSSAO].programId());
        glUniform1f(glGetUniformLocation(m_program[SepSSAO].programId(), "height"), height_);
        glUniform1f(glGetUniformLocation(m_program[SepSSAO].programId(), "width"), width_);
        glUniform1f(glGetUniformLocation(m_program[SepSSAO].programId(), "radius"), radi);
        glUniform1f(glGetUniformLocation(m_program[SepSSAO].programId(), "bias"), bias);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texturePosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureNoise);
        renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }




    if(showSSAO || showSepSSAO){
        if(timerG.isCreated()){
             timerG.end();
             resTime = timerG.waitForResult();
             if(resTime==0){return;}

             fps = 1.0/((float)resTime/1000000000.0);

             std::stringstream s_fps;
             s_fps << std::fixed << std::setprecision(1);
             s_fps << fps << " FPS" << std::ends;
             s_fps << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
             fpsString = QString::fromStdString(s_fps.str());
             m_fpsCount->setText(fpsString);
         }
        return;}

    if(showLight && !useSepBlur){ glBindFramebuffer(GL_FRAMEBUFFER, fboBlurSSAO);}//
    if(useSepBlur||showSepBlur)glBindFramebuffer(GL_FRAMEBUFFER, fboBlurSSAO1);
    glClear(GL_COLOR_BUFFER_BIT);



    if(!useSepBlur){

        glUseProgram(m_program[BlurSSAO].programId());
        glUniform1f(glGetUniformLocation(m_program[BlurSSAO].programId(), "height"), height_);
        glUniform1f(glGetUniformLocation(m_program[BlurSSAO].programId(), "width"), width_);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fboColorSSAO);
        renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


    }else{
        glUseProgram(m_program[Blur1].programId());
        glUniform1f(glGetUniformLocation(m_program[Blur1].programId(), "height"), height_);
        glUniform1f(glGetUniformLocation(m_program[Blur1].programId(), "width"), width_);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fboColorSSAO);
        renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if(showLight)glBindFramebuffer(GL_FRAMEBUFFER, fboBlurSSAO2);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(m_program[Blur2].programId());
        glUniform1f(glGetUniformLocation(m_program[Blur2].programId(), "height"), height_);
        glUniform1f(glGetUniformLocation(m_program[Blur2].programId(), "width"), width_);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fboColorBlurSSAO1);
        renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }

    if(showBlur || showSepBlur){
        if(timerG.isCreated()){
             timerG.end();
             resTime = timerG.waitForResult();
             if(resTime==0){return;}

             fps = 1.0/((float)resTime/1000000000.0);

             std::stringstream s_fps;
             s_fps << std::fixed << std::setprecision(1);
             s_fps << fps << " FPS" << std::ends;
             s_fps << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
             fpsString = QString::fromStdString(s_fps.str());
             m_fpsCount->setText(fpsString);
         }
        return;}

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_program[LightSSAO].programId());

    // send light relevant uniforms

    Eigen::Vector4f lightPos = Eigen::Vector4f(0, 2, 1, 1.0);
    Eigen::Vector3f lightColor = Eigen::Vector3f(0.8,0.8,0.8);
    Eigen::Vector4f viewPos4 = view * lightPos;
    Eigen::Vector3f lightPosView = Eigen::Vector3f(viewPos4[0] , viewPos4[1], viewPos4[2]);// Eigen::Vector3f(view * Eigen::Vector4f(lightPos, 1.0));

    glUniform3f(glGetUniformLocation(m_program[LightSSAO].programId(), "light.Position"), lightPosView[0],lightPosView[1],lightPosView[2]);
    glUniform3f(glGetUniformLocation(m_program[LightSSAO].programId(), "light.Color"), lightColor[0],lightColor[1],lightColor[2]);
    glUniform1f(glGetUniformLocation(m_program[LightSSAO].programId(), "height"), height_);
    glUniform1f(glGetUniformLocation(m_program[LightSSAO].programId(), "width"), width_);

    // Update attenuation parameters
    const float linear    = 0.09;
    const float quadratic = 0.032;

    glUniform1f(glGetUniformLocation(m_program[LightSSAO].programId(), "light.Linear"), linear);
    glUniform1f(glGetUniformLocation(m_program[LightSSAO].programId(), "light.Quadratic"), quadratic);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texturePosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureAlbedo);
    glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
    glBindTexture(GL_TEXTURE_2D, fboColorBlurSSAO);
    renderQuad();




    if(timerG.isCreated()){
         timerG.end();
         resTime = timerG.waitForResult();
         if(resTime==0){return;}

         fps = 1.0/((float)resTime/1000000000.0);

         std::stringstream s_fps;
         s_fps << std::fixed << std::setprecision(1);
         s_fps << fps << " FPS" << std::ends;
         s_fps << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
         fpsString = QString::fromStdString(s_fps.str());
         m_fpsCount->setText(fpsString);
     }



}

QGroupBox* ViewerFRR1::controlPanel()
{
    // init group box
    QGroupBox *groupBox = HW::controlPanel();
    groupBox->setStyleSheet(GroupBoxStyle);
    mesh_importer.AddImportExportPLY(groupBox);
    radiusAndBias(groupBox);
    showPasses(groupBox);
    usePassesSSAO(groupBox);
    usePassesBlur(groupBox);
    showFramerate(groupBox);


    return(groupBox);
}

void ViewerFRR1::reset()
{
    mesh_ = nullptr;	// recompute geometry
    initVertexBuffer();
    // draw
    updateGL();
}

void ViewerFRR1::initShaders()
{
    // init uniforms hash table based on uniform variable names and location IDs
    UniformMap uniforms;
    // compile shader, bind attribute vars, link shader, and initialize uniform var table
    initShader(GeometryPass, QString(":FRR/Shaders/geometry_ssao.vert"), QString(":FRR/Shaders/geometry_ssao.frag"), uniforms);
    initShader(SSAO, QString(":FRR/Shaders/ssao.vert"), QString(":FRR/Shaders/ssao.frag"), uniforms);
    initShader(LightSSAO, QString(":FRR/Shaders/ssao.vert"), QString(":FRR/Shaders/light_ssao.frag"), uniforms);
    initShader(BlurSSAO, QString(":FRR/Shaders/ssao.vert"), QString(":FRR/Shaders/blur_ssao.frag"), uniforms);
    initShader(SepSSAO, QString(":FRR/Shaders/ssao.vert"), QString(":FRR/Shaders/ssao_sep.frag"), uniforms);
    initShader(Blur1, QString(":FRR/Shaders/ssao.vert"), QString(":FRR/Shaders/blur1.frag"), uniforms);
    initShader(Blur2, QString(":FRR/Shaders/ssao.vert"), QString(":FRR/Shaders/blur2.frag"), uniforms);


    glUseProgram(m_program[LightSSAO].programId());
    unsigned int texLoc = glGetUniformLocation(m_program[LightSSAO].programId(), "ssaoInput");
    glUniform1i(texLoc, 3);
    texLoc = glGetUniformLocation(m_program[LightSSAO].programId(), "gPosition");
    glUniform1i(texLoc, 0);
    texLoc = glGetUniformLocation(m_program[LightSSAO].programId(), "gNormal");
    glUniform1i(texLoc, 1);
    texLoc = glGetUniformLocation(m_program[LightSSAO].programId(), "gAlbedo");
    glUniform1i(texLoc, 2);

    glUseProgram(m_program[SSAO].programId());
    texLoc = glGetUniformLocation(m_program[SSAO].programId(), "gPosition");
    glUniform1i(texLoc, 0);
    texLoc = glGetUniformLocation(m_program[SSAO].programId(), "gNormal");
    glUniform1i(texLoc, 1);
    texLoc = glGetUniformLocation(m_program[SSAO].programId(), "texNoise");
    glUniform1i(texLoc, 2);

    glUseProgram(m_program[SepSSAO].programId());
    texLoc = glGetUniformLocation(m_program[SepSSAO].programId(), "gPosition");
    glUniform1i(texLoc, 0);
    texLoc = glGetUniformLocation(m_program[SepSSAO].programId(), "gNormal");
    glUniform1i(texLoc, 1);
    texLoc = glGetUniformLocation(m_program[SepSSAO].programId(), "texNoise");
    glUniform1i(texLoc, 2);

    glUseProgram(m_program[Blur1].programId());
    texLoc = glGetUniformLocation(m_program[Blur1].programId(), "ssaoInput");
    glUniform1i(texLoc, 0);



    glUseProgram(m_program[Blur2].programId());
    texLoc = glGetUniformLocation(m_program[Blur2].programId(), "blur1stPass");
    glUniform1i(texLoc, 0);


}

void ViewerFRR1::initVertexBuffer()
{


    if (mesh_ == nullptr) return;
    if(width_==0||height_==0){return;}
    initVAO();
    //Create the FrameBuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Position buffer
    glGenTextures(1, &texturePosition);
    glBindTexture(GL_TEXTURE_2D, texturePosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width_, height_, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texturePosition, 0);

    // Normal buffer
    glGenTextures(1, &textureNormal);
    glBindTexture(GL_TEXTURE_2D, textureNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width_, height_, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textureNormal, 0);

    // color + specular color buffer
    glGenTextures(1, &textureAlbedo);
    glBindTexture(GL_TEXTURE_2D, textureAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, textureAlbedo, 0);

    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers = (_glDrawBuffers) QGLWidget::context()->getProcAddress("glDrawBuffers");
    glDrawBuffers(3,attachments);

    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width_, height_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);


    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    else
        std::cout << "Framebuffer Complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    // Buffer for the ssao
    glGenFramebuffers(1, &fboSSAO);
    glGenFramebuffers(1, &fboBlurSSAO);
    glGenFramebuffers(1, &fboBlurSSAO2);
    glBindFramebuffer(GL_FRAMEBUFFER, fboSSAO);


    // SSAO color buffer
    glGenTextures(1, &fboColorSSAO);
    glBindTexture(GL_TEXTURE_2D, fboColorSSAO);
    glBindTexture(GL_TEXTURE_2D, fboColorSSAO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width_, height_, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColorSSAO, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO buffer object NOT complete" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    else
        std::cout << "SSAO buffer object complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, fboBlurSSAO);
    glGenTextures(1, &fboColorBlurSSAO);
    glBindTexture(GL_TEXTURE_2D, fboColorBlurSSAO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width_, height_, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColorBlurSSAO, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &fboBlurSSAO1);
    glGenFramebuffers(1, &fboBlurSSAO2);

    glBindFramebuffer(GL_FRAMEBUFFER, fboBlurSSAO1);
    glGenTextures(1, &fboColorBlurSSAO1);
    glBindTexture(GL_TEXTURE_2D, fboColorBlurSSAO1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width_, height_, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColorBlurSSAO1, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //second blur pass
    glBindFramebuffer(GL_FRAMEBUFFER, fboBlurSSAO2);
    glGenTextures(1, &fboColorBlurSSAO2);
    glBindTexture(GL_TEXTURE_2D, fboColorBlurSSAO2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width_, height_, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColorBlurSSAO2, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur buffer object NOT complete" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    else
        std::cout << "SSAO Blur buffer object complete!" << std::endl;

    std::cout << width_ << "  " << height_ << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //// KERNEL
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 64; ++i)
    {
        Eigen::Vector3f sample(
                    randomFloats(generator) * 2.0 - 1.0,
                    randomFloats(generator) * 2.0 - 1.0,
                    randomFloats(generator)
                    );
        sample.normalize();
        sample *= randomFloats(generator);

        float scale = float(i) / 64.0;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    //// NOISE
    for (unsigned int i = 0; i < 16; i++)
    {
        Eigen::Vector3f noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }

    glGenTextures(1, &textureNoise);
    glBindTexture(GL_TEXTURE_2D, textureNoise);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &textureNoise);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    std::cout << "Complete" << std::endl;


    update();
}

float ViewerFRR1::lerp(float a, float b, float f)
{
    return a + f * (b - a);
}
void ViewerFRR1::importModel(){
    QString filename;

    filename = QFileDialog::getOpenFileName(this, tr("Load model"), "./",
                                            tr("PLY Files ( *.ply )"));
    if (!filename.isNull()) {
        if(!mesh_importer.LoadModel(filename))
            QMessageBox::warning(this, tr("Error"),
                                 tr("The file could not be opened"));
    }

}
void ViewerFRR1::initVAO(){
    init = true;
    // set flag for creating buffers (1st time only)
    static bool flag = 1;

    if(flag) {
        glGenBuffers(1, &facesBufferVBO_ID);
        flag = 0;
    }

    if (mesh_ == nullptr) return;

    std::vector<GLfloat> data;
    data.clear();
    for(int i =0;i<(int)mesh_->vertices_.size()/3;i++){

        data.push_back(mesh_->vertices_[3*i]);
        data.push_back(mesh_->vertices_[3*i+1]);
        data.push_back(mesh_->vertices_[3*i+2]);
        data.push_back(mesh_->normals_[3*i]);
        data.push_back(mesh_->normals_[3*i+1]);
        data.push_back(mesh_->normals_[3*i+2]);
    }

    glGenVertexArrays = (_glGenVertexArrays) QGLWidget::context()->getProcAddress("glGenVertexArrays");
    glBindVertexArray = (_glBindVertexArray) QGLWidget::context()->getProcAddress("glBindVertexArray");

    //       /* Allocate and assign a Vertex Array Object to our handle */
    glGenVertexArrays(1, &VAO_ID);

    //        /* Bind our Vertex Array Object as the current used object */
    glBindVertexArray(VAO_ID);
    glGenBuffers(1, &VBO_ID);

    //        /* Bind our first VBO as being the active buffer and storing vertex attributes (coordinates) */
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);
    glBufferData(GL_ARRAY_BUFFER,2*mesh_->vertices_.size() * sizeof(GLfloat), &data[0], GL_STATIC_DRAW);

    //        /* Specify that our coordinate data is going into attribute index 0, and contains two floats per vertex */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);

    //        /* Enable attribute index 0 as being used */
    glEnableVertexAttribArray(0);

    //        /* Specify that our normal data is going into attribute index 1, and contains three floats per vertex */
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float),(void*)(3*sizeof(float)));

    //        /* Enable attribute index 1 as being used */
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBufferVBO_ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_->faces_.size() * sizeof(int), &mesh_->faces_[0], GL_STATIC_DRAW);


}
void ViewerFRR1::drawVAO(){

    // draw triangles
    if (mesh_ == nullptr) return;
    if(!init) initVAO();
    // VAO
    glBindVertexArray = (_glBindVertexArray) QGLWidget::context()->getProcAddress("glBindVertexArray");

    glBindVertexArray(VAO_ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBufferVBO_ID);
    glDrawElements(GL_TRIANGLES,mesh_->faces_.size(), GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glBindVertexArray(0);
}

void ViewerFRR1::renderQuad(){
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);


}

void ViewerFRR1::showPasses(QGroupBox *groupBox){
    showSSAObox = new QRadioButton("SSAO");
    showSepSSAObox = new QRadioButton("SSAO separable");
    showBlurbox = new QRadioButton("Blur");
    showSepBlurbox = new QRadioButton("Blur separable");
    showLightbox = new QRadioButton("Light");


    showLightbox->setChecked(true);

    QGridLayout *layout2 = new QGridLayout;
    QLabel *passesL= new QLabel(tr("Choose what pass to show:"));
    QWidget *w1 = new QWidget();
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);

    connect(showSSAObox, SIGNAL(clicked()),this, SLOT(changePass()));
    connect(showSepSSAObox, SIGNAL(clicked()),this, SLOT(changePass()));
    connect(showBlurbox, SIGNAL(clicked()),this, SLOT(changePass()));
    connect(showSepBlurbox, SIGNAL(clicked()),this, SLOT(changePass()));
    connect(showLightbox, SIGNAL(clicked()),this, SLOT(changePass()));

    auto layout = dynamic_cast<QGridLayout*>(groupBox->layout());

    int row = layout2->rowCount() + 1;
    int headRow = row;
    row++;
    layout2->addWidget(passesL,row,0);
    row++;
    layout2->addWidget(showSSAObox,row, 0);
    layout2->addWidget(showSepSSAObox,row, 1);
    row++;
    layout2->addWidget(showBlurbox,row, 0);
    layout2->addWidget(showSepBlurbox,row, 1);
    row++;
    layout2->addWidget(showLightbox,row, 0);
    w1->setLayout(layout2);
    layout->addWidget(w1,layout->rowCount() + 1,0);
    layout->addWidget(line,headRow, 0, 1 ,layout->columnCount());

    groupBox->setLayout(layout);

}
void ViewerFRR1::changePass(){
    if(showSSAObox->isChecked()){
        useSepSSAO=false;
        useSSAObox->setChecked(true);
        useSepSSAObox->setChecked(false);
        showSSAO=true;
        showBlur=false;
        showSepSSAO=false;
        showSepBlur=false;
        showLight =false;
    }
    else if(showSepSSAObox->isChecked()){
        useSSAObox->setChecked(false);
        useSepSSAObox->setChecked(true);
        useSepSSAO=true;
        showSSAO=false;
        showBlur=false;
        showSepSSAO=true;
        showSepBlur=false;
        showLight =false;
    }
    else if(showBlurbox->isChecked()){
        useBlurbox->setChecked(true);
        useSepBlurbox->setChecked(false);
        useSepBlur=false;
        showSSAO=false;
        showBlur=true;
        showSepSSAO=false;
        showSepBlur=false;
        showLight =false;
    }
    else if(showSepBlurbox->isChecked()){
        useBlurbox->setChecked(false);
        useSepBlurbox->setChecked(true);
        useSepBlur=true;
        showSSAO=false;
        showBlur=false;
        showSepSSAO=false;
        showSepBlur=true;
        showLight =false;
    }
    else if(showLightbox->isChecked()){
        showSSAO=false;
        showBlur=false;
        showSepSSAO=false;
        showSepBlur=false;
        showLight =true;
    }

    update();
}

void ViewerFRR1::usePassesSSAO(QGroupBox *groupBox){
    useSSAObox = new QRadioButton("SSAO");
    useSepSSAObox = new QRadioButton("SSAO separable");
    useSSAObox->setChecked(true);

    QGridLayout *layout2 = new QGridLayout;
    QLabel *useL= new QLabel(tr("Choose what SSAO pass to use:"));
    QWidget *w1 = new QWidget();
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);

    connect(useSSAObox, SIGNAL(clicked()),this, SLOT(changeUseSSAO()));
    connect(useSepSSAObox, SIGNAL(clicked()),this, SLOT(changeUseSSAO()));

    auto layout = dynamic_cast<QGridLayout*>(groupBox->layout());

    int row = layout2->rowCount() + 1;
    int headRow = row;
    row++;
    layout2->addWidget(useL,row,0);
    row++;
    layout2->addWidget(useSSAObox,row, 0);
    layout2->addWidget(useSepSSAObox,row, 1);
    layout2->addWidget(line,headRow, 0, 1 ,layout->columnCount());
    w1->setLayout(layout2);
    layout->addWidget(w1,layout->rowCount() + 1,0);
    groupBox->setLayout(layout);

}
void ViewerFRR1::changeUseSSAO(){
    useSepSSAO=useSepSSAObox->isChecked();
    update();
}

void ViewerFRR1::usePassesBlur(QGroupBox *groupBox){

    useBlurbox = new QRadioButton("Blur");
    useSepBlurbox = new QRadioButton("Blur separable");
    useBlurbox->setChecked(true);

    QGridLayout *layout2 = new QGridLayout;



    QLabel *useL= new QLabel(tr("Choose what blur pass to use:"));
    QWidget *w1 = new QWidget();
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);

    connect(useSSAObox, SIGNAL(clicked()),this, SLOT(changeUseBlur()));
    connect(useSepSSAObox, SIGNAL(clicked()),this, SLOT(changeUseBlur()));


    auto layout = dynamic_cast<QGridLayout*>(groupBox->layout());

    int row = layout2->rowCount() + 1;
    int headRow = row;
    row++;
    layout2->addWidget(useL,row,0);
    row++;
    layout2->addWidget(useBlurbox,row, 0);
    layout2->addWidget(useSepBlurbox,row, 1);
    layout2->addWidget(line,headRow, 0, 1 ,layout->columnCount());

    w1->setLayout(layout2);
    layout->addWidget(w1,layout->rowCount() + 1,0);
    groupBox->setLayout(layout);

}
void ViewerFRR1::changeUseBlur(){
    useSepBlur=useSepBlurbox->isChecked();
    //initVertexBuffer();
    update();
}
void ViewerFRR1::radiusAndBias(QGroupBox *groupBox){

    QLabel *radiusL= new QLabel(tr("Radius:"));
    QLabel *biasL= new QLabel(tr("Bias:"));

    QDoubleSpinBox *radiusS = new QDoubleSpinBox;
    radiusS->setRange(0.0, 800.0);
    radiusS->setSingleStep(1);
    radiusS->setValue((double)radi);
    QDoubleSpinBox *biasS= new QDoubleSpinBox;
    biasS->setRange(0.0,0.5);
    biasS->setSingleStep(0.0001);
    biasS->setValue((double)bias);
    biasS->setDecimals(4);
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    QLabel *title = new QLabel("SSAO:");


    connect(radiusS, SIGNAL(valueChanged(double)),this, SLOT(setRadi(double)));
    connect(biasS, SIGNAL(valueChanged(double)),this, SLOT(setBias(double)));
    auto layout = dynamic_cast<QGridLayout*>(groupBox->layout());

    int row = layout->rowCount() + 1;
    int headRow = row;
    row++;
    layout->addWidget(title,row,0);
    row++;
    layout->addWidget(radiusL,row, 0);
    layout->addWidget(radiusS,row, 1);
    row++;
    layout->addWidget(biasL,row, 0);
    layout->addWidget(biasS,row, 1);
    layout->addWidget(line,headRow, 0, 1 ,layout->columnCount());

    groupBox->setLayout(layout);

}
void ViewerFRR1::setBias(double b){
    bias=(float)b;
    update();
}
void ViewerFRR1::setRadi(double r){
    radi=(float)r;
    update();
}

void ViewerFRR1::showFramerate(QGroupBox *groupBox){
    m_fpsCount = new QLabel("0");
    QFrame* line = new QFrame();

    line->setFrameShape(QFrame::HLine);
    QLabel *title = new QLabel(" FPS:");

    auto layout = dynamic_cast<QGridLayout*>(groupBox->layout());

    int row = layout->rowCount() + 1;
    int headRow = row;
    row++;
    layout->addWidget(title,row,0);
 //   row++;
    layout->addWidget(m_fpsCount,row, 1);

    layout->addWidget(line,headRow, 0, 1 ,layout->columnCount());

    groupBox->setLayout(layout);
}
