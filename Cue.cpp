#include "Cue.h"
#include <iostream>


Cue::Cue(float l_, float w_, float x, float y) : length(l_), width(w_), position(x, y), isDragged(false), wBall(NULL), canRotate(false) {};


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



/// <summary>
/// Gets angle between a given coordonate and the white ball
/// </summary>
/// <param name="pos">Position X,Y</param>
/// <returns>Angle</returns>
float Cue::GetBallAngle(glm::vec2 pos) {
	glm::vec2 delta = pos - wBall->position;
	return atan2(delta.y, delta.x);
}


/// <summary>
/// Given the angle and distance desired, gives back the position of where the cue should be
/// in relation to the white ball.
/// </summary>
/// <param name="angle">Angle of drawing</param>
/// <param name="dist">Distance between tip of cue and white ball</param>
/// <returns>Position X,Y of where the center of the cue should be </returns>
glm::vec2 Cue::PosToAngle(float angle, float dist) {
	//float dist = 10.0f; // distanta arbitrara intre varful tacului si marginea bilei
	float radius = wBall->r;
	glm::vec2 coord;
	coord.x = wBall->position.x + (dist + radius + this->length / 2) * glm::cos(angle);
	coord.y = wBall->position.y + (dist + radius + this->length / 2) * glm::sin(angle);
	return coord;
}



void Cue::BringToBall() {
	///un numar arbitrar de grade
	float degrees = 170;
	this->angle = PI * degrees / 180.0f; // 180 grade = pi => 150 grade = pi*150/180
	this->position = this->PosToAngle(angle,10.0f);


/*	std::cout << angle << "unghi\n";
	std::cout << wBall->position.x << " " << wBall->position.y << "=Bila \n";
	std::cout << this->position.x << " " << this->position.y << "=Cue \n";*/
}


//void Cue::RotateAroundBall() {}

void Cue::startHit() {
	isHitting = true;
	direction = glm::normalize(wBall->position - position);
}

bool Cue::updateHit() {
	if (!isHitting) return false;

	position += direction * 0.1f;

	float stoppingDistance = Ball::r + length / 2.0;
	float currentDistance = glm::length(position - wBall->position);

	if (currentDistance <= stoppingDistance) {
		wBall->v = direction * 0.9f;
		isHitting = false;
		stoppedHitting = true;

		//std::cout << "viteza imprimata bilei: " << wBall->v.x << " " << wBall->v.y << std::endl;
		return true;
	}

	return false;
}

