

// System includes
#include <iostream>
#include <sstream>

#include "stdafx.h"

#include "LandmarkDetectorParameters.h"

//// Boost includes
//#include <filesystem.hpp>
//#include <filesystem/fstream.hpp>

//using namespace std;

using namespace LandmarkDetector;

FaceModelParameters::FaceModelParameters()
{
    // initialise the default values
//    init();
}

void FaceModelParameters::init()
{
    
    // number of iterations that will be performed at each scale
    num_optimisation_iteration = 5;
    
    // using an external face checker based on SVM
    validate_detections = true;
    
    // Using hierarchical refinement by default (can be turned off)
    refine_hierarchical = true;
    
    // Refining parameters by default
    refine_parameters = true;
    
    window_sizes_small = std::vector<int>(4);
    window_sizes_init = std::vector<int>(4);
    
    // For fast tracking
    window_sizes_small[0] = 0;
    window_sizes_small[1] = 9;
    window_sizes_small[2] = 7;
    window_sizes_small[3] = 5;
    
    // Just for initialisation
    window_sizes_init.at(0) = 11;
    window_sizes_init.at(1) = 9;
    window_sizes_init.at(2) = 7;
    window_sizes_init.at(3) = 5;
    
    face_template_scale = 0.3;
    // Off by default (as it might lead to some slight inaccuracies in slowly moving faces)
    use_face_template = false;
    
    // For first frame use the initialisation
    window_sizes_current = window_sizes_init;
    
    model_location = std::string("/Users/user/Desktop/OpenFACE/test_console/model/main_clnf_general.txt");
//    std::cout << "model_location = " << model_location << std::endl;
    
    sigma = 1.5;
    reg_factor = 25;
    weight_factor = 0; // By default do not use NU-RLMS for videos as it does not work as well for them
    
    validation_boundary = -0.45;
    
    limit_pose = true;
    multi_view = false;
    
    reinit_video_every = 4;
    
    // Face detection
    
    char* pHome;
    pHome = getenv ("HOME");
    std::string configPath(pHome);
    configPath += "/Desktop/Fatigue control/Programm/";
    
    
    face_detector_location = std::string(configPath + "classifiers/haarcascade_frontalface_alt.xml");
//    std::cout << "face_detector_location = " << face_detector_location << std::endl;
    
    quiet_mode = false;
    
    // By default use HOG SVM
    curr_face_detector = HAAR_DETECTOR; //HOG_SVM_DETECTOR;
    
    // The gaze tracking has to be explicitly initialised
    track_gaze = true;
}

