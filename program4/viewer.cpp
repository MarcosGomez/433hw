// Marcos Gomez
// viewer.cpp
//
// .obj files must only be triangles
//////////////////////////////////////////////////////////
//  assignment 4 for CSC 433
//////////////////////////////////////////////////////////
#define IS_LINUX //Also change all comments for other files, Makefile, and shaders

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <vector>
#include <algorithm>
using namespace std;

#include <math.h>
#include "vgl.h"
#include "LoadShaders.h"
#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"
#include "glm/glm/gtc/type_ptr.hpp"
#include "./linuxSOIL/SOIL.h"

//Can be changed at will
#define SHOW_DISPLAY_INFO 0
#define MAX_OBJECTS 128
#define MAX_MATERIALS 128
#define MAX_TEXTURES 128

#define NUM_ATTRIB 7 //Number of defines below
#define POSITION 0
#define NORMAL 1
#define KA 2
#define KD 3
#define KS 4
#define NS 5
#define TEXTURE 6

GLuint vaoObject;		// vertex array objects
GLuint VAOs[MAX_OBJECTS];

GLuint VBOs[MAX_OBJECTS][NUM_ATTRIB];

const GLuint vecLength = 3;

bool displayObject = true;
bool displayOneTriangle = false;
bool displayCircle = false;

GLfloat *circleVertices = NULL;	// array of 2D vertices for circle
bool wireframeRendering = false;
GLfloat radius;
GLint steps;

GLuint classicProgram;

//Matrices
glm::mat4 world;//transformation, default to identity, how everything looks, what is rotated
glm::mat4 view;//camera
glm::mat4 proj;//projection/frustum
glm::vec3 cameraPosition;
glm::vec3 focalPoint;
glm::vec3 viewUp;
GLfloat fov;
GLfloat aspectRatio;
GLfloat nearPlane;
GLfloat farPlane;

GLint viewLoc;
GLint projLoc;
GLint modelLoc;

glm::mat4 MVPMatrix;
glm::mat4 MVMatrix;
glm::mat3 NormalMatrix;

glm::vec3 light;

glm::vec3 defaultCP(3.0f, 3.0f, 1.f);
glm::vec3 defaultFP(0.0f, 0.0f, 0.0f);
glm::vec3 defaultVU(0.0f, 0.0f, 1.0f);

//Make sure camera is looking at object
GLfloat yPlus = 0.0f;
GLfloat yMinus = 0.0f;
GLfloat xMinus = 0.0f;
GLfloat xPlus = 0.1f;
GLfloat zPlus = 0.1f;
GLfloat zMinus = 0.0f;


std::vector<glm::vec3> vertVector;
std::vector<glm::vec3> normVector;
std::vector<glm::vec3> kaVector;
std::vector<glm::vec3> kdVector;
std::vector<glm::vec3> ksVector;
std::vector<GLfloat> nsVector;
std::vector<glm::vec2> texVector;

std::vector<glm::mat4> modelVector;

//Made global to support multiple objects
glm::mat4 transformMat = glm::mat4();
GLuint vertexCount = 0;

GLuint numOfObjects = 0;
GLuint objectOffset[MAX_OBJECTS];
GLuint numVertices[MAX_OBJECTS];

bool cameraSpecified = false;

const GLuint MAX_LIGHTS = 4;
int numLights = 0;
struct LightProperties {
    bool isEnabled;
    bool isLocal;
    bool isSpot;
    glm::vec3 ambient;
    glm::vec3 color;
    glm::vec3 position;
    glm::vec3 halfVector;
    glm::vec3 coneDirection;
    float spotCosCutoff;
    float spotExponent;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
};

struct LightLocations{
	GLuint isEnabled;
    GLuint isLocal;
    GLuint isSpot;
    GLuint ambient;
    GLuint color;
    GLuint position;
    GLuint halfVector;   //Vec halfway between eye and light, normalized
    GLuint coneDirection;
    GLuint spotCosCutoff;
    GLuint spotExponent;
    GLuint constantAttenuation;
    GLuint linearAttenuation;
    GLuint quadraticAttenuation;
};


struct LightProperties lights[MAX_LIGHTS];
struct LightLocations lightsLoc[MAX_LIGHTS];

char textureNames[MAX_TEXTURES][128];
GLuint numMaterials[MAX_TEXTURES];
GLuint mtlOffset[MAX_OBJECTS][MAX_MATERIALS];
GLuint totalMaterialAmt = 0;
bool hasTexture[MAX_TEXTURES];



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

}


void recalcProj(){
	proj = glm::perspective(fov, aspectRatio, nearPlane, farPlane);

}

void setVertUniforms(int i){
	recalcProj();
	recalcView();

	MVPMatrix = proj * view * modelVector[i];
	MVMatrix = view * modelVector[i];
	NormalMatrix = glm::mat3(glm::transpose(glm::inverse(MVMatrix)));//mat4 normalMatrix = transpose(inverse(modelView));

	GLuint MVPMatrixLoc = glGetUniformLocation( classicProgram, "MVPMatrix");
	glUniformMatrix4fv(MVPMatrixLoc, 1, GL_FALSE, glm::value_ptr(MVPMatrix));
	//cout<<"Location: "<<MVPMatrixLoc<<"  "<<GL_INVALID_OPERATION<<endl;

	GLuint MVMatrixLoc = glGetUniformLocation( classicProgram, "MVMatrix");
	glUniformMatrix4fv(MVMatrixLoc, 1, GL_FALSE, glm::value_ptr(MVMatrix));

	GLuint NormalMatrixLoc = glGetUniformLocation( classicProgram, "NormalMatrix");
	glUniformMatrix3fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
}

