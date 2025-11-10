#include "Cue.h"



Cue::Cue(float l_, float w_, float x, float y) : length(l_), width(w_), position(x, y), isDragged(false), wBall(NULL) {};


bool Cue::isMouseInsideBox(glm::vec2 mouseCoord) {
	float left = position.x - length / 2.0f;
	float right = position.x + length / 2.0f;
	float top = position.y + width / 2.0f;
	float bottom = position.y - width / 2.0f;

	return (mouseCoord.x >= left && mouseCoord.x <= right && mouseCoord.y >= bottom && mouseCoord.y <= top);
}


void Cue::SetBall(Ball* wBall) {
	this->wBall = wBall;
}


void Cue::BringToBall() {
	float dist = 5.0f; // distanta arbitrara intre varful tacului si marginea bilei
	float radius = wBall->r;
	float degrees = 90;
	this->angle = PI * degrees / 180.0f; // 180 grade = pi => 150 grade = pi*150/180
	this->position.x = wBall->position.x + (dist + radius + this->length / 2) * glm::cos(angle);
	this->position.y = wBall->position.y + (dist + radius + this->length / 2) * glm::sin(angle);

	//std::cout << unghi << "unghi\n";
	//std::cout << wBall->position.x << " " << wBall->position.y << "=Bila \n";
	//std::cout << this->position.x << " " << this->position.y << "=Cue \n";

}