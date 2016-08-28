#include <pqxx/pqxx>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/core.hpp>
#include <fstream>
#include "lib.hpp"
#include "features.hpp"


using namespace cv;
using namespace cv::ml;
using namespace std;

const int NUM_INPUTS = 7;
const int NUM_OUTPUTS = 2;
const string statement=
      "SELECT " 
          "image, "
          "s1.rpwm, s1.lpwm, "
          "s2.rpwm, s2.lpwm, "
          "s3.rpwm, s3.lpwm, "
          "s4.rpwm, s4.lpwm, "
          "s5.rpwm, s5.lpwm "
        "FROM images "
          "INNER JOIN states s1 ON s1.id = images.state1 "
          "INNER JOIN states s2 ON s2.id = images.state2 "
          "INNER JOIN states s3 ON s3.id = images.state3 "
          "INNER JOIN states s4 ON s4.id = images.state4 "
          "INNER JOIN states s5 ON s5.id = images.state5 "
        "ORDER BY images.id DESC LIMIT 1";

int main(int argc, char **argv)
{
  pqxx::connection c("dbname=mlimages user=jason");
  pqxx::work txn(c);

  while(1) {
    pqxx::result res = txn.exec(statement);
    Mat orig_img, disp_img;
    strToMat(res[0][0].as<string>(), orig_img);
    cvtColor(orig_img, disp_img, CV_BGR2GRAY);
    threshold(disp_img, disp_img, 128, 255, 0);

    Point pix = firstBlackLeft(disp_img);
    Point pix2 = firstBlackRight(disp_img);
    Point highest = highestLine(disp_img);
    int whiteBal = whitePercent(disp_img);

    cout << "PixL: (" << pix.x << ", " << pix.y << ")" << endl;
    cout << "PixR: (" << pix2.x << ", " << pix2.y << ")" << endl;
    cout << "Highest: (" << highest.x << ", " << highest.y << ")" << endl;
    cout << "White Balance: " << whiteBal << endl;
    cout << "Pwm:";
    cout << "(" << res[0][1].as<int>() << ", "<<  res[0][2].as<int>() << ")" << endl;
    cout << "Prev pwm:";
    cout << "(" << res[0][3].as<int>() << ", "<<  res[0][4].as<int>() << ") ";
    cout << "(" << res[0][5].as<int>() << ", "<<  res[0][6].as<int>() << ") ";
    cout << "(" << res[0][7].as<int>() << ", "<<  res[0][8].as<int>() << ") ";
    cout << "(" << res[0][9].as<int>() << ", "<<  res[0][10].as<int>() << ")" << endl;

    cvtColor(disp_img, disp_img, CV_GRAY2BGR);
    circle(disp_img, pix, 10, CV_RGB(0,255,0), 5);
    circle(disp_img, pix2, 10, CV_RGB(0,255,0), 5);
    line(disp_img, highest, Point(highest.x, disp_img.rows), CV_RGB(0,255,0), 5);

    Mat both;
    std::vector<Mat> lifted = {orig_img, disp_img};
    hconcat(lifted,both);

    namedWindow("Image" , cv::WINDOW_AUTOSIZE);
    cv::imshow("Image", both);
    cv::waitKey(20);
  }
}
