
#include <windows.h>        //	Utilizarea functiilor de sistem Windows (crearea de ferestre, manipularea fisierelor si directoarelor);
#include <stdlib.h>         //  Biblioteci necesare pentru citirea shaderelor;
#include <stdio.h>
#include <GL/glew.h>        //  Definește prototipurile functiilor OpenGL si constantele necesare pentru programarea OpenGL moderna; 
#include <GL/freeglut.h>    //	Include functii pentru: 
//	- gestionarea ferestrelor si evenime ntelor de tastatura si mouse, 
//  - desenarea de primitive grafice precum dreptunghiuri, cercuri sau linii, 
//  - crearea de meniuri si submeniuri;
#include "loadShaders.h"	//	Fisierul care face legatura intre program si shadere;
#include "glm/glm.hpp"		//	Bibloteci utilizate pentru transformari grafice;
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include <math.h>
#include <iostream>
#include <memory.h> //no idea if i'll actually use it
#include "Ball.h"
#include "Cue.h"
//  Identificatorii obiectelor de tip OpenGL;
GLuint
VaoId, VaoId2, VaoId3,
VboId, VboId2, VboId3,
EboId, EboId2, EboId3,
ColorBufferId, ColorBufferId2,
ProgramId,
codColLocation,
myMatrixLocation;
//	Dimensiunile ferestrei de afisare;
GLfloat
winWidth = 800, winHeight = 600;
//	Variabile catre matricile de transformare;
glm::mat4
myMatrix, resizeMatrix, matrTransl, matrScale1, matrScale2, matrRot, matrDepl, normalMatrix;

//	Variabile pentru proiectia ortogonala;
float xMin = -400.0, xMax = 400.0f, yMin = -300.0f, yMax = 300.0f;
GLsizei IndexCount;
int BALL_COUNT = 7;
int codCol;

//Pointer catre bila alba, may be usefull
//Initializat in Initialize()
Ball* whiteBall = NULL;

struct Vertex {
	GLfloat x, y, z, w;
	GLfloat r, g, b, a;
};

std::vector<Vertex> vertices;
std::vector<GLuint> indices;

// Functie pentru a adauga un dreptunghi
void addRectangle(float x1, float y1, float x2, float y2, glm::vec4 color) {
	GLuint start = vertices.size();
	vertices.push_back({ x1, y1, 0, 1, color.r, color.g, color.b, color.a });
	vertices.push_back({ x2, y1, 0, 1, color.r, color.g, color.b, color.a });
	vertices.push_back({ x2, y2, 0, 1, color.r, color.g, color.b, color.a });
	vertices.push_back({ x1, y2, 0, 1, color.r, color.g, color.b, color.a });

	indices.push_back(start);
	indices.push_back(start + 1);
	indices.push_back(start + 2);
	indices.push_back(start);
	indices.push_back(start + 2);
	indices.push_back(start + 3);
}

// Functie pentru a adauga un trapez
void addTrapezoid(glm::vec2 a1, glm::vec2 a2, glm::vec2 b2, glm::vec2 b1, glm::vec4 color) {
	GLuint start = vertices.size();
	vertices.push_back({ a1.x, a1.y, 0, 1, color.r, color.g, color.b, color.a });
	vertices.push_back({ a2.x, a2.y, 0, 1, color.r, color.g, color.b, color.a });
	vertices.push_back({ b2.x, b2.y, 0, 1, color.r, color.g, color.b, color.a });
	vertices.push_back({ b1.x, b1.y, 0, 1, color.r, color.g, color.b, color.a });
	indices.push_back(start);
	indices.push_back(start + 1);
	indices.push_back(start + 2);
	indices.push_back(start);
	indices.push_back(start + 2);
	indices.push_back(start + 3);
}

// Functie pentru a adauga un cerc
void addCircle(float cx, float cy, float r, glm::vec4 color, int segments = 40) {
	GLuint start = vertices.size();
	vertices.push_back({ cx, cy, 0, 1, color.r, color.g, color.b, color.a });
	for (int i = 0; i <= segments; i++) {
		float angle = i * 2.0f * 3.1415926f / segments;
		float x = cx + r * cos(angle);
		float y = cy + r * sin(angle);
		vertices.push_back({ x, y, 0, 1, color.r, color.g, color.b, color.a });
	}
	for (int i = 1; i <= segments; i++) {
		indices.push_back(start);
		indices.push_back(start + i);
		indices.push_back(start + i + 1);
	}
}

