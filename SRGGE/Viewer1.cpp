#include "Viewer1.h"
#include "Helpers/mesh_importer.h"
#include <QOpenGLFunctions>
#include <QtOpenGL>
#include <vector>
#include <Eigen>
#include <QOpenGLFunctions>
#include <QtOpenGL>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <QRadioButton>
#include <QLineEdit>



//shader ID
enum {VIEW1};

Viewer1::Viewer1(const QGLFormat &glf, QWidget *parent) : HW(glf, parent) , mesh_importer(this)
{
    // init vars
    setFocusPolicy(Qt::StrongFocus);
}

void Viewer1::initializeGL()
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

    if(!timer.isCreated()){
        timer.create();
    }

}

void Viewer1::resizeGL(int w, int h)
{

    if (h == 0) h = 1;
    width_ = w;
    height_ = h;

    camera_.SetViewport(0, 0, w, h);
    camera_.SetProjection(kFieldOfView, kZNear, kZFar);

    m_winW = w;
    m_winH = h;

    return;
}

void Viewer1::paintGL()
{

     if(timer.isCreated()) timer.begin();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use glsl program
    glUseProgram(m_program[VIEW1].programId());

    // pass the following parameters to vertex the shader:
    // projection matrix, modelview matrix, theta, and twist
    camera_.SetViewport();

    Eigen::Matrix4f projection = camera_.SetProjection();
    Eigen::Matrix4f view = camera_.SetView();
    Eigen::Matrix4f model = camera_.SetModel();

    Eigen::Matrix4f t = view * model;
    Eigen::Matrix3f normal;
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) normal(i, j) = t(i, j);

    normal = normal.inverse().transpose();

    glUniformMatrix4fv(const_uniform[VIEW1][CONST_UNIFORM_PROJ], 1, GL_FALSE, projection.data());
    glUniformMatrix4fv(const_uniform[VIEW1][CONST_UNIFORM_MODEL], 1, GL_FALSE, model.data());
    glUniformMatrix4fv(const_uniform[VIEW1][CONST_UNIFORM_VIEW], 1, GL_FALSE, view.data());
    glUniformMatrix3fv(const_uniform[VIEW1][CONST_UNIFORM_NORMAL], 1, GL_FALSE, normal.data());

    const Eigen::Affine3f piRotation(Eigen::AngleAxisf(M_PI, Eigen::Vector3f(0.0, 1.0, 0.0)));
    model = piRotation* model ;
    // draw triangles
    if (mesh_ == nullptr) return;



        //    /***************VBO**************************************/
 if(!m_vao){

            glBindBuffer(GL_ARRAY_BUFFER, vertexBufferVBO_ID);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, NULL);
            glBindBuffer(GL_ARRAY_BUFFER, normalBufferVBO_ID);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, NULL);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,facesBufferVBO_ID);
           Eigen::Matrix4f translation;
           Eigen::Affine3f kSc (Eigen::Scaling(Eigen::Vector3f(1.5/N,1.5/N,1.5/N)));
           for(float k =0;k<N;k++){
               for(float l =0;l<N;l++){

               Eigen::Affine3f kTran(Eigen::Translation3f(Eigen::Vector3f((k/N+(k+1)/N)/2*cos(M_PI*k)+0.5/N*cos(M_PI*N),(l/N+(l+1)/N)/2*cos(M_PI*l)+0.5/N*cos(M_PI*N), 0)));
               translation = (kTran.matrix()*kSc* model);
               glUniformMatrix4fv(const_uniform[VIEW1][CONST_UNIFORM_MODEL], 1, GL_FALSE,translation.data());

               glDrawElements(GL_TRIANGLES,mesh_->faces_.size(), GL_UNSIGNED_INT, 0);
           }}
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

 }else{
// VAO
    glBindVertexArray = (_glBindVertexArray) QGLWidget::context()->getProcAddress("glBindVertexArray");

        glBindVertexArray(VAO_ID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBufferVBO_ID);
        Eigen::Matrix4f translation;
        Eigen::Affine3f kSc (Eigen::Scaling(Eigen::Vector3f(1.5/N,1.5/N,1.5/N)));
        for(float k =0;k<N;k++){
            for(float l =0;l<N;l++){

            Eigen::Affine3f kTran(Eigen::Translation3f(Eigen::Vector3f((k/N+(k+1)/N)/2*cos(M_PI*k)+0.5/N*cos(M_PI*N),(l/N+(l+1)/N)/2*cos(M_PI*l)+0.5/N*cos(M_PI*N), 0)));
            translation = (kTran.matrix()*kSc* model);
            glUniformMatrix4fv(const_uniform[VIEW1][CONST_UNIFORM_MODEL], 1, GL_FALSE,translation.data());

            glDrawElements(GL_TRIANGLES,mesh_->faces_.size(), GL_UNSIGNED_INT, 0);
        }}
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
        glBindVertexArray(0);
}
    // terminate program; rendering is done
    glUseProgram(0);


    if(timer.isCreated()){
         timer.end();
         resTime = timer.waitForResult();
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

QGroupBox* Viewer1::controlPanel()
{
    // init group box
    QGroupBox *groupBox = HW::controlPanel();
    groupBox->setStyleSheet(GroupBoxStyle);
    mesh_importer.AddImportExportPLY(groupBox);
    AddChooseBuffer(groupBox);
    implementNxN(groupBox);
    showFramerate(groupBox);
    //HW::AddImportExportPLY(groupBox);

    return(groupBox);
}

void Viewer1::reset()
{
    mesh_ = nullptr;	// recompute geometry
    initVertexBuffer();
    // draw
    updateGL();
}

void Viewer1::initShaders()
{
    // init uniforms hash table based on uniform variable names and location IDs
    UniformMap uniforms;

    // compile shader, bind attribute vars, link shader, and initialize uniform var table
    initShader(VIEW1, QString(":/hw2/try.vert"), QString(":/hw2/try.frag"), uniforms);
}

void Viewer1::initVertexBuffer()
{
    // set flag for creating buffers (1st time only)
    static bool flag = 1;

    if(flag) {
        glGenBuffers(1, &vertexBufferVBO_ID);
        glGenBuffers(1, &normalBufferVBO_ID);
        glGenBuffers(1, &facesBufferVBO_ID);
        flag = 0;
    }

    if (mesh_ == nullptr) return;
// VBO
    if(!m_vao){
        glBindVertexArray = (_glBindVertexArray) QGLWidget::context()->getProcAddress("glBindVertexArray");

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferVBO_ID);
        glBufferData(GL_ARRAY_BUFFER, mesh_->vertices_.size() * sizeof(float), &mesh_->vertices_[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, normalBufferVBO_ID);
        glBufferData(GL_ARRAY_BUFFER, mesh_->normals_.size() * sizeof(int), &mesh_->normals_[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBufferVBO_ID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_->faces_.size() * sizeof(int), &mesh_->faces_[0], GL_STATIC_DRAW);
    }

//   VAO
else{
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



    updateGL();



}


void Viewer1::importModel(){
    QString filename;

      filename = QFileDialog::getOpenFileName(this, tr("Load model"), "./",
                                              tr("PLY Files ( *.ply )"));
      if (!filename.isNull()) {
          if(!mesh_importer.LoadModel(filename))
            QMessageBox::warning(this, tr("Error"),
                               tr("The file could not be opened"));
      }

}

void Viewer1::AddChooseBuffer(QGroupBox*groupBox){

QRadioButton *buttonVBO = new QRadioButton("VBO");
QRadioButton *buttonVAO = new QRadioButton("VAO");
buttonVBO->setChecked(true);
QFrame* line = new QFrame();
line->setFrameShape(QFrame::HLine);
auto layout = dynamic_cast<QGridLayout*>(groupBox->layout());

int row = layout->rowCount() + 1;

QLabel *titleB = new QLabel(" Choose: ");

HW::connect(buttonVAO, SIGNAL(clicked()), this,SLOT(switchBuffer()));
HW::connect(buttonVBO, SIGNAL(clicked()), this,SLOT(switchBuffer()));
layout->addWidget(line,row,0, 1 ,layout->columnCount());
row++;
layout->addWidget(titleB,row,0);
row++;
layout->addWidget(buttonVBO,row,0);
layout->addWidget(buttonVAO,row,1);
groupBox->setLayout(layout);
}
void Viewer1::switchBuffer(){
    m_vao= !m_vao;
    initVertexBuffer();
}

void Viewer1::implementNxN(QGroupBox*groupBox){
    QPushButton *buttonOK  = new QPushButton("OK");
    echoLineEdit->setPlaceholderText("Introduce N");
    echoLineEdit->setFocus();

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    auto layout = dynamic_cast<QGridLayout*>(groupBox->layout());

    int row = layout->rowCount() + 1;

    QLabel *titleB = new QLabel(" NxN ");

    layout->addWidget(line,row,0, 1 ,layout->columnCount());
    row++;
    layout->addWidget(titleB,row,0);
    row++;
    layout->addWidget(echoLineEdit,row,0);
    layout->addWidget(buttonOK,row,1);
    HW::connect(buttonOK, SIGNAL(clicked()), this,SLOT(getLineText()));
}

void Viewer1::getLineText(){
    N=(echoLineEdit->text()).toFloat();
    mesh_importer.updateNumVert((int)N);
    initVertexBuffer();
}

void Viewer1::showFramerate(QGroupBox *groupBox){
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
