#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include "features.hpp"

// Returns the row and column index given the edge index
cv::Point edgeToIndexL(int idx, int width, int height) {
  if (idx > width/2 + height) {
    return cv::Point(idx - height - width/2, 0);
  } else if (idx > width/2) {
    return cv::Point(0, height- (idx - width/2));
  } else {
    return cv::Point(width/2 - idx, height);
  }
  std::stringstream ss;
  ss << idx << "Out of range: " << width + height << std::endl;
  throw ss.str();
}

// Finds the first black pixel on the left hand side
cv::Point firstBlackLeft(cv::Mat& m) {
  int width = m.cols;
  int height = m.rows;

  // Perform a linear search to find the first black pixel
  for (int i = 0; i < width + height; i++) {
    // Check if the current pixel is black, if so
    // return it.
    auto point = edgeToIndexL(i, width-1, height-1);
    uchar color = m.at<uchar>(point);
    if (color == 0) {
      return point;
    }
  }
  return cv::Point(width/2, 0);
}

// Returns the row and column index given the edge index
cv::Point edgeToIndexR(int idx, int width, int height) {
  if (idx > width/2 + height) {
    return cv::Point(idx - height + width/2, 0);
  } else if (idx > width/2) {
    return cv::Point(width, height - (idx - width/2));
  } else {
    return cv::Point(width/2 + idx, height);
  }
  std::stringstream ss;
  ss << idx << "Out of range: " << width + height << std::endl;
  throw ss.str();
}

// Finds the first black pixel on the right hand side
cv::Point firstBlackRight(cv::Mat& m) {
  int width = m.cols;
  int height = m.rows;

  // Perform a linear search to find the first black pixel
  for (int i = 0; i < width + height; i++) {
    // Check if the current pixel is black, if so
    // return it.
    auto point = edgeToIndexR(i, width-1, height-1);
    uchar color = m.at<uchar>(point);
    if (color == 0) {
      return point;
    }
  }
  return cv::Point(width/2, 0);
}

// Highest line returns the height of the highest continous
// white column
cv::Point highestLine(cv::Mat& m) {
  cv::Point highest = cv::Point(0, m.rows);

  for (int x = 0; x < m.cols; x++) {
    for (int y = m.rows-1; y >= 0; y--) {
      uchar color = m.at<uchar>(cv::Point(x, y));
      if (color == 255) {
        if (y < highest.y) {
          highest = cv::Point(x, y);
        }
      } else {
        break;
      }
    }
  }
  return highest;
}

int whitePercent(cv::Mat &m) {
  int black = 0;
  int white = 0;
  for (auto it = m.begin<uchar>(); it != m.end<uchar>(); it++) {
    if (*it == 0) {
      black++;
    } else {
      white++;
    }
  }
  return white*100/(black+white);
}
