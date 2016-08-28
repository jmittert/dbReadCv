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
#include "features.hpp"


using namespace cv;
using namespace cv::ml;
using namespace std;

struct addrinfo *servinfo;
int s;
int new_fd;

void cleanup(int signal)
{
  freeaddrinfo(servinfo);
  close(s);
  shutdown(new_fd, 2);
  close(new_fd);
  exit(0);
}

int main(int argc, char **argv)
{
  signal(SIGINT, cleanup);
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

  while (1) {
    // Try to get a connection
    int status;
    struct addrinfo hints;
    struct sockaddr_storage their_addr;
    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, "2718", &hints, &servinfo)) != 0) {
      cerr << "gettaddrinfo error: " << status << endl;
      exit(1);
    }

    s = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    if (s == -1) {
      cerr << "socket error: " << errno << endl;
      exit(1);
    }


    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (bind(s, servinfo->ai_addr, servinfo->ai_addrlen) ==  -1) {
      cerr << "bind error: " << errno << endl;
      exit(1);
    }

    if (listen(s, 5) == -1) {
      cerr << "listen error: " << errno << endl;
      exit(1);
    }

    socklen_t addr_size = sizeof their_addr;
    new_fd = accept(s, (struct sockaddr *)&their_addr, &addr_size);
    if (new_fd == -1) {
      cerr << "accept error: " << errno << endl;
      exit(1);
    }

    // Close the listening port
    close(s);

    int resp = 1;
    cout << "gotcha!" << endl;
    namedWindow("Image" , cv::WINDOW_AUTOSIZE);
    do {
      pqxx::result res = txn.exec("SELECT "
          "image, "
          "s1.rpwm, s1.lpwm, "
          "s2.rpwm, s2.lpwm, "
          "s3.rpwm, s3.lpwm, "
          "s4.rpwm, s4.lpwm, "
          "s5.rpwm, s5.lpwm "
        "FROM readImgs "
          "INNER JOIN states s1 ON s1.id = images.state1 "
          "INNER JOIN states s2 ON s2.id = images.state2 "
          "INNER JOIN states s3 ON s3.id = images.state3 "
          "INNER JOIN states s4 ON s4.id = images.state4 "
        "ORDER BY id DESC LIMIT 1");
      Mat img;
      Mat disp_img;
      strToMat(res[0][0].as<std::string>(), img);
      cvtColor(img, disp_img, CV_BGR2GRAY);
      threshold(disp_img, disp_img, 128, 255, 0);
      Point pix = firstBlackLeft(disp_img);
      Point pix2 = firstBlackRight(disp_img);
      Point highest = highestLine(disp_img);
      int wperc = whitePercent(disp_img);
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

      Mat out;
      brain->predict(inputs, out); 
      uint8_t lpwm = out.at<uint8_t>(0,0);
      uint8_t rpwm = out.at<uint8_t>(0,1);

      char pack[6];
      memset(pack, 0, 6);
      pack[0] = 1;
      pack[1] = 0;
      pack[2] = 1;
      pack[3] = 0;
      pack[4] = lpwm;
      pack[5] = rpwm;
      resp = send(new_fd, pack, 6, 0);

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
    } while (resp != 0);
  }
}
