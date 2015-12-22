#include "castlewindow.h"


CastleWindow::CastleWindow()
    : m_program(0)
    , m_frame(0)
{
}



//Charger les shaders
GLuint CastleWindow::loadShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    return shader;
}


void CastleWindow::initialize()
{
    // Enable depth buffer    
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LESS );
     //glDepthFunc( GL_GREATER );

    // Enable back face culling
    glEnable(GL_CULL_FACE);


    glClearColor(1.0f,1.0f,1.0f,1.0f);

    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_posAttr = m_program->attributeLocation("posAttr");
    m_uvAttr = m_program->attributeLocation( "uvAttr" );
    m_matrixUniform = m_program->uniformLocation( "matrix" );
    m_camPos = m_program->uniformLocation( "camPos" );
    m_nmAttr = m_program->attributeLocation( "nmAttr" );
    m_tangent = m_program->attributeLocation( "tangent" );
    m_bitangent = m_program->attributeLocation( "bitangent" );

    //TEXTURES
    textures = (QOpenGLTexture**)malloc(sizeof(QOpenGLTexture*) * nbTextures * 2);

    //Sol
    textures[textureGround*2] = new QOpenGLTexture(QImage( nameTextureGround ));
    textures[textureGround*2 + 1] = new QOpenGLTexture( QImage( nameTextureGroundN ) );
    //Mur
    textures[textureWall*2] = new QOpenGLTexture(QImage( nameTextureWall ));
    textures[textureWall*2 + 1] = new QOpenGLTexture(QImage( nameTextureWallN ));
    //Wood
    textures[textureWood*2] = new QOpenGLTexture(QImage( nameTextureWood ));
    textures[textureWood*2 + 1] = new QOpenGLTexture(QImage( nameTextureWoodN ));


    for(int i = 0; i < nbTextures;  i++){
         // Set nearest filtering mode for texture minification
        textures[i]->setMinificationFilter( QOpenGLTexture::NearestMipMapLinear );
        // Set bilinear filtering mode for texture magnification
        textures[i]->setMagnificationFilter( QOpenGLTexture::Linear );
        // Wrap texture coordinates by repeating
        // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
        textures[i]->setWrapMode( QOpenGLTexture::MirroredRepeat );

        textures[i+1]->setMinificationFilter( QOpenGLTexture::NearestMipMapLinear );
        textures[i+1]->setMagnificationFilter( QOpenGLTexture::Linear );
        textures[i+1]->setWrapMode( QOpenGLTexture::MirroredRepeat );

    }


    // initialisation de la camera
    cam = Camera();



}

//Transmettre les events et les tailles à la caméra
void CastleWindow::mouseMoveEvent ( QMouseEvent * event ){
    static const qreal retinaScale = devicePixelRatio();
    static float hei = height() * retinaScale;
    static float wid = width() * retinaScale;
    cam.mouseMoveEvent(event, retinaScale, hei, wid);
}

//Transmettre les events à la caméra
void CastleWindow::keyPressEvent(QKeyEvent *event){
    cam.keyPressEvent(event);
}


void CastleWindow::render()
{
    //Petits calculs de la fenêtre (taille...)
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


    //-------------------------------
    //Binder les textures avec le programme
    m_program->bind();
    for(int i = 0; i < nbTextures ; i++){
        textures[i*2]->bind(i*2);
        textures[i*2 + 1]->bind(i*2 + 1);
    }

    //-------------------------------


    //Gérer la vue
    // la matrice finale = matWORLD * matVIEW * matPROJ
    // on place donc la caméra; la matVIEW
    // OpenGL -> matrices inversees... donc cacul inverse; matPROJ * matVIEW * matWORLD

    QMatrix4x4 matWorld, matView, matProj;
    matWorld.setToIdentity();
    matView.setToIdentity();
    matProj.setToIdentity();

    matWorld.rotate(0,0,0);
    matWorld.translate(0, 0, 0);
    matWorld.scale(1,1,1);

    //Calcul du vecteur directeur de la camera
    float camRotX_rad = (cam.getCamRotX() / 360.0f) * 6.2831853f;
    float camRotY_rad = (cam.getCamRotY() / 360.0f) * 6.2831853f;
    QVector3D cameraDirection(cam.getCamPosX() + cosf(camRotY_rad), cam.getCamPosY() + sinf(camRotX_rad), cam.getCamPosZ() + sinf(camRotY_rad));

    //Où l'on regarde
    matView.lookAt(QVector3D(cam.getCamPosX(), cam.getCamPosY(), cam.getCamPosZ()), cameraDirection, QVector3D(0, 1, 0));

    //Projection, angle de vue...
    float near = 0.1f;
    float far = 100.0f;
    matProj.perspective(80.0f, 4.0f/3.0f, near, far);

    //-------------------------------


    //Fournir au programme l'ensemble des données utiles
    m_program->setUniformValue( m_matrixUniform, matProj * matView * matWorld );
    m_program->setUniformValue( m_camPos, QVector3D( cam.getCamPosX(), cam.getCamPosY(), cam.getCamPosZ() ) );
    m_program->setUniformValue( "tex", 0 );
    m_program->setUniformValue( "nmTex", 1 );
    m_program->setUniformValue( "lightPos", QVector3D(0.5f,1.0f,0.3f));
    m_program->setUniformValue( "model" ,matWorld);
    //m_program->setUniformValue( "near" ,near);
    //m_program->setUniformValue( "far" ,far);


    createFace(QVector3D(0.0, 0.0, 0.0), 50.0, 50.0, matProj*matView*matWorld,QVector3D(0,0,0),QVector3D(-90,0,0),QVector3D(1,1,1),0);

    //createCube(1.0, matProj*matView*matWorld,QVector3D(0,0,0),QVector3D(0,0,0),QVector3D(1,1,1),1);

    //createCube(2.0, matProj*matView*matWorld,QVector3D(5,1,0),QVector3D(0,45,0),QVector3D(1,1,1),1);

    //createMur(4.0,QVector3D(3.0,2.0,2.0),matProj*matView*matWorld,QVector3D(1,1,0),QVector3D(0,90,0),QVector3D(1,1,1),1,1);

    //createTour(2.0, QVector3D(2.0,5.0,2.0), matProj*matView*matWorld, QVector3D(5,0,0),QVector3D(0,0,0),QVector3D(1,1,1),1,0);

    //createFaceTriangle(QVector3D(0.0, 0.0, 0.0), 5.0, 5.0, matProj*matView*matWorld,QVector3D(0,5,0),QVector3D(0,0,0),QVector3D(1,1,1),0);

    //createTourCylinder(2.0,QVector2D(10,5),matProj*matView*matWorld, QVector3D(0,0,0),QVector3D(0,0,0), QVector3D(1,1,1),1,0);

    ///Petit probleme en Y pour les tours cylindres
    attributesElement attrMur;
    attrMur.dimensions = QVector3D(5,2,1);
    attributesElement attrTour;
    attrTour.dimensions = QVector3D(2,4,2);
    generateEnceinte(2.0,5,attrMur,1,attrTour,matProj*matView*matWorld, QVector3D(0,0,0),QVector3D(0,0,0), QVector3D(1,1,1),1,0);

    m_program->release();

    ++m_frame;

}


