#include <string.h>
#include <windows.h>  // for MS Windows
#include <GL/glut.h>  // GLUT, include glu.h and gl.h
#include <Math.h>     // Needed for sin, cos
#include <iostream>
using namespace std;

#define PI 3.14159265f

 // Global variables
char title[] = "Lebanese Judge's Gavel";  // title
int windowWidth = 640;     // Windowed mode's width
int windowHeight = 480;     // Windowed mode's height
int windowPosX = 50;      // Windowed mode's top-left corner x
int windowPosY = 50;      // Windowed mode's top-left corner y
GLfloat angleCube = 0.0f;     // Rotational angle for 3D drawings
GLfloat control_3d = -1.0f;
GLfloat speed_3d_Saved= control_3d; // To support resume

GLfloat cercleRadius = 0.5f;   // Cercle Radius that contains the bouncing Gavel
GLfloat cercleX = 0.0f;        // Cercle center (x, y) position that contains the bouncing Gavel
GLfloat cercleY = 0.0f;


GLfloat cercleXMax, cercleXMin, cercleYMax, cercleYMin; // Cercle center (x, y) bounds
GLfloat xSpeed = 0.013f;      // to adjust Gavel's speed in x and y directions
GLfloat ySpeed = 0.01f;

// colors
GLfloat RED_RGB[] = { 1.0, 0.0, 0.0 };
GLfloat WHITE_RGB[] = { 1.0, 1.0, 1.0 };
GLfloat BROWN1_RGB[] = { 0.9f, 0.5f, 0.0f, 0.0f };
GLfloat BROWN2_RGB[] = { 0.5f, 0.0f, 0.0f }; 
GLfloat Green_RGB[] = { 0.2f, 0.7f, 0.2f };

int refreshMillis = 30;      // Refresh period in milliseconds

/* Set range for world coordinates. */
GLfloat xwcMin = 0.0, xwcMax = 225.0;
GLfloat ywcMin = 0.0, ywcMax = 225.0;

class wcPt2D {
public:
    GLfloat x, y;
};

class wcPt3D {
public:
    GLfloat x, y, z;
};
typedef GLfloat Matrix3x3[3][3];
Matrix3x3 matComposite;
const GLdouble pi = 3.14159;

// Projection clipping area
GLdouble clipAreaXLeft, clipAreaXRight, clipAreaYBottom, clipAreaYTop;

bool fullScreenMode = true; // Full-screen or windowed mode?
bool paused = false;         // Movement paused or resumed
static bool isReversed = false;                 // draw reversed colors?
GLfloat xSpeedSaved, ySpeedSaved;  // To support resume

/* Initialize OpenGL Graphics */
void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
    glClearDepth(1.0f);                   // Set background depth to farthest
    glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
    glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
    glShadeModel(GL_SMOOTH);   // Enable smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections
}

///////////////////////////////////////////
/* Construct the 3 x 3 identity matrix. */
void matrix3x3SetIdentity(Matrix3x3 matIdent3x3)
{
    GLint row, col;
    for (row = 0; row < 3; row++)
        for (col = 0; col < 3; col++)
            matIdent3x3[row][col] = (row == col);
}

/* Premultiply matrix m1 times matrix m2, store result in m2. */
void matrix3x3PreMultiply(Matrix3x3 m1, Matrix3x3 m2)
{
    GLint row, col;
    Matrix3x3 matTemp;
    for (row = 0; row < 3; row++)
        for (col = 0; col < 3; col++)
            matTemp[row][col] = m1[row][0] * m2[0][col] +
            m1[row][1] * m2[1][col] +
            m1[row][2] * m2[2][col];
    for (row = 0; row < 3; row++)
        for (col = 0; col < 3; col++)
            m2[row][col] = matTemp[row][col];
}

// Since the rotation matrix only works for those rotations centered to the origin((0, 0)), 
// i need to switch the origin to the center of my object in a back - and -forth manner.
// Say my object locates at(Xr, Yr).Then if i would like to rotate centered to that object, 
// i need to first translate(-Xr, -Yr), then do the rotation, 
// then translate(Xr, Yr) back to the original position in order to perform rotation center to my object.