void setFragUniforms(){
	cout<<"setting frag uniforms"<<endl;
	//Get all the locations for lightsLoc
	for(int i = 0; i < MAX_LIGHTS; i++){
		char name[128];
        memset(name, 0, sizeof(name));
        snprintf(name, sizeof(name), "Lights[%d].isEnabled", i);
        lightsLoc[i].isEnabled = glGetUniformLocation(classicProgram, name);

        snprintf(name, sizeof(name), "Lights[%d].isLocal", i);
        lightsLoc[i].isLocal = glGetUniformLocation(classicProgram, name);

        snprintf(name, sizeof(name), "Lights[%d].isSpot", i);
        lightsLoc[i].isSpot = glGetUniformLocation(classicProgram, name);

        snprintf(name, sizeof(name), "Lights[%d].ambient", i);
        lightsLoc[i].ambient = glGetUniformLocation(classicProgram, name);

        snprintf(name, sizeof(name), "Lights[%d].color", i);
        lightsLoc[i].color = glGetUniformLocation(classicProgram, name);

        snprintf(name, sizeof(name), "Lights[%d].position", i);
        lightsLoc[i].position = glGetUniformLocation(classicProgram, name);

        snprintf(name, sizeof(name), "Lights[%d].halfVector", i);
        lightsLoc[i].halfVector = glGetUniformLocation(classicProgram, name);

        snprintf(name, sizeof(name), "Lights[%d].coneDirection", i);
        lightsLoc[i].coneDirection = glGetUniformLocation(classicProgram, name);

        snprintf(name, sizeof(name), "Lights[%d].spotCosCutoff", i);
        lightsLoc[i].spotCosCutoff = glGetUniformLocation(classicProgram, name);

        snprintf(name, sizeof(name), "Lights[%d].spotExponent", i);
        lightsLoc[i].spotExponent = glGetUniformLocation(classicProgram, name);

        snprintf(name, sizeof(name), "Lights[%d].constantAttenuation", i);
        lightsLoc[i].constantAttenuation = glGetUniformLocation(classicProgram, name);

        snprintf(name, sizeof(name), "Lights[%d].linearAttenuation", i);
        lightsLoc[i].linearAttenuation = glGetUniformLocation(classicProgram, name);

        snprintf(name, sizeof(name), "Lights[%d].quadraticAttenuation", i);
        lightsLoc[i].quadraticAttenuation = glGetUniformLocation(classicProgram, name);

        //Error check
        if(lightsLoc[i].isEnabled == GL_INVALID_OPERATION ||
        	lightsLoc[i].isLocal == GL_INVALID_OPERATION ||
        	lightsLoc[i].isSpot == GL_INVALID_OPERATION ||
        	lightsLoc[i].ambient == GL_INVALID_OPERATION ||
        	lightsLoc[i].color == GL_INVALID_OPERATION ||
        	lightsLoc[i].position == GL_INVALID_OPERATION ||
        	lightsLoc[i].halfVector == GL_INVALID_OPERATION ||
        	lightsLoc[i].coneDirection == GL_INVALID_OPERATION ||
        	lightsLoc[i].spotCosCutoff == GL_INVALID_OPERATION ||
        	lightsLoc[i].spotExponent == GL_INVALID_OPERATION ||
        	lightsLoc[i].constantAttenuation == GL_INVALID_OPERATION ||
        	lightsLoc[i].linearAttenuation == GL_INVALID_OPERATION ||
        	lightsLoc[i].quadraticAttenuation == GL_INVALID_OPERATION){
        	cout<<"Error setting uniform light variables in fragment shader!"<<endl;
        }
		
	}

	//Set all the variables in shaders
	for(int i = 0; i < MAX_LIGHTS; i++){
		glUniform1i(lightsLoc[i].isEnabled, lights[i].isEnabled);
		glUniform1i(lightsLoc[i].isLocal, lights[i].isLocal);
		glUniform1i(lightsLoc[i].isSpot, lights[i].isSpot);
		glUniform3f(lightsLoc[i].ambient, lights[i].ambient.x, lights[i].ambient.y, lights[i].ambient.z);
		glUniform3f(lightsLoc[i].color, lights[i].color.x, lights[i].color.y, lights[i].color.z);
		glUniform3f(lightsLoc[i].position, lights[i].position.x, lights[i].position.y, lights[i].position.z);
		glUniform3f(lightsLoc[i].halfVector, lights[i].halfVector.x, lights[i].halfVector.y, lights[i].halfVector.z);
		glUniform3f(lightsLoc[i].coneDirection, lights[i].coneDirection.x, lights[i].coneDirection.y, lights[i].coneDirection.z);
		glUniform1f(lightsLoc[i].spotCosCutoff, lights[i].spotCosCutoff);
		glUniform1f(lightsLoc[i].spotExponent, lights[i].spotExponent);
		glUniform1f(lightsLoc[i].constantAttenuation, lights[i].constantAttenuation);
		glUniform1f(lightsLoc[i].linearAttenuation, lights[i].linearAttenuation);
		glUniform1f(lightsLoc[i].quadraticAttenuation, lights[i].quadraticAttenuation);
	}
}



void setDefault(){

	cameraPosition = defaultCP;
	focalPoint = defaultFP;
	viewUp = defaultVU;

	fov = glm::radians(70.0f);
	aspectRatio = 512.0f/512.0f;
	nearPlane = 0.4f;
	farPlane = 5000.0f; //defaultCP.z

	world = glm::mat4();
	for(int i = 0; i < MAX_LIGHTS; i++){
		lights[i].isEnabled = false;
	}
}


