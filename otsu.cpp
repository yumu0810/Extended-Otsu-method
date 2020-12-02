//OpenCV position: pkg-config --cflags --libs /usr/local/Cellar/opencv/4.5.0/lib/pkgconfig/opencv4.pc
/*
-----Use below instructions to run the program on Mac Terminal------
g++ $(pkg-config --cflags --libs opencv4) -std=c++11  otsu.cpp -o otsu
./otsu
*/
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <glob.h>
#include <float.h>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;

int histogram[256] = {0}, thresholds[3] = {0}, total = 0;

//calculate weight * variances
double weight_variances(int left, int right){
	int nums = 0;
	double mean = 0, variances = 0;
	for(int i = left; i <= right; i++){
		mean += i * histogram[i];
		nums += histogram[i];
	}
	mean = mean / nums;

	for(int i = left; i <= right; i++){
		variances += (i - mean) * (i - mean) * histogram[i];
	}

	//weight = nums / total
	//variances = variances / nums
	//weight * variances = (nums / total) * (variances / nums) = variances / total
	return variances / total;
}

//find the three thresholds let the weighted sum of the within-group variances is minimized
void otsu(){
	double w1, w2, w3, w4, minw = DBL_MAX;

	for(int t1 = 0; t1 < 255; t1++){
		for(int t2 = t1 + 1; t2 < 255; t2++){
			for(int t3 = t2 + 1; t3 < 255; t3++){
				w1 = weight_variances(0, t1);
				w2 = weight_variances(t1 + 1, t2);
				w3 = weight_variances(t2 + 1, t3);
				w4 = weight_variances(t3 + 1, 255);

				double w = w1 + w2 + w3 + w4;
				if(w < minw){
					minw = w;
					thresholds[0] = t1;
					thresholds[1] = t2;
					thresholds[2] = t3;
				}
			}
		}
	}
}

//use the threshold to decide each pixel's group and output to its corresponding's grayscale
//4 classes value: 0, 85, 170, 255
void output_img(Mat& img, int thresholds[]){
	for(int i = 0; i < img.rows; i++){
		for(int j = 0; j < img.cols; j++){
			if(img.at<uchar>(i, j) <= thresholds[0]) 
				img.at<uchar>(i, j) = 0;
			else if(img.at<uchar>(i, j) > thresholds[0] && img.at<uchar>(i, j) <= thresholds[1])
				img.at<uchar>(i, j) = 85; 	//255 / 3
			else if(img.at<uchar>(i, j) > thresholds[1] && img.at<uchar>(i, j) <= thresholds[2])
				img.at<uchar>(i, j) = 170;	//255 * 2 / 3
			else img.at<uchar>(i, j) = 255;
		}
	}
}

int main(){
	//Read every pictures in the folder
	vector<string> filenames;
	string folder = "*.bmp";
	glob(folder, filenames);

	for(size_t i = 0; i < filenames.size(); i++){
		Mat src = imread(filenames[i]);
		//reset histogram's and thresholds's array value
		memset(histogram, 0, sizeof(histogram));
		memset(thresholds, 0, sizeof(thresholds));
		total = src.rows * src.cols;
		//create the blank gray image
		Mat img(src.size(), CV_8UC1, Scalar(0));
		//convert the input color image into a grayscale image using the formula I=Round(0.299R+0.587G+0.114B) 
		//also accumulate the histogram
		for(int i = 0; i < src.rows; i++){
			for(int j = 0; j < src.cols; j++){
				img.at<uchar>(i, j) = round(0.299 * src.at<Vec3b>(i, j)[2] + 0.587 * src.at<Vec3b>(i, j)[1] + 0.114 * src.at<Vec3b>(i, j)[0]);
				histogram[img.at<uchar>(i, j)]++;
			}
		}
	
		otsu();
		output_img(img, thresholds);
		//handle the input picture's name
		string file = filenames[i].substr(2, filenames[i].size() - 2);
		cout << file << "'s thresholds:" << endl;
		cout << "t1: " << thresholds[0] << endl;
		cout << "t2: " << thresholds[1] << endl;
		cout << "t3: " << thresholds[2] << endl;
		cout << "----------------------" << endl;
		//handle the output picture's name
		size_t p = file.find(".bmp");
		string outputName = file.substr(0, p) + "_out.bmp"; 
		imwrite(outputName, img);
	}
	return 0;
}
