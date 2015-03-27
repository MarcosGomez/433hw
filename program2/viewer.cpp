// viewer.cpp
//
//////////////////////////////////////////////////////////
//  assignment 2 for CSC 433
//////////////////////////////////////////////////////////
//#define IS_LINUX

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
using namespace std;

#include <math.h>
#include "vgl.h"
#include "LoadShaders.h"
#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"
#include "glm/glm/gtc/type_ptr.hpp"

#define MaxVertices 10000

const GLuint Triangles = 0, NumVAOs = 3;
const GLuint ArrayBuffer=0, NumBuffers=4;
const GLuint vPosition=0;

GLuint vaoObject;		// vertex array objects
GLuint vaoOneTriangle = 0;
GLuint vaoCircle = 0;


GLuint vboObject;		// vertex buffer objects
GLuint vboOneTriangleVertices;
GLuint vboOneTriangleColors;
GLuint vboCircle;
//
//GLuint vboVertexColors;
GLuint vboVertexNormals;
GLuint vboVertexKa;
GLuint vboVertexKd;

GLuint NumVertices = 0;
//const int MaxVertices = 300000;
const GLuint vecLength = 3;

bool displayObject = true;
bool displayOneTriangle = false;
bool displayCircle = false;

GLfloat *circleVertices = NULL;	// array of 2D vertices for circle
bool wireframeRendering = false;
GLfloat radius;
GLint steps;

GLfloat red = 0.f, green = 1.f, blue = 1.f;	// colors for two triangles and circle

GLuint singleColorProgram;		// shader program for single color
GLuint gouraudColorProgram;		// shader program for Gouraud shading

//Matrices
glm::mat4 model;//transformation, default to identity, how everything looks, what is rotated
glm::mat4 view;//camera
glm::vec3 cameraPosition;
glm::vec3 focalPoint;
glm::vec3 viewUp;
glm::mat4 proj;//projection/frustum
GLfloat fov;
GLfloat aspectRatio;
GLfloat nearPlane;
GLfloat farPlane;
GLint viewLoc;
GLint projLoc;
GLint modelLoc;

glm::vec3 light;

glm::vec3 defaultCP(3.0f, 3.0f, 1.f);
glm::vec3 defaultFP(0.0f, 0.0f, 0.0f);

//Make sure camera is looking at object
GLfloat yPlus = 0.0f;
GLfloat yMinus = 0.0f;
GLfloat xMinus = 0.0f;
GLfloat xPlus = 0.1f;
GLfloat zPlus = 0.1f;
GLfloat zMinus = 0.0f;

//Lighting
// glm::vec3 Ka(0.1f, 0.1f, 0.1f);
// glm::vec3 Kd(0.9f, 0.9f, 0.9f);
//glm::vec3 lightSource = glm::normalize(glm::vec3(-1.0f, 1.0f, 1.0f));

GLfloat vertices[MaxVertices][vecLength];
GLfloat vertexKa[MaxVertices][vecLength];
GLfloat vertexKd[MaxVertices][vecLength];
GLfloat vertexNormals[MaxVertices][vecLength];

//Made global to support multiple objects
int objCount = 0;

int vIndex = 0;
int vtIndex = 0;
int vnIndex = 0;
int vertexCount = 0;




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


void setLight(){
	// glm::normalize(glm::vec3((camX * 3.0/2.0) - focX, (camY * 3.0/2.0) - focY, (camZ * 3.0/2.0) - focZ));
	GLuint lightLoc = glGetUniformLocation( gouraudColorProgram, "light");
	glUniform3fv(lightLoc, 1, glm::value_ptr(light));
}

