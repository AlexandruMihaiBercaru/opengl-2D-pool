
#include <windows.h>        //	Utilizarea functiilor de sistem Windows (crearea de ferestre, manipularea fisierelor si directoarelor);
#include <stdlib.h>         //  Biblioteci necesare pentru citirea shaderelor;
#include <stdio.h>
#include <GL/glew.h>        //  Definește prototipurile functiilor OpenGL si constantele necesare pentru programarea OpenGL moderna; 
#include <GL/freeglut.h>    //	Include functii pentru: 
//	- gestionarea ferestrelor si evenimentelor de tastatura si mouse, 
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

//  Identificatorii obiectelor de tip OpenGL;
GLuint
VaoId,
VboId,
EboId,
ColorBufferId,
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
#define PI 3.141592653
int BALL_COUNT = 7 ;
int PT_COUNT = 16;

float dim_patrat = 30.0f;
int codCol;

bool startAnimation = false;


class Ball
{
public:
	float r; // raza bilei (fiza in cazul nostru)
	glm::vec2 position; // pozitia (centrul bilei la un moment dat)
	// pozitia initiala este data prin constructor (x_, y_)
	glm::vec2 v; // viteza (deplasarea la fiecare randare)
	int nrPuncte; // numarul de pucnte de pe cerc din care este "alcauit" cercul
	bool isRendered = true; // daca afisam bila sau nu (pentru cazul in care sunt introduse in gauri - cum simulam ca "dispar" de pe masa?
	glm::mat4 matrTransl = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0)); // matricea de deplasare 

	// constructor
	Ball(float r_, float x_, float y_, int nrPuncte_, float vx_, float vy_) {
		r = r_;
		position.x = x_;
		position.y = y_;
		nrPuncte = nrPuncte_;
		v.x = vx_;
		v.y = vy_;

		UpdateTranslationMatrix();
	}

	~Ball() {}

	void AddVertices(GLfloat Vertices[], int& start) {
		for (int k = 0; k < nrPuncte; k++)
		{
			float theta = 2 * k * PI / nrPuncte;
			float X = r * cos(theta), Y = r * sin(theta);

			Vertices[start++] = X; Vertices[start++] = Y;
			Vertices[start++] = 0.0f; Vertices[start++] = 1.0f;
		}
	}

	void UpdateTranslationMatrix() {
		matrTransl = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0));
	}

	float distance(Ball otherBall) {
		return sqrt(pow(position.x - otherBall.position.x, 2) + pow(position.y - otherBall.position.y, 2));
	}

};


std::vector<Ball> createBalls() {
	std::vector<Ball> balls;
	balls.push_back(Ball(10.0, 0.0, 0.0, PT_COUNT, 0.0, 0.0));
	balls.push_back(Ball(10.0, 15.0, -15.0, PT_COUNT, 0.0, 0.0));
	balls.push_back(Ball(10.0, 15.0, 15.0, PT_COUNT, 0.0, 0.0));
	balls.push_back(Ball(10.0, 30.0, -30.0, PT_COUNT, 0.0, 0.0));
	balls.push_back(Ball(10.0, 30.0, 0.0, PT_COUNT, 0.0, 0.0));
	balls.push_back(Ball(10.0, 30.0, 30.0, PT_COUNT, 0.0, 0.0));
	balls.push_back(Ball(10.0, -200.0, 0.0, PT_COUNT, 0.3, 0.0));

	return balls;
}
std::vector<Ball> bile = createBalls();


void rotateBall(float angle, int i) {
	float vxOld = bile[i].v.x, vyOld = bile[i].v.y;
	bile[i].v.x = vxOld * cos(angle) - vyOld * sin(angle);
	bile[i].v.y = vxOld * sin(angle) + vyOld * cos(angle);
	//std::cout << "Dupa rotire: " << bile[i].vx << " " << bile[i].vy;
}

void check2DCollisions() {
	for (int i = 0; i < BALL_COUNT - 1; i++) {
		for (int j = i + 1; j < BALL_COUNT; j++) {
			float minDist = bile[i].r + bile[j].r, dist = bile[i].distance(bile[j]);
			//std::cout << "Distanta dintre bile: " << dist << " suma razelor: " << minDist << std::endl;
			if (dist < minDist) { // echivalent cu 2 * bile[i].r

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
				rotateBall(-theta, i);
				rotateBall(-theta, j);

				// aplicam formulele pt coliziunea elastica 1D pe componenta vx
				std::swap(bile[i].v.x, bile[j].v.x);

				// rotatia de unghi theta (pt a aduce vectorul la orientarea initiala)
				rotateBall(theta, i);
				rotateBall(theta, j);
				

				/*std::cout << "Bila " << i << " DUPA COLIZIUNE : " << bile[i].vx << " " << bile[i].vy << std::endl;
				std::cout << "Bila " << j << " DUPA COLIZIUNE : " << bile[j].vx << " " << bile[j].vy << std::endl;*/

			}
		}
	}
}

void IdleFunction() {
	if (!startAnimation)
		return;
	for (int i = 0; i < BALL_COUNT; i++) {

		//TODO: functie separata pentru coliziunile cu bounding box / pereti
		bile[i].position.x += bile[i].v.x;
		bile[i].position.y += bile[i].v.y;

		if (bile[i].position.x + bile[i].r > xMax) {
			bile[i].position.x = xMax - bile[i].r;
			bile[i].v.x = -bile[i].v.x;
		}
		else if (bile[i].position.x - bile[i].r < xMin) {
			bile[i].position.x = xMin + bile[i].r;
			bile[i].v.x = -bile[i].v.x;
		}

		if (bile[i].position.y + bile[i].r > yMax) {
			bile[i].position.y = yMax - bile[i].r;
			bile[i].v.y = -bile[i].v.y;
		}
		else if (bile[i].position.y - bile[i].r < yMin) {
			bile[i].position.y = yMin + bile[i].r;
			bile[i].v.y = -bile[i].v.y;
		}
	}

	check2DCollisions();

	for (int i = 0; i < BALL_COUNT; i++)
		bile[i].UpdateTranslationMatrix();

	glutPostRedisplay();
}

void UseMouse(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
			startAnimation = true;
		break;
	default:
		break;
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
	int size = BALL_COUNT * PT_COUNT * 4;
	GLfloat* Vertices = new GLfloat[size];

	int start_idx = 0;
	for (int i = 0; i < BALL_COUNT; i++)
		bile[i].AddVertices(Vertices, start_idx);

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

	//  Eliberaea obiectelor de tip VAO;
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
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
	glClearColor(1.0f, 1.0f, 0.75f, 0.0f);		//  Culoarea de fond a ecranului;
	CreateVBO();								//  Trecerea datelor de randare spre bufferul folosit de shadere;
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
	for (int i = 0; i < BALL_COUNT; i++)
	{
		myMatrix = resizeMatrix * bile[i].matrTransl;
		glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
		if (i % 2 == 0) codCol = 1;
		else codCol = 2;
		glUniform1i(codColLocation, codCol);
		glDrawArrays(GL_TRIANGLE_FAN, 2 + i * PT_COUNT, PT_COUNT);
	}

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
	glutIdleFunc(IdleFunction);
	glutCloseFunc(Cleanup);				//  Eliberarea resurselor alocate de program;

	//  Bucla principala de procesare a evenimentelor GLUT (functiile care incep cu glut: glutInit etc.) este pornita;
	//  Prelucreaza evenimentele si deseneaza fereastra OpenGL pana cand utilizatorul o inchide;

	glutMainLoop();

	return 0;
}