/////////////////////////////////////////////////////
// loadTexture
/////////////////////////////////////////////////////
bool loadTexture(char* fileName, bool invertY)
{
    unsigned char *image;
    int width, height, channels;

    cout<<"Loading image: "<<fileName<<endl;

    image = SOIL_load_image(fileName, &width, &height, &channels, SOIL_LOAD_AUTO);
    if (image == NULL)
    {
        cerr << "Unable to load image: " << fileName <<"!"<< endl;
        return false;
    }
    else
    {
        bool invert_y = true;
        if (invert_y)
        {
            for (int j = 0; j * 2 < height; ++j)
            {
                int index1 = j * width * channels;
                int index2 = (height - 1 - j) * width * channels;
                for (int i = width * channels; i > 0; --i)
                {
                    unsigned char temp = image[index1];
                    image[index1] = image[index2];
                    image[index2] = temp;
                    ++index1;
                    ++index2;
                }
            }
        }
        GLuint tex2;
        glGenTextures(1, &tex2);
        glBindTexture(GL_TEXTURE_2D, tex2);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//ALSO GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//ALSO GL_LINEAR_MIPMAP_LINEAR
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        //Actually loads the image pixels into the buffer
        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
            GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, image);

        //glGenerateMipmap(GL_TEXTURE_2D);
        SOIL_free_image_data(image);    // free up memory allocated by SOIL_load_image

        return true;
    }
}


/////////////////////////////////////////////////////
//  init
/////////////////////////////////////////////////////
void init ( void )
{
	//Convert vectors to arrays
	cout << "Number of vertices drawing. All should be equal - " << vertexCount<<":"
	<<vertVector.size()<<":"<<normVector.size()<<":"<<kaVector.size()<<":"<<kdVector.size()
	<<":"<<ksVector.size()<<":"<<nsVector.size()<<":"<<texVector.size()<<endl;

	glm::vec3 buf;
	
	GLfloat (*vertices)[vecLength] = new GLfloat[vertVector.size()][vecLength];
	GLfloat (*vertexNormals)[vecLength] = new GLfloat[normVector.size()][vecLength];
	GLfloat (*vertexKa)[vecLength] = new GLfloat[kaVector.size()][vecLength];
	GLfloat (*vertexKd)[vecLength] = new GLfloat[kaVector.size()][vecLength];
	GLfloat (*vertexKs)[vecLength] = new GLfloat[ksVector.size()][vecLength];
	GLfloat (*vertexNs) = new GLfloat[nsVector.size()];
    GLfloat (*vertexTextures)[2] = new GLfloat[texVector.size()][2];
	
	for(int i = 0; i < vertVector.size(); i++){
		buf = vertVector[i];
		vertices[i][0] = buf.x;
		vertices[i][1] = buf.y;
		vertices[i][2] = buf.z;
	}
	for(int i = 0; i < normVector.size(); i++){
		buf = normVector[i];
		vertexNormals[i][0] = buf.x;
		vertexNormals[i][1] = buf.y;
		vertexNormals[i][2] = buf.z;
	}
	for(int i = 0; i < kaVector.size(); i++){
		buf = kaVector[i];
		vertexKa[i][0] = buf.x;
		vertexKa[i][1] = buf.y;
		vertexKa[i][2] = buf.z;
	}
	for(int i = 0; i < kdVector.size(); i++){
		buf = kdVector[i];
		vertexKd[i][0] = buf.x;
		vertexKd[i][1] = buf.y;
		vertexKd[i][2] = buf.z;
	}
	for(int i = 0; i < ksVector.size(); i++){
		buf = ksVector[i];
		vertexKs[i][0] = buf.x;
		vertexKs[i][1] = buf.y;
		vertexKs[i][2] = buf.z;
	}
	for(int i = 0; i < nsVector.size(); i++){
		vertexNs[i] = nsVector[i];
	}
    for(int i = 0; i < texVector.size(); i++){
        glm::vec2 texBuf = texVector[i];
        vertexTextures[i][0] = texBuf.s;
        vertexTextures[i][1] = texBuf.t;
    }
	
	for(int i = 0; i < numOfObjects; i++){
		// set up the vertex array object 
		glGenVertexArrays( 1, &VAOs[i] );
		glBindVertexArray( VAOs[i] );

		//Position
		glGenBuffers( 1, &VBOs[i][POSITION] );
		glBindBuffer( GL_ARRAY_BUFFER, VBOs[i][POSITION] );
		glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * numVertices[i] * vecLength, vertices + objectOffset[i], GL_STATIC_DRAW );

		//Normals
		glGenBuffers( 1, &VBOs[i][NORMAL] );
		glBindBuffer( GL_ARRAY_BUFFER, VBOs[i][NORMAL] );
		glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * numVertices[i] * vecLength, vertexNormals + objectOffset[i],
			GL_STATIC_DRAW );

		//Ka
		glGenBuffers( 1, &VBOs[i][KA] );
		glBindBuffer( GL_ARRAY_BUFFER, VBOs[i][KA] );
		glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * numVertices[i] * vecLength, vertexKa + objectOffset[i],
			GL_STATIC_DRAW );

		//Kd
		glGenBuffers( 1, &VBOs[i][KD] );
		glBindBuffer( GL_ARRAY_BUFFER, VBOs[i][KD] );
		glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * numVertices[i] * vecLength, vertexKd + objectOffset[i],
			GL_STATIC_DRAW );

		//Ks
		glGenBuffers( 1, &VBOs[i][KS] );
		glBindBuffer( GL_ARRAY_BUFFER, VBOs[i][KS] );
		glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * numVertices[i] * vecLength, vertexKs + objectOffset[i],
			GL_STATIC_DRAW );

		//Ns
		glGenBuffers( 1, &VBOs[i][NS] );
		glBindBuffer( GL_ARRAY_BUFFER, VBOs[i][NS] );
		glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * numVertices[i], vertexNs + objectOffset[i],
			GL_STATIC_DRAW );

        //TextureIndices
        glGenBuffers( 1, &VBOs[i][TEXTURE] );
        glBindBuffer( GL_ARRAY_BUFFER, VBOs[i][TEXTURE] );
        glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * numVertices[i] * 2, vertexTextures + objectOffset[i],
            GL_STATIC_DRAW );


		glEnableVertexAttribArray( 0 ); // position
		glEnableVertexAttribArray( 1 ); // normals
		glEnableVertexAttribArray( 2 ); // Ka
		glEnableVertexAttribArray( 3 ); // Kd
		glEnableVertexAttribArray( 4 ); // Ks
		glEnableVertexAttribArray( 5 ); // Ns
        glEnableVertexAttribArray( 6 ); // texture


		//Map 0 to position
		glBindBuffer( GL_ARRAY_BUFFER, VBOs[i][POSITION] );
		glVertexAttribPointer( 0, vecLength, GL_FLOAT, GL_FALSE, 0, NULL );

		//Map index  to normals
		glBindBuffer( GL_ARRAY_BUFFER, VBOs[i][NORMAL] );
		glVertexAttribPointer(1, vecLength, GL_FLOAT, GL_FALSE, 0, NULL );

		//Map to Ka
		glBindBuffer( GL_ARRAY_BUFFER, VBOs[i][KA] );
		glVertexAttribPointer(2, vecLength, GL_FLOAT, GL_FALSE, 0, NULL );

		//Map to Kd
		glBindBuffer( GL_ARRAY_BUFFER, VBOs[i][KD] );
		glVertexAttribPointer(3, vecLength, GL_FLOAT, GL_FALSE, 0, NULL );

		//Map to Ks
		glBindBuffer( GL_ARRAY_BUFFER, VBOs[i][KS] );
		glVertexAttribPointer(4, vecLength, GL_FLOAT, GL_FALSE, 0, NULL );

		//Map to Ns
		glBindBuffer( GL_ARRAY_BUFFER, VBOs[i][NS] );
		glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 0, NULL );

        //Map to Texture
        glBindBuffer( GL_ARRAY_BUFFER, VBOs[i][TEXTURE] );
        glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, 0, NULL );
	}
    //Activate all textures
    for(int k = 0; k < totalMaterialAmt; k++){
        if(hasTexture[k]){
            glActiveTexture(GL_TEXTURE0 + k);
            
            if(!loadTexture(textureNames[k], true)){
                cerr<<"Error reading texture"<<endl;
            }
            cout<<"at location "<<k<<endl;
        }else{
            cout<<"Skipped loading a texture at materal number "<<k + 1<<endl;
        }
    }
	

	ShaderInfo classicShaders[] = {
		{ GL_VERTEX_SHADER, "classic.vert" },
		{ GL_FRAGMENT_SHADER, "classic.frag" },
		{ GL_NONE, NULL }
	};

	classicProgram = LoadShaders( classicShaders );


	glLineWidth( 1.f );	// thicker lines for wireframe
	glUseProgram( classicProgram );
	setFragUniforms();
	glClearColor(0.0f, 0.25f, 0.3f, 1.0f);

	//Free up mem
	delete[](vertices);
	delete[](vertexNormals);
	delete[](vertexKa);
	delete[](vertexKd);
	delete[](vertexKs);
	delete[](vertexNs);
    delete[](vertexTextures);
}