void recalcView(){
	view = glm::lookAt(cameraPosition, focalPoint, viewUp);

	viewLoc = glGetUniformLocation( gouraudColorProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void setView(){
	viewLoc = glGetUniformLocation( gouraudColorProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void recalcProj(){
	proj = glm::perspective(fov, aspectRatio, nearPlane, farPlane);

	projLoc = glGetUniformLocation( gouraudColorProgram, "proj");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
}

void setProj(){
	projLoc = glGetUniformLocation( gouraudColorProgram, "proj");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
}

void setModel(){
	// glm::normalize(glm::vec3((camX * 3.0/2.0) - focX, (camY * 3.0/2.0) - focY, (camZ * 3.0/2.0) - focZ));
	// glm::vec4 tempLight = glm::vec4((camX * 3.0/2.0) - focX, (camY * 3.0/2.0) - focY, (camZ * 3.0/2.0) - focZ), 1.0);
	// tempLight = tempLight * model;
	// light.x = tempLight.x;
	// light.y = tempLight.y;
	// light.z = tempLight.z;
	// setLight();
	//tan(180/3.14159) * sqrt(pow(x, 2) + pow(y, 2));
	modelLoc = glGetUniformLocation( gouraudColorProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
}



void setDefault(){

	cameraPosition = defaultCP;
	focalPoint = defaultFP;
	viewUp = glm::vec3(0.0f, 0.0f, 1.0f);

	fov = glm::radians(70.0f);
	aspectRatio = 512.0f/512.0f;
	nearPlane = 1.0f;
	farPlane = 5000.0f;

	model = glm::mat4();


}

/////////////////////////////////////////////////////
//  int
/////////////////////////////////////////////////////
void init (void )
{

	//NumVertices = 18;
	// orig[][]={
	// 	0. 0. 0.
	// 	1. 0. 0.
	// 	1. .1 0.
	// 	0. .1 0.

	// 	0. 0. 0.
	// 	0. 1. 0.
	// 	.1 1. 0.
	// 	.1 0. 0.

	// 	0. 0. 0.
	// 	0. 0. 1.
	// 	.1 0. 1.
	// 	.1 0. 0.
	// }
	GLfloat temp[MaxVertices][vecLength] = {
		//red
		{0., 0., 0.},
		{1., 0., 0.},
		{1., .1, 0.},

		{0., 0., 0.},
		{1., .1, 0.},
		{0., .1, 0.},

		//green
		{0., 0., 0.},
		{0., 1., 0.},
		{.1, 1., 0.},

		{0., 0., 0.},
		{.1, 1., 0.},
		{.1, 0., 0.},

		//blue
		{0., 0., 0.},
		{0., 0., 1.},
		{.1, 0., 1.},

		{0., 0., 0.},
		{.1, 0., 1.},
		{.1, 0., 0.}

	};

	// GLfloat tempKd[MaxVertices][vecLength] = {
	// 	//red
	// 	{1., 0., 0.},
	// 	{1., 0., 0.},
	// 	{1., 0., 0.},

	// 	{1., 0., 0.},
	// 	{1., 0., 0.},
	// 	{1., 0., 0.},

	// 	//green
	// 	{0., 1., 0.},
	// 	{0., 1., 0.},
	// 	{0., 1., 0.},

	// 	{0., 1., 0.},
	// 	{0., 1., 0.},
	// 	{0., 1., 0.},

	// 	//blue
	// 	{0., 0., 1.},
	// 	{0., 0., 1.},
	// 	{0., 0., 1.},

	// 	{0., 0., 1.},
	// 	{0., 0., 1.},
	// 	{0., 0., 1.},

	// };

	//vertices = temp;
	// for(int k = 0; k < NumVertices; k++){
	// 	vertices[k][0] = temp[k][0];
	// 	vertices[k][1] = temp[k][1];
	// 	vertices[k][2] = temp[k][2];
	// }

	//memcpy(vertices, temp, sizeof(GLuint) * NumVertices * vecLength);
	//memcpy(vertexKd, tempKd, sizeof(GLuint) * NumVertices * vecLength);


	//Calculate normals
	if(vertexNormals[0][0] == 0.0 && vertexNormals[0][1] == 0.0 && vertexNormals[0][2] == 0.0){
		cout << "Calculating Normals" << endl;
		//std::vector<glm::vec3>* normal_buffer = new std::vector<glm::vec3>[NumVertices];
		//unsigned int num_indices = NumVertices/3.0;

		//Calc for each face
		for( int i = 0; i < NumVertices - 2; i += 3 )
		{
		  // get the three vertices that make the faces
			glm::vec3 p1(vertices[i+0][0],vertices[i+0][1],vertices[i+0][2]);
			glm::vec3 p2(vertices[i+1][0],vertices[i+1][1],vertices[i+1][2]);
			glm::vec3 p3(vertices[i+2][0],vertices[i+2][1],vertices[i+2][2]);

			// Calculate the 2 vectors
			glm::vec3 v1 = p2 - p1;
			glm::vec3 v2 = p3 - p1;
			//Cross to get normal
			glm::vec3 normal = glm::cross(v1, v2);

		  glm::normalize(normal);

		  // Store the face's normal for each of the vertices that make up the face.


			vertexNormals[i+0][0] = normal.x;
			vertexNormals[i+0][1] = normal.y;
			vertexNormals[i+0][2] = normal.z;

			vertexNormals[i+1][0] = normal.x;
			vertexNormals[i+1][1] = normal.y;
			vertexNormals[i+1][2] = normal.z;

			vertexNormals[i+2][0] = normal.x;
			vertexNormals[i+2][1] = normal.y;
			vertexNormals[i+2][2] = normal.z;

		}
	}


	// set up the vertex array object for the two triangles
	glGenVertexArrays( 1, &vaoObject );
	glBindVertexArray( vaoObject );

	// set up the vertex buffer object for two triangles
	glGenBuffers( 1, &vboObject );
	glBindBuffer( GL_ARRAY_BUFFER, vboObject );
	// populate the position buffer
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );



	//Normals
	glGenBuffers( 1, &vboVertexNormals );
	glBindBuffer( GL_ARRAY_BUFFER, vboVertexNormals );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertexNormals), vertexNormals,
		GL_STATIC_DRAW );

	//Ka
	glGenBuffers( 1, &vboVertexKa );
	glBindBuffer( GL_ARRAY_BUFFER, vboVertexKa );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertexKa), vertexKa,
		GL_STATIC_DRAW );

	//Kd
	glGenBuffers( 1, &vboVertexKd );
	glBindBuffer( GL_ARRAY_BUFFER, vboVertexKd );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertexKd), vertexKd,
		GL_STATIC_DRAW );


	//glVertexAttribPointer( 0, vecLength, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray( 0 ); // position
	//glEnableVertexAttribArray( 1 ); // color
	glEnableVertexAttribArray( 1 ); // normals
	glEnableVertexAttribArray( 2 ); // Ka
	glEnableVertexAttribArray( 3 ); // Kd

	//Map 0 to vertexes
	glBindBuffer( GL_ARRAY_BUFFER, vboObject );
	glVertexAttribPointer( 0, vecLength, GL_FLOAT, GL_FALSE, 0, NULL );

	// //Map index 1 to colors
	// glBindBuffer( GL_ARRAY_BUFFER, vboVertexColors );
	// glVertexAttribPointer( 1, vecLength, GL_FLOAT, GL_FALSE, 0, NULL );

	//Map index  to normals
	glBindBuffer( GL_ARRAY_BUFFER, vboVertexNormals );
	glVertexAttribPointer(1, vecLength, GL_FLOAT, GL_FALSE, 0, NULL );

	//Map to Ka
	glBindBuffer( GL_ARRAY_BUFFER, vboVertexKa );
	glVertexAttribPointer(2, vecLength, GL_FLOAT, GL_FALSE, 0, NULL );

	//Map to Kd
	glBindBuffer( GL_ARRAY_BUFFER, vboVertexKd );
	glVertexAttribPointer(3, vecLength, GL_FLOAT, GL_FALSE, 0, NULL );





	ShaderInfo gouraudColorShaders[] = {
		{ GL_VERTEX_SHADER, "gouraudColor.vert" },
		{ GL_FRAGMENT_SHADER, "gouraudColor.frag" },
		{ GL_NONE, NULL }
	};

	//singleColorProgram = LoadShaders( singleColorShaders );
	gouraudColorProgram = LoadShaders( gouraudColorShaders );



	glLineWidth( 5.f );	// thicker lines for wireframe


	//NEW
	glUseProgram( gouraudColorProgram );

	setModel();

	recalcView();

	recalcProj();

	setLight();

	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);

}



