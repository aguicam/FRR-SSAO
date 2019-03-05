#ifndef MESH_IMPORTER_H
#define MESH_IMPORTER_H

#include "HW.h"

#include <QtWidgets>
#include <QGLWidget>
#include <QGLFunctions>
#include <QGLShaderProgram>
#include <QtOpenGL>
#include <Helpers/mesh_io.h>

class MI
{
public:
    MI              (HW *hw);
    bool            LoadModel(QString filename);
    void            AddImportExportPLY(QGroupBox*groupBox );
    void            updateNumVert(int N);

private:
    QLabel *m_vertexCount;
    QLabel *m_facesCount;
    HW     *m_hw;

};

#endif // MESH_IMPORTER_H
