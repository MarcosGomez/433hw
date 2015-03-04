// program1.cpp
//
//////////////////////////////////////////////////////////
//  program1.cpp  assignment 1 for CSC 433
//	based on example 1 from the OpenGL Red Book   Chapter 1
//////////////////////////////////////////////////////////
//#define IS_LINUX

#include <iostream>
using namespace std;

#include <math.h>
#include "vgl.h"
#include "LoadShaders.h"

const GLuint Triangles = 0, NumVAOs = 3;
const GLuint ArrayBuffer=0, NumBuffers=4;
const GLuint vPosition=0;

GLuint vaoTwoTriangles;		// vertex array objects
GLuint vaoOneTriangle = 0;
GLuint vaoCircle = 0;

GLuint vboTwoTriangles;		// vertex buffer objects
GLuint vboOneTriangleVertices;
GLuint vboOneTriangleColors;
GLuint vboCircle;

const GLuint NumVertices = 6;
bool displayTwoTriangles = true;
bool displayOneTriangle = true;
bool displayCircle = false;

GLfloat *circleVertices = NULL;	// array of 2D vertices for circle
bool wireframeRendering = false;
GLfloat radius;
GLint steps;

GLfloat red = 0.f, green = 0.f, blue = 1.f;	// colors for two triangles and circle

GLuint singleColorProgram;		// shader program for single color
GLuint gouraudColorProgram;		// shader program for Gouraud shading

/////////////////////////////////////////////////////
//  check OpenGL error
/////////////////////////////////////////////////////
void checkGLError(char *info )
{
	cerr << "Checking OpenGL error " << info << endl;
	GLenum status = glGetError();
	if ( status != GL_NO_ERROR )
	{
		switch (status)
		{
		case 1280:
			cerr << "OpenGL error, GL_INVALID_ENUM " << status << endl;
			break;
		case 1281:
			cerr << "OpenGL error, GL_INVALID_VALUE " << status << endl;
			break;
		case 1282:
			cerr << "OpenGL error, GL_INVALID_OPERATION " << status << endl;
			break;
		case 1283:
			cerr << "OpenGL error, GL_STACK_OVERFLOW " << status << endl;
			break;
		case 1284:
			cerr << "OpenGL error, GL_STACK_UNDERFLOR " << status << endl;
		case 1285:
			cerr << "OpenGL error, GL OUT OF MEMORY " << status << endl;
			break;
		}
	}
}

/////////////////////////////////////////////////////
//  int
/////////////////////////////////////////////////////
void init (void )
{

	GLfloat vertices[NumVertices][2] = {
		{ -0.90f, -0.9f },	// Triangle 1
		{  0.85f, -0.9f },
		{ -0.90f,  0.85f },
		{ 0.90f, -0.85f },	// Triangle 2
		{ 0.90f,  0.90f },
		{ -0.85f, 0.90f }
	};

	GLfloat singleTriangle[3][2] = {
		{ 0.f, 0.75 } ,    // top vertex
		{ -0.75, -0.25 },
		{  0.75, -0.25 }
	};
	GLfloat singleTriangleColors[3][3] = {
		{ 1.f, 0.f, 0.f },
		{ 0.f, 0.f, 1.f },
		{ 0.f, 1.f, 0.f }
	};


	// set up the vertex array object for the two triangles
	glGenVertexArrays( 1, &vaoTwoTriangles );
	glBindVertexArray( vaoTwoTriangles );

	// set up the vertex buffer object for two triangles
	glGenBuffers( 1, &vboTwoTriangles );
	glBindBuffer( GL_ARRAY_BUFFER, vboTwoTriangles );
	// populate the position buffer
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );

	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray( 0 );

	// set up the single triangle
	glGenBuffers( 1, &vboOneTriangleVertices );
	glBindBuffer( GL_ARRAY_BUFFER, vboOneTriangleVertices );
	glBufferData( GL_ARRAY_BUFFER, sizeof( singleTriangle ),
		singleTriangle, GL_STATIC_DRAW );

	// set up the vertex array object for the one triangle
	glGenVertexArrays( 1, &vaoOneTriangle );
	glBindVertexArray( vaoOneTriangle );

	glGenBuffers( 1, &vboOneTriangleColors );
	glBindBuffer( GL_ARRAY_BUFFER, vboOneTriangleColors );
	glBufferData( GL_ARRAY_BUFFER, sizeof( singleTriangleColors ),
		singleTriangleColors, GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 ); //vertex position
	glEnableVertexAttribArray( 1 );	//vertex color

	// map index 0 to the vertexes
	glBindBuffer( GL_ARRAY_BUFFER, vboOneTriangleVertices );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, NULL );

	// map index 1 to the colors
	glBindBuffer( GL_ARRAY_BUFFER, vboOneTriangleColors );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );

	// set up the circle object
	glGenVertexArrays( 1, &vaoCircle );
	glBindVertexArray( vaoCircle );

	glGenBuffers( 1, &vboCircle );
	glBindBuffer( GL_ARRAY_BUFFER, vboCircle );
	// we'll put the data into buffer later, after it's generated
	glBufferData( GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW );


	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray( 0 );

	ShaderInfo  singleColorShaders[] = {
		{ GL_VERTEX_SHADER, "singleColor.vert" },
		{ GL_FRAGMENT_SHADER, "singleColor.frag" },
		{ GL_NONE, NULL }
	};
	ShaderInfo gouraudColorShaders[] = {
		{ GL_VERTEX_SHADER, "gouraudColor.vert" },
		{ GL_FRAGMENT_SHADER, "gouraudColor.frag" },
		{ GL_NONE, NULL }
	};

	singleColorProgram = LoadShaders( singleColorShaders );
	gouraudColorProgram = LoadShaders( gouraudColorShaders );

	glUseProgram( singleColorProgram );
	GLuint location = glGetUniformLocation( singleColorProgram, "staticColor" );
	if ( location >= 0 )
		glUniform3f( location, red, green, blue );

	glLineWidth( 5.f );	// thicker lines for wireframe
}

