#ifndef CASTLEWINDOW
#define CASTLEWINDOW

#include "openglwindow.h"
#include "camera.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>

#include <QtCore/qmath.h>

#include <QMouseEvent>
#include <QKeyEvent>

#include <QOpenGLTexture>

#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GL/gl.h>

static const char *vertexShaderSource =
    "#version 120\n"
    "attribute highp vec4 posAttr;\n" //Positions des vertex
    "attribute highp vec3 nmAttr;\n"  //Positions des vertex normales
    "attribute lowp vec2 uvAttr;\n"   //Position de la texture
    "attribute highp vec3 tangent;\n" //Vecteur tangent
    "attribute highp vec3 bitangent;\n"//Vecteur bitangent

                                     //A transmettre au fragment shader
    "varying lowp vec2 uv;\n"        //Positions de la texture
    "varying lowp vec4 p;\n"         //Positions du vertex
    "varying lowp vec3 fragPos;\n"   //Positions du fragment
    "varying lowp vec3 nm;\n"        //Positions des vertex normales
    "varying lowp vec3 tangentLightPos;\n" //Les tangents pour la lumière
    "varying lowp vec3 tangentViewPos;\n"  //vue
    "varying lowp vec3 tangentFragPos;\n"  // fragment


    "uniform highp mat4 matrix;\n"    //WorldMatrix * ViewMatrix * ProjectionMatrix
    "uniform lowp vec3 camPos;\n"    //Vecteur de position de la camera
    "uniform lowp vec3 lightPos;\n"  //Position de la lumière
    "uniform highp mat4 model;\n"     //WorldMatrix
    "void main() {\n"
    "   uv = uvAttr;\n"
    "	p = posAttr;\n"
    "	nm = nmAttr;\n"
    "   fragPos = vec3(model * posAttr);\n"

    "   mat3 normalMatrix = transpose(inverse(mat3(model)));\n"
    "   vec3 T = normalize(normalMatrix * tangent);\n"
    "   vec3 B = normalize(normalMatrix * bitangent);\n"
    "   vec3 N = normalize(normalMatrix * nmAttr);\n"

    "   mat3 TBN = transpose(mat3(T, B, N));\n"
    "   tangentLightPos = TBN * lightPos;\n"
    "   tangentViewPos  = TBN * camPos;\n"
    "   tangentFragPos  = TBN * fragPos;\n"

    "	vec4 wvp_pos = matrix * posAttr;\n"
    "   gl_Position = wvp_pos;\n"
    "}\n";

static const char *fragmentShaderSource =
    "#version 120\n"
    "varying lowp vec2 uv;\n"
    "varying lowp vec3 nm;\n"
    "varying lowp vec4 p;\n"
    "varying lowp vec3 fragpos;\n"
    "varying lowp vec3 tangentLightPos;\n"
    "varying lowp vec3 tangentViewPos;\n"
    "varying lowp vec3 tangentFragPos;\n"

    "uniform sampler2D tex;\n"
    "uniform sampler2D nmTex;\n"

    //"uniform float near;\n"
    //"uniform float far;\n "
/*
        "void main()\n"
        "{\n"
            "float depth = ( 2.0 * near * far) / (far + near - (gl_FragCoord.z * 2.0 - 1.0) * (far -near) )/ far;\n" // divide by far for demonstration
            "gl_FragColor = vec4(vec3(depth), 1.0f);\n"
        "}\n";
*/
    "void main() {\n"
   // "   gl_FragDepth = ( 2.0 * near * far) / (far + near - (gl_FragCoord.z * 2.0 - 1.0) * (far -near) )/ far;\n"
        // Obtain normal from normal map in range [0,1]
    "       vec3 normal = texture2D(nmTex, uv).rgb;\n"
        // Transform normal vector to range [-1,1]
    "       normal = normalize(normal * 2.0 - 1.0);\n"// this normal is in tangent space
        // Get diffuse color
    "       vec3 color = texture2D(tex, uv).rgb;\n"
        // Ambient
    "       vec3 ambient = 0.8 * color;\n"
        // Diffuse
    "       vec3 lightDir = normalize(tangentLightPos - tangentFragPos);"
    "       float diff = max(dot(lightDir, normal), 0.0);\n"
    "       vec3 diffuse = diff * color;\n"
        // Specular
    "       vec3 viewDir = normalize(tangentViewPos - tangentFragPos);\n"
    "       vec3 reflectDir = reflect(-lightDir, normal);\n"
    "       vec3 halfwayDir = normalize(lightDir + viewDir);\n"
    "       float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);\n"
    "       vec3 specular = vec3(0.2) * spec;\n"
    "       gl_FragColor = vec4(ambient + diffuse + specular, 1.0f);\n"
    "}\n";




class CastleWindow : public OpenGLWindow
{
public:
    CastleWindow();

    void initialize() Q_DECL_OVERRIDE;
    void render() Q_DECL_OVERRIDE;

    //Récupérer les evenements souris/clavier, puis les transmettre à la caméra
    void mouseMoveEvent ( QMouseEvent * event );
    void keyPressEvent(QKeyEvent *event);

