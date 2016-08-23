///\file main.cpp
///\author Benjamin Knorlein
///\date 08/10/2016

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _MSC_VER 
#ifndef WITH_CONSOLE
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#endif

#ifdef __GNUC__
#define Sleep(x) usleep(x*100)
#endif

#include <iostream>
#include "Socket.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

RNG rng(12345);

//processing settings
bool show = true;
int step_start = 2;
int step_end = 2;
std::string datafolder = "data";

//general settings
int step_size = 100;
int min_depth = 1000;
int max_depth = 20000;
int width = 2048;
int height = 2048;

//step 2 options
double contour_minArea = 15.0;
double border_minDist = 250.0;


//saves the Data
void step1() {

  Socket *sock = new Socket("172.20.160.24", "1975");
  Sleep(500);

  if (sock->isConnected()) std::cout << "yay!" << std::endl;

  std::cout << "sending api version" << std::endl;
  sock->sendMessage("SET_API_VERSION 2\n");
  Sleep(500);

  std::string hologram = "Z:/data/holoyurt/4Deep_Training/capn_bert_july13_4m_7us/capt_burt_jul13-0g-7us_13-Jul-2015_08-23-44-984.bmp";

  std::string reply = sock->sendMessageGetReply(std::string("RECONSTRUCT_HOLOGRAMS ") + 
					hologram + sock->term, 6);

  int nDataBytes;

  if (reply.compare("RECONS") == 0) {
    std::cout << "1>>>" << reply << "<<<" << std::endl;
    reply = sock->receiveMessage(20);
    std::cout << "2>>>" << reply << "<<<" << std::endl;

    Sleep(500);
    reply = sock->sendMessageGetReply("STREAM_RECONSTRUCTION 10000" + sock->term, 41);

    sscanf(reply.substr(32,40).c_str(), "%d", &nDataBytes);
    std::cout << "N=" << nDataBytes << " from " << reply << std::endl;

  } else if (reply.compare("STREAM") == 0) {

    std::cout << "1>>>" << reply << "<<<" << std::endl;
    reply = sock->receiveMessage(35);
    std::cout << "2>>>" << reply << "<<<" << std::endl;
    
    sscanf(reply.substr(26,34).c_str(), "%d", &nDataBytes);
    std::cout << "N=" << nDataBytes << " from " << reply << std::endl;

  }


  if (show) cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.

  for (int output = 0; output < 3; output++){
    std::cout << "send output mode" << output << std::endl;
    sock->sendMessage("OUTPUT_MODE " + std::to_string((long long int)output) + "\n0\n");
    Sleep(500);
    std::string reply;
    while (reply.empty()){
      reply = sock->receiveMessage();
    }

    for (int d = min_depth; d <= max_depth; d += step_size){
      std::cerr << "Output " << output << " Depth " << d << std::endl;
      std::string message = "STREAM_RECONSTRUCTION " + std::to_string((long long int)d) + "\n0\n";

      sock->sendMessage(message);

      Sleep(500);

      cv::Mat image = sock->receiveImage();

      std::string name;
      switch (output)
	{
	default:
	case 0:
	  name = datafolder + "//Intensity_" + std::to_string((long long int)d) + ".ext";
	  break;
	case 1:
	  name = datafolder + "//Amplitude_" + std::to_string((long long int)d) + ".ext";
	  break;
	case 2:
	  name = datafolder + "//Phase_" + std::to_string((long long int)d) + ".ext";
	  break;
	}

      FILE* file = fopen(name.c_str(), "wb");
      fwrite(image.data, sizeof(float), image.size().area(), file);
      fclose(file);

      if (show){
	cv::Mat B;
	normalize(image, image, 0, 255, CV_MINMAX);
	image.convertTo(B, CV_8U);
	imshow("Display window", B);
	switch (output)
	  {
	  default:
	  case 0:
	    imwrite(datafolder + "//img//Intensity_" + std::to_string((long long int)d) + ".png", B);
	    break;
	  case 1:
	    imwrite(datafolder + "//img//Amplitude_" + std::to_string((long long int)d) + ".png", B);
	    break;
	  case 2:
	    imwrite(datafolder + "//img//Phase_" + std::to_string((long long int)d) + ".png", B);
	    break;
	  }
	cv::waitKey(1);
      }
    }
  }
}

int main(int argc, char** argv)
{
  step1();
  return 0;
}


