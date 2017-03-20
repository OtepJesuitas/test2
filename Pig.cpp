#include "Pig.h"


Pig::Pig()

{
}

Pig::~Pig()
{
}

Rect Pig::getPigRect(){
	return Pig::pigRect;
}

void Pig::setPigRect(Rect rectPig){
	Pig::pigRect = rectPig;
}

int Pig::getXPos(){
	return Pig::xPos; 
}

void Pig::setXPos(int x){
	Pig::xPos = x;
}

int Pig::getYPos(){
	return Pig::yPos;
}

void Pig::setYPos(int y){
	Pig::yPos = y;
}

Scalar Pig::getHSVmin(){
	return Pig::HSVmin;
}

Scalar Pig::getHSVmax(){
	return Pig::HSVmax;
}

void Pig::setHSVmin(Scalar min){
	Pig::HSVmin = min;
}

void Pig::setHSVmax(Scalar max){
	Pig::HSVmax = max;
}

int Pig::getPigNumber(){
	return Pig::pigNumber;
}
void Pig::setPigNumber(int pigNo){
	Pig::pigNumber = pigNo;
}