////////////////////////////////////////////////////////////////////
//	display
////////////////////////////////////////////////////////////////////
void display (void )
{

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glUseProgram( classicProgram );
    GLuint texUnit = 0;
    GLuint mtlIndex = 0;
    GLuint texLoc;
    GLuint hasTexLoc;

    texLoc = glGetUniformLocation( classicProgram, "tex");
    hasTexLoc = glGetUniformLocation( classicProgram, "hasTexture");
    if(SHOW_DISPLAY_INFO){
        cout<<"Number of objects is "<<numOfObjects<<endl;
        cout<<"Total material amount is "<<totalMaterialAmt<<endl;
    }
	for( int index = 0; index < numOfObjects; index++){
        if(SHOW_DISPLAY_INFO){
            cout<<"index:"<<index<<endl;
            cout<<"numVertices: "<<numVertices[index]<<" starting objectOffset: "<<objectOffset[index]<<endl;
            cout<<"Number of textures for object "<<index + 1<<" is "<<numMaterials[index]<<endl;
        }
        setVertUniforms(index);
        glBindVertexArray( VAOs[index] ); 
        
        for(int j = 0; j < numMaterials[index]; j++){
            //Set uniforms
            glUniform1i(texLoc, texUnit);
            glUniform1i(hasTexLoc, hasTexture[mtlIndex]);
            if(SHOW_DISPLAY_INFO){
                cout<<"hasTexture[" << mtlIndex<<"]: "<<hasTexture[mtlIndex]<<endl;
            }

            if((j + 1) == numMaterials[index]){
                if(SHOW_DISPLAY_INFO){
                    cout<<"Rendering final texture from vertices "<<mtlOffset[index][j]<<
                    " - "<< numVertices[index]<<" at texUnit "<<texUnit<<endl;
                }
                glDrawArrays( GL_TRIANGLES, mtlOffset[index][j], numVertices[index] - mtlOffset[index][j]);
                ++texUnit;
            }else{
                if(SHOW_DISPLAY_INFO){
                    cout<<"Rendering one texture from vertices "<<mtlOffset[index][j]<<
                    " - "<< mtlOffset[index][j + 1]<<" at texUnit "<<texUnit<<endl;
                }
                glDrawArrays( GL_TRIANGLES, mtlOffset[index][j], mtlOffset[index][j + 1] - mtlOffset[index][j]);
                ++texUnit;
            }
            mtlIndex++;
        }
        if(numMaterials[index] == 0){
            if(SHOW_DISPLAY_INFO){
                cout<<"Rendering all "<<numVertices[index]<<" vertices of the object"<<endl;
            }
            
            glUniform1i(texLoc, texUnit);
            glUniform1i(hasTexLoc, false);
            glDrawArrays( GL_TRIANGLES, 0, numVertices[index]);
        }
	}
	if(SHOW_DISPLAY_INFO){
        cout<<"Finished Display function\n\n"<<endl;
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

			break;
		case 'c':		// move camera down along view up vector
			cameraPosition -= viewUp;
			focalPoint -= viewUp;

			break;
		case 'v':		// move camera up along view up vector

			cameraPosition += viewUp;
			focalPoint += viewUp;

			break;
		case 'd':		// move focal point down along view up vector(tilt down)
			focalPoint.z -= std::fabs(glm::length(cameraPosition - defaultFP))/30.0;

			break;
		case 'f':		// move focal point up along view up vector(tilt up)
			focalPoint.z += std::fabs(glm::length(cameraPosition - defaultFP))/30.0;

			break;
		case 'z':		// rotate view up vector by 1 degree counter-clockwise around gaze vector
		{
			//Rotate world, not view
			glm::mat4 temp;
			glm::vec3 gaze((focalPoint.x - cameraPosition.x), (focalPoint.y - cameraPosition.y), (focalPoint.z - cameraPosition.z));
			temp = glm::rotate(temp, glm::radians(1.0f), gaze);
			viewUp = glm::vec3(glm::vec4(viewUp, 1.0f) * temp);

		}
			break;
		case 'x':		// rotate view up vector by 1 degree clockwise around gaze vector
		{
			glm::mat4 temp;
			glm::vec3 gaze((focalPoint.x - cameraPosition.x), (focalPoint.y - cameraPosition.y), (focalPoint.z - cameraPosition.z));
			temp = glm::rotate(temp, glm::radians(-1.0f), gaze);
			viewUp = glm::vec3(glm::vec4(viewUp, 1.0f) * temp);

		}
			break;
		default:
			cout << "This key doesn't do anything" << endl;

			break;
		}
	glutPostRedisplay();
}





