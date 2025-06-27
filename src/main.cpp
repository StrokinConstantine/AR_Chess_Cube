#include <util.h>
#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <cstdint>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

//#include <nlohmann/json.hpp> 
#include <json.hpp> 








void printMat(const std::string& name, const cv::Mat& mat) {
    std::cout << name << ":\n";
    std::cout << std::fixed << std::setprecision(3);
    for (int i = 0; i < mat.rows; i++) {
        for (int j = 0; j < mat.cols; j++) {
            std::cout << mat.at<double>(i, j) << "\t";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}

void drawCoordinateSystem(cv::Mat& frame, 
                         const cv::Mat& cameraMat,
                         const cv::Mat& distortion,
                         const cv::Mat& rvec,
                         const cv::Mat& tvec,
                         float length = 1.0f) {
    std::vector<cv::Point3f> axisPoints;
    axisPoints.push_back(cv::Point3f(0, 0, 0));
    axisPoints.push_back(cv::Point3f(length, 0, 0));
    axisPoints.push_back(cv::Point3f(0, length, 0));
    axisPoints.push_back(cv::Point3f(0, 0, length));
    
    std::vector<cv::Point2f> projectedPoints;
    cv::projectPoints(axisPoints, rvec, tvec, cameraMat, distortion, projectedPoints);
    
    // X-axis (red)
    cv::arrowedLine(frame, projectedPoints[0], projectedPoints[1], cv::Scalar(0, 0, 255), 2);
    // Y-axis (green)
    cv::arrowedLine(frame, projectedPoints[0], projectedPoints[2], cv::Scalar(0, 255, 0), 2);
    // Z-axis (blue)
    cv::arrowedLine(frame, projectedPoints[0], projectedPoints[3], cv::Scalar(255, 0, 0), 2);
}

bool loadCameraParameters(const std::string& filename, 
                        cv::Mat& cameraMatrix, 
                        cv::Mat& distCoeffs) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }
        
        nlohmann::json_abi_v3_11_2::json config;
        file >> config;
        
        std::vector<double> cam_data = config["camera_matrix"]["data"];
        cameraMatrix = cv::Mat(3, 3, CV_64F, cam_data.data()).clone();
        
        std::vector<double> dist_data = config["distortion_coefficients"]["data"];
        distCoeffs = cv::Mat(5, 1, CV_64F, dist_data.data()).clone();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading camera config: " << e.what() << std::endl;
        return false;
    }
}

void processChessboard(cv::Mat& frame, 
                      const std::vector<cv::Point2f>& corners,
                      const std::vector<cv::Point3f>& chessboard3DPoints,
                      const cv::Mat& cameraMat,
                      const cv::Mat& distortion,
                      const cv::Mat& rvec_cam,
                      const std::vector<cv::Point3f>& scaledVertices,
                      const std::vector<std::vector<int>>& faces,
                      const std::vector<cv::Point3f>& cube3DPoints,
                      const cv::Mat& modelR,
                      const cv::Mat& modelT,
                      const std::string& camName) {
    
    cv::Mat tvec, rvec;
    cv::solvePnP(chessboard3DPoints, corners, cameraMat, distortion, rvec, tvec);
    
    // Print chessboard pose relative to camera
    std::cout << "--- " << camName << " ---\n";
    printMat("Chessboard tvec (relative to camera)", tvec);
    printMat("Chessboard rvec (relative to camera)", rvec);
    
    // Draw chessboard coordinate system
    drawCoordinateSystem(frame, cameraMat, distortion, rvec, tvec, 2.0f);
    
    cv::Mat tvec_global = tvec + rvec_cam;
    
    cv::Mat rmat;
    cv::Rodrigues(rvec, rmat);
    cv::Mat composedR = rmat * modelR;
    cv::Mat modelRvec;
    cv::Rodrigues(composedR, modelRvec);
    cv::Mat modelTvec = composedR * modelT + tvec_global;

    // Print model pose relative to camera
    printMat("Model tvec (relative to camera)", modelTvec);
    printMat("Model rvec (relative to camera)", modelRvec);
    std::cout << "----------------------\n";

    // Draw model coordinate system
    drawCoordinateSystem(frame, cameraMat, distortion, modelRvec, modelTvec, 1.5f);
   
    std::vector<cv::Point2f> projectedCube;
    cv::projectPoints(cube3DPoints, modelRvec, modelTvec, cameraMat, distortion, projectedCube);
    for (int i = 0; i < 4; i++) {
        cv::line(frame, projectedCube[i], projectedCube[(i + 1) % 4], cv::Scalar(255, 0, 0), 2);
        cv::line(frame, projectedCube[i + 4], projectedCube[(i + 1) % 4 + 4], cv::Scalar(0, 255, 0), 2);
        cv::line(frame, projectedCube[i], projectedCube[i + 4], cv::Scalar(0, 0, 255), 2);
    }

    std::vector<cv::Point2f> projectedModel;
    cv::projectPoints(scaledVertices, modelRvec, modelTvec, cameraMat, distortion, projectedModel);

    for (const auto& face : faces) {
        if (face.size() >= 3) {
            for (size_t i = 0; i < face.size(); i++) {
                size_t j = (i + 1) % face.size();
                if (face[i] < projectedModel.size() && face[j] < projectedModel.size()) {
                    cv::line(frame, projectedModel[face[i]], projectedModel[face[j]], cv::Scalar(255, 255, 0), 1);
                }
            }
        }
    }
}

