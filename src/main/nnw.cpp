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


using namespace cv;
using namespace cv::ml;
using namespace std;

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
  Mat training_outputs = Mat(numImages, 2, CV_32F);
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
    pqxx::result res = txn.exec("SELECT image, lpwm, rpwm FROM images");
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
      training_inputs.at<float>(i,0) = pix.x;
      training_inputs.at<float>(i,1) = pix.y;
      training_inputs.at<float>(i,2) = pix2.x;
      training_inputs.at<float>(i,3) = pix2.y;
      training_inputs.at<float>(i,4) = highest.x;
      training_inputs.at<float>(i,5) = highest.y;
      training_inputs.at<float>(i,6) = wperc;

      training_outputs.at<float>(i,0) = res[i][1].as<float>();
      training_outputs.at<float>(i,1) = res[i][2].as<float>();
    }

    brain = ANN_MLP::create();
    Mat layers = (Mat_<int>(1,3) << 7, 7, 2);
    
    // You have to set the activationFunction AFTER the Layer sizes else
    // you get NaN in the outputs
    // WTF: http://stackoverflow.com/questions/36871277/opencv-3-1-ann-predict-returns-nan
    brain->setLayerSizes(layers);
    brain->setActivationFunction(cv::ml::ANN_MLP::SIGMOID_SYM);

    Ptr<TrainData> data = TrainData::create(training_inputs, SampleTypes::ROW_SAMPLE, training_outputs);
    data->setTrainTestSplitRatio(0.75);
    brain->train(data);

    Mat output;
    float err = brain->calcError(data, true, output);
    cout << "Err: " << err << endl;
    brain->save("ml.out");
  }

  // Make some predictions
  std::stringstream ss;
	pqxx::result res = txn.exec("SELECT image, lpwm, rpwm FROM images");
  Mat inputs = Mat(numImages, 7, CV_32F);
  for (int i = 0; i < numImages; ++i)
  {
			Mat img;
      strToMat(res[i][0].as<std::string>(), img);
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
  }
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
}