void transformationSequence(wcPt2D pivotPt, GLfloat theta)
{
    Matrix3x3 matRot;
    /* Initialize rotation matrix to identity. */
    matrix3x3SetIdentity(matRot);
    matRot[0][0] = cos(theta);
    matRot[0][1] = -sin(theta);
    matRot[0][2] = pivotPt.x * (1 - cos(theta)) + pivotPt.y * sin(theta);

    matRot[1][0] = sin(theta);
    matRot[1][1] = cos(theta);
    matRot[1][2] = pivotPt.y * (1 - cos(theta)) - pivotPt.x * sin(theta);
    /* Concatenate matRot with the composite matrix. */
    matrix3x3PreMultiply(matRot, matComposite);
}


/* Using the composite matrix, calculate transformed coordinates. */
void transformVerts2D(GLint nVerts, wcPt2D* verts)
{
    GLint k;
    GLfloat temp;
    for (k = 0; k < nVerts; k++) {
        temp = matComposite[0][0] * verts[k].x
            + matComposite[0][1] * verts[k].y
            + matComposite[0][2];

        verts[k].y = matComposite[1][0] * verts[k].x
            + matComposite[1][1] * verts[k].y
            + matComposite[1][2];
        verts[k].x = temp;
    }
}



void stroke_output(GLfloat x, GLfloat y, const char* format, ...)
{
    va_list args;
    char buffer[200], * p;

    va_start(args, format);
    vsprintf_s(buffer, format, args);
    va_end(args);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(0.001, 0.001, 0.001);
    glColor3fv(WHITE_RGB);
    for (p = buffer; *p; p++)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p); // GLUT_STROKE_MONO_ROMAN
    glPopMatrix();
}

void RotatePolygonCoordinates(wcPt2D* verts1, wcPt2D* verts2, wcPt2D* verts3, wcPt2D* verts4)
{
    //////////////// Define initial position for polygon:
    GLint nVerts = 4;

    // Calculate position of polygon centroid:
    wcPt2D centroidPt1;
    GLint k, xSum1 = 0, ySum1 = 0;
    for (k = 0; k < nVerts; k++) {
        xSum1 += verts1[k].x;
        ySum1 += verts1[k].y;
    }
    for (k = 0; k < nVerts; k++) {
        xSum1 += verts2[k].x;
        ySum1 += verts2[k].y;
    }
    for (k = 0; k < nVerts; k++) {
        xSum1 += verts3[k].x;
        ySum1 += verts3[k].y;
    }
    for (k = 0; k < nVerts; k++) {
        xSum1 += verts4[k].x;
        ySum1 += verts4[k].y;
    }

    centroidPt1.x = GLfloat(xSum1) / GLfloat(nVerts);
    centroidPt1.y = GLfloat(ySum1) / GLfloat(nVerts);


    // Set geometric transformation parameters:
    wcPt2D pivPt, fixedPt;
    pivPt = centroidPt1;
    fixedPt = centroidPt1;
    GLfloat tx = 0.0, ty = 100.0;
    GLfloat sx = 0.0, sy = 0.0;
    GLdouble theta = -pi / 2.0;

    matrix3x3SetIdentity(matComposite); // Initialize composite matrix to identity

    /* Construct composite matrix for transformation sequence */
    transformationSequence(pivPt, theta);

    /* Apply composite matrix to polygon vertices */
    transformVerts2D(nVerts, verts1);
    transformVerts2D(nVerts, verts2);
    transformVerts2D(nVerts, verts3);
    transformVerts2D(nVerts, verts4);

    glTranslatef(-0.76f, -1.94f, 0.0f);  // Translate to (xPos, yPos) // here i can adjust the boundaries of the bouncing Gavel
}



