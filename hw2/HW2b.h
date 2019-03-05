// ======================================================================
// Computer Graphics Homework Solutions
// Copyright (C) 2017 by George Wolberg
//
// HW2b.h - Header file for HW2b class
//
// Written by: George Wolberg, 2017
// ======================================================================

#ifndef HW2B_H
#define HW2B_H


#include "HW.h"


// ----------------------------------------------------------------------
// standard include files
//

class HW2b : public HW 
{
    Q_OBJECT
public:
	// constructor
	HW2b		(const QGLFormat &glf, QWidget *parent = 0);
	QGroupBox*	controlPanel	();		// create control panel
	void		reset		();		// reset parameters
	void		initVertexBuffer();		// init vertices
	void		initShaders	();		// init shaders

public slots:
	void		changeSubdiv	(int);		// respond to subdivision slider


protected:
	void		initializeGL	();		// init GL state
	void		resizeGL	(int, int);	// resize GL widget
	void		paintGL		();		// render GL scene
	void		divideTriangle(vec2, vec2, vec2, int);	// subdivide triangle
    void		triangle(vec3, vec3, vec3);	// process single triangle

private:
    int		  m_subdivisions;		// triangle subdivisions
	QSlider		 *m_sliderSubdiv;		// subdivision slider
	QSpinBox	 *m_spinBoxSubdiv;		// subdivision spinbox
	int		  m_numPoints;			// number of 2D points
	QMatrix4x4	  m_modelview;			// 4x4 modelview  matrix
	QMatrix4x4	  m_projection;			// 4x4 projection matrix
	GLuint		  m_vertexBuffer;		// handle to vertex buffer
	GLuint		  m_colorBuffer;		// handle to color buffer
};

#endif // HW2B_H
