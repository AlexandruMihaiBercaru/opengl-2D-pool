
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
#include "Ball.h"
#include <memory.h> //no idea if i'll actually use it
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

int BALL_COUNT = 7;

float dim_patrat = 30.0f;
int codCol;


std::vector<Ball> createBalls() {
	std::vector<Ball> balls;
	balls.emplace_back(Ball(1, 0.0, 0.0, 0.0, 0.0)); 
	balls.emplace_back(Ball(2, 15.0, -15.0, 0.0, 0.0));
	balls.emplace_back(Ball(1, 15.0, 15.0, 0.0, 0.0));
	balls.emplace_back(Ball(2, 30.0, -30.0, 0.0, 0.0));
	balls.emplace_back(Ball(1, 30.0, 0.0, 0.0, 0.0));
	balls.emplace_back(Ball(2, 30.0, 30.0, 0.0, 0.0));
	balls.emplace_back(Ball(3, -200.0, 0.0, 0.3, 0.0));//bila alba

	return balls;
}
std::vector<Ball> bile = createBalls();


//Pointer catre bila alba, may be usefull
//Initializat in Initialize()
Ball* whiteBall = NULL;


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


bool startAnimation = false;


/// <summary>
/// Deplasarea virtuala a bilelor, decelerare.
/// Si ne asiguram ca bilele raman nu ies din limitele mesei (bounce back off edges)
/// </summary>
static void CheckCollisionEdges() {
	for(Ball& bila : bile)
	{


		//Bila se deplaseaza
		bila.position.x += bila.v.x;
		bila.position.y += bila.v.y;


		//Bouncing off walls
		if (bila.position.x + Ball::r > xMax) {
			bila.position.x = xMax - Ball::r;
			bila.v.x = -bila.v.x;
		}
		else if (bila.position.x - Ball::r < xMin) {
			bila.position.x = xMin + Ball::r;
			bila.v.x = -bila.v.x;
		}

		if (bila.position.y + Ball::r > yMax) {
			bila.position.y = yMax - Ball::r;
			bila.v.y = -bila.v.y;
		}
		else if (bila.position.y - Ball::r < yMin) {
			bila.position.y = yMin + Ball::r;
			bila.v.y = -bila.v.y;
		}
	}
}

static void UpdateAllTranslationMatrix() {
	for (Ball& bila : bile)
		bila.UpdateTranslationMatrix();
}

static void IdleFunction() {
	if (!startAnimation)
		return;
	//am facut functie separata
	CheckCollisionEdges();
	check2DCollisions();

	//E practic un wrapper
	UpdateAllTranslationMatrix();

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
	whiteBall = &bile.back();
	glClearColor(0.082f, 0.36f, 0.08f, 1.0f);		//  Culoarea de fond a ecranului;
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

	//voi incerca sa mai modific aici

	for (int i = 0; i < BALL_COUNT; i++)
	{
		myMatrix = resizeMatrix * bile[i].matrTransl;
		glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
		//Bilele ar trebui sa aiba culoarea drept camp intern
	/*	if (i % 2 == 0) codCol = 1;
		else codCol = 2;*/


		codCol = bile[i].codCol;
		auto var = bile[i];
		glUniform1i(codColLocation, codCol);
		glDrawArrays(GL_TRIANGLE_FAN, i * bile.size(), Ball::nrPuncte);
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


