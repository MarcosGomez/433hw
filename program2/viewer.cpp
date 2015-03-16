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
//
//GLuint vboVertexColors;
GLuint vboVertexNormals;
GLuint vboVertexKa;
GLuint vboVertexKd;

GLuint NumVertices = 0;
const GLuint MaxVertices = 1000;
const GLuint vecLength = 3;

bool displayTwoTriangles = true;
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

glm::vec3 defaultCP(3.0f, 3.0f, 1.f);
glm::vec3 defaultFP(0.0f, 0.0f, 0.0f);
float maxHeight = 0.0;

//Lighting
// glm::vec3 Ka(0.1f, 0.1f, 0.1f);
// glm::vec3 Kd(0.9f, 0.9f, 0.9f);
//glm::vec3 lightSource = glm::normalize(glm::vec3(-1.0f, 1.0f, 1.0f));

GLfloat vertices[MaxVertices][vecLength];
GLfloat vertexKa[MaxVertices][vecLength];
GLfloat vertexKd[MaxVertices][vecLength];
GLfloat vertexNormals[MaxVertices][vecLength];




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
	farPlane = 100.0f;

	model = glm::mat4();


}

/////////////////////////////////////////////////////
//  int
/////////////////////////////////////////////////////
void init (void )
{



	//Calculate normals////////////////////
	if(vertexNormals[0][0] == 0.0){
		//std::vector<glm::vec3>* normal_buffer = new std::vector<glm::vec3>[NumVertices];
		//unsigned int num_indices = NumVertices/3.0;


		for( int i = 0; i < NumVertices - 2; i += 1 )
		{
		  // get the three vertices that make the faces
			glm::vec3 p1(vertices[i+0][0],vertices[i+0][1],vertices[i+0][2]);
			glm::vec3 p2(vertices[i+1][0],vertices[i+1][1],vertices[i+1][2]);
			glm::vec3 p3(vertices[i+2][0],vertices[i+2][1],vertices[i+2][2]);

			glm::vec3 v1 = p2 - p1;
			glm::vec3 v2 = p3 - p1;
			glm::vec3 normal = glm::cross(v1, v2);

		  glm::normalize(normal);

		  // Store the face's normal for each of the vertices that make up the face.

		  // normal_buffer[i+0].push_back( normal );
		  // normal_buffer[i+1].push_back( normal );
		  // normal_buffer[i+2].push_back( normal );


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
	glGenVertexArrays( 1, &vaoTwoTriangles );
	glBindVertexArray( vaoTwoTriangles );

	// set up the vertex buffer object for two triangles
	glGenBuffers( 1, &vboTwoTriangles );
	glBindBuffer( GL_ARRAY_BUFFER, vboTwoTriangles );
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
	glBindBuffer( GL_ARRAY_BUFFER, vboTwoTriangles );
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




}



////////////////////////////////////////////////////////////////////
//	display
////////////////////////////////////////////////////////////////////
void display (void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );



		glUseProgram( gouraudColorProgram );
		glBindVertexArray( vaoTwoTriangles );	// two triangles
		glDrawArrays( GL_TRIANGLE_FAN, 0, NumVertices );


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
			recalcView();

			break;
		case 'f':		// move focal point up along view up vector(tilt up)

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
				glm::vec3 gaze(focalPoint.x - cameraPosition.x, focalPoint.y - cameraPosition.y, focalPoint.z - cameraPosition.z);
				cameraPosition += glm::normalize(gaze);
				focalPoint += glm::normalize(gaze);
				recalcView();
		}
			break;
		case GLUT_KEY_DOWN :{		// move backward along gaze vector. scale distance based on bounds

			// fov += M_PI / 180.0; //Increase fov by 1 degree
			// recalcProj();
			glm::vec3 gaze(focalPoint.x - cameraPosition.x, focalPoint.y - cameraPosition.y, focalPoint.z - cameraPosition.z);
			cameraPosition -= glm::normalize(gaze);
			focalPoint -= glm::normalize(gaze);
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







void readObj(const char * fileName){


	FILE * inputFile;
	FILE * mtlFile;
	char mtlLibrary[60];
	char material[60] = "empty";
	char lineHeader[128];
	unsigned int index = 0;
	bool foundVT = false;
	bool foundVN = false;


	//char * fileName = argv[2];
	cout << fileName << endl;
	inputFile = fopen(fileName, "r");
	if(inputFile == NULL){
		cerr << "Failed to open obj file for reading" << endl;
		exit(1);
	}

	index = 0;
	//Read .obj
	while(!feof(inputFile)){

		int res = fscanf(inputFile, "%s", lineHeader);
		if(res == EOF){
			break;
		}

		if( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec3 vertex;
			fscanf( inputFile, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			// temp_vertices.push_back(vertex);

			vertices[index][0] = vertex.x;
			vertices[index][1] = vertex.y;
			vertices[index][2] = vertex.z;

			maxHeight = std::max(fabs(vertex.x), maxHeight);
			maxHeight = std::max(fabs(vertex.y), maxHeight);
			maxHeight = std::max(fabs(vertex.z), maxHeight);

			index ++;
		}else if( strcmp( lineHeader, "vt" ) == 0){
			glm::vec2 uv;
			fscanf( inputFile, "%f %f\n", &uv.x, &uv.y );
			foundVT = true;
			// temp_uvs.push_back(uv);

		}else if( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec3 normal;
			fscanf(inputFile, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			foundVN = true;
			// temp_normals.push_back(normal);

		}else if( strcmp( lineHeader, "f" ) == 0 ){
			//std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			if(foundVT && foundVN){
				int matches = fscanf( inputFile, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
				&vertexIndex[0], &uvIndex[0], &normalIndex[0],
				&vertexIndex[1], &uvIndex[1], &normalIndex[1],
				&vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				if(matches != 9){
					//cerr << "There weren't 9 matches for 0/0/0 0/0/0 0/0/0, but " << matches << endl;
					fscanf( inputFile, "%d %d\n", &vertexIndex[1], &vertexIndex[2]);

				}
			}else if(foundVT){
				int matches = fscanf( inputFile, "%d/%d %d/%d %d/%d\n",
				&vertexIndex[0], &uvIndex[0],
				&vertexIndex[1], &uvIndex[1],
				&vertexIndex[2], &uvIndex[2] );
				if(matches != 6){
					//cerr << "There weren't 6 matches , but " << matches << endl;
					fscanf( inputFile, "%d %d\n", &vertexIndex[1], &vertexIndex[2]);
				}
			}else if(foundVN){
				int matches = fscanf( inputFile, "%d//%d %d//%d %d//%d\n",
				&vertexIndex[0], &normalIndex[0],
				&vertexIndex[1], &normalIndex[1],
				&vertexIndex[2], &normalIndex[2] );
				if(matches != 6){
					//cerr << "There weren't 6 matches, but " << matches << endl;
					matches = fscanf( inputFile, "%d %d\n", &vertexIndex[1], &vertexIndex[2]);
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

			//cout << vertexIndex[0] << " : " << vertexIndex[1] << " : " << vertexIndex[2] << endl;


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

									vertexKa[vertexIndex[0] - 1][0] = ka.x;
									vertexKa[vertexIndex[0] - 1][1] = ka.y;
									vertexKa[vertexIndex[0] - 1][2] = ka.z;

									vertexKa[vertexIndex[1] - 1][0] = ka.x;
									vertexKa[vertexIndex[1] - 1][1] = ka.y;
									vertexKa[vertexIndex[1] - 1][2] = ka.z;

									vertexKa[vertexIndex[2] - 1][0] = ka.x;
									vertexKa[vertexIndex[2] - 1][1] = ka.y;
									vertexKa[vertexIndex[2] - 1][2] = ka.z;

								}else if(strcmp(lineHeader, "Kd") == 0){
									glm::vec3 kd;
									fscanf(mtlFile, "%f %f %f\n", &kd.x, &kd.y, &kd.z);
									// temp_Kd.push_back(kd);

									vertexKd[vertexIndex[0] - 1][0] = kd.x;
									vertexKd[vertexIndex[0] - 1][1] = kd.y;
									vertexKd[vertexIndex[0] - 1][2] = kd.z;

									vertexKd[vertexIndex[1] - 1][0] = kd.x;
									vertexKd[vertexIndex[1] - 1][1] = kd.y;
									vertexKd[vertexIndex[1] - 1][2] = kd.z;

									vertexKd[vertexIndex[2] - 1][0] = kd.x;
									vertexKd[vertexIndex[2] - 1][1] = kd.y;
									vertexKd[vertexIndex[2] - 1][2] = kd.z;

								}else if(strcmp(lineHeader, "newmtl") == 0){
									break;
								}
							}


						}
					}
				}

				fclose(mtlFile);
			}


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



	}
	// Process the data

	// For each vertex of each triangle
	// for( unsigned int i = 0; i < vertexIndices.size(); i++ ){
	// 	unsigned int vertexIndex = vertexIndices[i];
	// 	glm::vec3 vertex = temp_vertices[ vertexIndex - 1 ];
	// 	//out_vertices.push_back(vertex);
	// }

	fclose(inputFile);
	NumVertices = index;
}




void readControl(char * controlFile){
	FILE * inputFile;
	char lineHeader[128];
	char objName[128];

	inputFile = fopen(controlFile, "r");
	if(inputFile == NULL){
		cerr << "That control file is not in the directory" << endl;
		exit(1);
	}

	while(!feof(inputFile)){

		int res = fscanf(inputFile, "%s", lineHeader);
		if(res == EOF){
			break;
		}

		if( strcmp( lineHeader, "obj" ) == 0 ){
			fscanf(inputFile, "%s\n", objName );
			//cout << "Now reading .obj file" << endl;
			readObj(objName);
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

	glm::mat4 viewMat = glm::lookAt(glm::vec3(5.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
	cout << viewMat[0][0] << viewMat[0][1] << viewMat[0][2] << viewMat[0][3] << endl;
	cout << viewMat[1][0] << viewMat[1][1] << viewMat[1][2] << viewMat[1][3] << endl;
	cout << viewMat[2][0] << viewMat[2][1] << viewMat[2][2] << viewMat[2][3] << endl;
	cout << viewMat[3][0] << viewMat[3][1] << viewMat[3][2] << viewMat[3][3] << endl;

	if(argc < 3){
		cout << "Usage: " << argv[0] << " -c controlFile" << endl;
		exit(1);
	}

	memset(vertexNormals, 0, sizeof(vertexNormals[0][0]) * MaxVertices * vecLength);
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

	cameraPosition = defaultCP = glm::vec3(maxHeight * 2, maxHeight * 2, maxHeight/2.0);
	focalPoint = defaultFP = glm::vec3(0.0f, 0.0f, maxHeight/5.0f);



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