int CastleWindow::randomDetails(int details, int nbDetailsI){
    int rd = details;
    if(rd == 0){
        //srand (time(NULL));
        srand (0);
        rd = (rand() % nbDetailsI) + 1;
    }
    return rd;
}


void CastleWindow::initTextures(int nbText){
    m_program->setUniformValue( "tex", nbText*2 );
    m_program->setUniformValue( "nmTex", (nbText*2)+1 );
}

QMatrix4x4 CastleWindow::createMatrixWorld(QVector3D translate, QVector3D rotate, QVector3D scale){
    QMatrix4x4 matWorld;
    matWorld.setToIdentity();
    matWorld.scale(scale);
    matWorld.translate(translate);
    matWorld.rotate(rotate.x(),QVector3D(1.0,0.0,0.0));
    matWorld.rotate(rotate.y(),QVector3D(0.0,1.0,0.0));
    matWorld.rotate(rotate.z(),QVector3D(0.0,0.0,1.0));

    return matWorld;
}

void CastleWindow::createFace(QVector3D pos, GLfloat lenghtX, GLfloat lenghtY, QMatrix4x4  matGlobale,QVector3D translate, QVector3D rotate, QVector3D scale,int texture){

    QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);
    initTextures(texture);

    m_program->setUniformValue( m_matrixUniform, matGlobale * matWorld );


    //Vertex
    QVector3D pos1(pos.x()-lenghtX/2, pos.y()+lenghtY/2, pos.z());
    QVector3D pos2(pos.x()-lenghtX/2, pos.y()-lenghtY/2, pos.z());
    QVector3D pos3(pos.x()+lenghtX/2, pos.y()-lenghtY/2, pos.z());
    QVector3D pos4(pos.x()+lenghtX/2, pos.y()+lenghtY/2, pos.z());
    //Tex
    QVector2D uv1(0.0, 1.0);
    QVector2D uv2(0.0, 0.0);
    QVector2D uv3(1.0, 0.0);
    QVector2D uv4(1.0, 1.0);
    //Vecteur Normal
    QVector3D nm;//(0.0, 0.0, 1.0);
    nm.crossProduct(pos1,pos2);

    // calculate tangent/bitangent vectors of both triangles
    QVector3D tangent1, bitangent1;
    QVector3D tangent2, bitangent2;
    // - triangle 1
    QVector3D edge1 = pos2 - pos1;
    QVector3D edge2 = pos3 - pos1;
    QVector2D deltaUV1 = uv2 - uv1;
    QVector2D deltaUV2 = uv3 - uv1;

    GLfloat f = 1.0f / (deltaUV1.x() * deltaUV2.y() - deltaUV2.x() * deltaUV1.y());

    tangent1.setX(f * (deltaUV2.y() * edge1.x() - deltaUV1.y() * edge2.x()));
    tangent1.setY(f * (deltaUV2.y() * edge1.y() - deltaUV1.y() * edge2.y()));
    tangent1.setZ(f * (deltaUV2.y() * edge1.z() - deltaUV1.y() * edge2.z()));
    tangent1.normalize();

    bitangent1.setX(f * (-deltaUV2.x() * edge1.x() + deltaUV1.x() * edge2.x()));
    bitangent1.setY(f * (-deltaUV2.x() * edge1.y() + deltaUV1.x() * edge2.y()));
    bitangent1.setZ(f * (-deltaUV2.x() * edge1.z() + deltaUV1.x() * edge2.z()));
    bitangent1.normalize();

    // - triangle 2
    edge1 = pos3 - pos1;
    edge2 = pos4 - pos1;
    deltaUV1 = uv3 - uv1;
    deltaUV2 = uv4 - uv1;

    f = 1.0f / (deltaUV1.x() * deltaUV2.y() - deltaUV2.x() * deltaUV1.y());

    tangent2.setX(f * (deltaUV2.y() * edge1.x() - deltaUV1.y() * edge2.x()));
    tangent2.setY(f * (deltaUV2.y() * edge1.y() - deltaUV1.y() * edge2.y()));
    tangent2.setZ(f * (deltaUV2.y() * edge1.z() - deltaUV1.y() * edge2.z()));
    tangent2.normalize();


    bitangent2.setX(f * (-deltaUV2.x() * edge1.x() + deltaUV1.x() * edge2.x()));
    bitangent2.setY(f * (-deltaUV2.x() * edge1.y() + deltaUV1.x() * edge2.y()));
    bitangent2.setZ(f * (-deltaUV2.x() * edge1.z() + deltaUV1.x() * edge2.z()));
    bitangent2.normalize();



     GLfloat vertices[] = {
         pos1.x(), pos1.y(), pos1.z(),
         pos2.x(), pos2.y(), pos2.z(),
         pos3.x(), pos3.y(), pos3.z(),

         pos1.x(), pos1.y(), pos1.z(),
         pos3.x(), pos3.y(), pos3.z(),
         pos4.x(), pos4.y(), pos4.z(),
    };

    GLfloat uv[] = {
        uv1.x(),uv1.y(),
        uv2.x(),uv2.y(),
        uv3.x(),uv3.y(),

        uv1.x(),uv1.y(),
        uv3.x(),uv3.y(),
        uv4.x(),uv4.y(),
    };

    GLfloat tangent[] = {
        tangent1.x(), tangent1.y(), tangent1.z(),

        tangent2.x(), tangent2.y(), tangent2.z(),
    };

    GLfloat bitangent[] = {
        bitangent1.x(), bitangent1.y(), bitangent1.z(),

        bitangent2.x(), bitangent2.y(), bitangent2.z(),
    };

    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glEnableVertexAttribArray( 2 );
    glEnableVertexAttribArray( 3 );

    glVertexAttribPointer( m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, vertices );
    glVertexAttribPointer( m_uvAttr, 2, GL_FLOAT, GL_FALSE, 0, uv );
    glVertexAttribPointer( m_tangent, 3, GL_FLOAT, GL_FALSE, 0, tangent );
    glVertexAttribPointer( m_bitangent, 3, GL_FLOAT, GL_FALSE, 0, bitangent );

    glDrawArrays( GL_TRIANGLES, 0, 6 );

    glDisableVertexAttribArray( 3 );
    glDisableVertexAttribArray( 2 );
    glDisableVertexAttribArray( 1 );
    glDisableVertexAttribArray( 0 );
}