glm::vec2 dragStartPos, cueStartPos;

glm::vec2 screenToWorld(glm::vec2 mouseCoord) {
	float worldX = xMin + (mouseCoord.x / winWidth) * (xMax - xMin);
	float worldY = yMax - (mouseCoord.y / winHeight) * (yMax - yMin);
	return glm::vec2(worldX, worldY);
}

std::vector<Ball> createBalls() {
	std::vector<Ball> balls;
	balls.emplace_back(Ball(1, 0.0, 0.0, 0.0, 0.0)); 
	balls.emplace_back(Ball(2, 20.0, -10.0, 0.0, 0.0));
	balls.emplace_back(Ball(1, 20.0, 10.0, 0.0, 0.0));
	balls.emplace_back(Ball(2, 40.0, -20.0, 0.0, 0.0));
	balls.emplace_back(Ball(1, 40.0, 0.0, 0.0, 0.0));
	balls.emplace_back(Ball(2, 40.0, 20.0, 0.0, 0.0));
	balls.emplace_back(Ball(3, -200.0, 0.0, 0.0, 0.0));//bila alba

	return balls;
}
std::vector<Ball> bile = createBalls();

Cue cue(500.0f, 14.0f, 0.0f, -250.0f);

/// <summary>
/// Verificam daca 2 bile au coliziune
/// Calculam unghiul de coliziune elastica
/// Rotim bilele as needed
/// </summary>
static void check2DCollisions() {
	for (int i = 0; i < BALL_COUNT - 1; i++) {
		for (int j = i + 1; j < BALL_COUNT; j++) {
			float minDist = 2 * Ball::r;
			float dist = bile[i].distance(bile[j]);
			//std::cout << "Distanta dintre bile: " << dist << " suma razelor: " << minDist << std::endl;
			if (dist < minDist) {

				// unghiul dat de dreapta care uneste centrele celor doua bile si Ox
				float theta = atan2((bile[j].position.y - bile[i].position.y), (bile[j].position.x - bile[i].position.x));
				//std::cout << "theta = " << theta << std::endl;

				float overlap = (minDist - dist) / 2.0f + 1.0f;
				glm::vec2 separation = { overlap * cos(theta), overlap * sin(theta) };
				/*float separateX = overlap * cos(theta);
				float separateY = overlap * sin(theta);*/
				bile[i].position -= separation;
				bile[j].position += separation;

				// rotatie de unghi -theta aplicata componentelor vitezei (pt ambele bile)
				bile[i].rotateBall(-theta);
				bile[j].rotateBall(-theta);

				// aplicam formulele pt coliziunea elastica 1D pe componenta vx
				std::swap(bile[i].v.x, bile[j].v.x);

				// rotatia de unghi theta (pt a aduce vectorul la orientarea initiala)
				bile[i].rotateBall(theta);
				bile[j].rotateBall(theta);

				/*std::cout << "Bila " << i << " DUPA COLIZIUNE : " << bile[i].vx << " " << bile[i].vy << std::endl;
				std::cout << "Bila " << j << " DUPA COLIZIUNE : " << bile[j].vx << " " << bile[j].vy << std::endl;*/

			}
		}
	}
}

static void CheckBallsInHoles() {
	float holeRadius = 20.0f, ballRadius = Ball::r;
	std::vector<glm::vec2> centreGauri = {
		{-0.82 * 400.0, 0.51 * 300.0},
		{0.82 * 400.0,  0.51 * 300.0},
		{-0.82 * 400.0, -0.51 * 300.0},
		{0.82 * 400.0, -0.51 * 300.0},
		{0.0, 0.51 * 300.0},
		{0.0, -0.51 * 300.0},
	};
	for (Ball& b : bile) {
		for (glm::vec2 centru : centreGauri) {
			float dist = glm::length(b.position - centru);
			if (dist <= holeRadius) {
				b.isRendered = false; // hopa! a intrat 
			}
		}
	}
}

