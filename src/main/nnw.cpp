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
#include "cvlib.hpp"
#include "features.hpp"


using namespace cv;
using namespace cv::ml;
using namespace std;

const int NUM_INPUTS = 15;
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
          "INNER JOIN states s5 ON s5.id = images.state5;";

int main(int argc, char **argv)
{
  pqxx::connection c("dbname=mlimages user=jason");
  pqxx::work txn(c);
  const int NUM_IMAGES = txn.exec("SELECT count(*) FROM images")[0][0].as<int>(0);
  Mat training_inputs = Mat(NUM_IMAGES, NUM_INPUTS, CV_32F);
  Mat training_outputs = Mat(NUM_IMAGES, NUM_OUTPUTS, CV_32F);

  Ptr<ml::ANN_MLP> brain;
  // check if the data already exists
  ifstream f("ml.out");
  if (f.good())
  {
    brain = Algorithm::load<ANN_MLP>("ml.out");
  }
  else
  {
    Mat img;
    pqxx::result res = txn.exec(statement);
    for (int i = 0; i < NUM_IMAGES; ++i)
    {
      strToMat(res[i][0].as<std::string>(), img);
      cvtColor(img, img, CV_BGR2GRAY);
      threshold(img, img, 128, 255, 0);

      Point pix = firstBlackLeft(img);
      Point pix2 = firstBlackRight(img);
      Point highest = highestLine(img);
      int wperc = whitePercent(img);
      training_inputs.at<float>(i, 0) = pix.x;
      training_inputs.at<float>(i, 1) = pix.y;
      training_inputs.at<float>(i, 2) = pix2.x;
      training_inputs.at<float>(i, 3) = pix2.y;
      training_inputs.at<float>(i, 4) = highest.x;
      training_inputs.at<float>(i, 5) = highest.y;
      training_inputs.at<float>(i, 6) = wperc;
      for (int j = 3; j <= 10; ++j) {
        training_inputs.at<float>(i, j+4) = res[i][j].as<float>();
      }

      training_outputs.at<float>(i, 0) = res[i][1].as<float>();
      training_outputs.at<float>(i, 1) = res[i][2].as<float>();
    }

    // Create and set up the neural network
    brain = ANN_MLP::create();
    Mat layers = (Mat_<int>(1,3) << NUM_INPUTS, NUM_INPUTS, NUM_OUTPUTS);
    
    // You have to set the activationFunction AFTER the Layer sizes else
    // you get NaN in the outputs
    // WTF: http://stackoverflow.com/questions/36871277/opencv-3-1-ann-predict-returns-nan
    brain->setLayerSizes(layers);
    brain->setActivationFunction(cv::ml::ANN_MLP::SIGMOID_SYM);

    // Split the data into training and test sets
    Ptr<TrainData> data = TrainData::create(training_inputs, SampleTypes::ROW_SAMPLE, training_outputs);
    data->setTrainTestSplitRatio(0.75);
    brain->train(data);

    // Spit out the error and save to file
    Mat output;
    float err = brain->calcError(data, true, output);
    cout << "Err: " << err << endl;
    brain->save("ml.out");
  }

  // Make some predictions
  pqxx::result res = txn.exec(statement);
  Mat inputs = Mat(NUM_IMAGES, NUM_INPUTS, CV_32F);
  for (int i = 0; i < NUM_IMAGES; ++i)
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
      for (int j = 3; j <= 10; ++j) {
        training_inputs.at<float>(i, j+4) = res[i][j].as<float>();
      }
  }
  Mat out;
  brain->predict(inputs, out); 
  for (int i = 0; i < NUM_IMAGES; ++i) {
    cout << "Exp: [" << res[i][1] << ", " << res[i][2] << "]" << endl;

    Mat outData = out.row(i);
    cout << "Actual: " << outData << endl;

    Mat orig_img, img, disp_img;
    strToMat(res[i][0].as<string>(), orig_img);
    cvtColor(orig_img, img, CV_BGR2GRAY);
    threshold(img, img, 128, 255, 0);

    Point pix     = firstBlackLeft(img);
    Point pix2    = firstBlackRight(img);
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
