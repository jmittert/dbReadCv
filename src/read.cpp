#include <pqxx/pqxx>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
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
  if ( argc != 2 )
  {
    std::cerr << "usage: " << argv[0] << "<Image_id>" << std::endl;
    return 1;
  }
  pqxx::connection c("dbname=mlimages user=jason");
  pqxx::work txn(c);

  std::stringstream ss;
  ss << "SELECT image FROM images WHERE id=" << argv[1];
  pqxx::result res = txn.exec(ss.str());

  if (res.size() == 0)
  {
    std::cerr << "No result" << std::endl;
    return 1;
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

  cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE );
  cv::imshow("Display Image", image);
  cv::waitKey(0);
}