/// <summary>
/// Deplasarea virtuala a bilelor.
/// Si ne asiguram ca bilele raman nu ies din limitele mesei (bounce back off edges)
/// </summary>
static void CheckCollisionEdges() {
	for(Ball& bila : bile)
	{
		//Bila se deplaseaza
		bila.position += bila.v;

		//Bouncing off walls
		if (bila.position.x + Ball::r >= 0.81f * 400.0) {
			bila.position.x = 0.81f * 400.0 - Ball::r;
			bila.v.x = -bila.v.x * 0.9f;
		}
		else if (bila.position.x - Ball::r <= -0.81f * 400.0) {
			bila.position.x = -0.81f * 400.0 + Ball::r;
			bila.v.x = -bila.v.x * 0.9f;
		}

		if (bila.position.y + Ball::r >= 0.485f * 300.0) {
			bila.position.y = 0.485f * 300.0 - Ball::r;
			bila.v.y = -bila.v.y * 0.9f;
		}
		else if (bila.position.y - Ball::r <= -0.485f * 300.0) {
			bila.position.y = -0.485f * 300.0 + Ball::r;
			bila.v.y = -bila.v.y * 0.9f;
		}
	}
}

static void UpdateAllTranslationMatrix() {
	for (Ball& bila : bile)
		bila.UpdateTranslationMatrix();
}

bool startAnimation = true;

static void IdleFunction() {
	//std::cout << startAnimation << std::endl;
	if (cue.isHitting) {

		bool hit = cue.updateHit();
		if (hit) {
			cue.position = glm::vec2(0.0f, -250.0f); cue.angle = 0.0f;
			startAnimation = true;
		}
	}

	if (!startAnimation)
		return;

	CheckBallsInHoles();
	CheckCollisionEdges(); // INCLUDE DEPLASAREA BILELOR
	check2DCollisions();
	bool anyMoves = false;

	for (int i = 0; i < BALL_COUNT; i++) {
		bile[i].UpdateTranslationMatrix(); // actualizeaza pozitia pentru noua randare
		bile[i].applyFriction(0.999f); // reduce viteza
		bile[i].updateMovementStatus();
		anyMoves = anyMoves || bile[i].isMoving;
		//std::cout << "Viteza bilei " << i << " este: " << bile[i].v.x << " " << bile[i].v.y << std::endl; 
	}

	// daca nicio bila nu se mai afla in miscare, atunci putem trece la urmatoarea runda
	if (!anyMoves)
	{
		std::cout << "White ball X=" << whiteBall->position.x << " Vx=" << whiteBall->v.x << std::endl;
		std::cout << "White ball Y=" << whiteBall->position.y << " Vy=" << whiteBall->v.y << std::endl;
		for (auto &b : bile)
			b.v = glm::vec2(0.0, 0.0);
		whiteBall->v = glm::vec2(0.0, 0.0);
		cue.isHitting = false;
		cue.stoppedHitting = false;
		startAnimation = false;
    
		std::cout << "STOPPED\n"; // de completat cu controlul rundelor
		
		cue.BringToBall();
		cue.canRotate = true;
	}
	
	UpdateAllTranslationMatrix();

	glutPostRedisplay();
}


void UseMouse(int button, int state, int x, int y)
{
	glm::vec2 worldCoords = screenToWorld(glm::vec2(x, y));

	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {
			if (cue.isMouseInsideBox(worldCoords)) {
				glClearColor(0.082f, 0.36f, 0.08f, 1.0f);
				cue.isDragged = true;
				dragStartPos = worldCoords;
				cueStartPos = cue.position;
			}
		}
		else if (state == GLUT_UP) {
			if (cue.isDragged) {
				cue.isDragged = false;
			}
		}
			
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN && !whiteBall->isMoving) {
			cue.startHit();
		}
		break;
	default:
		break;
	}
}

void MouseMotion(int x, int y) {
	if (whiteBall->isMoving)
		return;
	if (cue.canRotate) {
		glm::vec2 worldPos = screenToWorld(glm::vec2(x, y));
		float angle = cue.GetBallAngle(worldPos);
		cue.angle = angle;
		cue.position = cue.PosToAngle(angle, Ball::r);
		std::cout <<"Angle = " <<angle << '\n';

		glutPostRedisplay();

	}

	if (cue.isDragged) {
		glm::vec2 worldPos = screenToWorld(glm::vec2(x, y));
		glm::vec2 delta = worldPos - dragStartPos;
		cue.position = cueStartPos + delta;

		glutPostRedisplay();
	}
}

