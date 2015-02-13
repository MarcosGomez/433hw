

#include <iostream>
#include <math.h>

using namespace std;

#include "vgl.h"
#include "LoadShaders.h"

//enum VAO_IDs { Triangles, NumVAOs };
const GLuint Triangles = 0, RainbowTri = 1, Circle = 2, NumVAOs = 3;
//enum Buffer_IDs { ArrayBuffer, NumBuffers };
const GLuint ArrayBuffer=0, ArrayBufferRain = 1, ArrayBufferCircle = 2, NumBuffers=3;
//enum Attrib_IDs { vPosition = 0 };
const GLuint vPosition=0, vColor = 1;

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];

//Everything defaulted to fit at most this
const int MAX_VERTICES = 100;

const GLuint NumVertices = 6;
const GLuint NumRainbowVert = 3;
const GLuint NumCircleVertices = MAX_VERTICES;

GLfloat changedColor[4] = {0.0f, 0.0f, 1.0f, 1.0f};//Default blue

//0 is invisible
int trianglesVisible = 1;
int rainbowTriVisible = 1;
int circleVisible = 0;

//To change color
GLuint programColour;
GLuint programR;

//Points for circle
float xCircle[MAX_VERTICES];
float yCircle[MAX_VERTICES];

//Variables to dynamically draw circle
float radius = 0.0f;
int steps = 0;



