#pragma once

#include <GL/glew.h>        //  Definește prototipurile functiilor OpenGL si constantele necesare pentru programarea OpenGL moderna; 
#include <GL/freeglut.h>    //	Include functii pentru: 
//	- gestionarea ferestrelor si evenimentelor de tastatura si mouse, 
//  - desenarea de primitive grafice precum dreptunghiuri, cercuri sau linii, 
//  - crearea de meniuri si submeniuri;
#include "glm/glm.hpp"		//	Bibloteci utilizate pentru transformari grafice;
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <math.h>
#define PI 3.141592653

class Ball
{
	bool isRendered = true; // daca afisam bila sau nu (pentru cazul in care sunt introduse in gauri - cum simulam ca "dispar" de pe masa?

public:
	int codCol=3;

	static int nrPuncte; // numarul de puncte de pe cerc din care este "alcatuit" cercul
	static float r;// raza bilei (fiza in cazul nostru) dar toate bilele sunt la fel
	//acceleration variables here when it's done
	glm::vec2 position; // pozitia (centrul bilei la un moment dat)
	glm::vec2 v; // viteza (deplasarea la fiecare randare)
	// pozitia initiala este data prin constructor (x_, y_)
	bool isMoving = false;
	glm::mat4 matrTransl = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0)); // matricea de deplasare 

	Ball(int codCol_, float x_, float y_, float vx_, float vy_);
	~Ball();
	void rotateBall(float angle);
	void AddVertices(GLfloat Vertices[], int& start);
	void UpdateTranslationMatrix();
	float distance(const Ball& otherBall);
	void applyFriction(float p);
	void updateMovementStatus();
};