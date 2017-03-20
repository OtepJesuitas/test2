#include <sstream>
#include <iostream>

#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "Pig.h"

//initial min and max HSV filter values.
//these will be changed using trackbars
/*int H_MIN = 0;
int H_MAX = 255;
int S_MIN = 0;
int S_MAX = 255;
int V_MIN = 0;
int V_MAX = 255;*/

int H_MIN = 0;
int H_MAX = 173;
int S_MIN = 0;
int S_MAX = 59;
int V_MIN = 140;
int V_MAX = 255;

int H_MIN1 = 0;
int H_MAX1 = 187;
int S_MIN1 = 0;
int S_MAX1 = 58;
int V_MIN1 = 90;//102;
int V_MAX1 = 255;

int H_MIN2 = 0;
int H_MAX2 = 177;
int S_MIN2 = 0;
int S_MAX2 = 46;
int V_MIN2 = 96;
int V_MAX2 = 249;

//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 110 * 110;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;

//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "trackbars";

bool calibrationMode;

bool mouseIsDragging;//used for showing a rectangle on screen as user clicks and drags mouse
bool mouseMove;
bool rectangleSelected;
cv::Point initialClickPoint, currentMousePoint; //keep track of initial point clicked and current position of mouse
cv::Rect rectangleROI; //this is the ROI that the user has selected
vector<int> H_ROI, S_ROI, V_ROI;// HSV values from the click/drag ROI region stored in separate vectors so that we can sort them easily

Pig thePig;
vector<Pig> pigs;

void on_trackbar(int, void*){
	//This function gets called whenever a
	// trackbar position is changed
}

string intToString(int number){
	std::stringstream ss;
	ss << number;
	return ss.str();
}

