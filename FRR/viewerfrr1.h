#ifndef VIEWERFRR1_H
#define VIEWERFRR1_H

#include "HW.h"
#include "Helpers/mesh_importer.h"
#include <iostream>
#include <random>

#include <QOpenGLFunctions>
#include <QtOpenGL>
#include <QSpinBox>
#include <QCheckBox>
#include <iostream>
#include <iomanip>

class ViewerFRR1 : public HW
{
    Q_OBJECT
public:
    ViewerFRR1	(const QGLFormat &glf, QWidget *parent = 0);
    QGroupBox*	controlPanel	();		// create control panel
    void		reset		();		// reset parameters
    void		initVertexBuffer();		// init vertices
    void		initShaders	();		// init shaders

protected:
    void		initializeGL	();		// init GL state
    void		resizeGL	(int, int);	// resize GL widget
    void		paintGL		();		// render GL scene
    void		divideTriangle(vec2, vec2, vec2, int);	// subdivide triangle
    void		triangle(vec3, vec3, vec3);	// process single triangle
    void        initVAO();
    void        drawVAO();
    void        renderQuad();
    float       lerp(float, float, float);
    void        radiusAndBias(QGroupBox *groupBox);
    void        showPasses(QGroupBox *groupBox);
    void        usePasses(QGroupBox *groupBox);
    void        usePassesSSAO(QGroupBox *groupBox);
    void        usePassesBlur(QGroupBox *groupBox);
    void        showFramerate(QGroupBox *groupBox);
    void        showNoBlurSSAO(QGroupBox *groupBox);


private:
    GLuint        facesBufferVBO_ID;
    GLuint        VAO_ID;
    GLuint        VBO_ID;
    typedef void (APIENTRY *_glGenVertexArrays) (GLsizei, GLuint*);
    typedef void (APIENTRY *_glBindVertexArray) (GLuint);
    typedef void (APIENTRY *_glDrawBuffers) (GLuint,GLuint*);
    _glGenVertexArrays glGenVertexArrays;
    _glBindVertexArray glBindVertexArray;
    _glDrawBuffers glDrawBuffers;
    MI            mesh_importer;
    unsigned int fbo;
    unsigned int texturePosition,textureNormal,textureAlbedo;
    unsigned int fboSSAO, fboBlurSSAO,fboBlurSSAO1,fboBlurSSAO2;
    unsigned int fboColorSSAO, fboColorBlurSSAO,fboColorBlurSSAO1,fboColorBlurSSAO2;

    unsigned int textureNoise;
    std::vector<Eigen::Vector3f> ssaoKernel;
    std::vector<Eigen::Vector3f> ssaoNoise;

    bool init=false;

    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    float radi=75.0;
    float bias = 0.005;

    QRadioButton *showSSAObox;
    QRadioButton *showSepSSAObox;
    QRadioButton *showBlurbox;
    QRadioButton *showSepBlurbox;
    QRadioButton *showLightbox;

    bool showSSAO=false;
    bool showBlur=false;
    bool showSepSSAO=false;
    bool showSepBlur=false;
    bool useSepSSAO =false;
    bool useSepBlur =false;
    bool showLight =true;


    QRadioButton *useSSAObox;
    QRadioButton *useSepSSAObox;
    QRadioButton *useBlurbox;
    QRadioButton *useSepBlurbox;

    QLabel *m_fpsCount;
    QString fpsString;
    QOpenGLTimerQuery timerG;
    GLuint64 resTime;
    float fps;



public slots:
    void importModel ();
    void setRadi(double r);
    void setBias(double b);
    void changePass();
    void changeUseSSAO();
    void changeUseBlur();


};

#endif // VIEWERFRR1_H