void CastleWindow::createFaceTriangle(QVector3D pos, GLfloat lenghtX, GLfloat lenghtY, QMatrix4x4  matGlobale,QVector3D translate, QVector3D rotate, QVector3D scale,int texture){

    QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);
    initTextures(texture);

    m_program->setUniformValue( m_matrixUniform, matGlobale * matWorld );


    //Vertex
    QVector3D pos1(pos.x(), pos.y()+lenghtY/2, pos.z());
    QVector3D pos2(pos.x()-lenghtX/2, pos.y()-lenghtY/2, pos.z());
    QVector3D pos3(pos.x()+lenghtX/2, pos.y()-lenghtY/2, pos.z());
    //Tex
    QVector2D uv1(0.5, 0.5);
    QVector2D uv2(0.0, 0.0);
    QVector2D uv3(1.0, 0.0);
    //Vecteur Normal
    QVector3D nm;//(0.0, 0.0, 1.0);
    nm.crossProduct(pos1,pos2);

    // calculate tangent/bitangent vectors of both triangles
    QVector3D tangent1, bitangent1;
    // - triangle 1
    QVector3D edge1 = pos2 - pos1;
    QVector3D edge2 = pos3 - pos1;
    QVector2D deltaUV1 = uv2 - uv1;
    QVector2D deltaUV2 = uv3 - uv1;

    GLfloat f = 1.0f / (deltaUV1.x() * deltaUV2.y() - deltaUV2.x() * deltaUV1.y());

    tangent1.setX(f * (deltaUV2.y() * edge1.x() - deltaUV1.y() * edge2.x()));
    tangent1.setY(f * (deltaUV2.y() * edge1.y() - deltaUV1.y() * edge2.y()));
    tangent1.setZ(f * (deltaUV2.y() * edge1.z() - deltaUV1.y() * edge2.z()));
    tangent1.normalize();

    bitangent1.setX(f * (-deltaUV2.x() * edge1.x() + deltaUV1.x() * edge2.x()));
    bitangent1.setY(f * (-deltaUV2.x() * edge1.y() + deltaUV1.x() * edge2.y()));
    bitangent1.setZ(f * (-deltaUV2.x() * edge1.z() + deltaUV1.x() * edge2.z()));
    bitangent1.normalize();



     GLfloat vertices[] = {
         pos1.x(), pos1.y(), pos1.z(),
         pos2.x(), pos2.y(), pos2.z(),
         pos3.x(), pos3.y(), pos3.z()
    };

    GLfloat uv[] = {
        uv1.x(),uv1.y(),
        uv2.x(),uv2.y(),
        uv3.x(),uv3.y()
    };

    GLfloat tangent[] = {
        tangent1.x(), tangent1.y(), tangent1.z()
    };

    GLfloat bitangent[] = {
        bitangent1.x(), bitangent1.y(), bitangent1.z()
    };

    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glEnableVertexAttribArray( 2 );
    glEnableVertexAttribArray( 3 );

    glVertexAttribPointer( m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, vertices );
    glVertexAttribPointer( m_uvAttr, 2, GL_FLOAT, GL_FALSE, 0, uv );
    glVertexAttribPointer( m_tangent, 3, GL_FLOAT, GL_FALSE, 0, tangent );
    glVertexAttribPointer( m_bitangent, 3, GL_FLOAT, GL_FALSE, 0, bitangent );

    glDrawArrays( GL_TRIANGLES, 0, 3 );

    glDisableVertexAttribArray( 3 );
    glDisableVertexAttribArray( 2 );
    glDisableVertexAttribArray( 1 );
    glDisableVertexAttribArray( 0 );
}

void CastleWindow::createFaceParam(QVector3D pos1, QVector3D pos2, QVector3D pos3, QVector3D pos4, QMatrix4x4  matGlobale,QVector3D translate, QVector3D rotate, QVector3D scale,int texture){

    QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);
    initTextures(texture);

    m_program->setUniformValue( m_matrixUniform, matGlobale * matWorld );

    //Tex
    QVector2D uv1(0.0, 1.0);
    QVector2D uv2(0.0, 0.0);
    QVector2D uv3(1.0, 0.0);
    QVector2D uv4(1.0, 1.0);
    //Vecteur Normal
    QVector3D nm;//(0.0, 0.0, 1.0);
    nm.crossProduct(pos1,pos2);

    // calculate tangent/bitangent vectors of both triangles
    QVector3D tangent1, bitangent1;
    QVector3D tangent2, bitangent2;
    // - triangle 1
    QVector3D edge1 = pos2 - pos1;
    QVector3D edge2 = pos3 - pos1;
    QVector2D deltaUV1 = uv2 - uv1;
    QVector2D deltaUV2 = uv3 - uv1;

    GLfloat f = 1.0f / (deltaUV1.x() * deltaUV2.y() - deltaUV2.x() * deltaUV1.y());

    tangent1.setX(f * (deltaUV2.y() * edge1.x() - deltaUV1.y() * edge2.x()));
    tangent1.setY(f * (deltaUV2.y() * edge1.y() - deltaUV1.y() * edge2.y()));
    tangent1.setZ(f * (deltaUV2.y() * edge1.z() - deltaUV1.y() * edge2.z()));
    tangent1.normalize();

    bitangent1.setX(f * (-deltaUV2.x() * edge1.x() + deltaUV1.x() * edge2.x()));
    bitangent1.setY(f * (-deltaUV2.x() * edge1.y() + deltaUV1.x() * edge2.y()));
    bitangent1.setZ(f * (-deltaUV2.x() * edge1.z() + deltaUV1.x() * edge2.z()));
    bitangent1.normalize();

    // - triangle 2
    edge1 = pos3 - pos1;
    edge2 = pos4 - pos1;
    deltaUV1 = uv3 - uv1;
    deltaUV2 = uv4 - uv1;

    f = 1.0f / (deltaUV1.x() * deltaUV2.y() - deltaUV2.x() * deltaUV1.y());

    tangent2.setX(f * (deltaUV2.y() * edge1.x() - deltaUV1.y() * edge2.x()));
    tangent2.setY(f * (deltaUV2.y() * edge1.y() - deltaUV1.y() * edge2.y()));
    tangent2.setZ(f * (deltaUV2.y() * edge1.z() - deltaUV1.y() * edge2.z()));
    tangent2.normalize();


    bitangent2.setX(f * (-deltaUV2.x() * edge1.x() + deltaUV1.x() * edge2.x()));
    bitangent2.setY(f * (-deltaUV2.x() * edge1.y() + deltaUV1.x() * edge2.y()));
    bitangent2.setZ(f * (-deltaUV2.x() * edge1.z() + deltaUV1.x() * edge2.z()));
    bitangent2.normalize();



     GLfloat vertices[] = {
         pos1.x(), pos1.y(), pos1.z(),
         pos2.x(), pos2.y(), pos2.z(),
         pos3.x(), pos3.y(), pos3.z(),

         pos1.x(), pos1.y(), pos1.z(),
         pos3.x(), pos3.y(), pos3.z(),
         pos4.x(), pos4.y(), pos4.z(),
    };

    GLfloat uv[] = {
        uv1.x(),uv1.y(),
        uv2.x(),uv2.y(),
        uv3.x(),uv3.y(),

        uv1.x(),uv1.y(),
        uv3.x(),uv3.y(),
        uv4.x(),uv4.y(),
    };

    GLfloat tangent[] = {
        tangent1.x(), tangent1.y(), tangent1.z(),

        tangent2.x(), tangent2.y(), tangent2.z(),
    };

    GLfloat bitangent[] = {
        bitangent1.x(), bitangent1.y(), bitangent1.z(),

        bitangent2.x(), bitangent2.y(), bitangent2.z(),
    };

    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glEnableVertexAttribArray( 2 );
    glEnableVertexAttribArray( 3 );

    glVertexAttribPointer( m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, vertices );
    glVertexAttribPointer( m_uvAttr, 2, GL_FLOAT, GL_FALSE, 0, uv );
    glVertexAttribPointer( m_tangent, 3, GL_FLOAT, GL_FALSE, 0, tangent );
    glVertexAttribPointer( m_bitangent, 3, GL_FLOAT, GL_FALSE, 0, bitangent );

    glDrawArrays( GL_TRIANGLES, 0, 6 );

    glDisableVertexAttribArray( 3 );
    glDisableVertexAttribArray( 2 );
    glDisableVertexAttribArray( 1 );
    glDisableVertexAttribArray( 0 );
}