////////////////////////////////////////////////////
void squareIn3D(wcPt3D* verts, GLfloat* color) {
    GLint k;

    glColor3fv(color);
    for (k = 0; k < 4; k++)
        glVertex3f(verts[k].x, verts[k].y, verts[k].z);
}

void POLYGON(wcPt2D* verts, int length)
{
    GLint k;
    glBegin(GL_POLYGON);
    for (k = 0; k < length; k++)
        glVertex2f(verts[k].x, verts[k].y);
    glEnd();
}

void Triangle(wcPt2D* verts)
{
    GLint k;
    glBegin(GL_TRIANGLES);

    for (k = 0; k < 3; k++)
        glVertex2f(verts[k].x, verts[k].y);
    glEnd();
}
void strikingSurface(GLfloat* topcolor, GLfloat* buttomColor) { // Judge's block
    glColor4fv(topcolor);
    glRectf(-0.525, -0.65, -1.225, -0.8);         // draw the striking surface of the Judge's Gavel (upper part)
    glColor3fv(buttomColor);
    glRectf(-0.325, -0.9, -1.425, -0.8);         // striking surface of the Judge's Gavel (lower part)
    // left x, buttom y, right x, top y
}


void Gavel (GLfloat* mheadColor, GLfloat* bodyColor) { // Judge's Gavel
    const int n = 4;
    wcPt2D verts1[n] = { {0.446, 0.933}, {0.683, 0.516}, {1.106, 0.763}, {0.866, 1.176} };
    wcPt2D verts2[n] = { {0.946, 1.026}, {1.73, 1.516}, {1.82, 1.336}, {1.02, 0.913} };
    wcPt2D verts3[n] = { {0.296, 1.086},  {0.353, 0.99}, {0.86, 1.283} , {0.8 , 1.383} };
    wcPt2D verts4[n] = { {0.673, 0.43},  {1.186, 0.716}, {1.246, 0.606} , {0.736, 0.316} };

    RotatePolygonCoordinates(verts1, verts2, verts3, verts4);

    glColor4fv(mheadColor);
    POLYGON(verts1, n); // draw the Main head of the hammer
    glColor4fv(bodyColor);
    POLYGON(verts2, n); // draw the body of the hammer
    POLYGON(verts3, n); // draw the upper part of the hammer head
    POLYGON(verts4, n); // draw the lower part of the hammer head
}
///////////////////////////////////////////////////////
//  Lebanese Flag
void Flag(GLfloat* Color1, GLfloat* Color2, GLfloat* Color3, GLfloat* Color4) {
    //  <Drawing the Lebanese Flag>
    glColor3fv(Color1);                      // set rectangle color
    glRectf(-0.25, 0.55, -1.5, 0.8);          // draw rectangle  // (x1,y1, x2,y2)// // bottom right, top left
    glColor3fv(Color2);                    // set rectangle color
    glRectf(-0.25, 0.05, -1.5, 0.55);          // draw rectangle
    glColor3fv(Color1);                      // set rectangle color
    glRectf(-0.25, 0.05, -1.5, -0.2);         // draw rectangle

    /////////////////////////////////// Drawing the cedar in the flag
    wcPt2D vertsT1[3] = { {-0.655, 0.12}, {-1.095, 0.12}, {-0.875,  0.55} }; // bottom right, bottom left, top
    glColor4fv(Color3);
    Triangle(vertsT1);
    glColor3fv(Color4);
    glRectf(-0.825, 0.05, -0.925, 0.12);
}
//  Lebanese Flag
void Flag1(GLfloat* Color1, GLfloat* Color2, GLfloat* Color3, GLfloat* Color4) {
    //  <Drawing the Lebanese Flag>
    glColor3fv(Color1);                      // set rectangle color
    glRectf(-0.25, 0.55, -1.5, 0.8);          // draw rectangle  // (x1,y1, x2,y2)// // bottom right, top left
   // glColor3fv(Color2);                    // set rectangle color
  //  glRectf(-0.25, 0.05, -1.5, 0.55);          // draw rectangle
    glColor3fv(Color1);                      // set rectangle color
    glRectf(-0.25, 0.05, -1.5, -0.2);         // draw rectangle

    /////////////////////////////////// Drawing the cedar in the flag
    wcPt2D vertsT1[3] = { {-0.655, 0.12}, {-1.095, 0.12}, {-0.875,  0.55} }; // bottom right, bottom left, top
    glColor4fv(Color3);
    Triangle(vertsT1);
    glColor3fv(Color4);
    glRectf(-0.825, 0.05, -0.925, 0.12);
}
////////////////////////////////////////////////
void Flag_3d(GLfloat X_lef, GLfloat Y_top, GLfloat X_rig, GLfloat Y_bot, GLfloat* Color) {
    const int n = 4;
    GLfloat Z_fro = 0.625f;  // Front face  (z = 1.0f)
    GLfloat Z_bac = -0.625f;  // Back face (z = -1.0f)

    wcPt3D verts1[n] = { {X_rig, Y_top, Z_bac}, {X_lef, Y_top, Z_bac}, {X_lef, Y_top, Z_fro}, {X_rig, Y_top, Z_fro} };
    wcPt3D verts2[n] = { {X_rig, Y_bot, Z_fro}, {X_lef, Y_bot, Z_fro}, {X_lef, Y_bot, Z_bac}, {X_rig, Y_bot, Z_bac} };
    wcPt3D verts3[n] = { {X_rig, Y_top, Z_fro}, {X_lef, Y_top, Z_fro}, {X_lef, Y_bot, Z_fro}, {X_rig, Y_bot, Z_fro} };
    wcPt3D verts4[n] = { {X_rig, Y_bot, Z_bac}, {X_lef, Y_bot, Z_bac}, {X_lef, Y_top, Z_bac}, {X_rig, Y_top, Z_bac} };
    wcPt3D verts5[n] = { {X_lef, Y_top, Z_fro}, {X_lef, Y_top, Z_bac}, {X_lef, Y_bot, Z_bac}, {X_lef, Y_bot, Z_fro} };
    wcPt3D verts6[n] = { {X_rig, Y_top, Z_bac}, {X_rig, Y_top, Z_fro}, {X_rig, Y_bot, Z_fro}, {X_rig, Y_bot, Z_bac} };

    squareIn3D(verts1, Color);
    squareIn3D(verts2, Color);
    squareIn3D(verts3, Color);
    squareIn3D(verts4, Color);
    squareIn3D(verts5, Color);
    squareIn3D(verts6, Color);

}
/////////////////////////////////////////////////

