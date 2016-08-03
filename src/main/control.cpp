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
  if (f.good())
  {
    brain = Algorithm::load<ANN_MLP>("ml.out");
  }
  else
  {
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

    if (s == -1)
    {
      cerr << "socket error: " << errno << endl;
      exit(1);
    }


    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (bind(s, servinfo->ai_addr, servinfo->ai_addrlen) ==  -1)
    {
      cerr << "bind error: " << errno << endl;
      exit(1);
    }

    if (listen(s, 5) == -1)
    {
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
    char* buff = new char[1024];
    int rec_size = recv(new_fd, buff, 1024, 0);
    send(new_fd, buff, rec_size, 0);
    delete[] buff;
    shutdown(new_fd, 2);
    close(new_fd);
  }

  /*
  // Make some predictions
  std::stringstream ss;
	pqxx::result res = txn.exec("SELECT image FROM images");
			Mat img;
      strToMat(res[0][0].as<std::string>(), img);
      cvtColor(img, img, CV_BGR2GRAY);
      threshold(img, img, 128, 255, 0);

      Point pix = firstBlackLeft(img);
      Point pix2 = firstBlackRight(img);
      Point highest = highestLine(img);
      int wperc = whitePercent(img);
      inputs.at<float>(i,0) = pix.x;
      inputs.at<float>(i,1) = pix.y;
      inputs.at<float>(i,2) = pix2.x;
      inputs.at<float>(i,3) = pix2.y;
      inputs.at<float>(i,4) = highest.x;
      inputs.at<float>(i,5) = highest.y;
      inputs.at<float>(i,6) = wperc;
  Mat out;
  brain->predict(inputs, out); 
  for (int i = 0; i < numImages; ++i) {
    cout << "Exp: [" << res[i][1] << ", " << res[i][2] << "]" << endl;

		Mat outData = out.row(i);
    cout << "Actual: " << outData << endl;

    Mat orig_img, img, disp_img;
    strToMat(res[i][0].as<string>(), orig_img);
    cvtColor(orig_img, img, CV_BGR2GRAY);
    threshold(img, img, 128, 255, 0);

    Point pix = firstBlackLeft(img);
    Point pix2 = firstBlackRight(img);
    Point highest = highestLine(img);
    cvtColor(img, disp_img, CV_GRAY2BGR);
    circle(disp_img, pix, 10, CV_RGB(0,255,0), 5);
    circle(disp_img, pix2, 10, CV_RGB(0,255,0), 5);
    line(disp_img, highest, Point(highest.x, disp_img.rows), CV_RGB(0,255,0), 5);

    Mat both;
    std::vector<Mat> lifted = {orig_img, disp_img};
    hconcat(lifted,both);

    namedWindow("Image" , cv::WINDOW_AUTOSIZE);
    cv::imshow("Image", both);
    cv::waitKey(0);
  }
  */
}