void CastleWindow::createCube(GLfloat lenght, QMatrix4x4  matGlobale,QVector3D translate, QVector3D rotate, QVector3D scale,int texture){

    QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);

    //Face Bas
    createFace(QVector3D(0.0, 0.0, 0.0), lenght, lenght, matGlobale * matWorld,QVector3D(0,-lenght/2.0,0),
               QVector3D(90,0,0),QVector3D(1,1,1),texture);

    //Face Haut
    createFace(QVector3D(0.0, 0.0, 0.0), lenght, lenght, matGlobale * matWorld,QVector3D(0,lenght/2.0,0),
               QVector3D(-90,0,0),QVector3D(1,1,1),texture);

   //Face Avant
   createFace(QVector3D(0.0, 0.0, 0.0), lenght, lenght, matGlobale * matWorld,QVector3D(0,0,lenght/2.0),
               QVector3D(0,0,0),QVector3D(1,1,1),texture);

    //Face Arrière
    createFace(QVector3D(0.0, 0.0, 0.0), lenght, lenght, matGlobale * matWorld,QVector3D(0,0,- lenght/2.0),
               QVector3D(0,180,0),QVector3D(1,1,1),texture);

    //Face Droite
    createFace(QVector3D(0.0, 0.0, 0.0), lenght, lenght, matGlobale * matWorld,QVector3D(lenght/2.0,0,0),
               QVector3D(0,90,0),QVector3D(1,1,1),texture);

    //Face Gauche
    createFace(QVector3D(0.0, 0.0, 0.0), lenght, lenght, matGlobale * matWorld,QVector3D(- lenght/2.0,0,0),
               QVector3D(0,0-90,0),QVector3D(1,1,1),texture);
}

//Pos  : 0---1
//       |\  |\
//       | \ | \
//Gauche |Av3--2 Droite
//       4--|5 |
//       \  |  |
//        \ |De|
//         \7--6
void CastleWindow::createCubeParam(QVector3D* pos, QMatrix4x4  matGlobale,QVector3D translate, QVector3D rotate, QVector3D scale,int texture){

    QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);

    GLfloat bas = 0;//-(pos[4] + pos[5] + pos[6] + pos[7]).y()/4;
    GLfloat haut = 0;// (pos[0] + pos[1] + pos[2] + pos[3]).y()/4;
    GLfloat av = 0;//(pos[0] + pos[1] + pos[4] + pos[5]).z()/4;
    GLfloat de = 0;//-(pos[3] + pos[2] + pos[6] + pos[7]).z()/4;
    GLfloat droite = 0;//(pos[1] + pos[2] + pos[6] + pos[5]).x()/4;
    GLfloat gauche = 0;//-(pos[0] + pos[3] + pos[4] + pos[7]).x()/4;

    //Face Bas
    createFaceParam(pos[4], pos[5], pos[6], pos[7],
               matGlobale * matWorld,QVector3D(0,0,bas),
                    QVector3D(0,0,0),QVector3D(1,1,1),texture);

    //Face Haut
    createFaceParam(pos[3], pos[2], pos[1], pos[0],
               matGlobale * matWorld,QVector3D(0,0,haut),
               QVector3D(0,0,0),QVector3D(1,1,1),texture);

   //Face Avant
   createFaceParam(pos[0], pos[1], pos[5], pos[4],
              matGlobale * matWorld,QVector3D(0,0,av),
               QVector3D(0,0,0),QVector3D(1,1,1),texture);

    //Face Arrière
    createFaceParam(pos[7], pos[6], pos[2], pos[3],
                    matGlobale * matWorld, QVector3D(0,0,de),
               QVector3D(0,0,0),QVector3D(1,1,1),texture);

    //Face Droite
    createFaceParam(pos[1], pos[2], pos[6], pos[5],
                    matGlobale * matWorld,QVector3D(droite,0,0),
               QVector3D(0,0,0),QVector3D(1,1,1),texture);

    //Face Gauche
    createFaceParam(pos[4], pos[7], pos[3], pos[0],
                    matGlobale * matWorld,QVector3D(gauche,0,0),
               QVector3D(0,0,0),QVector3D(1,1,1),texture);
}


void CastleWindow::createMur(GLfloat lenght,QVector3D nbCubes, QMatrix4x4 matGlobale, QVector3D translate, QVector3D rotate, QVector3D scale ,int texture, int details){
    QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);

    int rd = randomDetails(details);

    //Génération du mur
    for(int i = -nbCubes.x()/2; i < nbCubes.x()/2  ; i++){
        for(int j = -nbCubes.y()/2; j < nbCubes.y()/2 ;j++){
            for(int k = -nbCubes.z()/2; k < nbCubes.z()/2 ;k++){
                //Pour bien centrer le mur, si il y a un nombre pair de cube il faut décaler
                GLfloat deplX = i * lenght;
                GLfloat deplY = j * lenght;
                GLfloat deplZ = k * lenght;
                if((int)nbCubes.x() % 2 == 0) deplX += lenght/2;
                if((int)nbCubes.y() % 2 == 0) deplY += lenght/2;
                if((int)nbCubes.z() % 2 == 0) deplZ += lenght/2;

                createCube(lenght,matGlobale*matWorld,QVector3D(deplX,deplY,deplZ),QVector3D(0,0,0),QVector3D(1,1,1),texture);

                if((nbCubes.z() == 1 || k == (int)-nbCubes.z() / 2 )&& ( nbCubes.y() == 1 || j == (int)nbCubes.y() / 2 -1)){//Ne le faire qu'une seule fois, sur bord du rempart
                    if(rd != numeroHourd){
                        createCrenelage(lenght,nbCubes.x(),matGlobale*matWorld,QVector3D(deplX,deplY,deplZ),QVector3D(0,0,0),QVector3D(1,1,1),texture,rd);
                    }else{
                        createHourd(lenght,QVector2D(nbCubes.x(),nbCubes.z()),matGlobale*matWorld,QVector3D(deplX,deplY,deplZ),QVector3D(0,0,0),QVector3D(1,1,1),textureWood);
                     }
               }
            }
        }
    }


}

