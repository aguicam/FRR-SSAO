// ===============================================================
// Computer Graphics Homework Solutions
// Copyright (C) 2017 by George Wolberg
//
// HW.cpp - HW class. Base class of homework solutions.
//
// Written by: George Wolberg, 2017
// ===============================================================

#include "HW.h"


QString GroupBoxStyle = "QGroupBox {				\
			border: 1px solid gray;			\
			border-radius: 9px;			\
			font-weight: bold;			\
			margin-top: 0.5em;}			\
			QGroupBox::title {			\
			subcontrol-origin: margin;		\
			left: 10px;				\
			padding: 0 3px 0 3px;			\
			}";


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW::HW:
//
// HW constructor.
// This is base class for homework solutions that will replace
// the control panel, reset function, and add homework solution. 
//
HW::HW(const QGLFormat &glf, QWidget *parent) : QGLWidget (glf, parent),
    initialized_(false), width_(0.0), height_(0.0)
{
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW::controlPanel:
//
// Create a control panel of widgets for homework solution.
//
QGroupBox*
HW::controlPanel()
{
    //ELEMENTS ON TOP
    QGroupBox *groupBox = new QGroupBox("MIRI: CAM");
    groupBox->setStyleSheet(GroupBoxStyle);
    QGridLayout *layout = new QGridLayout;
    groupBox->setLayout(layout);
    return(groupBox);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW::reset:
//
// Reset parameters in control panel.
//
void
HW::reset() {}

void
HW::initVertexBuffer(){

}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW::initShader:
//
// Initialize vertex and fragment shaders.
//
void
HW::initShader(int shaderID, QString vshaderName, QString fshaderName, UniformMap uniforms)
{
    UniformMap constUniforms;
    constUniforms["u_projection"] = CONST_UNIFORM_PROJ;
    constUniforms["u_view"     ] = CONST_UNIFORM_VIEW;
    constUniforms["u_model"     ] = CONST_UNIFORM_MODEL;
    constUniforms["u_normal_matrix"     ] = CONST_UNIFORM_NORMAL;

	// due to bug in Qt, in order to use higher OpenGL version (>2.1), we need to add lines
	// up to initShader() to render properly


	typedef void (APIENTRY *_glGenVertexArrays) (GLsizei, GLuint*);
	typedef void (APIENTRY *_glBindVertexArray) (GLuint);

	_glGenVertexArrays glGenVertexArrays;
	_glBindVertexArray glBindVertexArray;

	glGenVertexArrays = (_glGenVertexArrays) QGLWidget::context()->getProcAddress("glGenVertexArrays");
	glBindVertexArray = (_glBindVertexArray) QGLWidget::context()->getProcAddress("glBindVertexArray");

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// compile vertex shader
	bool flag = m_program[shaderID].addShaderFromSourceFile(QGLShader::Vertex, vshaderName);
	if(!flag) {
		QMessageBox::critical(0, "Error", "Vertex shader error: " + vshaderName + "\n" +
					m_program[shaderID].log(), QMessageBox::Ok);
		exit(-1);
	}

	// compile fragment shader
	if(!m_program[shaderID].addShaderFromSourceFile(QGLShader::Fragment, fshaderName)) {
		QMessageBox::critical(0, "Error", "Fragment shader error: " + fshaderName + "\n" +
					m_program[shaderID].log(), QMessageBox::Ok);
		exit(-1);
	}

	// bind the attribute variable in the glsl program with a generic vertex attribute index;
    // values provided via ATTRIB_VERTEX will modify the value of "vert")
    //
    glBindAttribLocation(m_program[shaderID].programId(), ATTRIB_VERTEX,	"vert");
    glBindAttribLocation(m_program[shaderID].programId(), ATTRIB_NORMAL,	"normal"   );
    //glBindAttribLocation(m_program[shaderID].programId(), ATTRIB_TEXCOORD,	"TexCoord");
    //glBindAttribLocation(m_program[shaderID].programId(), ATTRIB_COLOR,	"color"  );

	// link shader pipeline; attribute bindings go into effect at this point
	if(!m_program[shaderID].link()) {
		QMessageBox::critical(0, "Error", "Could not link shader: " + vshaderName + "\n" +
					m_program[shaderID].log(), QMessageBox::Ok);
		qDebug() << m_program[shaderID].log();
		exit(-1);
    }

    for(std::map<QString, GLuint>::iterator iter = constUniforms.begin(); iter != constUniforms.end(); ++iter) {
        QString uniformName = iter->first;
        GLuint  uniformID   = iter->second;
        // get storage location
        const_uniform[shaderID][uniformID]=glGetUniformLocation(m_program[shaderID].programId(),
                      uniformName.toStdString().c_str());

        if(const_uniform[shaderID][uniformID] < 0) {
            qDebug() << m_program[shaderID].log();
            qDebug() <<fshaderName;
            qDebug() << "Failed to get the storage location of " + uniformName;
        }
    }

	// iterate over all uniform variables; map each uniform name to shader location ID
	for(std::map<QString, GLuint>::iterator iter = uniforms.begin(); iter != uniforms.end(); ++iter) {
		QString uniformName = iter->first;
		GLuint  uniformID   = iter->second;

		// get storage location
		m_uniform[shaderID][uniformID]=glGetUniformLocation(m_program[shaderID].programId(),
			  uniformName.toStdString().c_str());
		if(m_uniform[shaderID][uniformID] < 0) {
            qDebug() << fshaderName;
			qDebug() << "Failed to get the storage location of " + uniformName;
		}
	}
}



//
// CAMERA

void HW::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Up) camera_.Zoom(-1);
  if (event->key() == Qt::Key_Down) camera_.Zoom(1);

  if (event->key() == Qt::Key_Left) camera_.Rotate(-1);
  if (event->key() == Qt::Key_Right) camera_.Rotate(1);

  if (event->key() == Qt::Key_W) camera_.Zoom(-1);
  if (event->key() == Qt::Key_S) camera_.Zoom(1);

  if (event->key() == Qt::Key_A) camera_.Rotate(-1);
  if (event->key() == Qt::Key_D) camera_.Rotate(1);

  updateGL();
}


void HW::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    camera_.StartRotating(event->x(), event->y());
  }
  if (event->button() == Qt::RightButton) {
    camera_.StartZooming(event->x(), event->y());
  }
  updateGL();
}

void HW::mouseMoveEvent(QMouseEvent *event) {
  camera_.SetRotationX(event->y());
  camera_.SetRotationY(event->x());
  camera_.SafeZoom(event->y());
  updateGL();
}

void HW::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    camera_.StopRotating(event->x(), event->y());
  }
  if (event->button() == Qt::RightButton) {
    camera_.StopZooming(event->x(), event->y());
  }
  updateGL();
}