void CreateShaders(void)
{
	ProgramId = LoadShaders("Shader.vert", "Shader.frag");
	glUseProgram(ProgramId);
}


//  Se initializeaza un Vertex Buffer Object (VBO) pentru tranferul datelor spre memoria placii grafice (spre shadere);
//  In acesta se stocheaza date despre varfuri (coordonate, culori, indici, texturare etc.);
void CreateBalls(void)
{
	//  Coordonatele varfurilor;
	// numar de bile * numar puncte per bila * 4 coordonate per punct
	int size = BALL_COUNT * Ball::nrPuncte * 4;
	GLfloat* Vertices = new GLfloat[size];

	int start_idx = 0;
	for (Ball& bila: bile) 
		bila.AddVertices(Vertices, start_idx);
	
	
	//	Culorile ca atribute ale varfurilor;
	static const GLfloat Colors[] ={1.0f, 1.0f, 1.0f, 1.0f};

	//  Se creeaza / se leaga un VAO (Vertex Array Object) - util cand se utilizeaza mai multe VBO;
	glGenVertexArrays(1, &VaoId);                                                   //  Generarea VAO si indexarea acestuia catre variabila VaoId;
	glBindVertexArray(VaoId);

	//  Se creeaza un buffer comun pentru VARFURI - COORDONATE si CULORI;
	glGenBuffers(1, &VboId);																//  Generarea bufferului si indexarea acestuia catre variabila VboId;
	glBindBuffer(GL_ARRAY_BUFFER, VboId);													//  Setarea tipului de buffer - atributele varfurilor;
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(GLfloat), Vertices, GL_STATIC_DRAW);	//	Definirea bufferului, dimensiunea lui = dimensiunea(COLORS + VERTICES)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);


	//  Se creeaza un buffer pentru CULOARE;
	glGenBuffers(1, &ColorBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Colors), Colors, GL_STATIC_DRAW);
	//  Se asociaza atributul (1 =  culoare) pentru shader;
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

}

void CreateTable(void) {
	vertices.clear();
	indices.clear();

	glm::vec4 green(0.0f, 0.5f, 0.2f, 1.0f);
	glm::vec4 brown(0.6f, 0.3f, 0.1f, 1.0f);
	glm::vec4 black(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 lightGreen(0.0f, 0.8f, 0.4f, 1.0f);

	// Rama maro exterioară
	addRectangle(-0.9f, -0.6f, 0.9f, 0.6f, brown); // stanga-jos si dreapta-sus

	// Panza verde (zona centrala)
	addRectangle(-0.83f, -0.51f, 0.83f, 0.51f, green);

	// Margini verde deschis (trapeze)
	// stanga
	addTrapezoid({ -0.83f, 0.46f }, { -0.81f, 0.44f }, { -0.81f, -0.44f }, { -0.83f, -0.46f }, lightGreen);
	// dreapta
	addTrapezoid({ 0.83f, 0.46f }, { 0.81f, 0.44f }, { 0.81f, -0.44f }, { 0.83f, -0.46f }, lightGreen);
	// sus - stanga
	addTrapezoid({ -0.77f, 0.51f }, { -0.05f, 0.51f }, { -0.07f, 0.485f }, { -0.75f, 0.485f }, lightGreen);
	// sus - dreapta
	addTrapezoid({ 0.05f, 0.51f }, { 0.77f, 0.51f }, { 0.75f, 0.485f }, { 0.07f, 0.485f }, lightGreen);
	// jos - stanga 
	addTrapezoid({ -0.77f, -0.51f }, { -0.05f, -0.51f }, { -0.07f, -0.485f }, { -0.75f, -0.485f }, lightGreen);
	// jos - dreapta 
	addTrapezoid({ 0.05f, -0.51f }, { 0.77f, -0.51f }, { 0.75f, -0.485f }, { 0.07f, -0.485f }, lightGreen);

	// Gauri negre
	float rHole = 0.05f;
	addCircle(-0.82f, 0.51f, rHole, black);   // stanga sus
	addCircle(0.82f, 0.51f, rHole, black);    // dreapta sus
	addCircle(-0.82f, -0.51f, rHole, black);  // stanga jos
	addCircle(0.82f, -0.51f, rHole, black);   // dreapta jos
	addCircle(0.0f, 0.51f, rHole, black);    // mijloc sus
	addCircle(0.0f, -0.51f, rHole, black);   // mijloc jos

	IndexCount = indices.size();

	glGenVertexArrays(1, &VaoId3);
	glBindVertexArray(VaoId3);

	glGenBuffers(1, &VboId3);
	glBindBuffer(GL_ARRAY_BUFFER, VboId3);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &EboId3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(4 * sizeof(GLfloat)));

	glBindVertexArray(0);
}

