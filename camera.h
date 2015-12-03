#ifndef CAMERA
#define CAMERA

#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QMatrix4x4>
#include <QtCore/qmath.h>
#include <QMouseEvent>
#include <QKeyEvent>

class Camera{
public:
    Camera();
    ~Camera();

    void mouseMoveEvent( QMouseEvent * event, qreal retinaScale, float h, float w  );
    void keyPressEvent(QKeyEvent *event);

    GLfloat getCamPosX();
    GLfloat getCamPosY();
    GLfloat getCamPosZ();

    GLfloat getCamRotX();
    GLfloat getCamRotY();
    GLfloat getCamRotZ();


private:
    GLfloat camPosX, camPosY, camPosZ;
    GLfloat camRotX, camRotY, camRotZ;
};


#endif // CAMERA

