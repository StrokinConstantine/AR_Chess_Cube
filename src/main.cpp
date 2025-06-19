#include <util.h>
#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <cstdint>
#include <vector>


int main()
{

    int_fast8_t cameraIndex = selectAvailableCamera();
    if (cameraIndex == -1)
        return -1;

    cv::VideoCapture videoCapture(cameraIndex);
    if (!videoCapture.isOpened())
    {
        std::cerr << "Error: Failed to open the selected camera!" << std::endl;
        return -1;
    }

    std::cout << "Camera " << cameraIndex << " opened successfully." << std::endl;

    cv::Mat cameraMat = (cv::Mat_<double>(3, 3) <<
        806.7142265403371, 0, 317.2354485853214,
        0, 941.3350377116267, 236.6091014506515,
        0, 0, 1);
        
    cv::Mat distortion = (cv::Mat_<double>(5, 1) << -0.05256710861654016, 1.431356531301361,
                   -0.0002245010772262836, 0.003289240793738838, -8.353763001129924);

    cv::Size chessboardSize(6, 9);

    std::vector<cv::Point3f> chessboard3DPoints;

    for (int i = 0; i < chessboardSize.height; i++)
        for (int j = 0; j < chessboardSize.width; j++)
            chessboard3DPoints.push_back(cv::Point3f(j, i, 0));

    std::vector<cv::Point3f> cube3DPoints =
        {
            {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
            {0, 0, -1}, {1, 0, -1}, {1, 1, -1}, {0, 1, -1}
        };



    cv::Mat modelT = ( cv::Mat_<double>(3, 1) << 0.0, 0.0, 0.0 );

    cv::Mat modelR = ( cv::Mat_<double>(3, 3) <<
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0
    );

	while (true)
    {
        cv::Mat frame;
        videoCapture >> frame;

        int key = cv::waitKey(1);

        if (key == 'a' || key == 'd' || key == 's' || key == 'w' || key == 'e' || key == 'f')
        {
            double translationVectorStep = 0.1;
            cv::Mat invR;
            cv::invert(modelR, invR, cv::DECOMP_SVD);
            modelT = modelR * modelT;

            if (key == 'a')
                modelT.at<double>(0) += translationVectorStep;
            if (key == 'd')
                modelT.at<double>(0) -= translationVectorStep;
            if (key == 's')
                modelT.at<double>(1) += translationVectorStep;
            if (key == 'w')
                modelT.at<double>(1) -= translationVectorStep;
            if (key == 'e')
                modelT.at<double>(2) += translationVectorStep;
            if (key == 'f')
                modelT.at<double>(2) -= translationVectorStep;

            modelT = invR * modelT;
        }

        if (key == 'i' || key == 'k' || key == 'j' || key == 'l' || key == 'u' || key == 'o')
        {
            double rotationAngleStep = 0.1;
            cv::Mat R;

            if (key == 'i')
            {
                R = (cv::Mat_<double>(3, 3) <<
                    cos(rotationAngleStep), -sin(rotationAngleStep), 0.0,
                    sin(rotationAngleStep), cos(rotationAngleStep), 0.0,
                    0.0, 0.0, 1.0);
            }
            if (key == 'k')
            {
                R = (cv::Mat_<double>(3, 3) <<
                    cos(-rotationAngleStep), -sin(-rotationAngleStep), 0.0,
                    sin(-rotationAngleStep), cos(-rotationAngleStep), 0.0,
                    0.0, 0.0, 1.0);
            }
            if (key == 'j')
            {
                R = (cv::Mat_<double>(3, 3) <<
                    cos(rotationAngleStep), 0.0, -sin(rotationAngleStep),
                    0.0, 1.0, 0.0,
                    sin(rotationAngleStep), 0.0, cos(rotationAngleStep));
            }
            if (key == 'l')
            {
                R = (cv::Mat_<double>(3, 3) <<
                    cos(-rotationAngleStep), 0.0, -sin(-rotationAngleStep),
                    0.0, 1.0, 0.0,
                    sin(-rotationAngleStep), 0.0, cos(-rotationAngleStep));
            }
            if (key == 'u')
            {
                R = (cv::Mat_<double>(3, 3) <<
                    1.0, 0.0, 0.0,
                    0.0, cos(rotationAngleStep), -sin(rotationAngleStep),
                    0.0, sin(rotationAngleStep), cos(rotationAngleStep));
            }
            if (key == 'o')
            {
                R = (cv::Mat_<double>(3, 3) <<
                                            1.0, 0.0, 0.0,
                    0.0, cos(-rotationAngleStep), -sin(-rotationAngleStep),
                    0.0, sin(-rotationAngleStep), cos(-rotationAngleStep));
            }
            modelR = R * modelR;
        }

        std::vector<cv::Point2f> chessboardCorners;
        bool found = cv::findChessboardCorners(frame, chessboardSize, chessboardCorners);

        if (found)
        {
            cv::Mat tvec, rvec;
            cv::solvePnP(chessboard3DPoints, chessboardCorners, cameraMat, distortion, rvec, tvec);
            cv::Mat rmat;
            cv::Rodrigues(rvec, rmat);
            cv::Mat composedR = rmat * modelR;
            cv::Mat modelRvec;

            cv::Rodrigues(composedR, modelRvec);

            cv:: Mat modelTvec = composedR * modelT + tvec;

            std::vector<cv::Point2f> projectedCube;
            cv::projectPoints(cube3DPoints, modelRvec, modelTvec, cameraMat, distortion, projectedCube);

            for (int i = 0; i < 4; i++)
            {
                cv::line(frame, projectedCube[i], projectedCube[(i + 1) % 4], cv::Scalar(255, 0, 0), 2);
                cv::line(frame, projectedCube[i + 4], projectedCube[(i + 1) % 4 + 4], cv::Scalar(0, 255, 0), 2);
                cv::line(frame, projectedCube[i], projectedCube[i + 4], cv::Scalar(0, 0, 255), 2);
            }
        }
        cv::imshow("Chessboard and Cube", frame);
        if (key == 27) break; // ESC for exit
    }

    return 0;

}