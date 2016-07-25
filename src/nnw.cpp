#include <pqxx/pqxx>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/core.hpp>
#include <fstream>


using namespace cv;
using namespace cv::ml;
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
    uchar color = m.at<uchar>(point);
    if (color == 0) {
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
    return Point(width, height - (idx - width/2));
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
    uchar color = m.at<uchar>(point);
    if (color == 0) {
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

  int numImages = txn.exec("SELECT count(*) FROM images")[0][0].as<int>(0);
  Mat training_inputs = Mat(numImages, 7, CV_32F);
  Mat training_outputs = Mat(numImages, 6, CV_32F);
  cout << "Reading data..." << endl;
  Ptr<ml::ANN_MLP> brain;
  // check if the data already exists
  ifstream f("ml.out");
  if (f.good())
  {
    brain = Algorithm::load<ANN_MLP>("ml.out");
  }
  else
  {
    pqxx::result res = txn.exec("SELECT image, a1, a2, b1, b2, lpwm, rpwm FROM images");
    Mat img;
    for (int i = 0; i < numImages; ++i)
    {
      strToMat(res[i][0].as<std::string>(), img);
      cvtColor(img, img, CV_BGR2GRAY);
      threshold(img, img, 128, 255, 0);

      Point pix = firstBlackLeft(img);
      Point pix2 = firstBlackRight(img);
      Point highest = highestLine(img);
      int wperc = whitePercent(img);

      int* ptr = training_inputs.ptr<int>(i);
      ptr[0] = pix.x;
      ptr[1] = pix.y;
      ptr[2] = pix2.x;
      ptr[3] = pix2.y;
      ptr[4] = highest.x;
      ptr[5] = highest.y;
      ptr[6] = wperc;

      ptr = training_outputs.ptr<int>(i);
      for (int j = 0; j < 6; ++j) {
        ptr[j] = res[i][j+1].as<int>();
      }
    }

    TermCriteria criteria = TermCriteria(TermCriteria::MAX_ITER + TermCriteria::EPS, 1000, 0.0001f);

    brain = ANN_MLP::create();
    Mat layers = (Mat_<int>(1,3) << 7, 6, 6);

    brain->setLayerSizes(layers);
    brain->setTermCriteria(criteria);

    Ptr<TrainData> data = TrainData::create(training_inputs, SampleTypes::ROW_SAMPLE, training_outputs);
    //data->setTrainTestSplitRatio(0.75);

    brain->train(data);
    //Mat output;
    //float err = brain->calcError(data, true, output);
    //cout << "Err: " << err << endl;
    brain->save("ml.out");
  }

  for (int i = 0; i < numImages; ++i)
  {
    std::stringstream ss;
    ss << "SELECT image, a1, a2, b1, b2, lpwm, rpwm FROM images WHERE id=" << i+1;
    pqxx::result res = txn.exec(ss.str());

    Mat orig_img, img;
    strToMat(res[0][0].as<string>(), orig_img);
    cvtColor(orig_img, img, CV_BGR2GRAY);
    threshold(img, img, 128, 255, 0);

    Point pix = firstBlackLeft(img);
    Point pix2 = firstBlackRight(img);
    Point highest = highestLine(img);
    int wperc = whitePercent(img);

    Mat inData = (Mat_<double>(1,7, CV_32F) << pix.x, pix.y, pix2.x, pix2.y, highest.x, highest.y, wperc);

    cout << "Exp: ["
      << res[0][1].as<int>() << ", "
      << res[0][2].as<int>() << ", "
      << res[0][3].as<int>() << ", "
      << res[0][4].as<int>() << ", "
      << res[0][5].as<int>() << ", "
      << res[0][6].as<int>() << "]" << endl;

    Mat out;
    brain->predict(inData, out);
    double* row = out.ptr<double>(0);
    cout << "Actual: ["
        << row[0] << ", "
        << row[1] << ", "
        << row[2] << ", "
        << row[3] << ", "
        << row[4] << ", "
        << row[5] << "]" << endl << endl;;

    Mat disp_img;
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
}