struct Model3D {
    std::vector<cv::Point3f> vertices;
    std::vector<std::vector<int>> faces;
};

Model3D loadOBJ(const std::string& path) {
    Model3D model;
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open OBJ file " << path << std::endl;
        return model;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            model.vertices.push_back(cv::Point3f(x, y, z));
        }
        else if (type == "f") {
            std::vector<int> face;
            std::string vertex;
            while (iss >> vertex) {
                size_t pos = vertex.find('/');
                if (pos != std::string::npos) {
                    vertex = vertex.substr(0, pos);
                }
                face.push_back(std::stoi(vertex) - 1);
            }
            model.faces.push_back(face);
        }
    }

    return model;
}

int main(int argc, char** argv) {

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <num_cameras> <config_path1> [<config_path2> ...]" << std::endl;
        return 1;
    }

    int num_cameras = std::stoi(argv[1]);
    std::vector<std::string> config_paths;
    
    for (int i = 0; i < num_cameras; ++i) {
        config_paths.push_back(argv[2 + i]);
    }

    selectAvailableCamera();
    
    int cameraIndex1 = 2;  
    int cameraIndex2 = 4; 
	
	std::cin >> cameraIndex1;
	std::cin >> cameraIndex2;
    
    cv::VideoCapture videoCapture1(cameraIndex1);
    cv::VideoCapture videoCapture2(cameraIndex2);
    
    videoCapture1.set(cv::CAP_PROP_FRAME_WIDTH, 640); 
    videoCapture1.set(cv::CAP_PROP_FRAME_HEIGHT, 480); 
    videoCapture2.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    videoCapture2.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    
    if (!videoCapture1.isOpened() || !videoCapture2.isOpened()) {
        std::cerr << "Error: Failed to open one or both cameras!" << std::endl;
        return -1;
    }

    std::cout << "Cameras " << cameraIndex1 << " and " << cameraIndex2 << " opened successfully." << std::endl;

    Model3D objModel = loadOBJ("model.obj");
    if (objModel.vertices.empty()) {
        std::cerr << "Error: Failed to load 3D model or model is empty." << std::endl;
        return -1;
    }

    std::vector<cv::Point3f> scaledVertices = objModel.vertices;
    
cv::Mat cameraMat1, distortion1, cameraMat2, distortion2;


