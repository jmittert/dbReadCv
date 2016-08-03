#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>

cv::Point edgeToIndexL(int idx, int width, int height);

// Finds the first black pixel on the left hand side
cv::Point firstBlackLeft(cv::Mat& m);

// Returns the row and column index given the edge index
cv::Point edgeToIndexR(int idx, int width, int height);

// Finds the first black pixel on the right hand side
cv::Point firstBlackRight(cv::Mat& m);

// Highest line returns the height of the highest continous
// white column
cv::Point highestLine(cv::Mat& m);

int whitePercent(cv::Mat &m);