void CastleWindow::createCrenelage(GLfloat lenght,GLfloat nbCubes,QMatrix4x4 matGlobale,QVector3D translate,QVector3D rotate, QVector3D scale,int texture, int details){
    QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);


    int rd = randomDetails(details);

   //qDebug()<<rd<<endl;
    if(rd == 1){
        GLfloat pas = lenght/2.0;
        QVector3D* pos = (QVector3D*)malloc(sizeof(QVector3D)*8);
        pos[0] = QVector3D(-pas,pas*2,-pas*3/2);
        pos[1] = QVector3D(pas,pas*2,-pas*3/2);
        pos[2] = QVector3D(pas,pas*2,-pas);
        pos[3] = QVector3D(-pas,pas*2,-pas);

        pos[4] = QVector3D(-pas,pas,-pas);
        pos[5] = QVector3D(pas,pas,-pas);
        pos[6] = QVector3D(pas,pas,-pas);
        pos[7] = QVector3D(-pas,pas,-pas);

        createCubeParam(pos,matGlobale*matWorld,QVector3D(0,-pas,0),QVector3D(0,0,0),QVector3D(1,1,1),texture);
    }

    bool creneau = false;
    for(int i = -2; i < 2  ; i++){
        GLfloat deplX = i*lenght + lenght*((int)nbCubes%2)/2;
        if((int)nbCubes % 2 == 0) deplX += lenght/2.0;
        GLfloat deplY = 3*lenght + lenght/2;
        GLfloat deplZ = -2*lenght + lenght/2;;

        creneau = !creneau;

        if(rd == 1){
            //On repousse les remparts et on ajoute un petit surplomb
            deplZ -= lenght;
            //createCube(lenght,matGlobale*matWorld,QVector3D(deplX,deplY-2*lenght,deplZ),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);
            //createCubeParam(pos,matGlobale*matWorld,QVector3D(deplX,deplY-2*lenght-lenght*3/2,deplZ+lenght*3/2),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);

        }
        //créneaux
        if(creneau){
            createCube(lenght,matGlobale*matWorld,QVector3D(deplX+lenght/2,deplY,deplZ),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);
        }
        //rebord
        createCube(lenght,matGlobale*matWorld,QVector3D(deplX,deplY-lenght,deplZ),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);
    }
}

void CastleWindow::createHourd(GLfloat lenght,QVector2D nbCubes,QMatrix4x4 matGlobale,QVector3D translate,QVector3D rotate, QVector3D scale,int texture){
    QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);

    GLfloat deplZB = -lenght;

    //createPlank( matGlobale*matWorld, QVector3D(lenght,lenght/10,lenght), QVector3D(0,lenght,deplZB),QVector3D(90,0,0));
    //createPlank( matGlobale*matWorld, QVector3D(lenght,lenght/10,lenght/2), QVector3D(0,lenght/2 - lenght/20,deplZB+lenght/4));

    createPerforedPlank( matGlobale*matWorld, QVector3D(lenght,lenght/10,lenght), QVector2D(lenght/3,lenght/3), QVector3D(0,lenght,deplZB),QVector3D(90,0,0),QVector3D(1,1,1),texture);
    createPerforedPlank( matGlobale*matWorld, QVector3D(lenght,lenght/10,lenght/2), QVector2D(lenght/3,lenght/6), QVector3D(0,lenght/2 - lenght/20,deplZB+lenght/4), QVector3D(0,0,0),QVector3D(1,1,1),texture);

    //createFace(QVector3D(0,0,0),lenght,lenght,matGlobale*matWorld,QVector3D(0,lenght,deplZB),QVector3D(180,0,0));
    //createFace(QVector3D(0,0,0),lenght,lenght/2,matGlobale*matWorld,QVector3D(0,lenght/2,deplZB+lenght/4),QVector3D(90,0,0));

    //Les soutiens sous le surplomb
    GLfloat pas = lenght/8;
    QVector3D* pos = (QVector3D*)malloc(sizeof(QVector3D)*8);
    pos[0] = QVector3D(-pas,pas*4,-pas);
    pos[1] = QVector3D(pas,pas*4,-pas);
    pos[2] = QVector3D(pas/2,pas,2*pas);
    pos[3] = QVector3D(-pas/2,pas,2*pas);

    pos[4] = QVector3D(-pas,0,-pas);
    pos[5] = QVector3D(pas,0,-pas);
    pos[6] = QVector3D(pas/2,0,2*pas);
    pos[7] = QVector3D(-pas/2,0,2*pas);

    createCubeParam(pos,matGlobale*matWorld,QVector3D(-lenght/3,pas*2  + pas/5,-lenght/2),QVector3D(-90,180,0),QVector3D(1,1,1),texture);
    createCubeParam(pos,matGlobale*matWorld,QVector3D(lenght/3,pas*2 + pas/5,-lenght/2),QVector3D(-90,180,0),QVector3D(1,1,1),texture);


    GLfloat deplYB = lenght*3/2;
    GLfloat angleX = -45;
    for(int i = 0; i < nbCubes.y()+2; i++){
        if(i == 0){
            angleX = -45;
            deplZB += lenght/4;
            deplYB += lenght/4;
        }else if( i == nbCubes.y() + 1){
            angleX = 45;
            deplZB -= lenght/4;
            deplYB += lenght/4;
        }else{
            angleX = 0;
        }

        createPlank( matGlobale*matWorld, QVector3D(lenght,lenght/10,lenght), QVector3D(0,deplYB,deplZB), QVector3D(angleX,0,0),QVector3D(1,1,1),texture);

        if(i == 0 ){
            deplYB += lenght/2;
            deplYB -= lenght/4;
            deplZB -= lenght/4;
        }else if(i == nbCubes.y()){
            deplYB -= lenght/2;
        }
        deplZB += lenght;
    }
    deplZB -= lenght;
    deplZB += lenght/4;


    //createPlank( matGlobale*matWorld, QVector3D(lenght,lenght/10,lenght), QVector3D(0,lenght,deplZB),QVector3D(90,0,0));
    //createPlank( matGlobale*matWorld, QVector3D(lenght,lenght/10,lenght/2), QVector3D(0,lenght/2 - lenght/20,deplZB-lenght/4));

    createPerforedPlank( matGlobale*matWorld, QVector3D(lenght,lenght/10,lenght), QVector2D(lenght/3,lenght/3), QVector3D(0,lenght,deplZB),QVector3D(90,0,0),QVector3D(1,1,1),texture);
    createPerforedPlank( matGlobale*matWorld, QVector3D(lenght,lenght/10,lenght/2), QVector2D(lenght/3,lenght/6), QVector3D(0,lenght/2 - lenght/20,deplZB-lenght/4),QVector3D(0,0,0),QVector3D(1,1,1),texture);


    //createFace(QVector3D(0,0,0),lenght,lenght,matGlobale*matWorld,QVector3D(0,lenght,deplZB),QVector3D(0,0,0));
    //createFace(QVector3D(0,0,0),lenght,lenght/2,matGlobale*matWorld,QVector3D(0,lenght/2,deplZB-lenght/4),QVector3D(90,0,0));


    createCubeParam(pos,matGlobale*matWorld,QVector3D(-lenght/3,pas*2  + pas/5,deplZB-lenght/2),QVector3D(90,0,0),QVector3D(1,1,1),texture);
    createCubeParam(pos,matGlobale*matWorld,QVector3D(lenght/3,pas*2 + pas/5,deplZB-lenght/2),QVector3D(90,0,0),QVector3D(1,1,1),texture);

}