    //Renvoie la matrix avec les modifications des 3 vecteurs (permet de modifier la matrice pour chaque dessin)
    QMatrix4x4 createMatrixWorld(QVector3D translate, QVector3D rotate, QVector3D scale);

    //Initiliase les textures, 0 pour le ground, 1 pour les murs
    void initTextures(int nbTexture);

    //Pos est le centre de la face
    //Texture est la texture à utiliser, 0 pour le sol, 1 pour les murs
    void createFace(QVector3D pos, GLfloat lenghtX, GLfloat lenghtY,
                    QMatrix4x4 matGlobale, QVector3D translate = QVector3D(0,0,0),
                    QVector3D rotate = QVector3D(0,0,0), QVector3D scale = QVector3D(1,1,1),int texture = 1);

    //On crée une face triangle
    void createFaceTriangle(QVector3D pos, GLfloat lenghtX, GLfloat lenghtY,
                    QMatrix4x4 matGlobale, QVector3D translate = QVector3D(0,0,0),
                    QVector3D rotate = QVector3D(0,0,0), QVector3D scale = QVector3D(1,1,1),int texture = 1);

    //On crée une face dont on donne les 4 points
    void createFaceParam(QVector3D pos1, QVector3D pos2, QVector3D pos3, QVector3D pos4,
                    QMatrix4x4 matGlobale, QVector3D translate = QVector3D(0,0,0),
                    QVector3D rotate = QVector3D(0,0,0), QVector3D scale = QVector3D(1,1,1),int texture = 1);



    void createCube(GLfloat lenght, QMatrix4x4 matGlobale, QVector3D translate = QVector3D(0,0,0),
                    QVector3D rotate = QVector3D(0,0,0), QVector3D scale = QVector3D(1,1,1),int texture = 1);

    //On crée un cube dont on donne les 8 points
    void createCubeParam(QVector3D* pos, QMatrix4x4 matGlobale, QVector3D translate = QVector3D(0,0,0),
                    QVector3D rotate = QVector3D(0,0,0), QVector3D scale = QVector3D(1,1,1),int texture = 1);


    //nbCubes stocke le nombre de cubes à afficher sur chaque dimension
    void createMur(GLfloat dim, QVector3D nbCubes, QMatrix4x4 matGlobale, QVector3D translate = QVector3D(0,0,0),
                    QVector3D rotate = QVector3D(0,0,0), QVector3D scale = QVector3D(1,1,1),int texture = 1, int rdCre = 0);

    //rdCre détermine si on a les créneaux : 1 - Prolongement Extérieur   2 - Prolongement Haut 0 - Random
    //nbCube est le nombre de cubes nécessitant des créneaux
    void createCrenelage(GLfloat lenght, GLfloat nbCubes, QMatrix4x4 matGlobale, QVector3D translate = QVector3D(0,0,0),
                    QVector3D rotate = QVector3D(0,0,0), QVector3D scale = QVector3D(1,1,1),int texture = 1, int rdCre = 0);

    void createTour(GLfloat dim, QVector3D nbCubes, QMatrix4x4 matGlobale, QVector3D translate = QVector3D(0,0,0),
                        QVector3D rotate = QVector3D(0,0,0), QVector3D scale = QVector3D(1,1,1),int texture = 1, int rdCre = 0);


    void createHautTour(GLfloat lenght, QVector2D nbCubes, QMatrix4x4 matGlobale, QVector3D translate = QVector3D(0,0,0),
                        QVector3D rotate = QVector3D(0,0,0), QVector3D scale = QVector3D(1,1,1),int texture = 1, int rdCre = 0);

    //Lenght est la longueur entre le centre du cylinder et le milieu de l'arête opposée d'une face
    //nbCubes.x est le nombre de triangle formant la base (4 pour un carré)
    //nbCubes.y est le nombre de faces en hauteur
    void createTourCylinder(GLfloat lenght, QVector2D nbCubes,QMatrix4x4 matGlobale, QVector3D translate = QVector3D(0,0,0),
                        QVector3D rotate = QVector3D(0,0,0), QVector3D scale = QVector3D(1,1,1),int texture = 1, int rdCre = 0);

    void createHautTourCylinder(GLfloat lenght, QVector2D nbCubes, QMatrix4x4 matGlobale, QVector3D translate = QVector3D(0,0,0),
                        QVector3D rotate = QVector3D(0,0,0), QVector3D scale = QVector3D(1,1,1),int texture = 1, int rdCre = 0);


private:
    GLuint loadShader(GLenum type, const char *source);

    //Les différents paramètres à passer aux shaders
    GLuint m_posAttr;
    GLuint m_uvAttr;
    GLuint m_matrixUniform;
    GLuint m_camPos;
    GLuint m_nmAttr;
    GLuint m_tangent;
    GLuint m_bitangent;

    QOpenGLShaderProgram *m_program;
    int m_frame;

    //Textures
    QOpenGLTexture *texGround;
    QOpenGLTexture *texWall;
    QOpenGLTexture *nmWall;

    //Gestion de la camera
    Camera cam;


};


#endif // CASTLEWINDOW

