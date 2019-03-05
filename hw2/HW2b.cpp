// ===============================================================
// Computer Graphics Homework Solutions
// Copyright (C) 2017 by George Wolberg
//
// HW2b.cpp - HW2b class
//
// Written by: George Wolberg, 2017
// ===============================================================

#include "HW2b.h"

// shader ID
enum {HW2B};

// uniform ID
enum {
	PROJ,
    VIEW,
    MODEL,
    NORMAL
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2b::HW2b:
//
// HW2b constructor.
//
HW2b::HW2b(const QGLFormat &glf, QWidget *parent) : HW(glf, parent)
{
	// init vars
    setFocusPolicy(Qt::StrongFocus);
    m_subdivisions = 4;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2b::initializeGL:
//
// Initialization routine before display loop.
// Gets called once before the first time resizeGL() or paintGL() is called.
//
void
HW2b::initializeGL()
{
	// initialize GL function resolution for current context
	initializeGLFunctions();

	// init vertex and fragment shaders
	initShaders();

   // glEnable(GL_NORMALIZE);
   // glEnable(GL_CULL_FACE);
   // glCullFace(GL_BACK);
   // glEnable(GL_DEPTH_TEST);


	// initialize vertex buffer and write positions to vertex shader
	initVertexBuffer();
    
	// init state variables
    glClearColor(1.0, 1.0, 1.0, 1.0);	// set background color
	glColor3f   (1.0, 1.0, 0.0);		// set foreground color
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2b::resizeGL:
//
// Resize event handler.
// The input parameters are the window width (w) and height (h).
//
void
HW2b::resizeGL(int w, int h)
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



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2b::paintGL:
//
// Update GL scene.
//
void
HW2b::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use glsl program
    glUseProgram(m_program[HW2B].programId());
    
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

    glUniformMatrix4fv(const_uniform[HW2B][CONST_UNIFORM_PROJ], 1, GL_FALSE, projection.data());
    glUniformMatrix4fv(const_uniform[HW2B][CONST_UNIFORM_MODEL], 1, GL_FALSE, model.data());
    glUniformMatrix4fv(const_uniform[HW2B][CONST_UNIFORM_VIEW], 1, GL_FALSE, view.data());
    glUniformMatrix3fv(const_uniform[HW2B][CONST_UNIFORM_NORMAL], 1, GL_FALSE, normal.data());
    
    
    // draw triangles
    glDrawArrays(GL_TRIANGLES, 0, m_numPoints);
    
    // terminate program; rendering is done
    glUseProgram(0);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2b::controlPanel:
//
// Create control panel groupbox.
//
QGroupBox* HW2b::controlPanel()
{
	// init group box
    QGroupBox *groupBox = HW::controlPanel();
	groupBox->setStyleSheet(GroupBoxStyle);

	// create labels
    QLabel *label[1];
    label[0] = new QLabel("Subdivide");
	m_sliderSubdiv = new QSlider(Qt::Horizontal);

    m_sliderSubdiv->setRange(0, 5);
	m_sliderSubdiv->setValue(m_subdivisions);

	m_spinBoxSubdiv = new QSpinBox;
    m_spinBoxSubdiv->setRange(0, 5);
	m_spinBoxSubdiv->setValue(m_subdivisions);

	// layout for assembling widgets
    auto layout = dynamic_cast<QGridLayout*>(groupBox->layout());
    layout->addWidget(label[0],	   0, 0);
    layout->addWidget(m_sliderSubdiv,  0, 1);
    layout->addWidget(m_spinBoxSubdiv, 0, 2);

	// assign layout to group box
	groupBox->setLayout(layout);

	// init signal/slot connections
	connect(m_sliderSubdiv,  SIGNAL(valueChanged(int)), this, SLOT(changeSubdiv(int)));
	connect(m_spinBoxSubdiv, SIGNAL(valueChanged(int)), this, SLOT(changeSubdiv(int)));

	return(groupBox);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2b::reset:
//
// Reset parameters.
//
void
HW2b::reset()
{
	// reset parameters
	m_subdivisions	= 4;


	m_sliderSubdiv->blockSignals(true);
	m_sliderSubdiv->setValue(m_subdivisions);
	m_sliderSubdiv->blockSignals(false);

	m_spinBoxSubdiv->blockSignals(true);
	m_spinBoxSubdiv->setValue(m_subdivisions);
	m_spinBoxSubdiv->blockSignals(false);
    mesh_ = nullptr;	// recompute geometry
	initVertexBuffer();

	// draw
	updateGL();
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2b::initShaders:
//
// Initialize vertex and fragment shaders.
//
void
HW2b::initShaders()
{

	// init uniforms hash table based on uniform variable names and location IDs
    UniformMap uniforms;

	// compile shader, bind attribute vars, link shader, and initialize uniform var table
    initShader(HW2B, QString(":/hw2/try.vert"), QString(":/hw2/try.frag"), uniforms);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2b::initVertexBuffer:
//
// Initialize vertex buffer.
//
void
HW2b::initVertexBuffer()
{
	// set flag for creating buffers (1st time only)
	static bool flag = 1;

	// verify that we have valid vertex/color buffers
	static GLuint vertexBuffer = -1;
	static GLuint colorBuffer  = -1;
	if(flag) {      // create vertex and color buffers
		glGenBuffers(1, &vertexBuffer);
		glGenBuffers(1, &colorBuffer );
		flag = 0;       // reset flag
	}

    if (mesh_ != nullptr) {

    }
    else{
        // init geometry data
        const vec2 vertices[] = {
            vec2( 10.0f,   10.75f ),
            vec2( 10.65f, -0.375f),
            vec2(-10.65f, -0.375f)
        };

        Eigen::Vector3f v1(-10,-10,-10);
        Eigen::Vector3f v2(10,10,10);

        camera_.UpdateModel(v1, v2);

        // recursively subdivide triangle into triangular facets;
        // store vertex positions and colors in m_points and m_colors, respectively
        divideTriangle(vertices[0], vertices[1], vertices[2], m_subdivisions);

        m_numPoints = (int) m_points.size();

        // save number of vertices
        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_numPoints*sizeof(vec3), &m_colors[0], GL_STATIC_DRAW);

            // enable the assignment of attribute color variable
        glEnableVertexAttribArray(ATTRIB_COLOR);

            // assign the buffer object to the attribute color variable
         glVertexAttribPointer(ATTRIB_COLOR, 3, GL_FLOAT, false, 0, NULL);
    }

    m_numPoints = (int) m_points.size();


    // bind vertex buffer to the GPU and copy the vertices from CPU to GPU
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_numPoints*sizeof(vec3), &m_points[0], GL_STATIC_DRAW);

    // enable the assignment of attribute vertex variable
    glEnableVertexAttribArray(ATTRIB_VERTEX);

    // assign the buffer object to the attribute vertex variable
    glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, false, 0, NULL);

    // bind color buffer to the GPU and copy the colors from CPU to GPU
    /* */

    // clear vertex and color vectors because they have already been copied into GPU

    m_points.clear();
    m_colors.clear();

    updateGL();
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2b::divideTriangle:
//
// Recursive subdivision of triangle (a,b,c). Recurse count times.
//
void
HW2b::divideTriangle(vec2 a, vec2 b, vec2 c, int count)
{
    if(count > 0) {
        // find midpoint vertices of triangle
        vec2 ab = vec2((a[0] + b[0]) / 2.0, (a[1]+b[1]) / 2.0);
        vec2 ac = vec2((a[0] + c[0]) / 2.0, (a[1]+c[1]) / 2.0);
        vec2 bc = vec2((b[0] + c[0]) / 2.0, (b[1]+c[1]) / 2.0);
        // create the triangles out of the midpoints, count - 1 times.
        divideTriangle( a, ab, ac, count - 1);
        divideTriangle( b, bc, ab, count - 1);
        divideTriangle( c, ac, bc, count - 1);
        divideTriangle(ab, ac, bc, count - 1);
    } else{
        vec3 a3(a[0],a[1], 1);
        vec3 b3(b[0], b[1], 1);
        vec3 c3(c[0], c[1], 1);
        triangle(a3, b3, c3);
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2b::triangle:
//
// Push positions and colors of the three triangle vertices.
//
void
HW2b::triangle(vec3 v1, vec3 v2, vec3 v3)
{
	// init geometry
	m_points.push_back(v1);
	m_points.push_back(v2);
	m_points.push_back(v3);

	// init color
	float r = (float) rand() / RAND_MAX;
	float g = (float) rand() / RAND_MAX;
	float b = (float) rand() / RAND_MAX;
	m_colors.push_back(vec3(r, g, b));
	m_colors.push_back(vec3(r, g, b));
	m_colors.push_back(vec3(r, g, b));
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW2b::changeSubdiv:
//
// Slot function to change number of recursive subdivisions.
//
void
HW2b::changeSubdiv(int subdivisions)
{
    if(mesh_ != nullptr) return;

	// update slider and spinbox
	m_sliderSubdiv->blockSignals(true);
	m_sliderSubdiv->setValue(subdivisions);
	m_sliderSubdiv->blockSignals(false);

	m_spinBoxSubdiv->blockSignals(true);
	m_spinBoxSubdiv->setValue(subdivisions);
	m_spinBoxSubdiv->blockSignals(false);

	// init vars
	m_subdivisions = subdivisions;

	// compute new vertices and colors
	initVertexBuffer();

	// draw
	updateGL();
}