void CastleWindow::createPlank(QMatrix4x4 matGlobale, QVector3D dimensions,QVector3D translate,QVector3D rotate, QVector3D scale,int texture){
     QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);

     //Face Bas
     createFace(QVector3D(0.0, 0.0, 0.0), dimensions.x(), dimensions.z(), matGlobale * matWorld,QVector3D(0,-dimensions.y()/2.0,0),
                QVector3D(90,0,0),QVector3D(1,1,1),texture);

     //Face Haut
     createFace(QVector3D(0.0, 0.0, 0.0), dimensions.x(), dimensions.z(), matGlobale * matWorld,QVector3D(0,dimensions.y()/2.0,0),
                QVector3D(-90,0,0),QVector3D(1,1,1),texture);

    //Face Avant
    createFace(QVector3D(0.0, 0.0, 0.0), dimensions.x(), dimensions.y(), matGlobale * matWorld,QVector3D(0,0,dimensions.z()/2.0),
                QVector3D(0,0,0),QVector3D(1,1,1),texture);

     //Face Arrière
     createFace(QVector3D(0.0, 0.0, 0.0), dimensions.x(), dimensions.y(), matGlobale * matWorld,QVector3D(0,0,- dimensions.z()/2.0),
                QVector3D(0,180,0),QVector3D(1,1,1),texture);

     //Face Droite
     createFace(QVector3D(0.0, 0.0, 0.0), dimensions.z(), dimensions.y(), matGlobale * matWorld,QVector3D(dimensions.x()/2.0,0,0),
                QVector3D(0,90,0),QVector3D(1,1,1),texture);

     //Face Gauche
     createFace(QVector3D(0.0, 0.0, 0.0), dimensions.z(), dimensions.y(), matGlobale * matWorld,QVector3D(-dimensions.x()/2.0,0,0),
                QVector3D(0,0-90,0),QVector3D(1,1,1),texture);
}

void CastleWindow::createPerforedPlank(QMatrix4x4 matGlobale, QVector3D dimensions, QVector2D hole,QVector3D translate,QVector3D rotate, QVector3D scale,int texture){
     QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);

     //On crée 9 planks, dont une vide pour le trou
     //Il faut trouver les positions des 8 autres ainsi que leurs nouvelles dimensions
     /*
      *     1   2   3
      *     4       6
      *     7   8   9
      *
      * */

     GLfloat x147 = -(hole.x()/2 + (dimensions.x()/2 - hole.x()/2)/2);
     GLfloat x369 = hole.x()/2 + (dimensions.x()/2 - hole.x()/2)/2;

     GLfloat z123 = -(hole.y()/2 + (dimensions.z()/2 - hole.y()/2)/2);
     GLfloat z789 = hole.y()/2 + (dimensions.z()/2 - hole.y()/2)/2;


     GLfloat dimX147369 = dimensions.x()/2 - hole.x()/2;
     GLfloat dimZ123789 = dimensions.z()/2 - hole.y()/2;

     createPlank( matGlobale*matWorld, QVector3D(dimX147369,dimensions.y(),dimZ123789), QVector3D(x147,0,z123),QVector3D(0,0,0),QVector3D(1,1,1),texture);
     createPlank( matGlobale*matWorld, QVector3D(hole.x(),dimensions.y(),dimZ123789), QVector3D(0,0,z123),QVector3D(0,0,0),QVector3D(1,1,1),texture);
     createPlank( matGlobale*matWorld, QVector3D(dimX147369,dimensions.y(),dimZ123789), QVector3D(x369,0,z123),QVector3D(0,0,0),QVector3D(1,1,1),texture);

     createPlank( matGlobale*matWorld, QVector3D(dimX147369,dimensions.y(),hole.y()), QVector3D(x147,0,0),QVector3D(0,0,0),QVector3D(1,1,1),texture);
     createPlank( matGlobale*matWorld, QVector3D(dimX147369,dimensions.y(),hole.y()), QVector3D(x369,0,0),QVector3D(0,0,0),QVector3D(1,1,1),texture);

     createPlank( matGlobale*matWorld, QVector3D(dimX147369,dimensions.y(),dimZ123789), QVector3D(x147,0,z789),QVector3D(0,0,0),QVector3D(1,1,1),texture);
     createPlank( matGlobale*matWorld, QVector3D(hole.x(),dimensions.y(),dimZ123789), QVector3D(0,0,z789),QVector3D(0,0,0),QVector3D(1,1,1),texture);
     createPlank( matGlobale*matWorld, QVector3D(dimX147369,dimensions.y(),dimZ123789), QVector3D(x369,0,z789),QVector3D(0,0,0),QVector3D(1,1,1),texture);


}

void CastleWindow::createTour(GLfloat lenght,QVector3D nbCubes, QMatrix4x4 matGlobale, QVector3D translate, QVector3D rotate, QVector3D scale ,int texture, int details){

    QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);

    //Génération de la tour
    for(int i = -nbCubes.x()/2; i < nbCubes.x()/2  ; i++){
        for(int j = -nbCubes.y()/2; j < nbCubes.y()/2 ;j++){
            for(int k = -nbCubes.z()/2; k < nbCubes.z()/2 ;k++){
                //Pour bien centrer le mur, si il y a un nombre pair de cube il faut décaler
                GLfloat deplX = i * lenght;
                GLfloat deplY = j * lenght;
                GLfloat deplZ = k * lenght;
                if((int)nbCubes.x() % 2 == 0) deplX += lenght/2;
                if((int)nbCubes.y() % 2 == 0) deplY += lenght/2;
                if((int)nbCubes.z() % 2 == 0) deplZ += lenght/2;

                createCube(lenght,matGlobale*matWorld,QVector3D(deplX,deplY,deplZ),QVector3D(0,0,0),QVector3D(1,1,1),texture);               
            }
        }
    }

    createHautTour(lenght,QVector2D(nbCubes.x(),nbCubes.z()),matGlobale*matWorld,QVector3D(0,lenght*nbCubes.y()/2 + lenght/2,0),QVector3D(0,0,0),QVector3D(1,1,1),texture,details);

}