void CreateCue(void) 
{
	static const GLfloat Vertices2[] = {
		-cue.length / 2.0f, -cue.width / 2.0f + 2, 0.0f, 1.0f,
		-cue.length / 2.0f,  cue.width / 2.0f - 2, 0.0f, 1.0f,
		 cue.length / 2.0f,  cue.width / 2.0f, 0.0f, 1.0f,
		 cue.length / 2.0f, -cue.width / 2.0f, 0.0f, 1.0f
	};

	static const GLfloat Colors2[] = {
		1.0f, 0.95f, 0.8f, 1.0f,
		1.0f, 0.95f, 0.8f, 1.0f,
		0.55f, 0.45f, 0.1f, 1.0f,
		0.55f, 0.45f, 0.1f, 1.0f,
	};
	static const GLuint Indices2[] = {0, 1, 2, 3};

	glGenVertexArrays(1, &VaoId2);         //  Generarea VAO si indexarea acestuia catre variabila VaoId2;
	glBindVertexArray(VaoId2);

	//  Se creeaza un buffer pentru VARFURI;
	glGenBuffers(1, &VboId2);                                                        //  Generarea bufferului si indexarea acestuia catre variabila VboId2;
	glBindBuffer(GL_ARRAY_BUFFER, VboId2);                                           //  Setarea tipului de buffer - atributele varfurilor;
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);
	//  Se asociaza atributul (0 = coordonate) pentru shader;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	//  Se creeaza un buffer pentru CULOARE;
	glGenBuffers(1, &ColorBufferId2);
	glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Colors2), Colors2, GL_STATIC_DRAW);
	//  Se asociaza atributul (1 =  culoare) pentru shader;
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	//	Se creeaza un buffer pentru INDICI;
	glGenBuffers(1, &EboId2);														//  Generarea bufferului si indexarea acestuia catre variabila EboId2;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices2), Indices2, GL_STATIC_DRAW);

}

//  Elimina obiectele de tip shader dupa rulare;
void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

//  Eliminarea obiectelor de tip VBO dupa rulare;
void DestroyVBO(void)
{
	//  Eliberarea atributelor din shadere (pozitie, culoare, texturare etc.);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	//  Stergerea bufferelor pentru VARFURI(Coordonate + Culori), INDICI;
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VboId);
	glDeleteBuffers(1, &ColorBufferId);
	glDeleteBuffers(1, &VboId2);
	glDeleteBuffers(1, &ColorBufferId2);
	glDeleteBuffers(1, &EboId2);
	glDeleteBuffers(1, &VboId3);
	glDeleteBuffers(1, &EboId3);

	//  Eliberaea obiectelor de tip VAO;
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
	glDeleteVertexArrays(1, &VaoId2);
	glDeleteVertexArrays(1, &VaoId3);

}

//  Functia de eliberare a resurselor alocate de program;
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

//  Setarea parametrilor necesari pentru fereastra de vizualizare;
void Initialize(void)
{
	whiteBall = &bile.back();
	cue.SetBall(whiteBall);

	glClearColor(0.082f, 0.36f, 0.08f, 1.0f);		//  Culoarea de fond a ecranului;
	CreateBalls();								//  Trecerea datelor de randare spre bufferul folosit de shadere;
	CreateCue();								// VAO pentru tac
	CreateTable();								// VAO pentru masa
	CreateShaders();							//  Initilizarea shaderelor;
	//	Instantierea variabilelor uniforme pentru a "comunica" cu shaderele;
	myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
	codColLocation = glGetUniformLocation(ProgramId, "codCol");
	//	Dreptunghiul "decupat"; 
	resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);
	normalMatrix = glm::ortho(-1.0, 1.0, -1.0, 1.0);
}

