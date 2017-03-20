#pragma once
#include <string>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

class Pig
{
public:
	Pig();
	~Pig();

	Rect getPigRect();
	void setPigRect(Rect rectPig);

	int getXPos();
	void setXPos(int x);

	int getYPos();
	void setYPos(int y);

	Scalar getHSVmin();
	Scalar getHSVmax();

	void setHSVmin(Scalar min);
	void setHSVmax(Scalar max);

	int getPigNumber();
	void setPigNumber(int pigNo);

private:
	int xPos, yPos, pigNumber;
	Rect pigRect;
	string type;

	Scalar HSVmin, HSVmax;

};

