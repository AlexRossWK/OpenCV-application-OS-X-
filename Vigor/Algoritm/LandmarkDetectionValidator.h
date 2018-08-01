
#ifndef __LANDMARK_DETECTION_VALIDATOR_h_
#define __LANDMARK_DETECTION_VALIDATOR_h_

// OpenCV includes
#include <opencv2/core/core.hpp>

// System includes
#include <vector>

// Local includes
#include "PAW.h"

//using namespace std;

namespace LandmarkDetector
{

	class DetectionValidator
{
		
public:    
	
	// What type of validator we're using - 0 - linear svr, 1 - feed forward neural net, 2 - convolutional neural net
	int validator_type;

	// The orientations of each of the landmark detection validator
    std::vector<cv::Vec3d> orientations;

	// Piecewise affine warps to the reference shape (per orientation)
    std::vector<PAW>     paws;

	//==========================================
	// Linear SVR

	// SVR biases
    std::vector<double>  bs;

	// SVR weights
    std::vector<cv::Mat_<double> > ws;
	
	//==========================================
	// Neural Network

	// Neural net weights
    std::vector<std::vector<cv::Mat_<double> > > ws_nn;

	// What type of activation or output functions are used
	// 0 - sigmoid, 1 - tanh_opt, 2 - ReLU
    std::vector<int> activation_fun;
    std::vector<int> output_fun;

	//==========================================
	// Convolutional Neural Network

	// CNN layers for each view
	// view -> layer -> input maps -> kernels
    std::vector<std::vector<std::vector<std::vector<cv::Mat_<float> > > > > cnn_convolutional_layers;
	// Bit ugly with so much nesting, but oh well
    std::vector<std::vector<std::vector<std::vector<std::pair<int, cv::Mat_<double> > > > > > cnn_convolutional_layers_dft;
    std::vector<std::vector<std::vector<float > > > cnn_convolutional_layers_bias;
    std::vector< std::vector<int> > cnn_subsampling_layers;
    std::vector< std::vector<cv::Mat_<float> > > cnn_fully_connected_layers;
    std::vector< std::vector<float > > cnn_fully_connected_layers_bias;
	// 0 - convolutional, 1 - subsampling, 2 - fully connected
    std::vector<std::vector<int> > cnn_layer_types;
	
	//==========================================

	// Normalisation for face validation
    std::vector<cv::Mat_<double> > mean_images;
    std::vector<cv::Mat_<double> > standard_deviations;

	// Default constructor
	DetectionValidator(){;}

	// Copy constructor
	DetectionValidator(const DetectionValidator& other);

	// Given an image, orientation and detected landmarks output the result of the appropriate regressor
	double Check(const cv::Vec3d& orientation, const cv::Mat_<uchar>& intensity_img, cv::Mat_<double>& detected_landmarks);

	// Reading in the model
    void Read(std::string location);
			
	// Getting the closest view center based on orientation
	int GetViewId(const cv::Vec3d& orientation) const;

private:

	// The actual regressor application on the image

	// Support Vector Regression (linear kernel)
	double CheckSVR(const cv::Mat_<double>& warped_img, int view_id);

	// Feed-forward Neural Network
	double CheckNN(const cv::Mat_<double>& warped_img, int view_id);

	// Convolutional Neural Network
	double CheckCNN(const cv::Mat_<double>& warped_img, int view_id);

	// A normalisation helper
	void NormaliseWarpedToVector(const cv::Mat_<double>& warped_img, cv::Mat_<double>& feature_vec, int view_id);

};

}
#endif

