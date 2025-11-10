#include "Cue.h"



Cue::Cue(float l_, float w_, float x, float y, Ball* wBall) : length(l_), width(w_), position(x, y), isDragged(false), wBall(wBall) {};


bool Cue::isMouseInsideBox(glm::vec2 mouseCoord) {
	float left = position.x - length / 2.0f;
	float right = position.x + length / 2.0f;
	float top = position.y + width / 2.0f;
	float bottom = position.y - width / 2.0f;
	return (mouseCoord.x >= left && mouseCoord.x <= right && mouseCoord.y >= bottom && mouseCoord.y <= top);

}