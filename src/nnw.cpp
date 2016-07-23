#include <pqxx/pqxx>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <fstream>


int asciiToByte(char x) {
  if (x >= '0' && x <= '9') {
    return x - '0';
  } else if (x >= 'A' && x <= 'F') {
    return x + 10 - 'A';
  } else if (x >= 'a' && x <= 'f') {
    return x + 10 - 'a' ;
  }
  return 0;
}
char hexToByte(char x, char y) {
  return (asciiToByte(x) << 4) | asciiToByte(y);
}
int main(int argc, char **argv)
{
  using namespace cv;

  pqxx::connection c("dbname=mlimages user=jason");
  pqxx::work txn(c);

  for (int i = 1; i < 200; ++i)
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

    std::ofstream f;
    f.open("/tmp/db.jpg");
    std::string pic = res[0][0].as<std::string>();

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
    Mat image = imread( "/tmp/db.jpg", 1 );
    Mat grey_image;
    cvtColor(image, grey_image, CV_BGR2GRAY);
    Mat timage;

    threshold(grey_image, timage, 128, 255, 0);

    Mat color_timage;
    cvtColor(timage, color_timage, CV_GRAY2BGR);

    Mat both;
    std::vector<Mat> lifted = {image, color_timage};
    hconcat(lifted, both);
    cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE );
    cv::imshow("Display Image", both);
    cv::waitKey(0);
  }
}