if (!loadCameraParameters(config_paths[0], cameraMat1, distortion1) ||
    !loadCameraParameters(config_paths[1], cameraMat2, distortion2))
    std::cerr << "Failed to load camera parameters." << std::endl;


    const cv::Mat R = cv::Mat::eye(3, 3, CV_64F); 
    const cv::Mat T = (cv::Mat_<double>(3, 1) << -0.1, 0.0, 0.0);

    // Print relative pose between cameras
    cv::Mat rvec_cam2;
    cv::Rodrigues(R, rvec_cam2);
    std::cout << "\n=== Camera 2 relative to Camera 1 ===\n";
    printMat("Translation (T)", T);
    printMat("Rotation (R)", R);
    std::cout << "===================================\n\n";

    const cv::Mat rvec_cam1 = cv::Mat::zeros(3, 1, CV_64F);

    const cv::Size chessboardSize(6, 9);
    std::vector<cv::Point3f> chessboard3DPoints;
    for (int i = 0; i < chessboardSize.height; i++)
        for (int j = 0; j < chessboardSize.width; j++)
            chessboard3DPoints.push_back(cv::Point3f(j, i, 0));

    const std::vector<cv::Point3f> cube3DPoints = {
        {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
        {0, 0, -1}, {1, 0, -1}, {1, 1, -1}, {0, 1, -1}
    };

    double modelScale = 1.0;
    const double scaleStep = 0.1;
    cv::Mat modelT = (cv::Mat_<double>(3, 1) << 0.0, 0.0, 0.0);
    cv::Mat modelR = cv::Mat::eye(3, 3, CV_64F);

    cv::Mat stereoFrame;
    std::vector<cv::Point2f> chessboardCorners1, chessboardCorners2;
    std::vector<cv::Point2f> projectedCube, projectedModel;
    cv::Mat frame1, frame2, gray1, gray2;

    while (true) {
        videoCapture1 >> frame1;
        videoCapture2 >> frame2;
        
        if (frame1.empty() || frame2.empty()) {
            std::cerr << "Error: Failed to grab frames from cameras!" << std::endl;
            break;
        }

        int key = cv::waitKey(1);
        
        if (key == '+' || key == '=') modelScale += scaleStep;
        if (key == '-' || key == '_') modelScale = std::max(scaleStep, modelScale - scaleStep);

        if (key == 'a' || key == 'd' || key == 's' || key == 'w' || key == 'e' || key == 'f') {
            double step = 0.1;
            cv::Mat invR;
            cv::invert(modelR, invR, cv::DECOMP_SVD);
            
            modelT = modelR * modelT;

            if (key == 'a') modelT.at<double>(0) += step;
            if (key == 'd') modelT.at<double>(0) -= step;
            if (key == 's') modelT.at<double>(1) += step;
            if (key == 'w') modelT.at<double>(1) -= step;
            if (key == 'e') modelT.at<double>(2) += step;
            if (key == 'f') modelT.at<double>(2) -= step;

            modelT = invR * modelT;
        }

        if (key == 'i' || key == 'k' || key == 'j' || key == 'l' || key == 'u' || key == 'o') {
            double angle = 0.1;
            cv::Mat rot;
            
            if (key == 'i') rot = (cv::Mat_<double>(3, 3) << cos(angle), -sin(angle), 0, sin(angle), cos(angle), 0, 0, 0, 1);
            if (key == 'k') rot = (cv::Mat_<double>(3, 3) << cos(-angle), -sin(-angle), 0, sin(-angle), cos(-angle), 0, 0, 0, 1);
            if (key == 'j') rot = (cv::Mat_<double>(3, 3) << cos(angle), 0, -sin(angle), 0, 1, 0, sin(angle), 0, cos(angle));
            if (key == 'l') rot = (cv::Mat_<double>(3, 3) << cos(-angle), 0, -sin(-angle), 0, 1, 0, sin(-angle), 0, cos(-angle));
            if (key == 'u') rot = (cv::Mat_<double>(3, 3) << 1, 0, 0, 0, cos(angle), -sin(angle), 0, sin(angle), cos(angle));
            if (key == 'o') rot = (cv::Mat_<double>(3, 3) << 1, 0, 0, 0, cos(-angle), -sin(-angle), 0, sin(-angle), cos(-angle));
            
            modelR = rot * modelR;
        }

        for (size_t i = 0; i < objModel.vertices.size(); ++i) {
            scaledVertices[i] = objModel.vertices[i] * modelScale;
        }

        cv::cvtColor(frame1, gray1, cv::COLOR_BGR2GRAY);
        cv::cvtColor(frame2, gray2, cv::COLOR_BGR2GRAY);

        bool found1 = cv::findChessboardCorners(gray1, chessboardSize, chessboardCorners1, 
            cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_ADAPTIVE_THRESH);
        bool found2 = cv::findChessboardCorners(gray2, chessboardSize, chessboardCorners2, 
            cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_ADAPTIVE_THRESH);

        if (found1) processChessboard(frame1, chessboardCorners1, chessboard3DPoints, 
                                    cameraMat1, distortion1, rvec_cam1, 
                                    scaledVertices, objModel.faces, cube3DPoints, modelR, modelT, "Camera 1");
        
        if (found2) processChessboard(frame2, chessboardCorners2, chessboard3DPoints, 
                                    cameraMat2, distortion2, rvec_cam2, 
                                    scaledVertices, objModel.faces, cube3DPoints, modelR, modelT, "Camera 2");

        cv::hconcat(frame1, frame2, stereoFrame);
        cv::imshow("Stereo Cameras with AR", stereoFrame);
        
        if (key == 27) break; // ESC
    }

    videoCapture1.release();
    videoCapture2.release();
    
    return 0;
}