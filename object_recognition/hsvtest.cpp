#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
using namespace std;

int main()
{
  while(1)
  {
    string path;
    getline(cin, path);
    cv::Mat input = cv::imread(path+".jpg");
    cv::Mat hsv_filtered15;//画像の初期化
    cv::Mat hsv_filtered180;//画像の初期化
    cv::Mat hsv;
    cv::Mat output;
    cv::cvtColor(input,hsv,CV_BGR2HSV);//入力画像(src)をhsv色空間(dst)に変換
    //inRange(入力画像,下界画像,上界画像,出力画像)
    //「HSV」は、色を色相(Hue)・彩度(Saturation)・明度(Value)
    cv::inRange(hsv,cv::Scalar(0,80,70),cv::Scalar(10,255,255),hsv_filtered15);
    cv::inRange(hsv,cv::Scalar(175,80,70),cv::Scalar(180,255,255),hsv_filtered180);
    cv::add(hsv_filtered15,hsv_filtered180,output);
    cv::imwrite(path+"BINARY"+".jpg",output);
  }
return 0;
}