/////////////////////////////////////////////////////
//  int
/////////////////////////////////////////////////////
void init ( int object )
{

    struct VertexData{
        GLfloat color[4];
        GLfloat position[4];
    };
    glGenVertexArrays( NumVAOs, VAOs );
    glGenBuffers( NumBuffers, Buffers );


    switch( object ){
        case Triangles:{
            //Example 1 code

            glBindVertexArray( VAOs[Triangles] );

            VertexData vertices[NumVertices] = {
                {{changedColor[0], changedColor[1],changedColor[2], trianglesVisible}, { -0.90f, -0.9f }},  // Triangle 1
                {{changedColor[0], changedColor[1],changedColor[2], trianglesVisible}, { 0.85, -0.90 }},
                {{changedColor[0], changedColor[1],changedColor[2], trianglesVisible}, { -0.90, 0.85 }},

                {{changedColor[0], changedColor[1],changedColor[2], trianglesVisible}, { 0.90, -0.85 }}, // Triangle 2
                {{changedColor[0], changedColor[1],changedColor[2], trianglesVisible}, { 0.90, 0.90 }},
                {{changedColor[0], changedColor[1],changedColor[2], trianglesVisible}, { -0.85, 0.90 }}
            };


            glBindBuffer( GL_ARRAY_BUFFER, Buffers[ArrayBuffer] );
            glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );

            ShaderInfo  shaders[] = {
                { GL_VERTEX_SHADER, "triangles.vert" },
                { GL_FRAGMENT_SHADER, "triangles.frag" },
                { GL_NONE, NULL }
            };

            GLuint program = LoadShaders( shaders );
            glUseProgram( program );


            glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(0));
            glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(sizeof(vertices[0].color)));


            glEnableVertexAttribArray( vColor );
            glEnableVertexAttribArray( vPosition );
        }
        break;


        case RainbowTri:{
            //Rainbow Triangle

            glBindVertexArray( VAOs[RainbowTri] );

            VertexData verticesRain[NumRainbowVert] = {
                {{   0, 1, 0, rainbowTriVisible }, { 0.80, -0.4 }}, // Rainbow Triangle 3
                {{   1, 0, 0, rainbowTriVisible }, { 0.0, 0.7 }},     //red
                {{   0, 0, 1, rainbowTriVisible }, { -0.8, -0.4 }} //blue
            };

            glBindBuffer( GL_ARRAY_BUFFER, Buffers[ArrayBufferRain] );
            glBufferData( GL_ARRAY_BUFFER, sizeof(verticesRain), verticesRain, GL_STATIC_DRAW );

            ShaderInfo  shadersR[] = {
                { GL_VERTEX_SHADER, "triangles.vert" },
                { GL_FRAGMENT_SHADER, "triangles.frag" },
                { GL_NONE, NULL }
            };

            programR = LoadShaders( shadersR );
            glUseProgram( programR );


            glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(0));
            glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(sizeof(verticesRain[0].color)));


            glEnableVertexAttribArray( vColor );
            glEnableVertexAttribArray( vPosition );

        }
        break;

        case Circle:{
            glBindVertexArray( VAOs[Circle] );

            //Calculate each of the vertices based on steps and radius
            float angle = (2*3.14159)/(steps-2);
            float newAngle = angle;

            for(int i = 0; i < steps; i++){
                if(i == 0){
                    xCircle[i] = yCircle[i] = 0.0f;
                }else if(i == 1){
                    xCircle[i] = radius;
                    yCircle[i] = 0.0f;
                }else{
                    xCircle[i] = radius*cos(newAngle);
                    yCircle[i] = radius*sin(newAngle);

                    newAngle += angle;
                }

            }
            //Initialize all 100 vertices
            //Max 100 vertices
            VertexData verticesCircle[NumCircleVertices] = {
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[0], yCircle[0] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[1], yCircle[1] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[2], yCircle[2] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[3], yCircle[3] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[4], yCircle[4] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[5], yCircle[5] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[6], yCircle[6] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[7], yCircle[7] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[8], yCircle[8] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[9], yCircle[9] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[10], yCircle[10] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[11], yCircle[11] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[12], yCircle[12] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[13], yCircle[13] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[14], yCircle[14] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[15], yCircle[15] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[16], yCircle[16] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[17], yCircle[17] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[18], yCircle[18] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[19], yCircle[19] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[20], yCircle[20] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[21], yCircle[21] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[22], yCircle[22] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[23], yCircle[23] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[24], yCircle[24] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[25], yCircle[25] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[26], yCircle[26] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[27], yCircle[27] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[28], yCircle[28] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[29], yCircle[29] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[30], yCircle[30] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[31], yCircle[31] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[32], yCircle[32] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[33], yCircle[33] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[34], yCircle[34] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[35], yCircle[35] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[36], yCircle[36] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[37], yCircle[37] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[38], yCircle[38] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[39], yCircle[39] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[40], yCircle[40] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[41], yCircle[41] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[42], yCircle[42] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[43], yCircle[43] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[44], yCircle[44] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[45], yCircle[45] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[46], yCircle[46] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[47], yCircle[47] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[48], yCircle[48] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[49], yCircle[49] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[50], yCircle[50] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[51], yCircle[51] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[52], yCircle[52] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[53], yCircle[53] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[54], yCircle[54] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[55], yCircle[55] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[56], yCircle[56] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[57], yCircle[57] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[58], yCircle[58] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[59], yCircle[59] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[60], yCircle[60] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[61], yCircle[61] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[62], yCircle[62] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[63], yCircle[63] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[64], yCircle[64] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[65], yCircle[65] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[66], yCircle[66] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[67], yCircle[67] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[68], yCircle[68] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[69], yCircle[69] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[70], yCircle[70] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[71], yCircle[71] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[72], yCircle[72] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[73], yCircle[73] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[74], yCircle[74] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[75], yCircle[75] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[76], yCircle[76] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[77], yCircle[77] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[78], yCircle[78] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[79], yCircle[79] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[80], yCircle[80] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[81], yCircle[81] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[82], yCircle[82] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[83], yCircle[83] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[84], yCircle[84] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[85], yCircle[85] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[86], yCircle[86] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[87], yCircle[87] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[88], yCircle[88] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[89], yCircle[89] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[90], yCircle[90] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[91], yCircle[91] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[92], yCircle[92] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[93], yCircle[93] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[94], yCircle[94] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[95], yCircle[95] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[96], yCircle[96] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[97], yCircle[97] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[98], yCircle[98] }},
                {{   changedColor[0], changedColor[1],changedColor[2], circleVisible }, { xCircle[99], yCircle[99] }},
            };


            glBindBuffer( GL_ARRAY_BUFFER, Buffers[ArrayBufferCircle] );
            glBufferData( GL_ARRAY_BUFFER, sizeof(verticesCircle), verticesCircle, GL_STATIC_DRAW );

            ShaderInfo  shadersC[] = {
                { GL_VERTEX_SHADER, "triangles.vert" },
                { GL_FRAGMENT_SHADER, "triangles.frag" },
                { GL_NONE, NULL }
            };

            GLuint programC = LoadShaders( shadersC );
            glUseProgram( programC );


            glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(0));
            glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(sizeof(verticesCircle[0].color)));


            glEnableVertexAttribArray( vColor );
            glEnableVertexAttribArray( vPosition );
        }
        break;

        default:
        printf("Wrong type of object to initialize\n");
        break;
    }

}


////////////////////////////////////////////////////////////////////
//	display
////////////////////////////////////////////////////////////////////
void display (void )
{
    glClear( GL_COLOR_BUFFER_BIT );

    if(trianglesVisible){
        init(Triangles);
        glBindVertexArray( VAOs[Triangles] );
        glDrawArrays( GL_TRIANGLES, 0, NumVertices );

    }

    if(rainbowTriVisible){
        init(RainbowTri);
        glBindVertexArray( VAOs[RainbowTri] );
        glDrawArrays( GL_TRIANGLES, 0, NumRainbowVert );

    }

    if(circleVisible){
        init(Circle);
        glBindVertexArray( VAOs[Circle] );
        glDrawArrays( GL_TRIANGLE_FAN, 0, NumCircleVertices );
    }
    glFlush();
}

