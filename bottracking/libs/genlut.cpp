#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

short * genlut(cv::Mat ref){
	int max = ref.cols;

	if(max < ref.rows){
		max = ref.rows;
	}
	float scale = 1;

	short * lut = (short *)malloc(sizeof(short)*max);

	for(int i = 0; i < max; i++){
		lut[i] = i*scale;
	}

	return lut;
}