//  Functia de desenarea a graficii pe ecran;
void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT);			//  Se curata ecranul OpenGL pentru a fi desenat noul continut;

	//	Se translateaza de-a lungul axei Ox;
// matrDepl = glm::translate(glm::mat4(1.0f), glm::vec3(0, 80.0, 0.0));		//	Se translateaza patratul ROSU fata de patratul ALBASTRU;
// matrScale1 = glm::scale(glm::mat4(1.0f), glm::vec3(1.1, 0.3, 0.0));			//	Se scaleaza coordonatele initiale si se obtine dreptunghiul ALABSTRU;
// matrScale2 = glm::scale(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.0));		//	Se scaleaza coordonatele initiale si se obtine patratul ROSU;
// matrRot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0, 0.0, 1.0));	//	Roatie folosita la deplasarea patratului ROSU;

	//DESENEZ MASA
	glBindVertexArray(VaoId3);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &normalMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0);


	// DESENEZ BILE
	glBindVertexArray(VaoId);
	for (int i = 0; i < BALL_COUNT; i++)
	{
		if (!bile[i].isRendered) continue;
		myMatrix = resizeMatrix * bile[i].matrTransl;
		glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

		codCol = bile[i].codCol;
		auto var = bile[i];
		glUniform1i(codColLocation, codCol);
		glDrawArrays(GL_TRIANGLE_FAN, i * Ball::nrPuncte, Ball::nrPuncte);
	}

	// DESENEZ TACUL
	glBindVertexArray(VaoId2);
	glm::mat4 cueTranslationMat = glm::translate(glm::mat4(1.0f), glm::vec3(cue.position, 0.0f));
	glm::mat4 cueRotateMat = glm::rotate(glm::mat4(1.0f), cue.angle, glm::vec3(0.0, 0.0, 1.0));
	myMatrix = resizeMatrix * cueTranslationMat*cueRotateMat;
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glUniform1i(codColLocation, 0);
	glDrawElements(GL_POLYGON, 4, GL_UNSIGNED_INT, (void*)(0));
	myMatrix = resizeMatrix;
	glutSwapBuffers();	//	Inlocuieste imaginea deseneata in fereastra cu cea randata; 
	glFlush();								//  Asigura rularea tuturor comenzilor OpenGL apelate anterior;
}

//	Punctul de intrare in program, se ruleaza rutina OpenGL;
int main(int argc, char* argv[])
{
	//  Se initializeaza GLUT si contextul OpenGL si se configureaza fereastra si modul de afisare;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);					//	Modul de afisare al ferestrei, se foloseste un singur buffer de afisare si culori RGB;
	glutInitWindowSize(winWidth, winHeight);						//  Dimensiunea ferestrei;
	glutInitWindowPosition(100, 100);								//  Pozitia initiala a ferestrei;
	glutCreateWindow("Patratul rotitor");		//	Creeaza fereastra de vizualizare, indicand numele acesteia;

	//	Se initializeaza GLEW si se verifica suportul de extensii OpenGL modern disponibile pe sistemul gazda;
	//  Trebuie initializat inainte de desenare;

	glewInit();

	Initialize();						//  Setarea parametrilor necesari pentru fereastra de vizualizare; 
	glutDisplayFunc(RenderFunction);	//  Desenarea scenei in fereastra;
	glutMouseFunc(UseMouse);
	glutMotionFunc(MouseMotion);
	glutIdleFunc(IdleFunction);
	glutCloseFunc(Cleanup);				//  Eliberarea resurselor alocate de program;

	//  Bucla principala de procesare a evenimentelor GLUT (functiile care incep cu glut: glutInit etc.) este pornita;
	//  Prelucreaza evenimentele si deseneaza fereastra OpenGL pana cand utilizatorul o inchide;

	glutMainLoop();

	return 0;
}