////////////////////////////////////////////////////////////////////
//	display
////////////////////////////////////////////////////////////////////
void display (void )
{
	glClear( GL_COLOR_BUFFER_BIT );

	glUseProgram( singleColorProgram );
	GLuint location = glGetUniformLocation( singleColorProgram, "staticColor" );
	if ( location >= 0 )
		glUniform3f( location, red, green, blue );

	if ( displayTwoTriangles )
	{
		glUseProgram( singleColorProgram );
		glBindVertexArray( vaoTwoTriangles );	// two triangles
		glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	}

	if ( displayOneTriangle )
	{
		glUseProgram( gouraudColorProgram );
		glBindVertexArray( vaoOneTriangle );	// single triangle
		glDrawArrays( GL_TRIANGLES, 0, 3 );
	}

	if ( displayCircle && circleVertices )
	{
		glUseProgram( singleColorProgram );
		glBindVertexArray( vaoCircle );
		glDrawArrays( GL_TRIANGLE_FAN, 0, steps+1 );
	}

	glFlush();
}

//////////////////////////////////////////////////////////
// keys
//////////////////////////////////////////////////////////
void keys ( unsigned char key, int x, int y )
{
	bool success = false;
	float angleInc;

	switch (key ) {
		case 27:
		case 'q':
			exit( 99 );
			break;
		case 'c':		// set color for triangles
			while ( !success )
			{
				cout << "enter color (r,g,b): ";
				cin >> red >> green >> blue;
				if ( ( 0.f <= red && red <= 1.0f ) &&
					 ( 0.f <= green && green <= 1.f ) &&
					 ( 0.f <= blue && blue <= 1.f ) )
						success = true;
				else
					cout << "\tcolor values out of range" << endl;
			}
			glutPostRedisplay();
			break;
		case 's':	// set solid surface rasterization
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			glutPostRedisplay();
			break;
		case 'w':
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			glutPostRedisplay();
			break;
		case 'g':		// parameters for circle description
			while ( !success )
			{
				cout << "enter radius and number of steps: " ;
				cin >> radius >> steps;
				if ( ( 0.f <= radius && radius <= 1.0f ) &&
					 ( steps > 0 ) )
						success = true;
				else
					cout << "\tradius or steps out of range" << endl;
			}
			// dynamically allocate storage for new circle description
			if ( circleVertices != NULL )
				free( circleVertices );
			// dynamically allocated space for circle description, with 'steps' rows of 2 columns each
			circleVertices = (GLfloat *)malloc( (steps+1) * 2 *sizeof( GLfloat ) );

			circleVertices[0] = 0.f;	// for triangle fan, need center point first
			circleVertices[1] = 0.f;

			angleInc = 3.14159f * 2.f / ( steps - 1 );
			for ( int a = 0; a < steps; a++ )
			{
				circleVertices[(a+1)*2+0] = radius * cos( a * angleInc );
				circleVertices[(a+1)*2+1] = radius * sin( a * angleInc );
			}
			// load the buffer with data
			glBindBuffer( GL_ARRAY_BUFFER, vboCircle );
			glBufferData( GL_ARRAY_BUFFER, (steps+1) * 2 * sizeof( GLfloat ), circleVertices, GL_DYNAMIC_DRAW );
			break;
		case 'x':		// toggle display of two triangles
			displayTwoTriangles = !displayTwoTriangles;
			glutPostRedisplay();
			break;
		case 'y':		// toggle display of single triangle
			displayOneTriangle = !displayOneTriangle;
			glutPostRedisplay();
			break;
		case 'z':		// toggle display of circle
			displayCircle = !displayCircle;
			glutPostRedisplay();
			break;
	}
}


////////////////////////////////////////////////////////////////////////
//	main
////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	glutInit( &argc, argv );


	#ifdef IS_LINUX
	glutInitDisplayMode(GLUT_RGBA);
	glutInitContextVersion( 4, 3 );
	glutInitContextProfile( GLUT_CORE_PROFILE );// GLUT_COMPATIBILITY_PROFILE );
	#else
	//OSX version
	glutInitDisplayMode( GLUT_3_2_CORE_PROFILE | GLUT_RGBA );
	#endif
	glutInitWindowSize( 512, 512 );
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
checkGLError( "versions" );

	cout << "GL Vendor            :" << vendor << endl;
	cout << "GL Renderer          :" << renderer  << endl;
	cout << "GL Version (string)  :" << version << endl;
	cout << "GL Version (integer) :" << major << " " << minor << endl;
	cout << "GLSL Version         :" << glslVersion << endl;

	init();
	glutDisplayFunc( display );
	glutKeyboardFunc( keys );
	glutMainLoop();

	return 0;
}
