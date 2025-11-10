#include "Cue.h"
#include <iostream>


Cue::Cue(float l_, float w_, float x, float y, Ball* wBall) : length(l_), width(w_), position(x, y), isDragged(false), wBall(wBall) {};


bool Cue::isMouseInsideBox(glm::vec2 mouseCoord) {
	float left = position.x - length / 2.0f;
	float right = position.x + length / 2.0f;
	float top = position.y + width / 2.0f;
	float bottom = position.y - width / 2.0f;
	return (mouseCoord.x >= left && mouseCoord.x <= right && mouseCoord.y >= bottom && mouseCoord.y <= top);

}

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
		wBall->v = direction * 3.0f;
		isHitting = false;
		stoppedHitting = true;

		//std::cout << "viteza imprimata bilei: " << wBall->v.x << " " << wBall->v.y << std::endl;
		return true;
	}

	return false;
}