void CastleWindow::createHautTour(GLfloat lenght, QVector2D nbCubes, QMatrix4x4 matGlobale, QVector3D translate,QVector3D rotate, QVector3D scale,int texture, int details){
    QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);

    int rd = randomDetails(details);

    GLfloat deplXH = lenght * nbCubes.x()*4/2 - lenght/2;
    GLfloat deplXB = -lenght * nbCubes.x()*4/2 + lenght/2;
    GLfloat deplY = -lenght*3/2;
    GLfloat deplZH = lenght * nbCubes.y()*4/2 - lenght/2;
    GLfloat deplZB = -lenght * nbCubes.y()*4/2 + lenght/2;
    int departX = 0;
    int finX = 4*nbCubes.x();
    int departZ = 0;
    int finZ = 4*nbCubes.y() - 1;
    if(rd == 1){
        deplXH += lenght;
        deplXB -= lenght;

        deplZH += lenght;
        deplZB -= lenght;

        //departX-=1;
        finX+=2;

        //departZ-=1;
        finZ+=2;
    }

    //Pour le surplomb
    if(rd == 1){
        GLfloat pas = lenght/2.0;
        QVector3D* pos = (QVector3D*)malloc(sizeof(QVector3D)*8);
        pos[0] = QVector3D(-pas,pas*2,-pas*3/2);
        pos[1] = QVector3D(pas,pas*2,-pas*3/2);
        pos[2] = QVector3D(pas,pas*2,-pas);
        pos[3] = QVector3D(-pas,pas*2,-pas);

        pos[4] = QVector3D(-pas,pas,-pas);
        pos[5] = QVector3D(pas,pas,-pas);
        pos[6] = QVector3D(pas,pas,-pas);
        pos[7] = QVector3D(-pas,pas,-pas);

        for(int i = -nbCubes.x()/2; i<nbCubes.x()/2;i++){
            GLfloat retouche = 0.0;
            if(((int)nbCubes.x() % 2) == 0){
                retouche+=pas;
            }
            createCubeParam(pos,matGlobale*matWorld,QVector3D(i*lenght + retouche, - lenght - pas,lenght*nbCubes.y()/2 - pas),QVector3D(0,180,0),QVector3D(1,1,1),texture);
            createCubeParam(pos,matGlobale*matWorld,QVector3D(i*lenght + retouche, - lenght - pas,lenght*-nbCubes.y()/2 + pas),QVector3D(0,0,0),QVector3D(1,1,1),texture);
        }

        for(int i = -nbCubes.y()/2; i<nbCubes.y()/2;i++){
            GLfloat retouche = 0.0;
            if(((int)nbCubes.y() % 2) == 0){
                retouche+=pas;
            }
            createCubeParam(pos,matGlobale*matWorld,QVector3D(lenght*nbCubes.x()/2 - pas, - lenght - pas,i*lenght + retouche),QVector3D(0,-90,0),QVector3D(1,1,1),texture);
            createCubeParam(pos,matGlobale*matWorld,QVector3D(-lenght*nbCubes.x()/2 + pas, - lenght - pas,i*lenght + retouche),QVector3D(0,90,0),QVector3D(1,1,1),texture);
        }
    }


    bool creneau = true;
    for(int i = departX; i < finX ; i++){
        GLfloat deplXHB = deplXH - i * lenght;
        createCube(lenght,matGlobale*matWorld,QVector3D(deplXHB,deplY,deplZH),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);
        createCube(lenght,matGlobale*matWorld,QVector3D(deplXHB,deplY,deplZB),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);
        /*if(rd == 1){//Surplomb
            createCube(lenght,matGlobale*matWorld,QVector3D(deplXHB,deplY-lenght,deplZH),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);
            createCube(lenght,matGlobale*matWorld,QVector3D(deplXHB,deplY-lenght,deplZB),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);
        }*/
        //Créneau
        if(creneau){
            GLfloat deplYB = deplY + lenght;
            createCube(lenght,matGlobale*matWorld,QVector3D(deplXHB-lenght/2,deplYB,deplZH),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);
            createCube(lenght,matGlobale*matWorld,QVector3D(deplXHB-lenght/2,deplYB,deplZB),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);

        }
        creneau = !creneau;
    }
    creneau = true;
    for(int i = departZ; i < finZ ; i++){
        GLfloat deplZHB = deplZH - i * lenght;
        createCube(lenght,matGlobale*matWorld,QVector3D(deplXH,deplY,deplZHB),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);
        createCube(lenght,matGlobale*matWorld,QVector3D(deplXB,deplY,deplZHB),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);
        /*if(rd == 1){//Surplomb
            createCube(lenght,matGlobale*matWorld,QVector3D(deplXH,deplY-lenght,deplZHB),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);
            createCube(lenght,matGlobale*matWorld,QVector3D(deplXB,deplY-lenght,deplZHB),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);
        }*/
        //Créneau
        if(creneau){
            GLfloat deplYB = deplY + lenght;
            createCube(lenght,matGlobale*matWorld,QVector3D(deplXH,deplYB,deplZHB-lenght/2),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);
            createCube(lenght,matGlobale*matWorld,QVector3D(deplXB,deplYB,deplZHB-lenght/2),QVector3D(0,0,0),QVector3D(0.25,0.25,0.25),texture);

        }
        creneau = !creneau;
    }
}


void CastleWindow::createTourCylinder(GLfloat lenght, QVector2D nbCubes,QMatrix4x4 matGlobale, QVector3D translate, QVector3D rotate, QVector3D scale,int texture, int details){
    QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);

    int rd = randomDetails(details);

    GLfloat rotPas = 360.0 / nbCubes.x();
    GLfloat rot = 0;
    GLfloat lenghtX = 2 * (lenght / tan((90 - ((360.0/nbCubes.x())/2)) * PI/180.0));//on calcule la longeur du segment de la face opposé au centre du cercle via les fonctions trigo de base
    for(int i = 0; i < nbCubes.x() ; i++){
        //Création du haut et du bas
        GLfloat posX = (lenght/2)*cos(rot *PI/180.0);
        GLfloat posY = (lenght/2)*sin(rot *PI/180.0);
        //Faces bas
        createFaceTriangle(QVector3D(0,0,0), lenghtX,lenght, matGlobale*matWorld,QVector3D(posX,-lenght*nbCubes.y()/2,posY),QVector3D(90,0,90 + rot),QVector3D(1,1,1),texture);
        //Faces haut
        createFaceTriangle(QVector3D(0,0,0), lenghtX,lenght, matGlobale*matWorld,QVector3D(posX,lenght*nbCubes.y()/2,posY),QVector3D(-90,0,90 - rot),QVector3D(1,1,1),texture);

        //Création des faces de côtés
        GLfloat posC = -lenght*nbCubes.y()/2 + lenght/2;
        for(int j = 0 ; j < nbCubes.y() ;j++){
            createFace(QVector3D(0,0,0), lenghtX,lenght, matGlobale*matWorld, QVector3D(posX*2,posC,posY*2),QVector3D(0,90-rot,0),QVector3D(1,1,1),texture);
            posC += lenght;
        }

        rot += rotPas;
    }

    if(rd == numeroHourd){
       createHautTourCylinderHourd(lenght, nbCubes, matGlobale*matWorld, QVector3D(0,lenght*nbCubes.y()/2,0),QVector3D(0,0,0),QVector3D(1,1,1),textureWood);
    }else{
        createHautTourCylinder(lenght,nbCubes,matGlobale*matWorld,QVector3D(0,lenght*nbCubes.y()/2,0),QVector3D(0,0,0),QVector3D(1,1,1),texture,rd);
    }
}

