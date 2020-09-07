#ifndef GLOBAL_H
#define GLOBAL_H

#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

typedef unsigned short DepthValueType;
const float c_depthScaleFactor = 1.0;	// Why would I want to scale the original depth???
const float c_visualDepthScaleFactor = 5.0;
const int c_frameSkipMax = 15;
const string c_saveFolder = "./saves";

#endif