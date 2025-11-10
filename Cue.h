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
#include "Ball.h"
#define PI 3.141592653



class Cue
{
	//Pointer catre bila alba, ca sa nu mai ii dam pass ca parametru mereu.
	Ball* wBall;
public:
	float length, width; // dimensiunile bounding box-ului
	bool isDragged;
	bool isHitting = false, stoppedHitting = false;;
	glm::vec2 position; // coordonatele centrului
	glm::vec2 direction;
	bool isMouseInsideBox(glm::vec2 mouseCoord);
	Cue(float l_, float w_, float x, float y, Ball* wBall);
	/// <summary>
	/// Luam bila din pozitia de repaus si o aducem (speram) smooth catre bila alba, cadrul de lovire
	/// </summary>
	void BringToBall();

	/// <summary>
	/// O functie ce cred va fi integrata in render function, aici ar fi trigonometria de calcul + desenat tacul in unghi in jurul bilei
	/// </summary>
	void RotateAroundBall();
	/// <summary>
	/// ChargeBar? Sau macar adagam ceva sa amplificam sau nu lovitura (optional)
	/// </summary>
	void ChargeBar();

	/// <summary>
	/// Seteaza isHitting la true se triggeruieste deplasarea tacului
	/// </summary>
	void startHit();

	// pentru fiecare frame, actualizeaza pozitia tacului, pana intalneste bila alba, apoi controlul este predat in IdleFunc
	bool updateHit();

};