////////////////////////////////////////////////////////////////////
//	display
////////////////////////////////////////////////////////////////////
void display (void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );



		glUseProgram( gouraudColorProgram );
		glBindVertexArray( vaoObject );	// two triangles

		glDrawArrays( GL_TRIANGLES, 0, NumVertices );


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
		case '\e':
		case 'q':
			exit( 99 );
			break;
		case 's':	// set solid surface rasterization
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

			break;
		case 'w':
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

			break;

		case 'r':		// reset camera

			setDefault();
			recalcProj();
			recalcView();
			setModel();


			break;
		case 'c':		// move camera down along view up vector
			cameraPosition -= viewUp;
			focalPoint -= viewUp;
			recalcView();

			break;
		case 'v':		// move camera up along view up vector

			cameraPosition += viewUp;
			focalPoint += viewUp;
			recalcView();

			break;
		case 'd':		// move focal point down along view up vector(tilt down)

			focalPoint -= viewUp;
			focalPoint -= viewUp;
			focalPoint -= viewUp;
			recalcView();

			break;
		case 'f':		// move focal point up along view up vector(tilt up)

			focalPoint += viewUp;
			focalPoint += viewUp;
			focalPoint += viewUp;
			recalcView();

			break;
		case 'z':		// rotate view up vector by 1 degree counter-clockwise around gaze vector
		{
			//Rotate model, not view
			glm::vec3 axis(focalPoint.x - cameraPosition.x, focalPoint.y - cameraPosition.y, focalPoint.z - cameraPosition.z);
			model = glm::rotate(model, glm::radians(1.0f), axis);

			setModel();
		}
			break;
		case 'x':		// rotate view up vector by 1 degree clockwise around gaze vector
		{

			glm::vec3 axis(focalPoint.x - cameraPosition.x, focalPoint.y - cameraPosition.y, focalPoint.z - cameraPosition.z);
			model = glm::rotate(model, glm::radians(-1.0f), axis);

			setModel();
		}
			break;
		default:
			cout << "This key doesn't do anything" << endl;

			break;
		}
	glutPostRedisplay();
}






