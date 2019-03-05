// ===============================================================
// Computer Graphics Homework Solutions
// Copyright (C) 2017 by George Wolberg
//
// HW2a.cpp - HW2a class
//
// Written by: George Wolberg, 2017
// ===============================================================

#include "HW2a.h"

// shader ID
enum { HW2A };

// uniform ID
enum { PROJ, VIEW, MODEL, NORMAL };

const int DrawModes[] = {
	GL_POINTS,
	GL_LINES,
	GL_LINE_STRIP,
	GL_LINE_LOOP,
	GL_TRIANGLES,
	GL_TRIANGLE_STRIP,
	GL_TRIANGLE_FAN,
	GL_QUADS,
	GL_POLYGON
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2a::HW2a:
//
// HW2a constructor.
//
HW2a::HW2a(const QGLFormat &glf, QWidget *parent) : HW(glf, parent)
{
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2a::initializeGL:
//
// Initialization routine before display loop.
// Gets called once before the first time resizeGL() or paintGL() is called.
//
void
HW2a::initializeGL()
{
	// initialize GL function resolution for current context
	initializeGLFunctions();

	// init vertex and fragment shaders
	initShaders();

    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    //glEnable(GL_DEPTH_TEST);

	// initialize vertex buffer and write positions to vertex shader
    //initVertexBuffer();

	// init state variables
    glClearColor(1.0, 1.0, 1.0, 1.0);	// set background color
	glColor3f   (1.0, 1.0, 0.0);		// set foreground color
    initialized_ = true;

}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2a::resizeGL:
//
// Resize event handler.
// The input parameters are the window width (w) and height (h).
//
void
HW2a::resizeGL(int w, int h)
{
    if (h == 0) h = 1;
    width_ = w;
    height_ = h;

    camera_.SetViewport(0, 0, w, h);
    camera_.SetProjection(kFieldOfView, kZNear, kZFar);

    m_winW = w;
    m_winH = h;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2a::paintGL:
//
// Update GL scene.
//
void
HW2a::paintGL()
{
	// clear canvas with background color
    //glClear(GL_COLOR_BUFFER_BIT);
    //glUseProgram(m_program[HW2A].programId());
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (initialized_) {
        camera_.SetViewport();

        Eigen::Matrix4f projection = camera_.SetProjection();
        Eigen::Matrix4f view = camera_.SetView();
        Eigen::Matrix4f model = camera_.SetModel();

        Eigen::Matrix4f t = view * model;
        Eigen::Matrix3f normal;
        for (int i = 0; i < 3; ++i)
          for (int j = 0; j < 3; ++j) normal(i, j) = t(i, j);

        normal = normal.inverse().transpose();

        if (mesh_ != nullptr) {
            m_program[HW2A].bind();
            GLuint projection_location = m_program[HW2A].uniformLocation("projection");
            glUniformMatrix4fv(projection_location, 1, GL_FALSE, projection.data());

            GLuint view_location = m_program[HW2A].uniformLocation("view");
            glUniformMatrix4fv(view_location, 1, GL_FALSE, view.data());

            GLuint model_location = m_program[HW2A].uniformLocation("model");
            glUniformMatrix4fv(model_location, 1, GL_FALSE, model.data());

            GLuint normal_matrix_location =
                m_program[HW2A].uniformLocation("normal_matrix");
            glUniformMatrix3fv(normal_matrix_location, 1, GL_FALSE, normal.data());


            glDrawArrays(GL_TRIANGLES, 0, mesh_->vertices_.size()/3);
            // TODO: Implement model rendering.

            /*glBegin(GL_TRIANGLES);
            const int kFaces = mesh_->faces_.size() / 3;
            for (int i = 0; i < kFaces; ++i) {
              for (int j = 0; j < 3; ++j) {
                const int kVidx = mesh_->faces_[i * 3 + j];
                glVertex3f(mesh_->vertices_[kVidx * 3],
                           mesh_->vertices_[kVidx * 3 + 1],
                           mesh_->vertices_[kVidx * 3 + 2]);
              }
            }
            glEnd();*/
        }

    }
    
    // use glsl program

    glFlush();
    
    // terminate program; rendering is done
    glUseProgram(0);
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2a::controlPanel:
//
// Create control panel groupbox.
//
QGroupBox*
HW2a::controlPanel()
{
	// init group box
    QGroupBox *groupBox = HW::controlPanel();
    groupBox->setStyleSheet(GroupBoxStyle);
    HW::AddImportExportPLY(groupBox);
	return(groupBox);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2a::reset:
//
// function to reset parameters.
//
void
HW2a::reset()
{
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2a::initShaders:
//
// Initialize shaders.
//
void
HW2a::initShaders()
{
	UniformMap uniforms;
    uniforms["projection"] = PROJ;
    uniforms["view"] = VIEW;
    uniforms["model"] = MODEL;
    uniforms["normal_matrix"]= NORMAL;

	// compile shader, bind attribute vars, link shader, and initialize uniform var table
    initShader(HW2A, QString(":/hw2/try.vert"), QString(":/hw2/try.frag"), uniforms);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2a::initVertexBuffer:
//
// Initialize vertex buffer.
//
void
HW2a::initVertexBuffer()
{
    m_vertNum = mesh_->vertices_.size();
    
	// create a vertex buffer
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);

	// bind vertex buffer to the GPU and copy the vertices from CPU to GPU
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, mesh_->vertices_.size()/3*sizeof(float), &mesh_->vertices_[0], GL_STATIC_DRAW);

	// enable vertex buffer to be accessed via the attribute vertex variable and specify data format
	glEnableVertexAttribArray(ATTRIB_VERTEX);
    glVertexAttribPointer	 (ATTRIB_VERTEX, 3, GL_FLOAT, false, 0, 0);
}
