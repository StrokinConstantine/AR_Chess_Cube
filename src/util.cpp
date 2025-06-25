#include "util.h"
#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <stdint.h>
#include <vector>

/**
 * Scans for available cameras (indices 0-9) and lets the user select one.
 * Returns the selected camera index, or -1 if no camera is available.
*/

int_fast8_t selectAvailableCamera()
{
    std::vector<int_fast8_t> availableCameras;

    for (int_fast8_t i = 9; i >= 0; --i)
    {
        cv::VideoCapture cap(i);
        if (cap.isOpened())
        {
            std::cout << "Camera " << i << " is available." << std::endl;
            availableCameras.push_back(i);
            cap.release(); // Release the camera for later use
        }
    }

    if (availableCameras.empty())
    {
        std::cerr << "Error: No cameras detected!" << std::endl;
        return -1;
    }

    if (availableCameras.size() == 1) {
        std::cout << "Auto-selected camera " << availableCameras[0] << std::endl;
        return availableCameras[0];
    }

    std::cout << "Available cameras: ";
    for (int camIdx : availableCameras)
        std::cout << camIdx << " ";

    std::cout << "\nEnter the camera index to use: ";

  //  int selectedCamera;
   // std::cin >> selectedCamera;

  //  if (std::find(availableCameras.begin(), availableCameras.end(), selectedCamera) == availableCameras.end())
   // {
    //    std::cerr << "Error: Selected camera is not available!" << std::endl;
     //   return -1;
   // }

    return 0;
}