//////////////////////////////////////////////////////////
// specialKeys
//////////////////////////////////////////////////////////
void specialKeys(int key, int x, int y)
{
	switch(key){
		case GLUT_KEY_UP :{		// move forward along gaze vector. scale distance based on bounds. decrease fov
			float distance;
			distance = std::fabs(glm::length(cameraPosition - defaultFP));
			glm::vec3 gaze((focalPoint.x - cameraPosition.x), (focalPoint.y - cameraPosition.y), (focalPoint.z - cameraPosition.z));
			gaze = glm::normalize(gaze);
			gaze *= distance/30.0f;
			cameraPosition += gaze;
			focalPoint += gaze;
		}
			break;
		case GLUT_KEY_DOWN :{		// move backward along gaze vector. scale distance based on bounds
			float distance;
			distance = std::fabs(glm::length(cameraPosition - defaultFP));
			glm::vec3 gaze((focalPoint.x - cameraPosition.x), (focalPoint.y - cameraPosition.y), (focalPoint.z - cameraPosition.z));
			gaze = glm::normalize(gaze);
			gaze *= distance/30.0f;
			cameraPosition -= gaze;
			focalPoint -= gaze;
		}
			break;
		case GLUT_KEY_LEFT :{		// rotate along z axis by 1 degree

			glm::mat4 temp;
			glm::vec3 axis(0.0f, 0.0f, 1.0f);
			temp = glm::rotate(temp, glm::radians(1.0f), axis);
			cameraPosition = glm::vec3(glm::vec4(cameraPosition, 1.0f) * temp);
			focalPoint = glm::vec3(glm::vec4(focalPoint, 1.0f) * temp);

		}
			break;
		case GLUT_KEY_RIGHT :{		// rotate along z axis by 1 degree


			glm::mat4 temp;
			glm::vec3 axis(0.0f, 0.0f, 1.0f);
			temp = glm::rotate(temp, glm::radians(-1.0f), axis);
			cameraPosition = glm::vec3(glm::vec4(cameraPosition, 1.0f) * temp);
			focalPoint = glm::vec3(glm::vec4(focalPoint, 1.0f) * temp);

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






//////////////////////////////////////////////////////////
// readObj
//////////////////////////////////////////////////////////
void readObj(const char * fileName){

	FILE * inputFile = NULL;
	FILE * mtlFile = NULL;
	char mtlLibrary[128] = "";
	char material[128] = "empty";
	char lineHeader[128] = "";
	
	bool foundVT = false;
	bool foundVN = false;

	bool sameAsVertex = true;
	unsigned int vertexIndex[3] = {1, 1, 1}, uvIndex[3] = {1, 1, 1}, normalIndex[3] = {1, 1, 1};

	int vIndex = 0;
	int vtIndex = 0;
	int vnIndex = 0;
	
	//vertices
	std::vector< glm::vec3 > tempVertices;
	std::vector< glm::vec3 > tempVertexNormals;
	std::vector< glm::vec2 > tempUvs;

    numMaterials[numOfObjects] = 0;
	objectOffset[numOfObjects] = vertexCount;


	cout << fileName << endl;
	inputFile = fopen(fileName, "r");
	if(inputFile == NULL){
		cerr << "Failed to open obj file: \"" << fileName << "\" for reading" << endl;
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
			
			tempVertices.push_back(vertex);

			vIndex ++;
		}else if( strcmp( lineHeader, "vt" ) == 0){
			//Texture coordinates
			glm::vec2 uv;
			fscanf( inputFile, "%f %f\n", &uv.x, &uv.y );
			foundVT = true;
			tempUvs.push_back(uv);
			vtIndex++;

		}else if( strcmp( lineHeader, "vn" ) == 0 ){
			//Vertex normals
			glm::vec3 normal;
			fscanf(inputFile, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			foundVN = true;
			
			glm::normalize(normal);

			tempVertexNormals.push_back(normal);

			vnIndex++;

		}else if( strcmp( lineHeader, "s" ) == 0){//smoothing group
			unsigned int normalPos = 0;
			fscanf(inputFile, "%d\n", &normalPos );
			
			normalIndex[0] = normalPos;
			normalIndex[1] = normalPos;
			normalIndex[2] = normalPos;
			sameAsVertex = false;

		}else if( strcmp( lineHeader, "f" ) == 0 ){
			
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
					cout<<"Using same as vertex Indices!"<<endl;
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
			//cout << vertexIndex[0] << " : " << vertexIndex[1] << " : " << vertexIndex[2] << endl;
			
			//Set the vertices in the xPlus location, face
			//cout << "Setting vertices for face starting at index " << vertexCount << endl;
			//cout << "vertex value is " << tempVertices[vertexIndex[0] - 1][0]
			//<< ", " << tempVertices[vertexIndex[0] - 1][1]<< ", " << tempVertices[vertexIndex[0] - 1][2]<< endl;
			
			//1
			glm::vec3 tempVec = glm::vec3();

			tempVec = tempVertices[vertexIndex[0] - 1];

			vertVector.push_back(tempVec);



			if(xPlus == xMinus && xMinus == yPlus && yPlus == yMinus &&
				yMinus == zPlus && zPlus == zMinus){
				xPlus = tempVec.x;
				xMinus = tempVec.x;
				yPlus = tempVec.y;
				yMinus = tempVec.y;
				zPlus = tempVec.z;
				zMinus = tempVec.z;
			}
			
			xPlus = std::max(tempVec.x, xPlus);
			xMinus = std::min(tempVec.x, xMinus);
			yPlus = std::max(tempVec.y, yPlus);
			yMinus = std::min(tempVec.y, yMinus);
			zPlus = std::max(tempVec.z, zPlus);
			zMinus = std::min(tempVec.z, zMinus);


			//2
			tempVec = tempVertices[vertexIndex[1] - 1];

			vertVector.push_back(tempVec);

			xPlus = std::max(tempVec.x, xPlus);
			xMinus = std::min(tempVec.x, xMinus);
			yPlus = std::max(tempVec.y, yPlus);
			yMinus = std::min(tempVec.y, yMinus);
			zPlus = std::max(tempVec.z, zPlus);
			zMinus = std::min(tempVec.z, zMinus);

			//3
			tempVec = tempVertices[vertexIndex[2] - 1];

			vertVector.push_back(tempVec);

			xPlus = std::max(tempVec.x, xPlus);
			xMinus = std::min(tempVec.x, xMinus);
			yPlus = std::max(tempVec.y, yPlus);
			yMinus = std::min(tempVec.y, yMinus);
			zPlus = std::max(tempVec.z, zPlus);
			zMinus = std::min(tempVec.z, zMinus);


			if(foundVN){
				//cout << "Setting normals for face" << endl;
				//Set normals
				//1
				tempVec = tempVertexNormals[normalIndex[0] - 1];

				normVector.push_back(tempVec);

				//2
				tempVec = tempVertexNormals[normalIndex[1] - 1];

				normVector.push_back(tempVec);

				//3
				tempVec = tempVertexNormals[normalIndex[2] - 1];

				normVector.push_back(tempVec);

			}else{
				//Calculate normals
				//cout << "Calculating normals" << endl;
				// get the three vertices that make the faces

				glm::vec3 p1(vertVector[vertexCount + 0]);
				glm::vec3 p2(vertVector[vertexCount + 1]);
				glm::vec3 p3(vertVector[vertexCount + 2]);

				// Calculate the 2 vectors
				glm::vec3 v1 = p2 - p1;
				glm::vec3 v2 = p3 - p1;
				//Cross to get normal
				glm::vec3 normal = glm::cross(v1, v2);

			  	glm::normalize(normal);

			  	// Store the face's normal for each of the vertices that make up the face.

				normVector.push_back(normal);
				normVector.push_back(normal);
				normVector.push_back(normal);

			}

            if(foundVT){
                glm::vec2 tempTex;
                //1
                tempTex = tempUvs[uvIndex[0] - 1];
                texVector.push_back(tempTex);
                //2
                tempTex = tempUvs[uvIndex[1] - 1];
                texVector.push_back(tempTex);
                //3
                tempTex = tempUvs[uvIndex[2] - 1];
                texVector.push_back(tempTex);
            }else{
                //Put default indices
                glm::vec2 tempTex = glm::vec2(0,0);
                //1
                texVector.push_back(tempTex);
                //2
                texVector.push_back(tempTex);
                //3
                texVector.push_back(tempTex);
            }

			


			//Find Ka and Kd values of these faces
			mtlFile = fopen(mtlLibrary, "r");
			if(mtlFile == NULL){
				//cout << "No mtllib provided. Using default colors" << endl;
				//Insert for all 3 vertices
				for(int i = 0; i < 3; i++){
					kaVector.push_back(glm::vec3(0.1f, 0.1f, 0.1f));
					kdVector.push_back(glm::vec3(0.9f, 0.9f, 0.9f));
					ksVector.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
					nsVector.push_back(60.0f);
				}
				
			}else if(strcmp(material, "empty") == 0){
				//cout << "No usemtl specified, using default" << endl;
				//Insert for all 3 vertices
				for(int i = 0; i < 3; i++){
					kaVector.push_back(glm::vec3(0.1f, 0.1f, 0.1f));
					kdVector.push_back(glm::vec3(0.9f, 0.9f, 0.9f));
					ksVector.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
					nsVector.push_back(60.0f);
				}
			}else{
				//cout << "Opened " << mtlLibrary << endl;
                //READING from .mtl --------------------------------------------
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

									//Insert for all 3 vertices
									for(int i = 0; i < 3; i++){
										kaVector.push_back(ka);
									}

								}else if(strcmp(lineHeader, "Kd") == 0){
									glm::vec3 kd;
									fscanf(mtlFile, "%f %f %f\n", &kd.x, &kd.y, &kd.z);

									//Insert for all 3 vertices
									for(int i = 0; i < 3; i++){
										kdVector.push_back(kd);
									}

								}else if(strcmp(lineHeader, "Ks") == 0){
									glm::vec3 ks;
									fscanf(mtlFile, "%f %f %f\n", &ks.x, &ks.y, &ks.z);

									//Insert for all 3 vertices
									for(int i = 0; i < 3; i++){
										ksVector.push_back(ks);
									}

								}else if(strcmp(lineHeader, "Ns") == 0){
									GLfloat exponent;
									fscanf(mtlFile, "%f\n", &exponent);

									//Insert for all 3 vertices
									for(int i = 0; i < 3; i++){
										nsVector.push_back(exponent);
									}

								}else if(strcmp(lineHeader, "map_Kd") == 0){
                                    char texName[128];
                                    fscanf(mtlFile, "%s\n", texName);
                                    strcpy(textureNames[totalMaterialAmt - 1], texName);
                                    hasTexture[totalMaterialAmt - 1] = true;
                                    
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
			char temp[70] = "";
			int length = strlen(fileName);
			// cout << "filename length = " << length << endl;
			// cout << "temp is currently " << temp << endl;
			int slashIndex;
			for(slashIndex = length - 1; slashIndex >= 0; slashIndex--){
				if(fileName[slashIndex] == '/'){
					break;
				}
			}
			strncpy(temp, fileName, slashIndex + 1);
			cout << "Reading from directory " << temp << endl;

			strcat(temp, mtlLibrary);
			strcpy(mtlLibrary, temp);
			cout << "Using material library: " << mtlLibrary << endl;

		}else if( strcmp( lineHeader, "usemtl") == 0 ){
			fscanf(inputFile, "%s\n", material);
            mtlOffset[numOfObjects][numMaterials[numOfObjects]] = vertexCount - objectOffset[numOfObjects];
            (numMaterials[numOfObjects])++;
            totalMaterialAmt++;
            hasTexture[totalMaterialAmt - 1] = false;
            
			cout << "Using material: " << material << endl;
		}
	}
	
	fclose(inputFile);
	numVertices[numOfObjects] = vertexCount - objectOffset[numOfObjects];
	numOfObjects++;
	modelVector.push_back(transformMat);
    cout<<"Finished reading object "<<numOfObjects<<"\n"<<endl;
}



//////////////////////////////////////////////////////////
// readControl
//////////////////////////////////////////////////////////
void readControl(char * controlFile){
	FILE * inputFile;
	char lineHeader[128] = "";
	char objName[128] = "";
	bool needToProcessObj = false;
    glm::mat4 xRotMat, yRotMat, zRotMat, scaleMat, translMat;
    xRotMat = yRotMat = zRotMat = scaleMat = translMat = glm::mat4();

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
			if(needToProcessObj){
                transformMat =  translMat * scaleMat * zRotMat * yRotMat * xRotMat;
				readObj(objName);
				transformMat = xRotMat = yRotMat = zRotMat = scaleMat = translMat = glm::mat4();//reset tranform matrix
			}
			fscanf(inputFile, "%s\n", objName );
			
			needToProcessObj = true;
			
		}else if(strcmp( lineHeader, "t" ) == 0){//translate
			glm::vec3 trans;
			fscanf(inputFile, "%f %f %f\n", &trans.x, &trans.y, &trans.z);


			translMat = glm::translate(translMat, trans);

		}else if(strcmp( lineHeader, "s" ) == 0){//Scale
			glm::vec3 scail;
			fscanf(inputFile, "%f %f %f\n", &scail.x, &scail.y, &scail.z);

			scaleMat = glm::scale(scaleMat, scail);

		}else if(strcmp( lineHeader, "rx" ) == 0){//Rotate X
			float angle;
			fscanf(inputFile, "%f\n", &angle);
			glm::vec3 axis(1.0f, 0.0f, 0.0f);
			xRotMat = glm::rotate(xRotMat, glm::radians(angle), axis);
		}else if(strcmp( lineHeader, "ry" ) == 0){//Rotate Y
			float angle;
			fscanf(inputFile, "%f\n", &angle);
			glm::vec3 axis(0.0f, 1.0f, 0.0f);
			yRotMat = glm::rotate(yRotMat, glm::radians(angle), axis);
		}else if(strcmp( lineHeader, "rz" ) == 0){//Rotate Z
			float angle;
			fscanf(inputFile, "%f\n", &angle);
			glm::vec3 axis(0.0f, 0.0f, 1.0f);
			zRotMat = glm::rotate(zRotMat, glm::radians(angle), axis);
		}else if(strcmp( lineHeader, "light" ) == 0){

			lights[numLights].isEnabled = true;
			fscanf(inputFile, "%s", lineHeader);
			char type[128] = "";
			for(int i = 0; i < 9; i++){
					fscanf(inputFile, "%s", type);
					glm::vec3 buf;
					if(strcmp( type, "ambient") == 0){
						fscanf(inputFile, "%f %f %f", &buf.x, &buf.y, &buf.z);
						lights[numLights].ambient = buf;
					}else if(strcmp( type, "color") == 0){
						fscanf(inputFile, "%f %f %f", &buf.x, &buf.y, &buf.z);
						lights[numLights].color = buf;
					}else if(strcmp( type, "position") == 0){
						if(strcmp( lineHeader, "directional") == 0){
							fscanf(inputFile, "%f %f %f\n", &buf.x, &buf.y, &buf.z);
							lights[numLights].position = buf;
							cout<<"Directional light"<<endl;
							break;
						}
						fscanf(inputFile, "%f %f %f", &buf.x, &buf.y, &buf.z);
						lights[numLights].position = buf;
					}else if(strcmp( type, "constAtt") == 0){
						fscanf(inputFile, "%f", &buf.x);
						lights[numLights].constantAttenuation = buf.x;
					}else if(strcmp( type, "linearAtt") == 0){
						fscanf(inputFile, "%f", &buf.x);
						lights[numLights].linearAttenuation = buf.x;
					}else if(strcmp( type, "quadAtt") == 0){
						lights[numLights].isLocal = true;
						if(strcmp( lineHeader, "local") == 0){
							fscanf(inputFile, "%f\n", &buf.x);
							lights[numLights].quadraticAttenuation = buf.x;
							break;
						}
						fscanf(inputFile, "%f", &buf.x);
						lights[numLights].quadraticAttenuation = buf.x;
					}else if(strcmp( type, "coneDirection") == 0){
						fscanf(inputFile, "%f %f %f", &buf.x, &buf.y, &buf.z);
						lights[numLights].coneDirection = buf;
					}else if(strcmp( type, "spotCosCutoff") == 0){
						fscanf(inputFile, "%f", &buf.x);
						lights[numLights].spotCosCutoff = buf.x;
					}else if(strcmp( type, "spotExponent") == 0){
						if(strcmp( lineHeader, "spot") != 0){
							cout<<"Wrong light syntax in control file"<<endl;
							cout<<"lineHeader is "<<lineHeader<< ", now breaking"<<endl;
						}
						lights[numLights].isSpot = true;
						fscanf(inputFile, "%f\n", &buf.x);
						lights[numLights].spotExponent = buf.x;
						break;
					}else{
						cout<<"unknown type is "<<type<< " now breaking"<<endl;
						break;
					}
				}

				cout<<"Light Properties: ambient-"<<lights[numLights].ambient.x<<lights[numLights].ambient.y<<lights[numLights].ambient.z
				<<" color-"<<lights[numLights].color.x<<lights[numLights].color.y<<lights[numLights].color.z<<" position-"
				<<lights[numLights].position.x<<lights[numLights].position.y<<lights[numLights].position.z<<endl;

			numLights++;
		}else if(strcmp( lineHeader, "view" ) == 0){
			cameraSpecified = true;
			
				for(int i = 0; i < 3; i++){
					fscanf(inputFile, "%s", lineHeader);
					glm::vec3 buf;
					if(strcmp( lineHeader, "camera") == 0){
						fscanf(inputFile, "%f %f %f", &buf.x, &buf.y, &buf.z);
						cameraPosition = buf;
					}else if(strcmp( lineHeader, "focal") == 0){
						fscanf(inputFile, "%f %f %f", &buf.x, &buf.y, &buf.z);
						focalPoint = buf;
					}else if(strcmp( lineHeader, "viewup") == 0){
						fscanf(inputFile, "%f %f %f\n", &buf.x, &buf.y, &buf.z);
						viewUp = buf;
					}else{
						cout<<"lineHeader is "<<lineHeader<< " now breaking"<<endl;
						break;
					}
				}
			cout<<"CamerPos: "<<cameraPosition.x<<cameraPosition.y<<cameraPosition.z<<" viewUp: "
			<<viewUp.x<<viewUp.y<<viewUp.z<<endl;
		}
	}
	if(needToProcessObj){
        transformMat =  translMat * scaleMat * zRotMat * yRotMat * xRotMat;
		readObj(objName);
	}else{
		cout << "control file contains no obj files" << endl;
	}

	fclose(inputFile);
	return;
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
	
	setDefault();

	readControl(argv[2]);

	if(cameraSpecified){
		defaultCP = cameraPosition;
		defaultFP = focalPoint;
		defaultVU = viewUp;
		cout<<"Camera pos = "<<defaultCP.x<<", "<<defaultCP.y<<", "<<defaultCP.z<<endl;
		cout<<"Focal point = "<<defaultFP.x<<", "<<defaultFP.y<<", "<<defaultFP.z<<endl;
	}else{
		cout<<"Using default camera position"<<endl;

		float largestDim = std::max(std::max(xPlus - xMinus, yPlus - yMinus), zPlus - zMinus);
		float camX = 3 * largestDim;
		float camY = 3 * largestDim;
		float camZ = largestDim;
		cameraPosition = defaultCP = glm::vec3(camX, camY, camZ);
		cout << "camerPos = " << camX << ", " << camY << ", " << camZ << endl;

		float focX = (xPlus - std::fabs(xMinus))/2.0;
		float focY = (yPlus - std::fabs(yMinus))/2.0;
		float focZ = (zPlus - std::fabs(zMinus))/2.0;
		focalPoint = defaultFP = glm::vec3(focX, focY, focZ);
		cout << "focalPoint = " << focX << ", " << focY << ", " << focZ << endl;
	}
	
	
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

	
	init();
	glutDisplayFunc( display );
	glutKeyboardFunc( keys );
	glutSpecialFunc( specialKeys );//For arrows
	glutReshapeFunc( windowResize );

	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LESS);
	glDepthMask( GL_TRUE );

	cout << "\nGL Vendor            :" << vendor << endl;
	cout << "GL Renderer          :" << renderer  << endl;
	cout << "GL Version (string)  :" << version << endl;
	cout << "GL Version (integer) :" << major << " " << minor << endl;
	cout << "GLSL Version         :" << glslVersion << endl;

	cout<<"\nControls:\n"
	<<"Arrow keys up/down: move forward and backward\n"
	<<"Arrow keys left/right: rotate around z axis\n"
	<<"Z and X: rotate around gaze vector\n"
	<<"V and C: Move up and down along view up vector\n"
	<<"F and D: Tilt camera up and down\n\n"
	<<"S: change to solid mode\n"
	<<"W: change to wireframe mode\n"
	<<"Q or ESC: to quit"<<endl;

	glutMainLoop();

	return 0;
}
