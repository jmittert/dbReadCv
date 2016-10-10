#include <pqxx/pqxx>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/core.hpp>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdint>
#include "lib.hpp"
#include "cvlib.hpp"
#include "features.hpp"
#include "server.hpp"


using namespace cv;
using namespace cv::ml;
using namespace std;

const string STATE_QUERY =
    "SELECT "
          "image, "
          "s1.rpwm, s1.lpwm, "
          "s2.rpwm, s2.lpwm, "
          "s3.rpwm, s3.lpwm, "
          "s4.rpwm, s4.lpwm "
        "FROM readImg "
          "INNER JOIN states s1 ON s1.id = readImg.state1 "
          "INNER JOIN states s2 ON s2.id = readImg.state2 "
          "INNER JOIN states s3 ON s3.id = readImg.state3 "
          "INNER JOIN states s4 ON s4.id = readImg.state4 "
        "ORDER BY readImg.id DESC LIMIT 1";

int main(int argc, char **argv)
{
  pqxx::connection c("dbname=mlimages user=jason");
  pqxx::work txn(c);

  Ptr<ml::ANN_MLP> brain;
  // check if the data already exists
  ifstream f("ml.out");
  if (f.good()) {
    brain = Algorithm::load<ANN_MLP>("ml.out");
  }
  else {
    cerr << "Program not trained yet..." << endl;
    exit(1);
  }

  Server serv = Server(2718);
  if (serv.Listen()) {
      return 1;
  }

  namedWindow("Image" , cv::WINDOW_AUTOSIZE);
  while (1)
  {
      Mat img;
      Mat disp_img;
      pqxx::result res = txn.exec(STATE_QUERY);

      // Get the features from the image
      strToMat(res[0][0].as<std::string>(), img);
      cvtColor(img, disp_img, CV_BGR2GRAY);
      threshold(disp_img, disp_img, 128, 255, 0);
      Point pix = firstBlackLeft(disp_img);
      Point pix2 = firstBlackRight(disp_img);
      Point highest = highestLine(disp_img);
      int wperc = whitePercent(disp_img);

      // Put the features into the matrix
      Mat inputs = Mat(1, 7, CV_32F);
      inputs.at<float>(0,0) = pix.x;
      inputs.at<float>(0,1) = pix.y;
      inputs.at<float>(0,2) = pix2.x;
      inputs.at<float>(0,3) = pix2.y;
      inputs.at<float>(0,4) = highest.x;
      inputs.at<float>(0,5) = highest.y;
      inputs.at<float>(0,6) = wperc;
      for (int j = 3; j <= 10; ++j) {
        inputs.at<float>(0, j+4) = res[0][j].as<float>();
      }

      // Predict!
      Mat out;
      brain->predict(inputs, out); 
      uint8_t lpwm = out.at<uint8_t>(0,0);
      uint8_t rpwm = out.at<uint8_t>(0,1);
      vector<uint8_t> pack = {1, 0, 1, 0, lpwm, rpwm};
      serv.Send(pack);

      // Visualizations
      cvtColor(disp_img, disp_img, CV_GRAY2BGR);
      circle(disp_img, pix, 10, CV_RGB(0,255,0), 5);
      circle(disp_img, pix2, 10, CV_RGB(0,255,0), 5);
      line(disp_img, highest, Point(highest.x, disp_img.rows), CV_RGB(0,255,0), 5);
      Mat both;
      std::vector<Mat> lifted = {img, disp_img};
      hconcat(lifted,both);

      cv::imshow("Image", both);
      cv::waitKey(20);
  }
}