void CastleWindow::createHautTourCylinder(GLfloat lenght, QVector2D nbCubes, QMatrix4x4 matGlobale, QVector3D translate, QVector3D rotate, QVector3D scale, int texture, int details){
    QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);

    int rd = randomDetails(details);

    GLfloat pas = 0.25;
    QVector3D* pos = (QVector3D*)malloc(sizeof(QVector3D)*8);
    pos[0] = QVector3D(-pas,pas*2,-pas);
    pos[1] = QVector3D(pas,pas*2,-pas);
    pos[2] = QVector3D(pas/2,pas,pas);
    pos[3] = QVector3D(-pas/2,pas,pas);

    pos[4] = QVector3D(-pas,0,-pas);
    pos[5] = QVector3D(pas,0,-pas);
    pos[6] = QVector3D(pas/2,0,pas);
    pos[7] = QVector3D(-pas/2,0,pas);

    QVector3D* posExt = (QVector3D*)malloc(sizeof(QVector3D)*8);
    posExt[0] = QVector3D(-pas,pas*2,-2*pas);
    posExt[1] = QVector3D(pas,pas*2,-2*pas);
    posExt[2] = QVector3D(pas,pas*2,-pas);
    posExt[3] = QVector3D(-pas,pas*2,-pas);

    posExt[4] = QVector3D(-pas,0,-2*pas);
    posExt[5] = QVector3D(pas,0,-2*pas);
    posExt[6] = QVector3D(pas,0,-pas);
    posExt[7] = QVector3D(-pas,0,-pas);

    QVector3D* posExtBas = (QVector3D*)malloc(sizeof(QVector3D)*8);
    posExtBas[0] = QVector3D(-pas,pas*2,-2*pas);
    posExtBas[1] = QVector3D(pas,pas*2,-2*pas);
    posExtBas[2] = QVector3D(pas,pas*2,-pas);
    posExtBas[3] = QVector3D(-pas,pas*2,-pas);

    posExtBas[4] = QVector3D(-pas,0,-pas);
    posExtBas[5] = QVector3D(pas,0,-pas);
    posExtBas[6] = QVector3D(pas,0,-pas);
    posExtBas[7] = QVector3D(-pas,0,-pas);

    GLfloat rotPas = 360.0 / nbCubes.x();
    GLfloat rot = 0;
    for(int i = 0; i < nbCubes.x() ; i++){
        GLfloat posX = (lenght-pas)*cos(rot *PI/180.0);
        GLfloat posY = (lenght-pas)*sin(rot *PI/180.0);
        createCubeParam(pos,matGlobale*matWorld,QVector3D(posX,0,posY),QVector3D(0,180+90-rot,0));

        if(rd == 1){
            createCubeParam(posExt,matGlobale*matWorld,QVector3D(posX,0,posY),QVector3D(0,180+90-rot,0));
            createCubeParam(posExtBas,matGlobale*matWorld,QVector3D(posX,-pas*2,posY),QVector3D(0,180+90-rot,0));

        }
        rot += rotPas;
    }


    //createCubeParam(pos,matGlobale*matWorld);
}

void CastleWindow::createHautTourCylinderHourd(GLfloat lenght, QVector2D nbCubes, QMatrix4x4 matGlobale, QVector3D translate, QVector3D rotate, QVector3D scale,int texture){
    QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);

    //createCube(4,matGlobale * matWorld,QVector3D(0,0,0),QVector3D(0,0,0),QVector3D(1,1,1));

    GLfloat hauteurToit = lenght/2;
    GLfloat epaisseurToit = lenght/20;

    GLfloat rotPas = 360.0 / nbCubes.x();
    GLfloat rot = 0;

    GLfloat posX = (lenght*1.1)*cos(rot *PI/180.0);
    GLfloat posY = (lenght*1.1)*sin(rot *PI/180.0);

    GLfloat oldPosX = (lenght*1.1)*cos((360-rotPas) *PI/180.0);
    GLfloat oldPosY = (lenght*1.1)*sin((360-rotPas) *PI/180.0);

    for(int i = 0; i < nbCubes.x()+1 ; i++){

        posX = (lenght*1.1)*cos(rot *PI/180.0);
        posY = (lenght*1.1)*sin(rot *PI/180.0);

        QVector3D* pos = (QVector3D*)malloc(sizeof(QVector3D)*8);
        pos[0] = QVector3D(oldPosX,0,oldPosY);
        pos[1] = QVector3D(posX,0,posY);
        pos[2] = QVector3D(0,hauteurToit,0);
        pos[3] = QVector3D(0,hauteurToit,0);

        pos[4] = QVector3D(oldPosX,0-epaisseurToit,oldPosY);
        pos[5] = QVector3D(posX,0-epaisseurToit,posY);
        pos[6] = QVector3D(0,hauteurToit-epaisseurToit,0);
        pos[7] = QVector3D(0,hauteurToit-epaisseurToit,0);

        //Toit
        createCubeParam(pos,matGlobale*matWorld, QVector3D(0,hauteurToit*11/10,0),QVector3D(0,0,0),QVector3D(1,1,1),texture);

        GLfloat murPosX = (lenght)*cos(rot *PI/180.0);
        GLfloat murPosY = (lenght)*sin(rot *PI/180.0);
        createPerforedPlank(matGlobale*matWorld, QVector3D(hauteurToit*4/3,hauteurToit/10,hauteurToit),QVector2D(hauteurToit/3,hauteurToit/3),QVector3D(murPosX,hauteurToit/2,murPosY),QVector3D(90,0,90+rot),QVector3D(1,1,1),texture);



        rot += rotPas;
        oldPosX = posX;
        oldPosY = posY;
    }

}


void CastleWindow::generateEnceinte(GLfloat lenght, int nbCotesEnceinte, attributesElement attrMur, int typeTour,attributesElement attrTour, QMatrix4x4 matGlobale, QVector3D translate, QVector3D rotate, QVector3D scale, int texture, int details){
    QMatrix4x4 matWorld = createMatrixWorld(translate,rotate,scale);

    int rd = randomDetails(details);
    int tT = randomDetails(typeTour,2);
    if(attrMur.details == 0) attrMur.details = rd;
    if(attrTour.details == 0) attrTour.details = rd;


    GLfloat rotPas = 360.0 / nbCotesEnceinte;
    GLfloat rot = 0;

    //Création des murs
    GLfloat radius = ((lenght*(attrMur.dimensions.x() + attrTour.dimensions.x()) )/ tan((90 - (rotPas/2)) * PI/180.0));
    //GLfloat radius = (lenght*(attrMur.dimensions.x()+attrTour.dimensions.x()) / 2.0) / (tan(rotPas /2.0));//lenght*attrMur.dimensions.x();
    for(int i = 0; i < nbCotesEnceinte ; i++){
        //Création du point central du mur
        GLfloat posX = radius*cos(rot *PI/180.0);
        GLfloat posZ = radius*sin(rot *PI/180.0);
        QVector3D pos = QVector3D(posX,0,posZ);
        createMur(lenght,attrMur.dimensions,matGlobale*matWorld,pos,QVector3D(0,270-rot,0),QVector3D(1,1,1),attrMur.texture,attrTour.details);

        rot += rotPas;
    }

    rot = rotPas/2.0;
    //Création des tours
    for(int i = 0; i < nbCotesEnceinte ; i++){
        //Création du point central de la tour
        GLfloat posX = radius*cos(rot *PI/180.0);
        GLfloat posZ = radius*sin(rot *PI/180.0);
        GLfloat posY = attrTour.dimensions.y() - attrMur.dimensions.y();
        QVector3D pos = QVector3D(posX,posY,posZ);
        if(tT == typeTourCylindre){
            createTourCylinder(lenght, QVector2D(attrTour.dimensions.x()*coeffMultTour,attrTour.dimensions.y()),matGlobale*matWorld,pos,QVector3D(0,270-rot,0),QVector3D(1,1,1),attrTour.texture,attrTour.details);
        }else{
            createTour(lenght,attrTour.dimensions,matGlobale*matWorld,pos,QVector3D(0,270-rot,0),QVector3D(1,1,1),attrTour.texture,attrTour.details);
        }

        rot += rotPas;
    }


}











