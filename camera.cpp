#include "camera.h"


Camera::Camera(){
    camPosX = 0;
    camPosY = 0;
    camPosZ = 0;

    camRotX = 0;
    camRotY = 0;
    camRotZ = 0;
}

Camera::~Camera(){

}


void Camera::mouseMoveEvent ( QMouseEvent * event, qreal retinaScale, float height, float width ){

    // recuperation des donnees de deplacement de la souris. Les donnees de position sont insuffisantes.
    static int mouseLastPosX = -1, mouseLastPosY = -1;

    int effectiveMoveX, effectiveMoveY;

    // init data
    if(mouseLastPosX == -1 || mouseLastPosY == -1){
        mouseLastPosX = event->x();
        mouseLastPosY = event->y();
        return;
    }

    effectiveMoveX = mouseLastPosX - event->x();
    effectiveMoveY = mouseLastPosY - event->y();

    // application de la rotation en fonction du mouvement de la souris
    camRotY -= (GLfloat)(((float)effectiveMoveX / width) * 360.f);
    camRotY = fmodf(camRotY, 360.0f);

    camRotX += (GLfloat)(((float)effectiveMoveY / height) * 360.f);
    if( camRotX > 80 ) camRotX = 80; else if( camRotX < -80 ) camRotX = -80;

    // sauvegarde pour calcul posterieur
    mouseLastPosX = event->x();
    mouseLastPosY = event->y();
}


void Camera::keyPressEvent(QKeyEvent *event){
    // --> 6 DDL
    // on convertis rotation -> vecteur directeur
    float camRotX_rad = (camRotX / 360.0f) * 6.2831853f;
    float camRotY_rad = (camRotY / 360.0f) * 6.2831853f;
    QVector3D cameraDirection(cosf(camRotY_rad), sinf(camRotX_rad), sinf(camRotY_rad));
    QVector3D cameraDirection_left(cosf(camRotY_rad + 1.57079632675f), sinf(camRotX_rad), sinf(camRotY_rad + 1.57079632675f));
    float speed = 0.05f; // vitesse de la camera

    if(event->key() == Qt::Key_Z){
        camPosX += cameraDirection.x() * speed;
        camPosY += cameraDirection.y() * speed;
        camPosZ += cameraDirection.z() * speed;
    } else if(event->key() == Qt::Key_S){
        camPosX -= cameraDirection.x() * speed;
        camPosY -= cameraDirection.y() * speed;
        camPosZ -= cameraDirection.z() * speed;
    }

    if(event->key() == Qt::Key_Q){
        camPosX -= cameraDirection_left.x() * speed;
        camPosY -= cameraDirection_left.y() * speed;
        camPosZ -= cameraDirection_left.z() * speed;
    } else if(event->key() == Qt::Key_D){
        camPosX += cameraDirection_left.x() * speed;
        camPosY += cameraDirection_left.y() * speed;
        camPosZ += cameraDirection_left.z() * speed;
    }
}

GLfloat Camera::getCamPosX(){
    return camPosX;
}

GLfloat Camera::getCamPosY(){
    return camPosY;
}

GLfloat Camera::getCamPosZ(){
    return camPosZ;
}


GLfloat Camera::getCamRotX(){
    return camRotX;
}

GLfloat Camera::getCamRotY(){
    return camRotY;
}

GLfloat Camera::getCamRotZ(){
    return camRotZ;
}
