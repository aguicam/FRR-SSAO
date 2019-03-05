#ifndef VIEWER1_H
#define VIEWER1_H

#include "HW.h"
#include "Helpers/mesh_importer.h"
#include <iostream>
#include <iomanip>

class Viewer1 : public HW
{
    Q_OBJECT
public:
    Viewer1		(const QGLFormat &glf, QWidget *parent = 0);
    QGroupBox*	controlPanel	();		// create control panel
    void		reset		();		// reset parameters
    void		initVertexBuffer();		// init vertices
    void		initShaders	();		// init shaders
    void        AddChooseBuffer(QGroupBox*groupBox);
    void        implementNxN(QGroupBox*groupBox);
    void        showFramerate(QGroupBox *groupBox);

protected:
    void		initializeGL	();		// init GL state
    void		resizeGL	(int, int);	// resize GL widget
    void		paintGL		();		// render GL scene
    void		divideTriangle(vec2, vec2, vec2, int);	// subdivide triangle
    void		triangle(vec3, vec3, vec3);	// process single triangle

private:
    GLuint        vertexBufferVBO_ID;
    GLuint        normalBufferVBO_ID;
    GLuint        facesBufferVBO_ID;
    GLuint        VAO_ID;
    GLuint        VBO_ID;
    typedef void (APIENTRY *_glGenVertexArrays) (GLsizei, GLuint*);
    typedef void (APIENTRY *_glBindVertexArray) (GLuint);
    _glGenVertexArrays glGenVertexArrays;
    _glBindVertexArray glBindVertexArray;
    bool m_vao=false;
    MI            mesh_importer;
    QLineEdit *echoLineEdit = new QLineEdit;
    float N=1;
    QLabel *m_fpsCount;
    QString fpsString;

    QOpenGLTimerQuery timer;
    GLuint64 resTime;
    float fps;


public slots:
    void importModel ();
    void switchBuffer();
    void getLineText();
};

#endif // VIEWER1_H
