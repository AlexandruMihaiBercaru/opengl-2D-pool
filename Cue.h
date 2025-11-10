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
#include <iostream>
#define PI 3.141592653



class Cue
{
	//Pointer catre bila alba, ca sa nu mai ii dam pass ca parametru mereu.
	Ball* wBall;
public:
	float length, width; // dimensiunile bounding box-ului
	float angle;
	bool isDragged;

	//True when all no balls move
	//False after hit
	bool canRotate; 
	glm::vec2 position; // coordonatele centrului
	bool isMouseInsideBox(glm::vec2 mouseCoord);
	Cue(float l_, float w_, float x, float y);

	void SetBall(Ball*);

	/// <summary>
	/// Luam bila din pozitia de repaus si o aducem (speram) smooth catre bila alba, cadrul de lovire
	/// La unghi de ...idk..180 grade sa zicem trigonometric
	/// De-a lungul unei translatii pe orizontala si verticala
	/// </summary>
	void BringToBall();


	/// <summary>
	/// ChargeBar? Sau macar adagam ceva sa amplificam sau nu lovitura (optional)
	/// </summary>
	void ChargeBar();

	/// <summary>
	/// 
	/// </summary>
	void Hit();


	/// <summary>
	/// Gets angle between a given coordonate and the white ball
	/// </summary>
	/// <param name="pos">Position X,Y</param>
	/// <returns>Angle</returns>
	float GetBallAngle(glm::vec2 pos);



	/// <summary>
	/// Given the angle and distance desired, gives back the position of where the cue should be
	/// in relation to the white ball.
	/// </summary>
	/// <param name="angle">Angle of drawing</param>
	/// <param name="dist">Distance between tip of cue and white ball</param>
	/// <returns>Position X,Y of where the center of the cue should be </returns>
	glm::vec2 PosToAngle(float angle,float dist);
};