////////////////////////////////////////////////////////////////////
//  keyboard
////////////////////////////////////////////////////////////////////
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
        case 'a':
        glEnable( GL_LINE_SMOOTH );
        break;
        case 'c':
        // color 2 triangles and circle. User provides 3 floats(0-1). default (0,0,1)
        printf("Enter 3 floats in the range [0 - 1]: ");
        float R, G, B;
        scanf("%f %f %f", &R, &G, &B);
        if((R < 0 || R > 1) || (G < 0 || G > 1) || (B < 0 || B > 1)){
            printf("!Error, values must be between 0 and 1 inclusive\n");
        }else{
            changedColor[0] = R;
            changedColor[1] = G;
            changedColor[2] = B;


            GLint loc = glGetUniformLocation(programColour, "colour");
            if(loc != -1){
              glUniform4fv(loc, 1, changedColor);
            }


        }

        break;
        case 's':
        // shaded surface display
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        break;
        case 'w':
        // wireframe display
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glLineWidth( 5.0f );

        break;
        case 'g':
        // generate geometry for the circle. User provides radius (0-1), and number of steps to make circle
        printf("Enter the radius of the circle in the range [0 - 1]: ");
        scanf("%f", &radius);
        printf("Enter the number of steps to make the circle: ");
        scanf("%d", &steps);
        if(radius < 0 || radius > 1){
            printf("!Error, radius must be between 0 and 1 inclusive\n");
            radius = 0.0f;
            steps = 0;
        }else if(steps < 0 || steps > MAX_VERTICES){
            printf("!Error, steps must be between 0 and 100 inclusive\n");
            radius = 0.0f;
            steps = 0;
        }else{
            //set visible
            circleVisible = 1;
            //Draws Circle on refresh
        }
        break;
        case 'x':
        // toggles double triangle, default on
        trianglesVisible = !trianglesVisible;
        break;
        case 'y':
        // toggles single triangle, default on
        rainbowTriVisible = !rainbowTriVisible;
        break;
        case 'z':
        // toggles circle. default is off
        circleVisible = !circleVisible;
        break;
        case '\e':
        // same as q

        case 'q':
        // quits app
        exit (0);
        break;

        default:
        break;
    }
    glutPostRedisplay(); /* this redraws the scene without
    waiting for the display callback so that any changes appear
    instantly */
}

////////////////////////////////////////////////////////////////////////
//	main
////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_3_2_CORE_PROFILE | GLUT_RGBA );
    glutInitWindowSize( 512, 512 );//512

    //glutInitContextVersion( 3, 2 );
    //glutInitContextProfile( GLUT_CORE_PROFILE );// GLUT_COMPATIBILITY_PROFILE );
    glutCreateWindow( argv[0] );

    glewExperimental = GL_TRUE;	// added for glew to work!
    if ( glewInit() )
    {
        cerr << "Unable to initialize GLEW ... exiting" << endl;
        exit (EXIT_FAILURE );
    }

    // code from OpenGL 4 Shading Language cookbook, second edition
    const GLubyte *renderer = glGetString( GL_RENDERER );
    const GLubyte *vendor = glGetString( GL_VENDOR );
    const GLubyte *version = glGetString( GL_VERSION );
    const GLubyte *glslVersion = glGetString (GL_SHADING_LANGUAGE_VERSION );
    GLint major, minor;
    glGetIntegerv( GL_MAJOR_VERSION, &major );
    glGetIntegerv( GL_MINOR_VERSION, &minor );

    cout << "GL Vendor            :" << vendor << endl;
    cout << "GL Renderer          :" << renderer  << endl;
    cout << "GL Version (string)  :" << version << endl;
    cout << "GL Version (integer) :" << major << " " << minor << endl;
    cout << "GLSL Version         :" << glslVersion << endl;
    //
    //GLint nExtensions;
    //glGetIntegerv( GL_NUM_EXTENSIONS, &nExtensions );
    //for ( int i = 0; i < nExtensions; i++ )
    //	cout << glGetStringi( GL_EXTENSIONS, i )  << endl;

    // init(Triangles);
    // init(RainbowTri);
    // init(Circle);

    glutDisplayFunc( display );

    //Added for keyboard
    glutKeyboardFunc( keyboard );

    glutMainLoop();

    return 0;
}