/* Callback handler for window re-paint event */
void display() {
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the screen
    // Set perspective view
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 1, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();                 // Reset the model-view matrix

    /////////////////// 3D drawings
    glTranslatef(1.6f, 0.7f, -7.0f);  // Moved to the right of the screen (x = +1.6f), & Moved slightly to the top of the screen (y = +0.7f), & far deeper into screen (z = -7)
    glRotatef(angleCube, 1.0f, 1.0f, 1.0f);  // Rotate around (1,1,1)-axis, with angle angleCube

    glBegin(GL_QUADS);                // Begin drawing the color cube with 6 quads
    Flag_3d(-0.25f, 0.55f, -1.5f, 0.8f, RED_RGB);
    glEnd();  // End of drawing color-cube

    glBegin(GL_QUADS);                // Begin drawing the color cube with 6 quads
    Flag_3d(-0.25f, 0.05f, -1.5f, 0.55f, WHITE_RGB);
    glEnd();  // End of drawing color-cube


    glBegin(GL_QUADS);                // Begin drawing the color cube with 6 quads
    Flag_3d(-0.25f, 0.05f, -1.5f, -0.2f, RED_RGB);
    glEnd();  // End of drawing color-cube

    glTranslatef(0.0f, 0.0f, 0.6251f);  // Move left and into the screen (to distant the flag from parallelepiped 0.0001f)
    Flag1(RED_RGB, WHITE_RGB, Green_RGB, BROWN2_RGB); // Drawing the Lebanese Flag

    stroke_output(-1.79, -0.4, " Lebanese Judicial System ");

    ///////////////////////////////// End of 3D drawings
    
    
    //Set ortho view
    glMatrixMode(GL_PROJECTION); // Tell opengl that we are doing project matrix work
    glLoadIdentity(); // Clear the matrix
    gluOrtho2D(clipAreaXLeft, clipAreaXRight, clipAreaYBottom, clipAreaYTop);
    cercleXMin = clipAreaXLeft + cercleRadius;
    cercleXMax = clipAreaXRight - cercleRadius;
    cercleYMin = clipAreaYBottom + cercleRadius;
    cercleYMax = clipAreaYTop - cercleRadius;

    glMatrixMode(GL_MODELVIEW); // Tell opengl that we are doing model matrix work. (drawing)
    glLoadIdentity(); // Clear the model matrix

    ///////////////////////////////// 2D drawings
    /// <Fixed Drawings>
    Flag(RED_RGB, WHITE_RGB, Green_RGB, BROWN2_RGB); // Drawing the Lebanese Flag

    if (isReversed) {
        strikingSurface(RED_RGB, BROWN2_RGB); //  <Drawing the striking surface of the Judge's Gavel>


        //////////
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluPerspective(40.0, 1.0, 0.1, 20.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        gluLookAt(0.0, 0.0, 4.0,  /* eye is at (0,0,30) */
            0.0, 0.0, 0.0,      /* center is at (0,0,0) */
            0.0, 1.0, 0.);      /* up is in postivie Y direction */
        glPushMatrix();
        glTranslatef(0, 0, -4);
        glRotatef(50, 0, 1, 0);
        stroke_output(-2.5, -1.1, "Judicial Decision is issued!");
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        //////////////////

    }
    else
        strikingSurface(BROWN1_RGB, BROWN2_RGB); //  <Drawing the striking surface of the Judge's Gavel>
    ////////////////////////////////////////////

    /// <Animated Drawing>
    glTranslatef(cercleX, cercleY, 0.0f);  // Translate to (xPos, yPos)
    Gavel(BROWN2_RGB, BROWN1_RGB); // drawing Judge's Gavel
    ///////////////////////////////// End of 2D drawings


    glutSwapBuffers();  // Swap front and back buffers (of double buffered mode)

    // Animation Control - compute the location for the next refresh
    cercleX += xSpeed;
    cercleY += ySpeed;

    // Check if the cercle exceeds the edges
    if (cercleX > cercleXMax) {
        cercleX = cercleXMax;
        xSpeed = -xSpeed;
    }
    else if (cercleX < cercleXMin+0.2) {
        cercleX = cercleXMin +0.2;
        xSpeed = -xSpeed;
    }
    if (cercleY > cercleYMax-0.5) {
        cercleY = cercleYMax-0.5;
        ySpeed = -ySpeed;
    }
    else if (cercleY < cercleYMin) {
        cercleY = cercleYMin;
        ySpeed = -ySpeed;
    }

    angleCube = angleCube + control_3d; // speed (3D objects), if the value which is used tp incremented/decremented angleCube is increased, then speed increase 
                        // the use of incrementation or decrementation will decide the rotation direction
}

// Call back when the windows is re-sized
void reshape(GLsizei width, GLsizei height) {
    // Compute aspect ratio of the new window
    if (height == 0) height = 1;                // To prevent dividing by 0
    GLfloat aspect = (GLfloat)width / (GLfloat)height;
    // Set the viewport to cover the new window
    glViewport(0, 0, width, height);

    // Set the aspect ratio of the clipping area to match the viewport
    glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
    
    glLoadIdentity();             // Reset the projection matrix
    
// Enable perspective projection with fovy, aspect, zNear and zFar
    
    if (width >= height) {
        clipAreaXLeft = -1.0 * aspect;
        clipAreaXRight = 1.0 * aspect;
        clipAreaYBottom = -1.0;
        clipAreaYTop = 1.0;
    }
    else {
        clipAreaXLeft = -1.0;
        clipAreaXRight = 1.0;
        clipAreaYBottom = -1.0 / aspect;
        clipAreaYTop = 1.0 / aspect;
    }
}

/* Called back when the timer expired */
void Timer1(int value) {
    glutPostRedisplay();    // Post a paint request to activate display()
    glutTimerFunc(refreshMillis, Timer1, 0); // subsequent timer call at milliseconds
}


/* Callback handler for normal-key event */
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27:     // ESC key
        exit(0);
        break;
    }
}

