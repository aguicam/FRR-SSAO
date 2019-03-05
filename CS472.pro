TEMPLATE = app
TARGET = ViewerPro
QT += widgets opengl 
CONFIG += qt debug_and_release
RESOURCES   = CS472.qrc

Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR     = release/.moc
Debug:OBJECTS_DIR   = debug/.obj
Debug:MOC_DIR       = debug/.moc

INCLUDEPATH     += . geometry camera lighting Eigen GL glm

win32-msvc2015{
	RC_FILE        += CS472.rc
        LIBS           += -lopengl32
	QMAKE_CXXFLAGS += /MP /Zi
	QMAKE_LFLAGS   += /MACHINE:X64
}

macx{
        QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
        QMAKE_LFLAGS += -Wl,-rpath,@executable_path/../Frameworks
	QMAKE_LFLAGS   -= -mmacosx-version-min=10.8
	QMAKE_LFLAGS   += -mmacosx-version-min=10.9
	QMAKE_CXXFLAGS -= -mmacosx-version-min=10.8
	QMAKE_CXXFLAGS += -mmacosx-version-min=10.9
	QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
	ICON = CS472.icns
}

unix:!macx{
        CONFIG += C++11
}

mingw{
        LIBS += -lopengl32
}

# Input
HEADERS +=	MainWindow.h		\
		HW.h			\
		dummy.h			\
		hw2/HW2b.h		\
		geometry/Geometry.h	\
		geometry/Cube.h		\
		geometry/Sphere.h	\
		geometry/Cylinder.h	\
		geometry/Cone.h		\
		geometry/Scene.h	\
		lighting/Light.h	\
		camera/Camera.h \
                Helpers/mesh_io.h \
                Helpers/triangle_mesh.h \
    Helpers/mesh_importer.h \
    SRGGE/Viewer1.h \
    Helpers/framerate.h \
    FRR/viewerfrr1.h \
    SRGGE/Viewer2.h
		
SOURCES +=	main.cpp		\ 
		MainWindow.cpp 		\
		HW.cpp			\
		dummy.cpp		\
		hw2/HW2b.cpp		\
		geometry/Cube.cpp	\
		geometry/Sphere.cpp	\
		geometry/Cylinder.cpp	\
		geometry/Cone.cpp	\
		geometry/Scene.cpp	\
		lighting/Light.cpp	\
		camera/Camera.cpp \
                Helpers/mesh_io.cc \
                Helpers/triangle_mesh.cc \
    Helpers/mesh_importer.cpp \
    SRGGE/Viewer1.cpp \
    Helpers/framerate.cpp \
    FRR/viewerfrr1.cpp \
    SRGGE/Viewer2.cpp

DISTFILES += \
    hw2/fshader2a.glsl \
    hw2/fshader2b.glsl \
    hw2/vshader2a.glsl \
    hw2/vshader2b.glsl \
    hw2/try.frag \
    hw2/try.vert \
    FRR/Shaders/ssao.frag \
    FRR/Shaders/ssao_blur.frag \
    FRR/Shaders/ssao_geometry.frag \
    FRR/Shaders/ssao_light.frag \
    FRR/Shaders/ssao.vert \
    FRR/Shaders/ssao_geometry.vert