void specialKeys(int key, int x, int y)
{
	switch(key){
		case GLUT_KEY_UP :{		// move forward along gaze vector. scale distance based on bounds. decrease fov

			// fov -= M_PI / 180.0; //Decrease fov by 1 degree
			// recalcProj();
			float divide = 60.0;
			glm::vec3 gaze((focalPoint.x - cameraPosition.x)/divide, (focalPoint.y - cameraPosition.y)/divide, (focalPoint.z - cameraPosition.z)/divide);
			cameraPosition += gaze;
			focalPoint += gaze;
			recalcView();
		}
			break;
		case GLUT_KEY_DOWN :{		// move backward along gaze vector. scale distance based on bounds

			// fov += M_PI / 180.0; //Increase fov by 1 degree
			// recalcProj();
			float divide = 60.0;
			glm::vec3 gaze((focalPoint.x - cameraPosition.x)/divide, (focalPoint.y - cameraPosition.y)/divide, (focalPoint.z - cameraPosition.z)/divide);
			//glm::vec3 gaze(focalPoint.x - cameraPosition.x, focalPoint.y - cameraPosition.y, focalPoint.z - cameraPosition.z);
			cameraPosition -= gaze;
			focalPoint -= gaze;
			recalcView();
		}
			break;
		case GLUT_KEY_LEFT :{		// rotate along z axis by 1 degree


			glm::vec3 axis(0.0f, 0.0f, 1.0f);
			model = glm::rotate(model, glm::radians(1.0f), axis);

			setModel();
		}
			break;
		case GLUT_KEY_RIGHT :{		// rotate along z axis by 1 degree


			glm::vec3 axis(0.0f, 0.0f, 1.0f);
			model = glm::rotate(model, glm::radians(-1.0f), axis);

			setModel();
		}
			break;
		default:
			cout << "This key doesn't do anything" << endl;
			break;
	}
	glutPostRedisplay();
}









