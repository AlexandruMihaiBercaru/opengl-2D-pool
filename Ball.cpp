#include "Ball.h"


// constructor
Ball::Ball(int codCol_, float x_, float y_, float vx_, float vy_) {
	codCol = codCol_;
	position.x = x_;
	position.y = y_;
	v.x = vx_;
	v.y = vy_;
	UpdateTranslationMatrix();
}

Ball::~Ball() {}

void Ball::AddVertices(GLfloat Vertices[], int& start) {
	float theta = 2 * PI / nrPuncte;
	float fsum = 0;
	GLfloat X, Y;
	for (int k = 0; k < nrPuncte; k++)
	{
		//float theta = 2 * k * PI / nrPuncte;  //e mai eficient sa fecem o adunare la fiecare loop in loc de 2 inmultiri+impartire 
		X = r * cos(fsum);
		Y = r * sin(fsum);
		fsum += theta;

		Vertices[start++] = X; Vertices[start++] = Y;
		Vertices[start++] = 0.0f; Vertices[start++] = 1.0f;
	}
}

//void rotateBall(float angle,int i) 

void Ball::rotateBall(float angle) {
	float vxOld = v.x;
	float vyOld = v.y;
	v.x = vxOld * cos(angle) - vyOld * sin(angle);
	v.y = vxOld * sin(angle) + vyOld * cos(angle);
	//std::cout << "Dupa rotire: " << bile[i].vx << " " << bile[i].vy;
}

void Ball::UpdateTranslationMatrix() {

	matrTransl = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0));
}

float Ball::distance(const Ball& otherBall) {
	return sqrt(pow(position.x - otherBall.position.x, 2) + pow(position.y - otherBall.position.y, 2));
}


int Ball::nrPuncte = 16;
float Ball::r = 10.0f;