void createTrackbars(){
	//create window for trackbars
	namedWindow(trackbarWindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf_s(TrackbarName, "H_MIN", H_MIN);
	sprintf_s(TrackbarName, "H_MAX", H_MAX);
	sprintf_s(TrackbarName, "S_MIN", S_MIN);
	sprintf_s(TrackbarName, "S_MAX", S_MAX);
	sprintf_s(TrackbarName, "V_MIN", V_MIN);
	sprintf_s(TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	createTrackbar("H_MIN", "trackbars", &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", "trackbars", &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", "trackbars", &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", "trackbars", &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", "trackbars", &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", "trackbars", &V_MAX, V_MAX, on_trackbar);
}

void clickAndDrag_Rectangle(int event, int x, int y, int flags, void* param){
	//only if calibration mode is true will we use the mouse to change HSV values
	
	if (calibrationMode == true){
		//get handle to video feed passed in as "param" and cast as Mat pointer
		Mat* videoFeed = (Mat*)param;

		if (event == CV_EVENT_LBUTTONDOWN && mouseIsDragging == false)
		{
			//keep track of initial point clicked
			initialClickPoint = cv::Point(x, y);
			//user has begun dragging the mouse
			mouseIsDragging = true;
		}
		//user is dragging the mouse
		if (event == CV_EVENT_MOUSEMOVE && mouseIsDragging == true)
		{	
			//keep track of current mouse point
			currentMousePoint = cv::Point(x, y);
			//user has moved the mouse while clicking and dragging
			mouseMove = true;
		}
		//user has released left button
		if (event == CV_EVENT_LBUTTONUP && mouseIsDragging == true)
		{
			//set rectangle ROI to the rectangle that the user has selected
			rectangleROI = Rect(initialClickPoint, currentMousePoint);

			//reset boolean variables
			mouseIsDragging = false;
			mouseMove = false;
			rectangleSelected = true;
		}

		if (event == CV_EVENT_RBUTTONDOWN){
			//user has clicked right mouse button
			//Reset HSV Values
			H_MIN = 0;
			S_MIN = 0;
			V_MIN = 0;
			H_MAX = 255;
			S_MAX = 255;
			V_MAX = 255;
		}
		if (event == CV_EVENT_MBUTTONDOWN){

			//user has clicked middle mouse button
			//enter code here if needed.
		}
	}

}

/*void recordHSV_Values(cv::Mat frame, cv::Mat hsv_frame){

	//save HSV values for ROI that user selected to a vector
	if (mouseMove == false && rectangleSelected == true){

		//clear previous vector values
		if (H_ROI.size()>0) H_ROI.clear();
		if (S_ROI.size()>0) S_ROI.clear();
		if (V_ROI.size()>0)V_ROI.clear();
		//if the rectangle has no width or height (user has only dragged a line) then we don't try to iterate over the width or height
		if (rectangleROI.width<1 || rectangleROI.height<1) cout << "Please drag a rectangle, not a line" << endl;
		else{
			for (int i = rectangleROI.x; i<rectangleROI.x + rectangleROI.width; i++){
				//iterate through both x and y direction and save HSV values at each and every point
				for (int j = rectangleROI.y; j<rectangleROI.y + rectangleROI.height; j++){
					//save HSV value at this point
					H_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[0]);
					S_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[1]);
					V_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[2]);
				}
			}
		}
		//reset rectangleSelected so user can select another region if necessary
		rectangleSelected = false;
		//set min and max HSV values from min and max elements of each array

		if (H_ROI.size()>0){
			//NOTE: min_element and max_element return iterators so we must dereference them with "*"
			H_MIN = *std::min_element(H_ROI.begin(), H_ROI.end());
			H_MAX = *std::max_element(H_ROI.begin(), H_ROI.end());
			cout << "MIN 'H' VALUE: " << H_MIN << endl;
			cout << "MAX 'H' VALUE: " << H_MAX << endl;
		}
		if (S_ROI.size()>0){
			S_MIN = *std::min_element(S_ROI.begin(), S_ROI.end());
			S_MAX = *std::max_element(S_ROI.begin(), S_ROI.end());
			cout << "MIN 'S' VALUE: " << S_MIN << endl;
			cout << "MAX 'S' VALUE: " << S_MAX << endl;
		}
		if (V_ROI.size()>0){
			V_MIN = *std::min_element(V_ROI.begin(), V_ROI.end());
			V_MAX = *std::max_element(V_ROI.begin(), V_ROI.end());
			cout << "MIN 'V' VALUE: " << V_MIN << endl;
			cout << "MAX 'V' VALUE: " << V_MAX << endl;
		}

	}

	if (mouseMove == true){
		//if the mouse is held down, we will draw the click and dragged rectangle to the screen
		rectangle(frame, initialClickPoint, cv::Point(currentMousePoint.x, currentMousePoint.y), cv::Scalar(0, 255, 0), 1, 8, 0);
	}
}*/

void morphOps(Mat &thresh){
	Mat erodeElement = getStructuringElement(MORPH_DILATE, Size(5, 5));
	Mat dilateElement = getStructuringElement(MORPH_ELLIPSE, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);

	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);
}

void trackFilteredObject(Mat threshold, Mat HSV, Mat &video, string streamAdd){

	int pigNumber = 0;
	//vector<Pig> pigs;

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	vector<vector<Point> > contours_poly(contours.size());
	vector<Rect> boundRect(contours.size());

	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects<MAX_NUM_OBJECTS){
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {
				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area>MIN_OBJECT_AREA){
					//cout << area << endl;

					thePig.setXPos(moment.m10 / area);
					thePig.setYPos(moment.m01 / area);
					thePig.setPigNumber(++pigNumber);
					//cout << streamAdd << " " << "PIG NUMBER = " << thePig.getPigNumber() << endl;
					int a = thePig.getPigNumber();

					approxPolyDP(Mat(contours[index]), contours_poly[index], 3, true);
					boundRect[index] = boundingRect(Mat(contours_poly[index]));
					//drawContours(video, contours_poly, (int)index, Scalar(0, 255, 0));
					rectangle(video, boundRect[index].tl(), boundRect[index].br(), Scalar(0, 255, 0));
					Rect pigR = Rect(boundRect[index].tl(), boundRect[index].br());
					thePig.setPigRect(pigR);

					pigs.push_back(thePig);
					putText(video, intToString(pigNumber), cv::Point(thePig.getXPos(), thePig.getYPos()), 1, 1, Scalar(0, 255, 0));
					//putText(video, intToString(thePig.getXPos()) + " , " + intToString(thePig.getYPos()), cv::Point(thePig.getXPos(), thePig.getYPos()), 1, 1, Scalar(0, 255, 0));

					objectFound = true;

					//if (streamAdd == "9_2015-11-27_15-31-13.mp4"){
					if (streamAdd == "test0.mp4"){
						int ftNum = 0;
						
						Rect food_tray = Rect(cv::Point(330, 20), cv::Point(390, 50));
						Rect food_tray1 = Rect(cv::Point(170, 20), cv::Point(220, 50));
						rectangle(video, food_tray, Scalar(0, 0, 255));
						rectangle(video, food_tray1, Scalar(0, 0, 255));

						vector<Rect> food_trays;
						food_trays.push_back(food_tray);
						food_trays.push_back(food_tray1);

						Rect meal = pigR & food_trays[pigNumber-1];
						//cout << "meal = " << meal << endl;
						//bool isEating = false;

						if (meal.area() == food_trays[pigNumber-1].area()){
							//isEating == true;
							//count seconds
							cout << "Pig " << pigNumber << " is eating" << endl;
						}
						else cout << "Pig " << pigNumber << " is not eating" << endl;
						
						//isEating == false;
					}
				}
				else objectFound = false;
			}
			//let user know you found an object
			//if (objectFound == true){
				
			//}
		}
		//else putText(video, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}

int main(int argc, char* argv[])
{	
	//if we would like to calibrate our filter values, set to true.
	calibrationMode = false; // true;

	//Matrix to store each frame of the webcam feed
	//Mat cameraFeed;
	Mat threshold, threshold1, threshold2;
	Mat HSV, HSV1, HSV2;

	vector<VideoCapture> vid;
	//string streamAddress = "9_2015-11-27_15-31-13.mp4";
	string streamAddress = "test0.mp4";
	string streamAddress1 = "test4.mp4";

	string videoStreamAddress = { "testFolder/" + streamAddress };
	string videoStreamAddress1 = { "testFolder/" + streamAddress1 };
	string videoStreamAddress2 = { "testFolder/test3.mp4" };
	Mat stream, stream1, stream2;
	Mat stream_hist, stream_hist1, stream_hist2;
	Mat stream_track, stream_track1, stream_track2;
	vector<Mat> channels, channels1, channels2;
	VideoCapture vcap, vcap1, vcap2;
	vcap.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	vcap.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	vcap1.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	vcap1.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	//vcap2.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	//vcap2.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

	int width = FRAME_WIDTH;
	int height = FRAME_HEIGHT;
	
	//std::cout << "MSEC " << vcap.get(CV_CAP_PROP_POS_MSEC);

	if (calibrationMode){
		//create slider bars for HSV filtering
		createTrackbars();
	}

	if (!vcap.open(videoStreamAddress)){
		//std::cout << "Error opening video stream from \"" << videoStreamAddress << "\" is not found.";
		return -1;
	}

	//namedWindow(windowName);
	//set mouse callback function to be active on "Webcam Feed" window
	//we pass the handle to our "frame" matrix so that we can draw a rectangle to it
	//as the user clicks and drags the mouse
	//cv::setMouseCallback(windowName, clickAndDrag_Rectangle, &stream);
	//initiate mouse move and drag to false 
	//mouseIsDragging = false;
	//mouseMove = false;
	//rectangleSelected = false;

	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	while (1){
		//store image to matrix
		vcap.read(stream);

		//test
		//change the color image from BGR to YCrCb format
		cv::cvtColor(stream, stream_hist, CV_BGR2YCrCb);
		//split the image into channels
		split(stream_hist, channels);
		//equalize histogram on the 1st channel (Y)
		equalizeHist(channels[0], channels[0]);
		//merge 3 channels including the modified 1st channel into one image
		merge(channels, stream_hist);
		//change the color image from YCrCb to BGR format (to display image properly)
		cv::cvtColor(stream_hist, stream_hist, CV_YCrCb2BGR);
		
		//test.mp4 H_MAX = 173, S_MAX = 59, V_MIN = 140
		stream_track = stream_hist(cv::Rect(30, 0, 530, stream_hist.rows));
		//stream_track = stream_hist(cv::Rect(0, 0, stream_hist.cols, stream_hist.rows));
		
		//test
		//convert frame from BGR to HSV colorspace
		cv::cvtColor(stream_track, HSV, COLOR_BGR2HSV);
		//recordHSV_Values(stream, HSV);
		cv::inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
		morphOps(threshold);
		trackFilteredObject(threshold, HSV, stream_track, streamAddress);

		//test4
		/*cvtColor(stream1, stream_hist1, CV_BGR2YCrCb);
		split(stream_hist1, channels1);
		equalizeHist(channels1[0], channels1[0]);
		merge(channels1, stream_hist1);
		cvtColor(stream_hist1, stream_hist1, CV_YCrCb2BGR);*/

		//test3.mp4 H_MAX = 185, S_MAX = 63, V_MIN = 182
		//stream_track = stream_hist(cv::Rect(200, 0, 230, stream_hist.rows));
		//stream_track.copyTo(stream_hist(cv::Rect(200, 0, 230, stream_hist.rows)));
		//stream_track1 = stream_hist(cv::Rect(420, 0, 220, stream_hist.rows));
		//stream_track1.copyTo(stream_hist(cv::Rect(420, 0, 220, stream_hist.rows)));

		//test4.mp4 H_MAX = 187, S_MAX = 58, V_MIN = 102
		//stream_track1 = stream_hist1(cv::Rect(0, 120, 120, 360));

		//test5.mp4 H_MAX = 189, S_MAX = 89, V_MIN = 117
		//stream_track = stream_hist(cv::Rect(0, 0, 320, stream_hist.rows));
		
		/*vcap1.read(stream1);
		//convert frame from BGR to HSV colorspace
		cvtColor(stream1, HSV1, COLOR_BGR2HSV);

		vcap2.read(stream2);
		//convert frame from BGR to HSV colorspace
		cvtColor(stream2, HSV2, COLOR_BGR2HSV);*/

		//test4
		/*cvtColor(stream_track1, HSV1, COLOR_BGR2HSV);
		inRange(HSV1, Scalar(H_MIN1, S_MIN1, V_MIN1), Scalar(H_MAX1, S_MAX1, V_MAX1), threshold1);
		morphOps(threshold1);
		trackFilteredObject(threshold1, HSV1, stream_track1, streamAddress1);*/

		//stream_track.copyTo(stream_hist(cv::Rect(200, 0, 230, stream_hist.rows)));

		//cvtColor(stream_track1, HSV1, COLOR_BGR2HSV);
		//inRange(HSV1, Scalar(H_MIN1, S_MIN1, V_MIN1), Scalar(H_MAX1, S_MAX1, V_MAX1), threshold1);
		//morphOps(threshold1);
		//trackFilteredObject(threshold1, HSV1, stream_track1, videoStreamAddress1);

		//stream_track1.copyTo(stream_hist(cv::Rect(420, 0, 220, stream_hist.rows)));

		//if (calibrationMode == true){
			//if in calibration mode, we track objects based on the HSV slider values.
			//cvtColor(stream, HSV, COLOR_BGR2HSV);
			//imshow("HSV Image", threshold);
			//imshow("HSV Image1", threshold1);
		//}
		/*else{

			destroyWindow(windowName1);
			destroyWindow(windowName2);
			destroyWindow(trackbarWindowName);
		}*/
		/*else{
			Pig thePig;

			thePig.setHSVmin(Scalar(0, 0, 0));
			thePig.setHSVmax(Scalar(255, 255, 255));

			cvtColor(stream, HSV, COLOR_BGR2HSV);
			inRange(HSV, thePig.getHSVmin(), thePig.getHSVmax(), threshold);
			morphOps(threshold);
			trackFilteredObject(threshold, HSV, stream);
		}*/

		//show frames 
		//imshow(windowName2,threshold);

		cv::imshow("Histogram", stream_hist);
		//imshow("Histogram1", stream_hist1);
		//imshow("Original Image", stream);
		/*imshow(windowName, stream1);
		imshow("Stream 2", stream2);*/

		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		cv::waitKey(30);
	}
	return 0;
}