////////////////////////////////////////////////////////////////////////
//	windowResize
////////////////////////////////////////////////////////////////////////
void windowResize(int width, int height)
{
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(height == 0)
		height = 1;

	//Change aspect ratio with screen
	aspectRatio = 1.0 * width/height;

	//fov = 70.0 + (height - 512.0) / 5000.0;
	//cout << "Width is " << width << " and fov is " << fov << endl;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

    // Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, width, height);// width, height


	recalcProj();
	glutPostRedisplay();


	// Set the correct perspective.
	gluPerspective(45,aspectRatio,1,1000);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);


}






//Process obj file
void readObj(const char * fileName){


	FILE * inputFile;
	FILE * mtlFile;
	char mtlLibrary[128];
	char material[128] = "empty";
	char lineHeader[128];
	
	bool foundVT = false;
	bool foundVN = false;

	bool sameAsVertex = true;
	unsigned int vertexIndex[3], uvIndex[3], normalIndex[3] = {1, 1, 1};

	
	//vertices
	GLfloat tempVertices[MaxVertices][vecLength];
	GLfloat tempVertexNormals[MaxVertices][vecLength];


	cout << fileName << endl;
	inputFile = fopen(fileName, "r");
	if(inputFile == NULL){
		cerr << "Failed to open obj file for reading" << endl;
		exit(1);
	}

	//Read .obj
	while(!feof(inputFile)){

		int res = fscanf(inputFile, "%s", lineHeader);
		if(res == EOF){
			break;
		}

		if( strcmp( lineHeader, "v" ) == 0 ){
			//Vertex locations
			glm::vec3 vertex;
			fscanf( inputFile, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			// temp_vertices.push_back(vertex);

			tempVertices[vIndex][0] = vertex.x;
			tempVertices[vIndex][1] = vertex.y;
			tempVertices[vIndex][2] = vertex.z;

			if(xPlus == xMinus && xMinus == yPlus && yPlus == yMinus &&
				yMinus == zPlus && zPlus == zMinus){
				xPlus = vertex.x;
				xMinus = vertex.x;
				yPlus = vertex.y;
				yMinus = vertex.y;
				zPlus = vertex.z;
				zMinus = vertex.z;
			}
			
			xPlus = std::max(vertex.x, xPlus);
			xMinus = std::min(vertex.x, xMinus);
			yPlus = std::max(vertex.y, yPlus);
			yMinus = std::min(vertex.y, yMinus);
			zPlus = std::max(vertex.z, zPlus);
			zMinus = std::max(vertex.z, zMinus);

			vIndex ++;
		}else if( strcmp( lineHeader, "vt" ) == 0){
			//Texture coordinates
			glm::vec2 uv;
			fscanf( inputFile, "%f %f\n", &uv.x, &uv.y );
			foundVT = true;
			// temp_uvs.push_back(uv);
			vtIndex++;

		}else if( strcmp( lineHeader, "vn" ) == 0 ){
			//Vertex normals
			glm::vec3 normal;
			fscanf(inputFile, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			foundVN = true;
			// temp_normals.push_back(normal);
			glm::normalize(normal);

			tempVertexNormals[vnIndex][0] = normal.x;
			tempVertexNormals[vnIndex][1] = normal.y;
			tempVertexNormals[vnIndex][2] = normal.z;

			vnIndex++;

		}else if( strcmp( lineHeader, "s" ) == 0){//smoothing group
			unsigned int normalPos;
			fscanf(inputFile, "%d\n", &normalPos );
			normalIndex[0] = normalPos;
			normalIndex[1] = normalPos;
			normalIndex[2] = normalPos;
			sameAsVertex = false;

		}else if( strcmp( lineHeader, "f" ) == 0 ){
			//TODO

			
			//Temperarily store info
			
			if(foundVT && foundVN){
				int matches = fscanf( inputFile, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
				&vertexIndex[0], &uvIndex[0], &normalIndex[0],
				&vertexIndex[1], &uvIndex[1], &normalIndex[1],
				&vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				if(matches != 9){
					//cerr << "There weren't 9 matches for 0/0/0 0/0/0 0/0/0, but " << matches << endl;
					fscanf( inputFile, "%d %d\n", &vertexIndex[1], &vertexIndex[2]);

					if(sameAsVertex){
						normalIndex[0] = vertexIndex[0];
						normalIndex[1] = vertexIndex[1];
						normalIndex[2] = vertexIndex[2];
					}
					

					uvIndex[0] = vertexIndex[0];
					uvIndex[1] = vertexIndex[1];
					uvIndex[2] = vertexIndex[2];
					

				}
			}else if(foundVT){
				int matches = fscanf( inputFile, "%d/%d %d/%d %d/%d\n",
				&vertexIndex[0], &uvIndex[0],
				&vertexIndex[1], &uvIndex[1],
				&vertexIndex[2], &uvIndex[2] );
				if(matches != 6){
					//cerr << "There weren't 6 matches , but " << matches << endl;
					fscanf( inputFile, "%d %d\n", &vertexIndex[1], &vertexIndex[2]);
					uvIndex[0] = vertexIndex[0];
					uvIndex[1] = vertexIndex[1];
					uvIndex[2] = vertexIndex[2];
				}
			}else if(foundVN){
				int matches = fscanf( inputFile, "%d//%d %d//%d %d//%d\n",
				&vertexIndex[0], &normalIndex[0],
				&vertexIndex[1], &normalIndex[1],
				&vertexIndex[2], &normalIndex[2] );
				if(matches != 6){
					//cerr << "There weren't 6 matches, but " << matches << endl;
					matches = fscanf( inputFile, "%d %d\n", &vertexIndex[1], &vertexIndex[2]);
					if(sameAsVertex){
						normalIndex[0] = vertexIndex[0];
						normalIndex[1] = vertexIndex[1];
						normalIndex[2] = vertexIndex[2];
					}
				}
			}else{
				int matches = fscanf( inputFile, "%d %d %d\n",
				&vertexIndex[0],
				&vertexIndex[1],
				&vertexIndex[2] );
				if(matches != 3){
					//cerr << "There weren't 3 matches , but " << matches << endl;
					fscanf( inputFile, "%d %d\n", &vertexIndex[1], &vertexIndex[2]);
				}
			}

			cout << vertexIndex[0] << " : " << vertexIndex[1] << " : " << vertexIndex[2] << endl;
			//Set the vertices in the xPlus location, face
			cout << "Setting vertices for face starting at index " << vertexCount << endl;
			vertices[vertexCount + 0][0] = tempVertices[vertexIndex[0] - 1][0];
			vertices[vertexCount + 0][1] = tempVertices[vertexIndex[0] - 1][1];
			vertices[vertexCount + 0][2] = tempVertices[vertexIndex[0] - 1][2];

			vertices[vertexCount + 1][0] = tempVertices[vertexIndex[1] - 1][0];
			vertices[vertexCount + 1][1] = tempVertices[vertexIndex[1] - 1][1];
			vertices[vertexCount + 1][2] = tempVertices[vertexIndex[1] - 1][2];

			vertices[vertexCount + 2][0] = tempVertices[vertexIndex[2] - 1][0];
			vertices[vertexCount + 2][1] = tempVertices[vertexIndex[2] - 1][1];
			vertices[vertexCount + 2][2] = tempVertices[vertexIndex[2] - 1][2];

			if(foundVN){
				cout << "Setting normals for face" << endl;
				//Set normals
				vertexNormals[vertexCount + 0][0] = tempVertexNormals[normalIndex[0] - 1][0];
				vertexNormals[vertexCount + 0][1] = tempVertexNormals[normalIndex[0] - 1][1];
				vertexNormals[vertexCount + 0][2] = tempVertexNormals[normalIndex[0] - 1][2];

				vertexNormals[vertexCount + 1][0] = tempVertexNormals[normalIndex[1] - 1][0];
				vertexNormals[vertexCount + 1][1] = tempVertexNormals[normalIndex[1] - 1][1];
				vertexNormals[vertexCount + 1][2] = tempVertexNormals[normalIndex[1] - 1][2];

				vertexNormals[vertexCount + 2][0] = tempVertexNormals[normalIndex[2] - 1][0];
				vertexNormals[vertexCount + 2][1] = tempVertexNormals[normalIndex[2] - 1][1];
				vertexNormals[vertexCount + 2][2] = tempVertexNormals[normalIndex[2] - 1][2];
			}
			


			//Find Ka and Kd values of these faces
			mtlFile = fopen(mtlLibrary, "r");
			if(mtlFile == NULL){
				cerr << "Failed to open material file for reading. Using default colors" << endl;
			}else if(strcmp(material, "empty") == 0){
				cout << "No material specified, using default" << endl;
			}else{
				//cout << "Opened " << mtlLibrary << endl;
				while(!feof(mtlFile)){
					int r = fscanf(mtlFile, "%s", lineHeader);
					if(r == EOF){
						break;
					}

					if(strcmp(lineHeader, "newmtl") == 0){
						fscanf(mtlFile, "%s\n", lineHeader);
						if(strcmp(lineHeader, material) == 0){
							while(fscanf(mtlFile, "%s", lineHeader) != EOF ){
								if(strcmp(lineHeader, "Ka") == 0){
									glm::vec3 ka;
									fscanf(mtlFile, "%f %f %f\n", &ka.x, &ka.y, &ka.z);
									// temp_Ka.push_back(ka);

									vertexKa[vertexCount + 0][0] = ka.x;
									vertexKa[vertexCount + 0][1] = ka.y;
									vertexKa[vertexCount + 0][2] = ka.z;

									vertexKa[vertexCount + 1][0] = ka.x;
									vertexKa[vertexCount + 1][1] = ka.y;
									vertexKa[vertexCount + 1][2] = ka.z;

									vertexKa[vertexCount + 2][0] = ka.x;
									vertexKa[vertexCount + 2][1] = ka.y;
									vertexKa[vertexCount + 2][2] = ka.z;

								}else if(strcmp(lineHeader, "Kd") == 0){
									glm::vec3 kd;
									fscanf(mtlFile, "%f %f %f\n", &kd.x, &kd.y, &kd.z);
									// temp_Kd.push_back(kd);

									vertexKd[vertexCount + 0][0] = kd.x;
									vertexKd[vertexCount + 0][1] = kd.y;
									vertexKd[vertexCount + 0][2] = kd.z;

									vertexKd[vertexCount + 1][0] = kd.x;
									vertexKd[vertexCount + 1][1] = kd.y;
									vertexKd[vertexCount + 1][2] = kd.z;

									vertexKd[vertexCount + 2][0] = kd.x;
									vertexKd[vertexCount + 2][1] = kd.y;
									vertexKd[vertexCount + 2][2] = kd.z;

								}else if(strcmp(lineHeader, "newmtl") == 0){
									break;
								}
							}


						}
					}
				}

				fclose(mtlFile);
			}
			vertexCount += 3;//Go to next face, 3 vertices

		}else if( strcmp( lineHeader, "mtllib") == 0 ){
			fscanf(inputFile, "%s\n", mtlLibrary);
			char temp[70] = "objFiles/";
			strcat(temp, mtlLibrary);
			strcpy(mtlLibrary, temp);
			cout << "Using material library: " << mtlLibrary << endl;

		}else if( strcmp( lineHeader, "usemtl") == 0 ){
			fscanf(inputFile, "%s\n", material);
			cout << "Using material: " << material << endl;

		}

		//cout << "NumVertices = " << vertexCount << endl;

	}
	

	fclose(inputFile);
	
	NumVertices = vertexCount;
}




void readControl(char * controlFile){
	FILE * inputFile;
	char lineHeader[128];
	char objName[128];

	inputFile = fopen(controlFile, "r");
	if(inputFile == NULL){
		cerr << "The control file: \"" << controlFile << "\"  is not in the same directory" << endl;
		exit(1);
	}

	while(!feof(inputFile)){

		int res = fscanf(inputFile, "%s", lineHeader);
		if(res == EOF){
			break;
		}

		if( strcmp( lineHeader, "obj" ) == 0 ){
			fscanf(inputFile, "%s\n", objName );
			readObj(objName);
			objCount++;
		}else if(strcmp( lineHeader, "t" ) == 0){//translate
			glm::vec3 trans;
			fscanf(inputFile, "%f %f %f\n", &trans.x, &trans.y, &trans.z);
			model = glm::translate(model, trans);

		}else if(strcmp( lineHeader, "s" ) == 0){//Scale
			glm::vec3 scail;
			fscanf(inputFile, "%f %f %f\n", &scail.x, &scail.y, &scail.z);
			model = glm::scale(model, scail);

		}else if(strcmp( lineHeader, "rx" ) == 0){//Rotate

		}
	}
}




////////////////////////////////////////////////////////////////////////
//	main
////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{

	if(argc < 3){
		cout << "Usage: " << argv[0] << " -c controlFile" << endl;
		exit(1);
	}

	//Set default values for shapes
	for(int i = 0; i < MaxVertices; i++){
		for(int j = 0; j < vecLength; j++){
			vertices[i][j] = 0.0f;
			vertexKa[i][j] = 0.1f;
			vertexKd[i][j] = 0.9f;
			vertexNormals[i][j] = 0.0f;
		}
	}
	
	setDefault();

	readControl(argv[2]);

	cout << "yPlus:" << yPlus << " yMinus:" << yMinus << " xMinus:" << xMinus << " xPlus:" << xPlus << " zPlus:" << zPlus << endl;
	float largestDim = std::max(std::max(xPlus - xMinus, yPlus - yMinus), zPlus - zMinus);
	float camX = 3 * largestDim;
	float camY = 3 * largestDim;
	float camZ = largestDim;
	cameraPosition = defaultCP = glm::vec3(camX, camY, camZ);
	cout << "camerPos = " << camX << ", " << camY << ", " << camZ << endl;

	float focX = (xPlus - std::fabs(xMinus))/2.0;
	float focY = (yPlus - std::fabs(yMinus))/2.0;
	float focZ = (zPlus - std::fabs(zMinus))/2.0;
	focalPoint = defaultFP = glm::vec3(0, 0, focZ);
	cout << "focalPoint = " << focX << ", " << focY << ", " << focZ << endl;
	// Set light based on camera position and focal
	//light = glm::normalize(glm::vec3((camX * 3.0/2.0) - focX, (camY * 3.0/2.0) - focY, (camZ * 3.0/2.0) - focZ));
	light = glm::normalize(glm::vec3(0, .2, .1));


	//GLUT stuff starts
	glutInit( &argc, argv );


	#ifdef IS_LINUX
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
	glutInitContextVersion( 4, 3 );
	glutInitContextProfile( GLUT_CORE_PROFILE );// GLUT_COMPATIBILITY_PROFILE );
	#else
	//OSX version
	glutInitDisplayMode( GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DEPTH );
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
	glutSpecialFunc( specialKeys );//For arrows
	glutReshapeFunc( windowResize );

	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LESS);
	glDepthMask( GL_TRUE );

	glutMainLoop();

	return 0;
}
