
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
VaoId,
VaoId2,
VboId, VboId2,
EboId, EboId2,
ColorBufferId, ColorBufferId2,
ProgramId,
codColLocation,
myMatrixLocation;
//	Dimensiunile ferestrei de afisare;
GLfloat
winWidth = 800, winHeight = 600;
//	Variabile catre matricile de transformare;
glm::mat4
myMatrix, resizeMatrix, matrTransl, matrScale1, matrScale2, matrRot, matrDepl;

//	Variabile pentru proiectia ortogonala;
float xMin = -400.0, xMax = 400.0f, yMin = -300.0f, yMax = 300.0f;

int BALL_COUNT = 7;

float dim_patrat = 30.0f;
int codCol;

//Pointer catre bila alba, may be usefull
//Initializat in Initialize()
Ball* whiteBall = NULL;

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

				float overlap = (minDist - dist) / 2.0f;
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
		if (bila.position.x + Ball::r >= xMax) {
			bila.position.x = xMax - Ball::r;
			bila.v.x = -bila.v.x * 0.9f;
		}
		else if (bila.position.x - Ball::r <= xMin) {
			bila.position.x = xMin + Ball::r;
			bila.v.x = -bila.v.x * 0.9f;
		}

		if (bila.position.y + Ball::r >= yMax) {
			bila.position.y = yMax - Ball::r;
			bila.v.y = -bila.v.y * 0.9f;
		}
		else if (bila.position.y - Ball::r <= yMin) {
			bila.position.y = yMin + Ball::r;
			bila.v.y = -bila.v.y * 0.9f;
		}
	}
}

static void UpdateAllTranslationMatrix() {
	for (Ball& bila : bile)
		bila.UpdateTranslationMatrix();
}

bool startAnimation = false;

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

	CheckCollisionEdges();
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
		whiteBall->v = glm::vec2(0.0, 0.0);
		cue.isHitting = false;
		cue.stoppedHitting = false;
		startAnimation = false;
    
		//glClearColor(0.75f, 1.0f, 1.0f, 1.0f);
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
		if (state == GLUT_DOWN) {
			cue.startHit();
		}
		break;
	default:
		break;
	}
}

void MouseMotion(int x, int y) {
	if (cue.canRotate) {
		glm::vec2 worldPos = screenToWorld(glm::vec2(x, y));
		float angle = cue.GetBallAngle(worldPos);
		cue.angle = angle;
		cue.position = cue.PosToAngle(angle, 10.0f);
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
void CreateVBO(void)
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

	//  Eliberaea obiectelor de tip VAO;
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
	glDeleteVertexArrays(1, &VaoId2);

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
	CreateVBO();								//  Trecerea datelor de randare spre bufferul folosit de shadere;
	CreateCue(); // VAO pentru tac
	CreateShaders();							//  Initilizarea shaderelor;
	//	Instantierea variabilelor uniforme pentru a "comunica" cu shaderele;
	myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
	codColLocation = glGetUniformLocation(ProgramId, "codCol");
	//	Dreptunghiul "decupat"; 
	resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);
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

	// DESENEZ BILE
	glBindVertexArray(VaoId);
	for (int i = 0; i < BALL_COUNT; i++)
	{
		myMatrix = resizeMatrix * bile[i].matrTransl;
		glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

		codCol = bile[i].codCol;
		auto var = bile[i];
		glUniform1i(codColLocation, codCol);
		glDrawArrays(GL_TRIANGLE_FAN, i * bile.size(), Ball::nrPuncte);
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


