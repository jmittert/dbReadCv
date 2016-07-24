#include <pqxx/pqxx>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <fstream>


using namespace cv;
using namespace std;
int asciiToByte(char x) {
  if (x >= '0' && x <= '9') {
    return x - '0';
  } else if (x >= 'A' && x <= 'F') {
    return x + 10 - 'A';
  } else if (x >= 'a' && x <= 'f') {
    return x + 10 - 'a' ;
  }
  std::stringstream ss;
  ss << "Can't encode character: " << x << std::endl;
  throw ss.str();
}

char hexToByte(char x, char y) {
  return (asciiToByte(x) << 4) | asciiToByte(y);
}

// Reads the data into the given Mat
void strToMat(std::string pic, cv::Mat& m) {
    std::ofstream f;
    f.open("/tmp/db.jpg");

    char prev;
    bool haveTwo = false;
    for (auto it=pic.begin(); it !=pic.end(); ++it) {
      if (*it == '\\' || *it == 'x') {
        continue;
      } else {
        if (haveTwo == false) {
          prev = *it;
          haveTwo = true;
          continue;
        }
        haveTwo = false;
        f << hexToByte(prev, *it);
      }
    }
    f.close();
    m = cv::imread( "/tmp/db.jpg", 1 );
}

// Returns the row and column index given the edge index
Point edgeToIndexL(int idx, int width, int height) {
  if (idx > width/2 + height) {
    return Point(idx - height - width/2, 0);
  } else if (idx > width/2) {
    return Point(0, height- (idx - width/2));
  } else {
    return Point(width/2 - idx, height);
  }
  std::stringstream ss;
  ss << idx << "Out of range: " << width + height << std::endl;
  throw ss.str();
}

// Finds the first black pixel on the left hand side
Point firstBlackLeft(Mat& m) {
  int width = m.cols;
  int height = m.rows;

  // Perform a linear search to find the first black pixel
  for (int i = 0; i < width + height; i++) {
    // Check if the current pixel is black, if so
    // return it.
    auto point = edgeToIndexL(i, width-1, height-1);
    Vec3b color = m.at<Vec3b>(point);
    if (color.val[0] == 0) {
      return point;
    }
  }
  return Point(width/2, 0);
}

// Returns the row and column index given the edge index
Point edgeToIndexR(int idx, int width, int height) {
  if (idx > width/2 + height) {
    return Point(idx - height + width/2, 0);
  } else if (idx > width/2) {
    return Point(width, height- (idx - width/2));
  } else {
    return Point(width/2 + idx, height);
  }
  std::stringstream ss;
  ss << idx << "Out of range: " << width + height << std::endl;
  throw ss.str();
}

// Finds the first black pixel on the right hand side
Point firstBlackRight(Mat& m) {
  int width = m.cols;
  int height = m.rows;

  // Perform a linear search to find the first black pixel
  for (int i = 0; i < width + height; i++) {
    // Check if the current pixel is black, if so
    // return it.
    auto point = edgeToIndexR(i, width-1, height-1);
    Vec3b color = m.at<Vec3b>(point);
    if (color.val[0] == 0) {
      return point;
    }
  }
  return Point(width/2, 0);
}

// Highest line returns the height of the highest continous 
// white column
Point highestLine(Mat& m) {
  Point highest = Point(0, m.rows);

  for (int x = 0; x < m.cols; x++) {
    for (int y = m.rows-1; y >= 0; y--) {
      uchar color = m.at<uchar>(Point(x, y));
      if (color == 255) {
        //cout << "Checking " << x << ", " << y << endl;
        if (y < highest.y) {
          highest = Point(x, y);
        }
      } else {
        break;
      }
    }
  }
  return highest;
}

int whitePercent(Mat &m) {
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

int main(int argc, char **argv)
{
  pqxx::connection c("dbname=mlimages user=jason");
  pqxx::work txn(c);

  for (int i = 1; ; ++i)
  {
    std::stringstream ss;
    ss << "SELECT image FROM images WHERE id=" << i;
    pqxx::result res = txn.exec(ss.str());

    if (res.size() == 0)
    {
      std::cerr << "Hit end" << std::endl;
      i = 1;
      continue;
    }

    Mat orig_img;
    strToMat(res[0][0].as<std::string>(), orig_img);

    Mat proc_img;
    cvtColor(orig_img, proc_img, CV_BGR2GRAY);
    threshold(proc_img, proc_img, 128, 255, 0);

    Mat disp_img;
    cvtColor(proc_img, disp_img, CV_GRAY2BGR);

    Point pix = firstBlackLeft(disp_img);
    circle(disp_img, pix, 10, CV_RGB(0,255,0), 5);

    Point pix2 = firstBlackRight(disp_img);
    circle(disp_img, pix2, 10, CV_RGB(0,255,0), 5);

    Point highest = highestLine(proc_img);
    line(disp_img, highest, Point(highest.x, disp_img.rows), CV_RGB(0,0,255), 5);

    Mat both;
    std::vector<Mat> lifted = {orig_img, disp_img};
    hconcat(lifted, both);

    cout << "White %: " << whitePercent(proc_img) << endl;

    cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE );
    cv::imshow("Display Image", both);
    cv::waitKey(0);
  }
}