/* Callback handler for special-key event */
void specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_F1:    // F1: Toggle between full-screen and windowed mode
        fullScreenMode = !fullScreenMode;         // Toggle state
        if (fullScreenMode) {                     // Full-screen mode
            windowPosX = glutGet(GLUT_WINDOW_X); // Save parameters for restoring later
            windowPosY = glutGet(GLUT_WINDOW_Y);
            windowWidth = glutGet(GLUT_WINDOW_WIDTH);
            windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
            glutFullScreen();                      // Switch into full screen
        }
        else {                                         // Windowed mode
            glutReshapeWindow(windowWidth, windowHeight); // Switch into windowed mode
            glutPositionWindow(windowPosX, windowPosX);   // Position top-left corner
        }
        break;
    case GLUT_KEY_UP:    // Up: increase the speed of “Judge’s Gavel” in 2D
        xSpeed *= 1.05f; 
        ySpeed *= 1.05f; 
        break;
    case GLUT_KEY_DOWN:     // Down: decrease the speed of “Judge’s Gavel” in 2D
        xSpeed *= 0.95f;
        ySpeed *= 0.95f; 
        break;

    case GLUT_KEY_RIGHT:       // Rigth: Increase the rotation speed of 3D drawings
        control_3d *= 1.05f; 
        break;
    case GLUT_KEY_LEFT:     // Left: Decrease rotation 3d speed of 3D drawings
        control_3d *= 0.95f; break;

    case GLUT_KEY_F2:       // reverse the rotation direction
        control_3d *= -1.0f; break;

    }
}

/* Callback handler for mouse event */
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) { // Pause/resume
        paused = !paused;         // Toggle state
        if (paused) {
            isReversed = !isReversed;      // reverse drawing colors
            glutPostRedisplay();

            xSpeedSaved = xSpeed;  // Save parameters for restore later
            ySpeedSaved = ySpeed;
            xSpeed = 0;            // Stop movement
            ySpeed = 0;

            speed_3d_Saved = control_3d;  // Save parameters for restore later
            control_3d = 0; // pause rotation of 3D drawings
        }
        else {
            isReversed = !isReversed;  // reverse drawing colors
            glutPostRedisplay();
            
            xSpeed = xSpeedSaved;  // Restore parameters
            ySpeed = ySpeedSaved;

            control_3d = speed_3d_Saved; // resume rotation of 3D drawings
        }
    }
}

/* Main function: GLUT runs as a console application starting at main() */
int main(int argc, char** argv) {
    glutInit(&argc, argv);            // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE); // Enable double buffered mode
    glutInitWindowSize(windowWidth, windowHeight);  // Initial window width and height
    glutInitWindowPosition(windowPosX, windowPosY); // Initial window top-left corner (x, y)
    glutCreateWindow(title);      // Create window with given title

    glutDisplayFunc(display);     // Register callback handler for window re-paint
    glutReshapeFunc(reshape);     // Register callback handler for window re-shape
    glutTimerFunc(0, Timer1, 0);   // First timer call immediately
    glutSpecialFunc(specialKeys); // Register callback handler for special-key event
    glutKeyboardFunc(keyboard);   // Register callback handler for normal-key event
    glutFullScreen();             // Put into full screen
    glutMouseFunc(mouse);   // Register callback handler for mouse event
    initGL();                     // OpenGL initialization
    glutMainLoop();               // Enter event-processing loop
    return 0;
}