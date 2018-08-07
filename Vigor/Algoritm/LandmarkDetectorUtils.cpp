
#include "stdafx.h"

#include "LandmarkDetectorUtils.h"

// OpenCV includes
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

#include <opencv2/core/utility.hpp>
#include "opencv2/video/background_segm.hpp"

#include <iostream>
#include <iomanip>
#include <ctime>


#include <chrono>
#include <curl/curl.h>


#include <iostream>
#include <fstream>
#include <string>

#include <iostream>
#include <fstream>

//using namespace std;

namespace LandmarkDetector
{

	/*
	struct ye_history
	{
		//std::string blink = "01010" initialization = 1 sleep = 2 date_time = 2018 - 02 - 23
		std::string user;
		std::string date_time;
		std::string sleep;
		std::string face_detect;
		std::string blint;

	};

	*/

	std::vector<int> time_open_eye_history;
	struct ye_history_blink
	{
		//std::string blink = "01010" initialization = 1 sleep = 2 date_time = 2018 - 02 - 23
		std::string user; //мак компа
		std::string date_time;
		
		int blink_count; //сколько раз моргнул., чем чаше тем хуже (1)
		int time_open_eye; //2. Как долго в совокупности глаза были открыты (мс)
		int max_time_blink_count; // максимальное время закрытия глаз, чем больше тем хуже (3)
		int disable_motion_face; // 4. как долго не двигалась голова (мс)


		/*int face_motion_count; //сколько раз было движение головой. В нормальной ситуации человек двигает головой, если же нет...человек "залип"
		//чем меньше, тем хуже
		int blink_count; //сколько раз моргнул., чем чаше тем хуже
		int max_time_blink_count; // максимальное время закрытия глаз, чем больше тем хуже
		int min_blink_close_time_interval;//минимальное время интервал между закрытия глазами, в средннем 5 сек, чем меньше, тем хуже 
		*/
	};

	//эталонный показатель, для сравнения.
	//в момент первого запуска программы он анализируется 5 минит, далее сохраняется в файлик.
	//в момент следущего запуска идет загрузка данных с файла для сравнения
	struct ye_reference_blink
	{
		std::string user; //мак компа
		std::string date_time;
		/*
		int face_motion_count; //сколько раз было движение головой. В нормальной ситуации человек двигает головой, если же нет...человек "залип"
							   //чем меньше, тем хуже
		int blink_count; //сколько раз моргнул., чем чаше тем хуже
		int max_time_blink_count; // максимальное время закрытия глаз, чем больше тем хуже
		int min_blink_close_time_interval;//минимальное время интервал между закрытия глазами, в средннем 5 сек, чем меньше, тем хуже */

		int blink_count; //сколько раз моргнул., чем чаше тем хуже (1)
		int time_open_eye; //2. Как долго в совокупности глаза были открыты (мс)
		int max_time_blink_count; // максимальное время закрытия глаз, чем больше тем хуже (3)
		int disable_motion_face; // 4. как долго не двигалась голова (мс)
		
/*1. Количество морганий тут все должно быть понятно
2. Как долго в совокупности глаза были открыты (мс)
3. как долго в совокупности глаза были закрыты (мс)
Если у нас сохраняется баг с открытием/закрытием глаз при перемещении головы, можно просто вычитать из общего интервала продолжительность пункта 2.
4. как долго не двигалась голова (мс)
*/

	};

	//std::vector<ye_history_blink> hist_yes;

	/*std::cout << "USER001" << std::endl;
							//TIME
							auto t = std::time(nullptr);
							auto tm = *std::localtime(&t);
							std::cout << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl;

							std::cout << "1: " << val_preview << " :: " << val_current << std::endl;

							std::string face_detect = "0";
							if (BlinkStatusEyes)
								face_detect = "1";
							std::cout << "FACE = " << face_detect << std::endl;

							std::string BLINK_DETERMINED = "1";
							if (InitHeadCloseStatusEyes)
								BLINK_DETERMINED = "0";
							std::cout << "BLINK = " << BLINK_DETERMINED << std::endl;

							*/

	// For subpixel accuracy drawing
	const int draw_shiftbits = 4;
	const int draw_multiplier = 1 << 4;

	// Extracting the following command line arguments -f, -fd, -op, -of, -ov (and possible ordered repetitions)
	void get_video_input_output_params(std::vector<std::string> &input_video_files, std::vector<std::string> &depth_dirs,
		std::vector<std::string> &output_files, std::vector<std::string> &output_video_files, bool& world_coordinates_pose, std::vector<std::string> &arguments)
	{
		bool* valid = new bool[arguments.size()];

		for (size_t i = 0; i < arguments.size(); ++i)
		{
			valid[i] = true;
		}

		// By default use rotation with respect to camera (not world coordinates)
		world_coordinates_pose = false;

		std::string root = "";
		// First check if there is a root argument (so that videos and outputs could be defined more easilly)
		for (size_t i = 0; i < arguments.size(); ++i)
		{
			if (arguments[i].compare("-root") == 0)
			{
				root = arguments[i + 1];
				// Do not discard root as it might be used in other later steps
				i++;
			}
		}

		for (size_t i = 0; i < arguments.size(); ++i)
		{
			if (arguments[i].compare("-f") == 0)
			{
				input_video_files.push_back(root + arguments[i + 1]);
				valid[i] = false;
				valid[i + 1] = false;
				i++;
			}
			else if (arguments[i].compare("-fd") == 0)
			{
				depth_dirs.push_back(root + arguments[i + 1]);
				valid[i] = false;
				valid[i + 1] = false;
				i++;
			}
			//        else if (arguments[i].compare("-of") == 0)
			//        {
			//            output_files.push_back(root + arguments[i + 1]);
			//            create_directory_from_file(root + arguments[i + 1]);
			//            valid[i] = false;
			//            valid[i+1] = false;
			//            i++;
			//        }
			//        else if (arguments[i].compare("-ov") == 0)
			//        {
			//            output_video_files.push_back(root + arguments[i + 1]);
			//            create_directory_from_file(root + arguments[i + 1]);
			//            valid[i] = false;
			//            valid[i+1] = false;
			//            i++;
			//        }
			else if (arguments[i].compare("-world_coord") == 0)
			{
				world_coordinates_pose = true;
			}
		}

		for (int i = arguments.size() - 1; i >= 0; --i)
		{
			if (!valid[i])
			{
				arguments.erase(arguments.begin() + i);
			}
		}

	}

	void get_camera_params(int &device, float &fx, float &fy, float &cx, float &cy, std::vector<std::string> &arguments)
	{
		bool* valid = new bool[arguments.size()];

		for (size_t i = 0; i < arguments.size(); ++i)
		{
			valid[i] = true;
			if (arguments[i].compare("-fx") == 0)
			{
				std::stringstream data(arguments[i + 1]);
				data >> fx;
				valid[i] = false;
				valid[i + 1] = false;
				i++;
			}
			else if (arguments[i].compare("-fy") == 0)
			{
				std::stringstream data(arguments[i + 1]);
				data >> fy;
				valid[i] = false;
				valid[i + 1] = false;
				i++;
			}
			else if (arguments[i].compare("-cx") == 0)
			{
				std::stringstream data(arguments[i + 1]);
				data >> cx;
				valid[i] = false;
				valid[i + 1] = false;
				i++;
			}
			else if (arguments[i].compare("-cy") == 0)
			{
				std::stringstream data(arguments[i + 1]);
				data >> cy;
				valid[i] = false;
				valid[i + 1] = false;
				i++;
			}
			else if (arguments[i].compare("-device") == 0)
			{
				std::stringstream data(arguments[i + 1]);
				data >> device;
				valid[i] = false;
				valid[i + 1] = false;
				i++;
			}
		}

		for (int i = arguments.size() - 1; i >= 0; --i)
		{
			if (!valid[i])
			{
				arguments.erase(arguments.begin() + i);
			}
		}
	}

	void get_image_input_output_params(std::vector<std::string> &input_image_files, std::vector<std::string> &input_depth_files, std::vector<std::string> &output_feature_files, std::vector<std::string> &output_pose_files, std::vector<std::string> &output_image_files,
		std::vector<cv::Rect_<double>> &input_bounding_boxes, std::vector<std::string> &arguments)
	{
		bool* valid = new bool[arguments.size()];

		std::string out_pts_dir, out_pose_dir, out_img_dir;

		for (size_t i = 0; i < arguments.size(); ++i)
		{
			valid[i] = true;
			if (arguments[i].compare("-f") == 0)
			{
				input_image_files.push_back(arguments[i + 1]);
				valid[i] = false;
				valid[i + 1] = false;
				i++;
			}
			else if (arguments[i].compare("-fd") == 0)
			{
				input_depth_files.push_back(arguments[i + 1]);
				valid[i] = false;
				valid[i + 1] = false;
				i++;
			}
			//        else if (arguments[i].compare("-fdir") == 0)
			//        {

			//            // parse the -fdir directory by reading in all of the .png and .jpg files in it
			//            path image_directory (arguments[i+1]);

			//            try
			//            {
			//                 // does the file exist and is it a directory
			//                if (exists(image_directory) && is_directory(image_directory))
			//                {

			//                    vector<path> file_in_directory;
			//                    copy(directory_iterator(image_directory), directory_iterator(), back_inserter(file_in_directory));

			//                    // Sort the images in the directory first
			//                    sort(file_in_directory.begin(), file_in_directory.end());

			//                    for (vector<path>::const_iterator file_iterator (file_in_directory.begin()); file_iterator != file_in_directory.end(); ++file_iterator)
			//                    {
			//                        // Possible image extension .jpg and .png
			//                        if(file_iterator->extension().string().compare(".jpg") == 0 || file_iterator->extension().string().compare(".png") == 0 || file_iterator->extension().string().compare(".bmp") == 0)
			//                        {


			//                            input_image_files.push_back(file_iterator->string());

			//                            // If there exists a .txt file corresponding to the image, it is assumed that it contains a bounding box definition for a face
			//                            // [minx, miny, maxx, maxy]
			//                            path current_file = *file_iterator;
			//                            path bbox = current_file.replace_extension("txt");

			//                            // If there is a bounding box file push it to the list of bounding boxes
			//                            if(exists(bbox))
			//                            {

			//                                std::ifstream in_bbox(bbox.string().c_str(), ios_base::in);

			//                                double min_x, min_y, max_x, max_y;

			//                                in_bbox >> min_x >> min_y >> max_x >> max_y;

			//                                in_bbox.close();

			//                                input_bounding_boxes.push_back(cv::Rect_<double>(min_x, min_y, max_x - min_x, max_y - min_y));
			//                            }
			//                        }
			//                    }
			//                }
			//            }
			//            catch (const filesystem_error& ex)
			//            {
			//                cout << ex.what() << '\n';
			//            }

			//            valid[i] = false;
			//            valid[i+1] = false;
			//            i++;
			//        }
			//        else if (arguments[i].compare("-ofdir") == 0)
			//        {
			//            out_pts_dir = arguments[i + 1];
			//            create_directories(out_pts_dir);
			//            valid[i] = false;
			//            valid[i+1] = false;
			//            i++;
			//        }
			//        else if (arguments[i].compare("-opdir") == 0)
			//        {
			//            out_pose_dir = arguments[i + 1];
			//            create_directories(out_pose_dir);
			//            valid[i] = false;
			//            valid[i + 1] = false;
			//            i++;
			//        }
			//        else if (arguments[i].compare("-oidir") == 0)
			//        {
			//            out_img_dir = arguments[i + 1];
			//            create_directories(out_img_dir);
			//            valid[i] = false;
			//            valid[i+1] = false;
			//            i++;
			//        }
			else if (arguments[i].compare("-op") == 0)
			{
				output_pose_files.push_back(arguments[i + 1]);
				valid[i] = false;
				valid[i + 1] = false;
				i++;
			}
			else if (arguments[i].compare("-of") == 0)
			{
				output_feature_files.push_back(arguments[i + 1]);
				valid[i] = false;
				valid[i + 1] = false;
				i++;
			}
			else if (arguments[i].compare("-oi") == 0)
			{
				output_image_files.push_back(arguments[i + 1]);
				valid[i] = false;
				valid[i + 1] = false;
				i++;
			}
		}

		// If any output directories are defined populate them based on image names
		//    if(!out_img_dir.empty())
		//    {
		//        for(size_t i=0; i < input_image_files.size(); ++i)
		//        {
		//            path image_loc(input_image_files[i]);

		//            path fname = image_loc.filename();
		//            fname = fname.replace_extension("jpg");
		//            output_image_files.push_back(out_img_dir + "/" + fname.string());

		//        }
		//        if(!input_image_files.empty())
		//        {
		//            create_directory_from_file(output_image_files[0]);
		//        }
		//    }

		//    if(!out_pts_dir.empty())
		//    {
		//        for(size_t i=0; i < input_image_files.size(); ++i)
		//        {
		//            path image_loc(input_image_files[i]);

		//            path fname = image_loc.filename();
		//            fname = fname.replace_extension("pts");
		//            output_feature_files.push_back(out_pts_dir + "/" + fname.string());
		//        }
		//        create_directory_from_file(output_feature_files[0]);
		//    }

		//    if (!out_pose_dir.empty())
		//    {
		//        for (size_t i = 0; i < input_image_files.size(); ++i)
		//        {
		//            path image_loc(input_image_files[i]);

		//            path fname = image_loc.filename();
		//            fname = fname.replace_extension("pose");
		//            output_pose_files.push_back(out_pose_dir + "/" + fname.string());
		//        }
		//        create_directory_from_file(output_pose_files[0]);
		//    }

		// Make sure the same number of images and bounding boxes is present, if any bounding boxes are defined
		if (input_bounding_boxes.size() > 0)
		{
			assert(input_bounding_boxes.size() == input_image_files.size());
		}

		// Clear up the argument list
		for (int i = arguments.size() - 1; i >= 0; --i)
		{
			if (!valid[i])
			{
				arguments.erase(arguments.begin() + i);
			}
		}

	}

	//===========================================================================
	// Fast patch expert response computation (linear model across a ROI) using normalised cross-correlation
	//===========================================================================

	void crossCorr_m(const cv::Mat_<float>& img, cv::Mat_<double>& img_dft, const cv::Mat_<float>& _templ, std::map<int, cv::Mat_<double> >& _templ_dfts, cv::Mat_<float>& corr)
	{
		// Our model will always be under min block size so can ignore this
		//const double blockScale = 4.5;
		//const int minBlockSize = 256;

		int maxDepth = CV_64F;

		cv::Size dftsize;

		dftsize.width = cv::getOptimalDFTSize(corr.cols + _templ.cols - 1);
		dftsize.height = cv::getOptimalDFTSize(corr.rows + _templ.rows - 1);

		// Compute block size
		cv::Size blocksize;
		blocksize.width = dftsize.width - _templ.cols + 1;
		blocksize.width = MIN(blocksize.width, corr.cols);
		blocksize.height = dftsize.height - _templ.rows + 1;
		blocksize.height = MIN(blocksize.height, corr.rows);

		cv::Mat_<double> dftTempl;

		// if this has not been precomputed, precompute it, otherwise use it
		if (_templ_dfts.find(dftsize.width) == _templ_dfts.end())
		{
			dftTempl.create(dftsize.height, dftsize.width);

			cv::Mat_<float> src = _templ;

			cv::Mat_<double> dst(dftTempl, cv::Rect(0, 0, dftsize.width, dftsize.height));

			cv::Mat_<double> dst1(dftTempl, cv::Rect(0, 0, _templ.cols, _templ.rows));

			if (dst1.data != src.data)
				src.convertTo(dst1, dst1.depth());

			if (dst.cols > _templ.cols)
			{
				cv::Mat_<double> part(dst, cv::Range(0, _templ.rows), cv::Range(_templ.cols, dst.cols));
				part.setTo(0);
			}

			// Perform DFT of the template
			dft(dst, dst, 0, _templ.rows);

			_templ_dfts[dftsize.width] = dftTempl;

		}
		else
		{
			// use the precomputed version
			dftTempl = _templ_dfts.find(dftsize.width)->second;
		}

        cv::Size bsz(fmin(blocksize.width, corr.cols), fmin(blocksize.height, corr.rows));
		cv::Mat src;

		cv::Mat cdst(corr, cv::Rect(0, 0, bsz.width, bsz.height));

		cv::Mat_<double> dftImg;

		if (img_dft.empty())
		{
			dftImg.create(dftsize);
			dftImg.setTo(0.0);

			cv::Size dsz(bsz.width + _templ.cols - 1, bsz.height + _templ.rows - 1);

            int x2 = fmin(img.cols, dsz.width);
            int y2 = fmin(img.rows, dsz.height);

			cv::Mat src0(img, cv::Range(0, y2), cv::Range(0, x2));
			cv::Mat dst(dftImg, cv::Rect(0, 0, dsz.width, dsz.height));
			cv::Mat dst1(dftImg, cv::Rect(0, 0, x2, y2));

			src = src0;

			if (dst1.data != src.data)
				src.convertTo(dst1, dst1.depth());

			dft(dftImg, dftImg, 0, dsz.height);
			img_dft = dftImg.clone();
		}

		cv::Mat dftTempl1(dftTempl, cv::Rect(0, 0, dftsize.width, dftsize.height));
		cv::mulSpectrums(img_dft, dftTempl1, dftImg, 0, true);
		cv::dft(dftImg, dftImg, cv::DFT_INVERSE + cv::DFT_SCALE, bsz.height);

		src = dftImg(cv::Rect(0, 0, bsz.width, bsz.height));

		src.convertTo(cdst, CV_32F);

	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////

	void matchTemplate_m(const cv::Mat_<float>& input_img, cv::Mat_<double>& img_dft, cv::Mat& _integral_img, cv::Mat& _integral_img_sq, const cv::Mat_<float>&  templ, std::map<int, cv::Mat_<double> >& templ_dfts, cv::Mat_<float>& result, int method)
	{

		int numType = method == CV_TM_CCORR || method == CV_TM_CCORR_NORMED ? 0 : method == CV_TM_CCOEFF || method == CV_TM_CCOEFF_NORMED ? 1 : 2;
		bool isNormed = method == CV_TM_CCORR_NORMED ||
			method == CV_TM_SQDIFF_NORMED ||
			method == CV_TM_CCOEFF_NORMED;

		// Assume result is defined properly
		if (result.empty())
		{
			cv::Size corrSize(input_img.cols - templ.cols + 1, input_img.rows - templ.rows + 1);
			result.create(corrSize);
		}
		LandmarkDetector::crossCorr_m(input_img, img_dft, templ, templ_dfts, result);

		if (method == CV_TM_CCORR)
			return;

		double invArea = 1. / ((double)templ.rows * templ.cols);

		cv::Mat sum, sqsum;
		cv::Scalar templMean, templSdv;
		double *q0 = 0, *q1 = 0, *q2 = 0, *q3 = 0;
		double templNorm = 0, templSum2 = 0;

		if (method == CV_TM_CCOEFF)
		{
			// If it has not been precomputed compute it now
			if (_integral_img.empty())
			{
				integral(input_img, _integral_img, CV_64F);
			}
			sum = _integral_img;

			templMean = cv::mean(templ);
		}
		else
		{
			// If it has not been precomputed compute it now
			if (_integral_img.empty())
			{
				integral(input_img, _integral_img, _integral_img_sq, CV_64F);
			}

			sum = _integral_img;
			sqsum = _integral_img_sq;

			meanStdDev(templ, templMean, templSdv);

			templNorm = templSdv[0] * templSdv[0] + templSdv[1] * templSdv[1] + templSdv[2] * templSdv[2] + templSdv[3] * templSdv[3];

			if (templNorm < DBL_EPSILON && method == CV_TM_CCOEFF_NORMED)
			{
				result.setTo(1.0);
				return;
			}

			templSum2 = templNorm + templMean[0] * templMean[0] + templMean[1] * templMean[1] + templMean[2] * templMean[2] + templMean[3] * templMean[3];

			if (numType != 1)
			{
				templMean = cv::Scalar::all(0);
				templNorm = templSum2;
			}

			templSum2 /= invArea;
			templNorm = std::sqrt(templNorm);
			templNorm /= std::sqrt(invArea); // care of accuracy here

			q0 = (double*)sqsum.data;
			q1 = q0 + templ.cols;
			q2 = (double*)(sqsum.data + templ.rows*sqsum.step);
			q3 = q2 + templ.cols;
		}

		double* p0 = (double*)sum.data;
		double* p1 = p0 + templ.cols;
		double* p2 = (double*)(sum.data + templ.rows*sum.step);
		double* p3 = p2 + templ.cols;

		int sumstep = sum.data ? (int)(sum.step / sizeof(double)) : 0;
		int sqstep = sqsum.data ? (int)(sqsum.step / sizeof(double)) : 0;

		int i, j;

		for (i = 0; i < result.rows; i++)
		{
			float* rrow = result.ptr<float>(i);
			int idx = i * sumstep;
			int idx2 = i * sqstep;

			for (j = 0; j < result.cols; j++, idx += 1, idx2 += 1)
			{
				double num = rrow[j], t;
				double wndMean2 = 0, wndSum2 = 0;

				if (numType == 1)
				{

					t = p0[idx] - p1[idx] - p2[idx] + p3[idx];
					wndMean2 += t*t;
					num -= t*templMean[0];

					wndMean2 *= invArea;
				}

				if (isNormed || numType == 2)
				{

					t = q0[idx2] - q1[idx2] - q2[idx2] + q3[idx2];
					wndSum2 += t;

					if (numType == 2)
					{
						num = wndSum2 - 2 * num + templSum2;
						num = MAX(num, 0.);
					}
				}

				if (isNormed)
				{
					t = std::sqrt(MAX(wndSum2 - wndMean2, 0))*templNorm;
					if (fabs(num) < t)
						num /= t;
					else if (fabs(num) < t*1.125)
						num = num > 0 ? 1 : -1;
					else
						num = method != CV_TM_SQDIFF_NORMED ? 0 : 1;
				}

				rrow[j] = (float)num;
			}
		}
	}


	//===========================================================================
	// Point set and landmark manipulation functions
	//===========================================================================
	// Using Kabsch's algorithm for aligning shapes
	//This assumes that align_from and align_to are already mean normalised
	cv::Matx22d AlignShapesKabsch2D(const cv::Mat_<double>& align_from, const cv::Mat_<double>& align_to)
	{

		cv::SVD svd(align_from.t() * align_to);

		// make sure no reflection is there
		// corr ensures that we do only rotaitons and not reflections
		double d = cv::determinant(svd.vt.t() * svd.u.t());

		cv::Matx22d corr = cv::Matx22d::eye();
		if (d > 0)
		{
			corr(1, 1) = 1;
		}
		else
		{
			corr(1, 1) = -1;
		}

		cv::Matx22d R;
		cv::Mat(svd.vt.t()*cv::Mat(corr)*svd.u.t()).copyTo(R);

		return R;
	}

	//=============================================================================
	// Basically Kabsch's algorithm but also allows the collection of points to be different in scale from each other
	cv::Matx22d AlignShapesWithScale(cv::Mat_<double>& src, cv::Mat_<double> dst)
	{
		int n = src.rows;

		// First we mean normalise both src and dst
		double mean_src_x = cv::mean(src.col(0))[0];
		double mean_src_y = cv::mean(src.col(1))[0];

		double mean_dst_x = cv::mean(dst.col(0))[0];
		double mean_dst_y = cv::mean(dst.col(1))[0];

		cv::Mat_<double> src_mean_normed = src.clone();
		src_mean_normed.col(0) = src_mean_normed.col(0) - mean_src_x;
		src_mean_normed.col(1) = src_mean_normed.col(1) - mean_src_y;

		cv::Mat_<double> dst_mean_normed = dst.clone();
		dst_mean_normed.col(0) = dst_mean_normed.col(0) - mean_dst_x;
		dst_mean_normed.col(1) = dst_mean_normed.col(1) - mean_dst_y;

		// Find the scaling factor of each
		cv::Mat src_sq;
		cv::pow(src_mean_normed, 2, src_sq);

		cv::Mat dst_sq;
		cv::pow(dst_mean_normed, 2, dst_sq);

		double s_src = sqrt(cv::sum(src_sq)[0] / n);
		double s_dst = sqrt(cv::sum(dst_sq)[0] / n);

		src_mean_normed = src_mean_normed / s_src;
		dst_mean_normed = dst_mean_normed / s_dst;

		double s = s_dst / s_src;

		// Get the rotation
		cv::Matx22d R = AlignShapesKabsch2D(src_mean_normed, dst_mean_normed);

		cv::Matx22d	A;
		cv::Mat(s * R).copyTo(A);

		cv::Mat_<double> aligned = (cv::Mat(cv::Mat(A) * src.t())).t();
		cv::Mat_<double> offset = dst - aligned;

		double t_x = cv::mean(offset.col(0))[0];
		double t_y = cv::mean(offset.col(1))[0];

		return A;

	}


	//===========================================================================
	// Visualisation functions
	//===========================================================================
	void Project(cv::Mat_<double>& dest, const cv::Mat_<double>& mesh, double fx, double fy, double cx, double cy)
	{
		dest = cv::Mat_<double>(mesh.rows, 2, 0.0);

		int num_points = mesh.rows;

		double X, Y, Z;


		cv::Mat_<double>::const_iterator mData = mesh.begin();
		cv::Mat_<double>::iterator projected = dest.begin();

		for (int i = 0; i < num_points; i++)
		{
			// Get the points
			X = *(mData++);
			Y = *(mData++);
			Z = *(mData++);

			double x;
			double y;

			// if depth is 0 the projection is different
			if (Z != 0)
			{
				x = ((X * fx / Z) + cx);
				y = ((Y * fy / Z) + cy);
			}
			else
			{
				x = X;
				y = Y;
			}

			// Project and store in dest matrix
			(*projected++) = x;
			(*projected++) = y;
		}

	}

	// Computing landmarks (to be drawn later possibly)
	std::vector<cv::Point2d> CalculateLandmarks(const cv::Mat_<double>& shape2D, cv::Mat_<int>& visibilities)
	{
		int n = shape2D.rows / 2;
		std::vector<cv::Point2d> landmarks;

		for (int i = 0; i < n; ++i)
		{
			if (visibilities.at<int>(i))
			{
				cv::Point2d featurePoint(shape2D.at<double>(i), shape2D.at<double>(i + n));

				landmarks.push_back(featurePoint);
			}
		}

		return landmarks;
	}

	// Computing landmarks (to be drawn later possibly)
	std::vector<cv::Point2d> CalculateLandmarks(cv::Mat img, const cv::Mat_<double>& shape2D)
	{

		int n;
		std::vector<cv::Point2d> landmarks;

		if (shape2D.cols == 2)
		{
			n = shape2D.rows;
		}
		else if (shape2D.cols == 1)
		{
			n = shape2D.rows / 2;
		}

		for (int i = 0; i < n; ++i)
		{
			cv::Point2d featurePoint;
			if (shape2D.cols == 1)
			{
				featurePoint = cv::Point2d(shape2D.at<double>(i), shape2D.at<double>(i + n));
			}
			else
			{
				featurePoint = cv::Point2d(shape2D.at<double>(i, 0), shape2D.at<double>(i, 1));
			}

			landmarks.push_back(featurePoint);
		}

		return landmarks;
	}

	//cv::Rect rect_first;
	//cv::Rect rect_current;

	// Computing landmarks (to be drawn later possibly)
	std::vector<cv::Point2d> CalculateLandmarks(CLNF& clnf_model)
	{

		int idx = clnf_model.patch_experts.GetViewIdx(clnf_model.params_global, 0);

		// Because we only draw visible points, need to find which points patch experts consider visible at a certain orientation
		return CalculateLandmarks(clnf_model.detected_landmarks, clnf_model.patch_experts.visibilities[0][idx]);

	}

	//close или open
	//  template <typename T>
	//  bool plotGraph(cv::Mat &image, std::vector<T>& vals, double morma)
	//  {
	//bool close = true;
	//int y = 100;
	//      cv::Point preview;
	//      cv::Point current;
	//      int step_cols = image.cols / 90;
	//      
	////morma = morma / 2;
	//      cv::line(image, cv::Point(10, /*morma*/y), cv::Point(image.cols - 20, /*morma*/y), cv::Scalar(0, 0, 255), 1);

	//if (vals.size() == 0)
	//	return true;

	//if (morma > vals[vals.size() - 1])
	//	close = true;
	//else
	//	close = false;
	//      
	//for (int i = 1; i < vals.size() - 1; i++)
	//{

	//	double val = vals[i];
	//	int y_current = val > morma ? y + 50 : y - 50;


	//	//std::cout << val << ":" << morma << std::endl;

	//	if (i == 1)
	//	{
	//		double val_prev = vals[i - 1];
	//		
	//		int y_preview = val_prev > morma ? morma + 50: morma - 50;
	//		preview = cv::Point(i * step_cols, y_preview);
	//		current = cv::Point(i * step_cols + 10, y_current);
	//	}
	//	else
	//		current = cv::Point(preview.x + 10, y_current);

	//	cv::line(image, preview, current, cv::Scalar(255, 0, 0), 2);


	//	preview = current;
	//}
	//      
	//      return close;
	//  }

	struct value_position
	{
		double min_val;
		double max_val;
		double current_val;
	};

	std::vector<bool> closeValues;
	void plotGraphNew(cv::Mat &image, bool close)
	{
		closeValues.push_back(close);


		cv::Point preview;
		cv::Point current;
		int step_cols = image.cols / 150;
		int y = 100;


		//morma = morma / 2;
		cv::line(image, cv::Point(10, y), cv::Point(image.cols - 20, y), cv::Scalar(0, 0, 255), 1);

		for (int i = 1; i < closeValues.size() - 1; i++)
		{
			int y_preview = closeValues[i - 1] ? y - 50 : y + 50;
			preview = cv::Point(i * step_cols, y_preview);

			int y_current = closeValues[i] ? y - 50 : y + 50;
			current = cv::Point(preview.x + 10, y_current);

			cv::line(image, preview, current, cv::Scalar(0, 0, 0), 2);

			//cv::line(image, preview, current, cv::Scalar(255, 0, 0), 2);

		}

		

		if (closeValues.size() > 150)
			closeValues.erase(closeValues.begin() + 0);
	}
	
	double coiff_eye_distance = 0.025;

	bool closeStatus(double current, double preview, bool last_status)
	{
		//last_status - предыдущий статус.
		//смотрим, если небыло существенных изменений, то отправляем его
		if (current == preview || abs(current - preview) < 0.0095)
			return last_status;

		if (current > preview)
		{//глаз на окрытие, т.к растояние между век увеличивается
			double val_calc = abs(current - preview);
			if (val_calc > coiff_eye_distance)
				return false;
		}
		else
		{
			double val_calc = abs(preview - current);
			if (val_calc > /*0.035*/coiff_eye_distance)
				return true;
		}


		return false;
	}

	//при смещение головы активируется переменная учета моргания, если < 1000 мсек, значит было моргание в кадре...иначе это ложные срабатывания.
	bool InitHeadCloseStatusEyes = false;
	bool InitHeadCloseStatusEyes_preview = false;

	//при смещение головы, статус моргания, если < 1000 мсек, значит было моргание в кадре...иначе это ложные срабатывания.
	bool BlinkStatusEyes = false;
	bool BlinkStatusEyes_preview = false;


	bool plotGraph1(cv::Mat &image, std::vector<value_position>& vals)
	{
		bool close = true;
		bool close_preview = false;
		int y = 100;
		cv::Point preview;
		cv::Point current;
		int step_cols = image.cols / 90;

		//morma = morma / 2;
		cv::line(image, cv::Point(10, y), cv::Point(image.cols - 20, y), cv::Scalar(0, 0, 255), 1);

		if (vals.size() < 1)
			return false;


		for (int i = 1; i < vals.size() - 1; i++)
		{

			double current_val = vals[i].current_val;
			double preview_val = vals[i - 1].current_val;

			close = closeStatus(current_val, preview_val, close_preview);
			close_preview = close;

			int y_current = close ? y - 50 : y + 50;

			if (preview.x == 0 || preview.y == 0)
			{
				if (vals[i - 1].min_val < 0)
					continue;

				int y_preview = close ? y - 50 : y + 50;

				//close = limit_max > limit_min;

				preview = cv::Point(i * step_cols, y_preview);
			}
			else
			{
				current = cv::Point(preview.x + 10, y_current);
			}


			current = cv::Point(i * step_cols + 10, y_current);

			if (InitHeadCloseStatusEyes || !BlinkStatusEyes)
				cv::line(image, preview, current, cv::Scalar(0, 0, 255), 2);
			else
				cv::line(image, preview, current, cv::Scalar(255, 0, 0), 2);

			preview = current;
		}
	
		std::string text_t = "preview: " + std::to_string(vals[vals.size() - 2].current_val) + " / current: " + std::to_string(vals[vals.size() - 1].current_val);

		cv::putText(image, text_t, cv::Point(50, 50), CV_FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 0, 255));


		return close;

	}

	//template <typename T>
	bool plotGraph(cv::Mat &image, std::vector<value_position>& vals)
	{
		bool close = true;
		int y = 100;
		cv::Point preview;
		cv::Point current;
		int step_cols = image.cols / 90;

		//morma = morma / 2;
		cv::line(image, cv::Point(10, y), cv::Point(image.cols - 20, y), cv::Scalar(0, 0, 255), 1);

		if (vals.size() == 0)
			return true;

		/*if (morma > vals[vals.size() - 1])
		close = true;
		else
		close = false;
		*/


		//clock_t start = clock();

		/*std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();*/

		for (int i = 1; i < vals.size() - 1; i++)
		{

			//value_position val_position = vals[i];

			//if (vals[i].max_val - vals[i].min_val < 10)
			//	return false; //т


			//if (vals[i].min_val < 0)
			//	continue;

			//double val = vals[i];
			//расчитывем к чему ближе.
			//где меньше число, тот и победил
			double limit_max = std::abs(vals[i].max_val - vals[i].current_val);
			double limit_min = std::abs(vals[i].min_val - vals[i].current_val);

			//close = ((double)limit_max - (double)limit_max * 0.5) >(double)limit_min;
			close = ((double)limit_max) >(double)limit_min;


			//int y_current = limit_max > limit_min ? y - 50 : y + 50;
			int y_current = close ? y - 50 : y + 50;


			//close = limit_max > limit_min;


			//std::cout << val << ":" << morma << std::endl;

			if (preview.x == 0 || preview.y == 0)
			{
				if (vals[i - 1].min_val < 0)
					continue;

				double limit_max = std::abs(vals[i - 1].max_val - vals[i - 1].current_val);
				double limit_min = std::abs(vals[i - 1].min_val - vals[i - 1].current_val);

				//close = (double)limit_max >((double)limit_min - (double)limit_min * 0.3);
				close = ((double)limit_max) >(double)limit_min;


				int y_preview = limit_max > limit_min ? y - 50 : y + 50;

				//close = limit_max > limit_min;

				preview = cv::Point(i * step_cols, y_preview);
			}
			else
			{
				current = cv::Point(preview.x + 10, y_current);
			}


			current = cv::Point(i * step_cols + 10, y_current);
			cv::line(image, preview, current, cv::Scalar(255, 0, 0), 2);

			preview = current;
		}

		std::string text_t = "min: " + std::to_string(vals[vals.size() - 1].min_val) + " / max: " + std::to_string(vals[vals.size() - 1].max_val) + " / current: " + std::to_string(vals[vals.size() - 1].current_val);
		cv::putText(image, text_t, cv::Point(50, 50), CV_FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 0, 255));

		//_sleep(0.999);
		//_sleep(2);
		// Execuatable code   


		//std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

		//// integral duration: requires duration_cast
		////auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

		//// fractional duration: no duration_cast needed
		//std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

		///*std::cout << "f() took " << fp_ms.count() << " ms, "
		//	<< "or " << int_ms.count() << " whole milliseconds\n";*/

		//cv::putText(image, std::to_string(fp_ms.count() * 1000), cv::Point(150, 350), CV_FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 0, 255));


		//printf("Time elapsed in ms: %f", elapsed);


		return close;
	}




	bool start = true;
	int frame_count = 1;
	double length_norm_eys;
	//double length_current = 0;

	double val_preview = 0.0;
	double val_current = 0.0;

	/*std::vector<double> numbers_graph;*/

	//min? max
	std::vector<value_position> numbers_graph;
	//double oklonenie;

	// текущий квадрат лица
	//double heightNorm;
	cv::Point heightNorm;


	//int minEyes;
	//int maxEyes;

	bool reinstallFace = false;

	//параметры минимума и максимума между верхним и нижним веком
	struct ParentEyesParam
	{
		double minEyes;
		double maxEyes;

		//время закрытия
		int minTimeInterval;
		int maxTimeInterval;
	};

	//карта состояний (размер высоты рамки лица + параметр минимумум)
	std::map<double, ParentEyesParam> map_parametars;

	bool close_current = false; //текущие состояние глаза
	//bool close_preview = false; //предыдущие состояние глаза

	std::chrono::high_resolution_clock::time_point t1;

	double max_interval_close = 0;
	double min_interval_close = 0;


	std::chrono::high_resolution_clock::time_point time_start_eye_open; //время открятия глаза
																		//double time_eye_close = 0;
	double min_time_eye_open = 0; //время между закрытием глаза, минимум
	double time_interval_eye_close = 0;
	double time_interval_eye_close_temp = 0; //для определения сколько открыт глаз, если значение не меняется (всегда одинаково), обновление открытости глаза не происходит.
	double time_interval_eye_close_result = 0;

	std::chrono::high_resolution_clock::time_point time_disable_motion_face_start = std::chrono::high_resolution_clock::now();;
	




	double current_interval_close = 0;


	bool close_status = false;


	bool refresh_value = false;


	int deviation = 0;


	std::vector<cv::Point> righteye;
	std::vector<cv::Point> lefteye;

	double compute_EAR(std::vector<cv::Point> vec)
	{
		if (vec.size()<6)
			return 0.0;

		double a = cv::norm(cv::Mat(vec[1]), cv::Mat(vec[5]));
		double b = cv::norm(cv::Mat(vec[2]), cv::Mat(vec[4]));
		double c = cv::norm(cv::Mat(vec[0]), cv::Mat(vec[3]));
		//compute EAR
		double ear = (a + b) / (2.0 * c);
		return ear;
	}

	double temp_val = 0;
	cv::Point p;
	
	cv::Ptr<cv::BackgroundSubtractor> bg_model =
		cv::createBackgroundSubtractorMOG2().dynamicCast<cv::BackgroundSubtractor>();



	//начала репорта, через 1 сек обновляем даннеые
	std::chrono::high_resolution_clock::time_point time_start_report = std::chrono::high_resolution_clock::now();
	//blink="01010" initialization=1 sleep=2 date_time=2018-02-23
	//std::string blink = "";
	int initialization = 0;
	int sleep = 0;

	//struct ReportBlink

	//инициализация, параметра для сравнения
	ye_history_blink hist_value;
	ye_reference_blink reference_value;
	
	bool first_start_program = true; //старт программы, проверяем есть ли конфик для сохранения данных
	bool enable_config_file = false; //есть ли конфиг_файл, если его нет то идет анализ 5 минут и далее усредняются значения с сохранение м данных. Даллее отправка данных рах в минуту
	
	//start или current
	void SendJSON(std::string type_message, std::string body_text, std::string mac, cv::Mat img, std::string uid)
	{

		//UID_preview = uid;

		//std::string mak = "USER01_";

		//test
		cv::Point textOrg(10, 10);
        //comment cv::rectangle
//        cv::rectangle(img, cv::Rect(0, 0, img.cols, img.rows), cv::Scalar(0, 0, 0), -1, cv::LINE_8, 0);

		//cv::putText(img, text, textOrg, fontFace, fontScale, cv::Scalar::all(255), thickness, 8);


		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);

		std::stringstream ss;
		ss << std::put_time(&tm, "%Y-%m-%d %H-%M-%S");
		std::string date_time = ss.str() + "_";


		std::ofstream myfile;
		myfile.open("uid.txt", std::ios_base::app);
		myfile << uid + "\n";
		myfile.close();

		//std::string text_message = uid + "_" + type_message + "_" + mac + "_" + date_time + body_text;
		std::string text_message = type_message + "_" + mac + "_" + date_time + body_text;

		std::cout << "---------------------" << std::endl;
		std::cout << text_message << std::endl;

		int count = time_open_eye_history.size();
		time_open_eye_history.clear();

		CURL *curl;
		CURLcode res;

		struct curl_slist *slist1;

		slist1 = NULL;
		std::string x_device = "X-Device-ID: " + uid;
		slist1 = curl_slist_append(slist1, x_device.c_str());


		/* In windows, this will init the winsock stuff */
		curl_global_init(CURL_GLOBAL_ALL);

		/* get a curl handle */
		curl = curl_easy_init();
		if (curl) {
			/* First set the URL that is about to receive our POST. This URL can
			just as well be a https:// URL if that is what should receive the
			data. */
			curl_easy_setopt(curl, CURLOPT_URL, "http://eye-server.woodenshark.com/api/v2/save_log");

			//curl_easy_setopt(curl, CURLOPT_URL, "http://eye-server.woodenshark.com/api/v4/save_log");
			/* Now specify the POST data */
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, text_message.c_str());

			/* Perform the request, res will get the return code */
			res = curl_easy_perform(curl);
			/* Check for errors */
			if (res != CURLE_OK)
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));

			/* always cleanup */
			curl_easy_cleanup(curl);
		}
		curl_global_cleanup();
	}
	

	int send_message_time;
	void Draw(cv::Mat img, cv::Mat graf_image, const cv::Mat_<double>& shape2D, const cv::Mat_<int>& visibilities, cv::Rect rectFace, 
		bool time_logs_refresh, char change_coiff_eye_distance, std::string mac, std::string uid)
	{

		if (first_start_program)
		{
			std::string line;
			std::ifstream myfile("param.txt");
			if (myfile.is_open())
			{
				std::vector<std::string> lines;
				while (getline(myfile, line))
				{
					std::cout << line << '\n';
					lines.push_back(line);
				}
				myfile.close();


				if (lines.size() > 3)
				{
					/*
					2. Как долго в совокупности глаза были открыты (сек)  
					3. как долго в совокупности глаза были закрыты (сек)  
					4. как долго не двигалась голова (сек)"
					*/
					reference_value.blink_count = std::stoi(lines[0]);
					reference_value.time_open_eye = std::stoi(lines[1]);
					reference_value.max_time_blink_count = std::stoi(lines[2]);
					reference_value.disable_motion_face = std::stoi(lines[3]);
				}

				std::string text_message = std::to_string(reference_value.blink_count) + "_" + std::to_string(reference_value.time_open_eye) +
					"_" + std::to_string(reference_value.max_time_blink_count) + "_" + std::to_string(reference_value.disable_motion_face);

				
				//старт_ отправки
				SendJSON("start", text_message, mac, img, uid);
				
				enable_config_file = true;
			}
			else
			{
				std::cout << "Unable to open config file";
			}

			first_start_program = false;
		}
		
		
		
		if (time_logs_refresh)
			refresh_value = true;


		if (change_coiff_eye_distance == '-')
		{
			if (coiff_eye_distance > 0.005)
				coiff_eye_distance = coiff_eye_distance - 0.001;
		}
		else
			if (change_coiff_eye_distance == '+')
				if (coiff_eye_distance < 0.65)
					coiff_eye_distance = coiff_eye_distance + 0.001;


		int n = shape2D.rows / 2;

		//отправляю репртт
		std::chrono::high_resolution_clock::time_point time_current_report = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> time_send_report = time_current_report  - time_start_report;

		
		if (time_send_report.count() > 10000 * 6 * 3 && !enable_config_file) //5 минут и нет конфиг_файла, сохраняеем данные и двигаемся дальше.
		//if (time_send_report.count() > 1000 * 5  && !enable_config_file) //5 минут и нет конфиг_файла, сохраняеем данные и двигаемся дальше.
		{
			//конфиг файла нет, делаем запись параметров и отправляем старт

			//порядок данных
			/*
			int face_motion_count; //сколько раз было движение головой. В нормальной ситуации человек двигает головой, если же нет...человек "залип"
								   //чем меньше, тем хуже
			int blink_count; //сколько раз моргнул., чем чаше тем хуже
			int max_time_blink_count; // максимальное время закрытия глаз, чем больше тем хуже
			int min_blink_close_time_interval;//минимальное время интервал между закрытия глазами, в средннем 5 сек, чем меньше, тем хуже 
			*/

			//расчет средних показателей
			/*int average_face_motion_count = hist_value.face_motion_count / 6;
			int average_blink_count = hist_value.blink_count / 6;

			reference_value.face_motion_count = average_face_motion_count;
			reference_value.blink_count = average_blink_count;
			reference_value.max_time_blink_count = hist_value.max_time_blink_count;
			reference_value.min_blink_close_time_interval = hist_value.min_blink_close_time_interval;*/

			/*
			2. Как долго в совокупности глаза были открыты (сек)
			3. как долго в совокупности глаза были закрыты (сек)
			4. как долго не двигалась голова (сек)"
			*/
			reference_value.blink_count = hist_value.blink_count;
			reference_value.time_open_eye = hist_value.time_open_eye;
			reference_value.max_time_blink_count = hist_value.max_time_blink_count;
			reference_value.disable_motion_face = hist_value.disable_motion_face;

			
			//сохраняем параметры
			std::ofstream myfile;
			myfile.open("param.txt");
			/*
			myfile << reference_value.face_motion_count << "\n";
			myfile << reference_value.blink_count << "\n";
			myfile << reference_value.max_time_blink_count << "\n";
			myfile << reference_value.min_blink_close_time_interval << "\n";
			*/

			myfile << reference_value.blink_count << "\n";
			myfile << reference_value.time_open_eye << "\n";
			myfile << reference_value.max_time_blink_count << "\n";
			myfile << reference_value.disable_motion_face << "\n";


			myfile.close();

			std::string text_message = std::to_string(reference_value.blink_count) + "_" + std::to_string(reference_value.time_open_eye) +
				"_" + std::to_string(reference_value.max_time_blink_count) + "_" + std::to_string(reference_value.disable_motion_face);

			
			//старт_ отправки
			SendJSON("start", text_message, mac, img, uid);

			/*
			hist_value.blink_count = 0;
			hist_value.face_motion_count = 0;
			hist_value.min_blink_close_time_interval = 0;
			hist_value.max_time_blink_count = 0;
			*/

			hist_value.blink_count = 0; //сколько раз моргнул., чем чаше тем хуже (1)
			hist_value.time_open_eye= 0; //2. Как долго в совокупности глаза были открыты (мс)
			hist_value.max_time_blink_count = 0; // максимальное время закрытия глаз, чем больше тем хуже (3)
			hist_value.disable_motion_face = 0; // 4. как долго не двигалась голова (мс)



			enable_config_file = true;
			time_start_report = std::chrono::high_resolution_clock::now();
		}

		
		
		std::string text = std::to_string(send_message_time);
		int fontFace = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
		double fontScale = 1.5;
		int thickness = 2;
		cv::Point textOrg(10, 110);
		cv::putText(img, text, textOrg, fontFace, fontScale, cv::Scalar::all(255), thickness, 8);
		cv::Point textOrg1(10, 145);
		cv::putText(img, std::to_string(hist_value.time_open_eye), textOrg1, fontFace, fontScale, cv::Scalar::all(255), thickness, 8);
		//закрытие
		cv::Point textOrg2(10, 175);
		cv::putText(img, std::to_string(time_interval_eye_close), textOrg2, fontFace, fontScale, cv::Scalar::all(255), thickness, 8);

		//time_spam
		cv::Point textOrg3(100, 75);
		cv::putText(img, std::to_string(time_send_report.count()), textOrg3, fontFace, fontScale, cv::Scalar::all(255), thickness, 8);

		
		if (time_send_report.count() > 10000 * 3 && enable_config_file) //прошло 30 сек и есть конфиг файл, выгружаем данные на сервер.
		//if (time_send_report.count() > 1000 * 3 && enable_config_file) //прошло 30 сек и есть конфиг файл, выгружаем данные на сервер.
		{
			/*if (hist_value.blink_count == 0)
			{
				hist_value.blink_count = 0;
				hist_value.disable_motion_face = 0;
				hist_value.time_open_eye = 0;
				hist_value.max_time_blink_count = 0;

				initialization = 0;
				sleep = 0;

				time_start_report = std::chrono::high_resolution_clock::now();
			}
			else
			{*/
			send_message_time = time_send_report.count();
			//hist_yes

			std::string text_message = "";

			//for (size_t i = 0; i < hist_yes.size(); i++)
			//{
			//	ye_history_blink hist = hist_yes[i];

			//	//USER[001] TIME[27-02-2018 20-52-43] OPEN[1] FACE[1] BLINK[1]

			//	text_message += "USER[" + hist.user + "]TIME[" + hist.date_time + "]OPEN[" + hist.sleep + "]FACE[" + hist.face_detect + "]BLINK[" + hist.blint + "]\n";

			//}


			/*
			auto t = std::time(nullptr);
			auto tm = *std::localtime(&t);

			std::stringstream ss;
			ss << std::put_time(&tm, "%Y-%m-%d %H-%M-%S");
			hist_value.date_time = ss.str();

			*/


			//start_USER01_2018-03-14 20-11-55_165_584_860_1395

			/*
				myfile << reference_value.blink_count << "\n";
		myfile << reference_value.time_open_eye << "\n";
		myfile << reference_value.max_time_blink_count << "\n";
		myfile << reference_value.disable_motion_face << "\n";

			*/
			/*int average_face_motion_count = hist_value.face_motion_count / 6;
			int average_blink_count = hist_value.blink_count / 6;*/

			reference_value.blink_count = hist_value.blink_count;
			reference_value.time_open_eye = hist_value.time_open_eye;
			reference_value.max_time_blink_count = (10000 * 3) - hist_value.time_open_eye; //hist_value.max_time_blink_count;
			reference_value.disable_motion_face = hist_value.disable_motion_face;




			std::string text_message_1 = std::to_string(reference_value.blink_count) + "_" + std::to_string(reference_value.time_open_eye) +
				"_" + std::to_string(reference_value.max_time_blink_count) + "_" + std::to_string(reference_value.disable_motion_face);

		

			//старт_ отправки
			SendJSON("current", text_message_1, mac, img, uid);

			/*
			hist_value.blink_count = 0;
			hist_value.face_motion_count = 0;
			hist_value.min_blink_close_time_interval = 0;
			hist_value.max_time_blink_count = 0;*/

			hist_value.blink_count = 0; //сколько раз моргнул., чем чаше тем хуже (1)
			hist_value.time_open_eye = 0; //2. Как долго в совокупности глаза были открыты (мс)
			hist_value.max_time_blink_count = 0; // максимальное время закрытия глаз, чем больше тем хуже (3)
			hist_value.disable_motion_face = 0; // 4. как долго не двигалась голова (мс)

			//blink = "";
			initialization = 0;
			sleep = 0;

			time_start_report = std::chrono::high_resolution_clock::now();
			//}
		}

		/*bool close = true;*/
		++frame_count;

		// Drawing feature points
		if (n >= 66)
		{
			cv::Point featurePointPreview;

			cv::Point featurePointHorizontRight;
			cv::Point nextFeaturePointHorizontRight;

			cv::Point featurePointHorizontLeft;
			cv::Point nextFeaturePointHorizontLeft;


			cv::Point startM;
			cv::Point endM;

			std::cout << std::endl;
			std::cout << "-------------------" << std::endl;

			for (int i = 0; i < n; ++i)
			{
				if (visibilities.at<int>(i))
				{
					cv::Point featurePoint(cvRound(shape2D.at<double>(i) * (double)draw_multiplier), cvRound(shape2D.at<double>(i + n) * (double)draw_multiplier));

					if (i >= 36 && i < 42)
					{
						p.x = featurePoint.x;
						p.y = featurePoint.y;
						lefteye.push_back(p);

						//std::cout << i << std::endl;
					}

					if (i >= 42 && i < 48)
					{
						p.x = featurePoint.x;
						p.y = featurePoint.y;
						righteye.push_back(p);

						/*std::cout << i << std::endl;*/

					}

					/*for (int b = 36; b < 42; ++b) {
					p.x = shape.part(b).x();
					p.y = shape.part(b).y();
					lefteye.push_back(p);
					}*/



					// A rough heuristic for drawn point size
					int thickness = (int)std::ceil(3.0* ((double)img.cols) / 640.0);
					int thickness_2 = (int)std::ceil(1.0* ((double)img.cols) / 640.0);


					//глаза
					if (i > 35 & i < 48)
					{
						cv::circle(img, featurePoint, 1 * draw_multiplier, cv::Scalar(255, 255, 255), thickness, CV_AA, draw_shiftbits);
						cv::circle(img, featurePoint, 1 * draw_multiplier, cv::Scalar(255, 255, 5), thickness_2, CV_AA, draw_shiftbits);

						int next_point = 0;
						if (i == 36 || i == 42)
						{
							next_point = i + 3;
							cv::Point nextFeaturePoint(cvRound(shape2D.at<double>(next_point) * (double)draw_multiplier), cvRound(shape2D.at<double>(next_point + n) * (double)draw_multiplier));
							nextFeaturePointHorizontRight = nextFeaturePoint;
							featurePointHorizontRight = featurePoint;
							////горизонталь
							//cv::line(img, featurePoint, nextFeaturePoint, cv::Scalar(255, 0, next_point * 2), thickness_2, CV_AA, draw_shiftbits);


							//--
							//cv::Point p1 = cv::Point(5, 0); // "start"
							//cv::Point p2 = cv::Point(10, 0); // "end"

							//// take care with division by zero caused by vertical lines
							//double slope = (p2.y - p1.y) / (double)(p2.x - p1.x);
							//// (0 - 0) / (10 - 5) -> 0/5 -> slope = 0 (that's correct, right?)

							//double length = cv::norm(p2 - p1);

						}
						else if (i == 37 || i == 43)
						{
							next_point = i + 4;
							cv::Point nextFeaturePoint(cvRound(shape2D.at<double>(next_point) * (double)draw_multiplier), cvRound(shape2D.at<double>(next_point + n) * (double)draw_multiplier));

							//cv::line(img, featurePoint, nextFeaturePoint, cv::Scalar(255, 0, next_point * 2), thickness_2, CV_AA, draw_shiftbits);

							/*cv::putText(img, to_string(length), cv::Point(50,50),
							cv::FONT_HERSHEY_COMPLEX_SMALL, 0.9, cv::Scalar(255, 0, 255), 1.2, CV_AA);*/

							//std::cout << length_current << ":" << length_norm_eys << std::endl;

							ParentEyesParam item_map;

							if (frame_count > 10)
							{

								//cv::circle(img, cv::Point(img.cols - 300, 300), 200, cv::Scalar(0, 255, 0), -1, CV_AA, draw_shiftbits);

								if (start)
								{

									//length_current = cv::norm(nextFeaturePoint - featurePoint);
									//length_norm_eys = length_current;

									heightNorm = rectFace.tl();
									start = false;
									reinstallFace = true;
								}
								else
								{
									//length_current = cv::norm(nextFeaturePoint - featurePoint);

									if (i == 37)
									{

										//}
										int x_shoft = std::abs(heightNorm.x - rectFace.tl().x);
										int y_shoft = std::abs(heightNorm.y - rectFace.tl().y);

										//std::cout << "x: " << x_shoft << " y: " << y_shoft << std::endl;

										if (x_shoft > 20 || y_shoft > 20)
										{//сместилось лицо
										 //heightNorm = (double)rectFace.height;

											heightNorm = rectFace.tl();
											reinstallFace = true;

											max_interval_close = 0.0;

										}

										//cv::putText(img, std::to_string(res), cv::Point(img.cols / 2, img.rows / 2 - 50), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255));

										//double res_yey = res * length_current;
										//cv::putText(img, std::to_string(res_yey), cv::Point(img.cols / 2, img.rows / 2 - 70), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255));

									}

								}


								if (reinstallFace)
								{
									//minEyes = length_current;//-1;
									//maxEyes = length_current;

									auto search = map_parametars.find(heightNorm.x + heightNorm.y);
									if (search == map_parametars.end())
									{	//не нашли
										std::pair<double, ParentEyesParam> item;
										item.first = heightNorm.x + heightNorm.y;

										item_map.minEyes = val_current;
										if (val_current < 1.0)
											item_map.maxEyes = val_current;


										//item_map.minEyes = length_current;
										//item_map.maxEyes = length_current;

										item.second = item_map;
										map_parametars.insert(item);
									}

									item_map = map_parametars.at(heightNorm.x + heightNorm.y);
									reinstallFace = false;

									numbers_graph.clear();//очищаем график, т.к он не акутален изза смещения головы

														  //инициализация анализа моргания
									InitHeadCloseStatusEyes = true;
									//val_preview = 0.0;
									//val_current = 0.0;

									return;
								}
								else
									item_map = map_parametars.at(heightNorm.x + heightNorm.y);

								//if (item_map.minEyes > length_current) //если больше при 90% то это минимум
								//	item_map.minEyes = length_current;
								//else
								//	if (item_map.maxEyes < length_current)
								//		item_map.maxEyes = length_current;


								if (item_map.minEyes > val_current) //если больше при 90% то это минимум
									item_map.minEyes = val_current;
								else
									if (item_map.maxEyes < val_current & val_current < 1.00)
										item_map.maxEyes = val_current;


								//update
								map_parametars.at(heightNorm.x + heightNorm.y) = item_map;


							}
							//else
							//{
							//	//if (length_current < cv::norm(nextFeaturePoint - featurePoint))
							//	//	length_current = cv::norm(nextFeaturePoint - featurePoint);

							//	if (val_current < cv::norm(nextFeaturePoint - featurePoint))
							//		val_current = cv::norm(nextFeaturePoint - featurePoint);


							//	cv::circle(img, cv::Point(img.cols - 300, 300), 200, cv::Scalar(0, 0, 255), -1, CV_AA, draw_shiftbits);

							//	//std::cout << length_current << std::endl;
							//}



							/*value_position pos;
							pos.min_val = minEyes;
							pos.max_val = maxEyes;
							pos.current_val = length_current;*/
							if (i >= 43)
							{
								value_position pos;
								pos.min_val = item_map.minEyes;
								pos.max_val = item_map.maxEyes;
								pos.current_val = val_current;
								//pos.current_val = length_current;



								numbers_graph.push_back(pos);
								if (numbers_graph.size() > 90)
									numbers_graph.erase(numbers_graph.begin() + 0);

								//int range[2] = { 0, 100 };
								//close_preview = close_current;

								//=============================================================================
								close_current = plotGraph(graf_image, numbers_graph);
								//close_current = plotGraph1(graf_image, numbers_graph);

								//не анализируем.
								//plotGraph(graf_image, numbers_graph);



								if (close_current)
								{
									if (!close_status)
									{
										t1 = std::chrono::high_resolution_clock::now();
										close_status = true;
									}
								}
								else
								{
									if (close_status)
									{
										std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
										// fractional duration: no duration_cast needed
										std::chrono::duration<double, std::milli> fp_ms = t2 - t1;


										if (fp_ms.count() > 1)
										{
											current_interval_close = fp_ms.count();

										}
										close_status = false;
									}
								}

								////по последнему глазу
								if (close_current)
								{

									//время_открытия_глаза = std::chrono::high_resolution_clock::now();
									//СТАРТ_АНАЛИЗА Детектор времени промежутка между закрытием
									time_start_eye_open = std::chrono::high_resolution_clock::now();

									time_interval_eye_close_result = time_interval_eye_close;
									
									////общеее время открытого глаза;
									if (time_interval_eye_close_result > 1000)
									{
										if (time_interval_eye_close_temp != time_interval_eye_close_result)
										{
											hist_value.time_open_eye += time_interval_eye_close_result;
											time_interval_eye_close_temp = time_interval_eye_close_result;
										}
										time_open_eye_history.push_back(time_interval_eye_close_result);
									}
									double oclonenie_t = current_interval_close / 250.0;
									if (oclonenie_t > 1.00)
									{
										oclonenie_t = (oclonenie_t * 100) - 100;

										//deviation += oclonenie_t / 10;

										double oclonenie_interval_temp = time_interval_eye_close_result / 3000;
										oclonenie_interval_temp = (oclonenie_interval_temp * 100) - 100;
										deviation += oclonenie_interval_temp / 5;
									}

									//}
								}
								else
								{
									//время_закрытия_глаза = std::chrono::high_resolution_clock::now();
									std::chrono::high_resolution_clock::time_point time_END_eye_open = std::chrono::high_resolution_clock::now();
									//расчет разницы

									std::chrono::duration<double, std::milli> time_eye_open_ms = time_END_eye_open - time_start_eye_open;


									if (time_eye_open_ms.count() > 1)
									{
										time_interval_eye_close = time_eye_open_ms.count();
									
										//общеее время открытого глаза;
										/*hist_value.time_open_eye = time_interval_eye_close_result;
										time_open_eye_history.push_back(time_interval_eye_close_result);*/
									}
								}


								if (current_interval_close > max_interval_close/* && refresh_value*/)
								{
									max_interval_close = current_interval_close;


								}
								cv::line(graf_image, cv::Point(0, 180), cv::Point(graf_image.cols, 180), CV_RGB(0, 0, 0), 1.4);



								cv::putText(graf_image, "eye closing time", cv::Point(5, 200), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 0), 1.7);
								cv::line(graf_image, cv::Point(5, 210), cv::Point(20, 210), CV_RGB(0, 0, 0), 1.4);
								cv::line(graf_image, cv::Point(145, 210), cv::Point(160, 210), CV_RGB(0, 0, 0), 1.4);

								cv::putText(graf_image, "last", cv::Point(5, 230), CV_FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 0.7);
								cv::putText(graf_image, std::to_string((int)current_interval_close), cv::Point(200, 230), CV_FONT_HERSHEY_DUPLEX, 0.7, cv::Scalar(0, 0, 0));

								cv::putText(graf_image, "max ", cv::Point(5, 250), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 0.7);
								cv::putText(graf_image, std::to_string((int)max_interval_close), cv::Point(200, 250), CV_FONT_HERSHEY_DUPLEX, 0.7, cv::Scalar(0, 0, 0));

								cv::putText(graf_image, "norma", cv::Point(5, 270), CV_FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0));
								cv::putText(graf_image, "250", cv::Point(200, 270), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 0));


								cv::putText(graf_image, "deviation", cv::Point(5, 290), CV_FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255));
								double oclonenie = current_interval_close / 250.0;
								if (oclonenie > 1.00)
								{
									oclonenie = (oclonenie * 100) - 100;
									cv::putText(graf_image, std::to_string((int)oclonenie) + "%", cv::Point(200, 290), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 255));

									/*deviation += oclonenie / 10;

									double oclonenie_interval_temp = time_interval_eye_close_result / 3000;
									oclonenie_interval_temp = (oclonenie_interval_temp * 100) - 100;
									deviation += oclonenie / 5;*/

								}
								else
								{
									cv::putText(graf_image, " ", cv::Point(200, 290), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 255));
									deviation = 0;
								}

								/*cv::putText(graf_image, std::to_string((int)current_interval_close), cv::Point(250, 310), CV_FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255));
								cv::putText(graf_image, std::to_string((int)max_interval_close), cv::Point(250, 340), CV_FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255));
								*/


								//time_interval_eye_close - интервал между закрытием глаза
								cv::putText(graf_image, "eye-closing interval", cv::Point(5, 330), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 0), 1.7);
								cv::line(graf_image, cv::Point(5, 340), cv::Point(20, 340), CV_RGB(0, 0, 0), 1.4);
								cv::line(graf_image, cv::Point(145, 340), cv::Point(160, 340), CV_RGB(0, 0, 0), 1.4);

								cv::putText(graf_image, "last", cv::Point(5, 360), CV_FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 0.7);
								cv::putText(graf_image, std::to_string((int)time_interval_eye_close), cv::Point(200, 360), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(255, 0, 0));

								cv::putText(graf_image, "min ", cv::Point(5, 380), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 0.7);
								cv::putText(graf_image, std::to_string((int)time_interval_eye_close_result), cv::Point(200, 380), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(255, 0, 0));

								cv::putText(graf_image, "norma", cv::Point(5, 400), CV_FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0));
								cv::putText(graf_image, "3000", cv::Point(200, 400), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 0));

								cv::putText(graf_image, "deviation", cv::Point(5, 420), CV_FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255));
								double oclonenie_interval = time_interval_eye_close_result / 3000;

								if (oclonenie_interval < 1.00)
								{
									oclonenie_interval = (oclonenie_interval * 100) - 100;
									cv::putText(graf_image, std::to_string(abs((int)oclonenie_interval)) + "%", cv::Point(200, 420), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 255));

									//deviation += oclonenie / 5;

								}
								else
								{
									cv::putText(graf_image, " ", cv::Point(200, 420), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 255));
									//deviation = 0;
								}



								////горизонталь на лице - глаза
								//if (close_current)
								//{
								//	cv::line(img, featurePointHorizontLeft, nextFeaturePointHorizontLeft, cv::Scalar(255, 255, 255), /*thickness_2*/10, CV_AA, draw_shiftbits);
								//	cv::line(img, featurePointHorizontRight, nextFeaturePointHorizontRight, cv::Scalar(255, 255, 255), /*thickness_2*/10, CV_AA, draw_shiftbits);
								//}
								//else
								//{
								//	cv::Point centerLeft((featurePointHorizontLeft.x + nextFeaturePointHorizontLeft.x) / 2, featurePointHorizontLeft.y);
								//	cv::circle(img, centerLeft, 150, cv::Scalar(0, 0, 255), 1, CV_AA, draw_shiftbits);

								//	cv::Point centerRight((featurePointHorizontRight.x + nextFeaturePointHorizontRight.x) / 2, featurePointHorizontRight.y);
								//	cv::circle(img, centerRight, 150, cv::Scalar(0, 0, 255), 1, CV_AA, draw_shiftbits);
								//}

								//cv::putText(img, close_current ? "CLOSE" : "OPEN", cv::Point(img.cols - 100, img.rows - 40),
								//	CV_FONT_HERSHEY_SIMPLEX, 0.8, close_current ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0), 2.5);

								cv::putText(graf_image, "Face Detection:", cv::Point(350, 210), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 0), 1.7);
								cv::putText(graf_image, "OK", cv::Point(520, 210),
									CV_FONT_HERSHEY_DUPLEX, 0.6, close_current ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 255, 0), 1.7);

								cv::putText(graf_image, "Mouth Detection:", cv::Point(350, 240), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 0), 1.7);
								cv::putText(graf_image, "OK", cv::Point(520, 240),
									CV_FONT_HERSHEY_DUPLEX, 0.6, close_current ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 255, 0), 1.7);


								if (refresh_value)
								{
									temp_val = std::abs(deviation);
									deviation = 1;
									refresh_value = false;
								}

								//Deviation
								//if (deviation < 0)
								//	deviation = 0;
								cv::putText(graf_image, "Deviation:", cv::Point(350, 300), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 0), 1.7);
								cv::putText(graf_image, std::to_string((int)temp_val) + " %", cv::Point(520, 300),
									CV_FONT_HERSHEY_DUPLEX, 0.7, deviation < 20 ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255), 1.7);


								if (temp_val > 25)
								{
									++sleep;
								}
								/*if (temp_val > 25)
								{
									cv::putText(graf_image, "YOU ARE TIRED", cv::Point(350, 350), cv::FONT_HERSHEY_DUPLEX, 1.1, cv::Scalar(0, 0, 255), 0.7, CV_AA);
								}*/

								//cv::circle(img, cv::Point(img.cols - 35, img.rows - 40), 30, close_current ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 255), -1, CV_AA);
							}
							else
							{
								featurePointHorizontLeft = featurePointHorizontRight;
								nextFeaturePointHorizontLeft = nextFeaturePointHorizontRight;
							}
						}


						featurePointPreview = featurePoint;
					}



					//рот


					if (i > 48)
					{
						cv::Mat fgmask;

						/*detect_motion*/
						//update the model
						cv::Mat motion = img.clone();
						resize(motion, motion, cv::Size(120, 120));

						bg_model->apply(motion, fgmask, -15);
						if (true)
						{
							cv::GaussianBlur(fgmask, fgmask, cv::Size(21, 21), 3.5, 3.5);
							cv::threshold(fgmask, fgmask, 10, 255, cv::THRESH_BINARY);
						}

						/*fgimg = Scalar::all(0);
						img.copyTo(fgimg, fgmask);

						Mat bgimg;
						bg_model->getBackgroundImage(bgimg);*/

						//imshow("image", img);
						//imshow("foreground mask", fgmask);

						/*imshow("foreground image", fgimg);
						if (!bgimg.empty())
						imshow("mean background image", bgimg);
						*/
						std::vector<std::vector<cv::Point> > contours;
						std::vector<cv::Vec4i > hierarchy;

						cv::findContours(fgmask, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);

						double aray = 0.0;
						for (unsigned int i = 0; i < contours.size(); i++)
						{
							aray += contourArea(contours[i]);
						}
						//std::cout << " Area: " << aray << std::endl;


						//Compute Eye aspect ration for eyes
						double right_ear = compute_EAR(righteye);
						double left_ear = compute_EAR(lefteye);


						//detect value
						val_current = (right_ear + left_ear) / 2;
						
						//print value
						/*
						[ID Машины] - 1
						[Время] - время
						[Координаты как сейчас], [Face=0/1; Blink=0/1], [Hotkey_Tired=0/1]
						[Ссылка на фото] - тут не понятно. )))

						// [Face=0/1; Blink=0/1], [Hotkey_Tired=0/1]

						*/
						//if (frame_count % 3 == 0)
						//{

						BlinkStatusEyes = aray <= 4000.0;

							std::cout << std::endl;
							//[ID Машины] - 1
							std::cout << "USER001" << std::endl;
							//TIME
							auto t = std::time(nullptr);
							auto tm = *std::localtime(&t);
							std::cout << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl;

							std::cout << "1: " << val_preview << " :: " << val_current << std::endl;

							std::string face_detect = "0";
							if (BlinkStatusEyes)
								face_detect = "1";
							std::cout << "FACE = " << face_detect << std::endl;

							std::string BLINK_DETERMINED = "1";
							if (InitHeadCloseStatusEyes)
								BLINK_DETERMINED = "0";
							std::cout << "BLINK = " << BLINK_DETERMINED << std::endl;

						

							

						//}
						//смотриитм разность значений, если значение увеличивает свое значение - глаз открыт.
						// усли уменьшает знаечние - глаз закрыт.
						//шаг оптимальный - 0.2
						//шащ более 0.2, признак изменения статуса.

						//aray >= 1000.0 - есть движение в кадре.
						//if (val_current == val_preview || abs(val_current - val_preview) < 0.0025/* || aray >= 2000.0*/)
						//if (val_current == val_preview || abs(val_current - val_preview) < 0.015/* || aray >= 2000.0*/)

						//детектор лица и его движения

						

						if (BlinkStatusEyes)
							cv::putText(graf_image, "FIXED FACE = TRUE", cv::Point(350, 350), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 255, 0), 1.7);
						else
							cv::putText(graf_image, "FIXED FACE = FALSE", cv::Point(350, 350), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 255), 1.7);
						/*if (aray >= 2000.0)
							cv::putText(graf_image, "FIXED FACE = FALSE", cv::Point(350, 350), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 255), 1.7);
						else
							cv::putText(graf_image, "FIXED FACE = TRUE", cv::Point(350, 350), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 250, 0), 1.7);*/

						
						
						if (InitHeadCloseStatusEyes_preview != InitHeadCloseStatusEyes)
						{
							//для отчета
							++initialization;
						}

						//идет ли анализ закрфытия глаз, после 1 секунды коллирбровки после движения.	
						if (InitHeadCloseStatusEyes)
						{
							//BLINK DETERMINED = TRUE/FALSE
							cv::putText(graf_image, "BLINK DETERMINED = FALSE", cv::Point(350, 380), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 255), 1.7);
						}
						else
							cv::putText(graf_image, "BLINK DETERMINED = TRUE", cv::Point(350, 380), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 250, 0), 1.7);

						InitHeadCloseStatusEyes_preview = InitHeadCloseStatusEyes;

						//coiff_eye_distance
						cv::putText(graf_image, "coiff_eye_distance:", cv::Point(350, 410), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 0), 1.7);
						cv::putText(graf_image, std::to_string(coiff_eye_distance), cv::Point(540, 410),
							CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 255), 1.7);

						bool close_current_new = false;
						if (aray <= 4000.0) //чувсвительнсть
						{
							if (InitHeadCloseStatusEyes)
							{
								if ((int)time_interval_eye_close < 1000)
									close_current_new = false;
								else
								{
									InitHeadCloseStatusEyes = false;
								}
							}
							else
							{
								if (val_current == val_preview || abs(val_current - val_preview) < 0.005)
									close_current_new = false;
								else
								{
									if (val_current > val_preview)
									{//глаз на окрытие, т.к растояние между век увеличивается
										double val_calc = abs(val_current - val_preview);
										if (val_calc > coiff_eye_distance)
											close_current_new = false;
									}
									else
									{
										double val_calc = abs(val_preview - val_current);
										if (val_calc > /*0.035*/coiff_eye_distance)
											close_current_new = true;
									}
								}
							}
						}

						std::stringstream ss;
						ss << std::put_time(&tm, "%Y-%m-%d %H-%M-%S");

					/*	ye_history hist_value;
						hist_value.user = "USER001";
						hist_value.date_time = ss.str();
						hist_value.sleep = close_current_new ? "0" : "1";
						hist_value.blint = InitHeadCloseStatusEyes ? "0" : "1";
						hist_value.face_detect = BlinkStatusEyes ? "0" : "1";*/

						

						////if the avarage eye aspect ratio of lef and right eye less than 0.2, the status is sleeping.
						//if ((right_ear + left_ear) / 2 < 0.32)
						//	close_current = true;
						////cv::putText(img, "CLOSE", cv::Point(50, 300), CV_FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 1.7);
						//else
						//	close_current = false;
						////cv::putText(img, "NOT_CLOSE", cv::Point(50, 300), CV_FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 255, 255), 1.7);

						//std::cout << (right_ear + left_ear) / 2 << std::endl;
						std::cout << std::endl;

						//if (val_preview == 0.0)
						val_preview = val_current;

						cv::putText(graf_image, "Eyes:", cv::Point(350, 270), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 0), 1.7);
						cv::putText(graf_image, close_current_new ? "CLOSE" : "OPEN", cv::Point(520, 270),
							CV_FONT_HERSHEY_DUPLEX, 0.6, close_current_new ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0), 1.7);

						//для анализа
						//blink += close_current_new ? "0" : "1";

						//анализ_отчет
						hist_value.blink_count += close_current_new ? 1 : 0; //коли-во морганий
						if (aray <= 4000.0)
						{
							//как долго не двигалась голова (максимальное значение).
							/*if(hist_value.disable_motion_face == 0)
								time_disable_motion_face_start = std::chrono::high_resolution_clock::now();
							else
							{*/
							std::chrono::high_resolution_clock::time_point time_disable_end = std::chrono::high_resolution_clock::now();
							std::chrono::duration<double, std::milli> interval = time_disable_end - time_disable_motion_face_start;

							if (hist_value.disable_motion_face < interval.count())
							{
								hist_value.disable_motion_face = interval.count();
							}

							time_disable_motion_face_start = std::chrono::high_resolution_clock::now();
							//}
							//++hist_value.face_motion_count;
						}
						hist_value.max_time_blink_count = max_interval_close;
						//hist_value.min_blink_close_time_interval = time_interval_eye_close_result;

										



						//горизонталь на лице - глаза
						if (close_current_new)
						{
							cv::line(img, featurePointHorizontLeft, nextFeaturePointHorizontLeft, cv::Scalar(255, 255, 255), /*thickness_2*/10, CV_AA, draw_shiftbits);
							cv::line(img, featurePointHorizontRight, nextFeaturePointHorizontRight, cv::Scalar(255, 255, 255), /*thickness_2*/10, CV_AA, draw_shiftbits);
						}
						else
						{
							cv::Point centerLeft((featurePointHorizontLeft.x + nextFeaturePointHorizontLeft.x) / 2, featurePointHorizontLeft.y);
							cv::circle(img, centerLeft, 150, cv::Scalar(0, 0, 255), 1, CV_AA, draw_shiftbits);

							cv::Point centerRight((featurePointHorizontRight.x + nextFeaturePointHorizontRight.x) / 2, featurePointHorizontRight.y);
							cv::circle(img, centerRight, 150, cv::Scalar(0, 0, 255), 1, CV_AA, draw_shiftbits);
						}

						//detect close
						plotGraphNew(graf_image, close_current_new);


						////cv::circle(img, featurePoint, 1 * draw_multiplier, cv::Scalar(0, 0, 250), thickness, CV_AA, draw_shiftbits);
						////cv::circle(img, featurePoint, 1 * draw_multiplier, cv::Scalar(0, 0, 0), thickness_2, CV_AA, draw_shiftbits);

						//if (i == 60)
						//{
						//	startM = featurePoint;
						//	cv::circle(img, featurePoint, 1 * draw_multiplier, cv::Scalar(0, 0, 0), thickness, CV_AA, draw_shiftbits);

						//	cv::line(img, endM, startM, cv::Scalar(255, 255, 255), 1, CV_AA, draw_shiftbits);

						//}
						//if (i == 54)
						//{
						//	endM = featurePoint;
						//	cv::circle(img, featurePoint, 1 * draw_multiplier, cv::Scalar(0, 0, 0), thickness, CV_AA, draw_shiftbits);
						//}
					}



				}
			}
		}
		/*
		else if(n == 28) // drawing eyes
		{
		for( int i = 0; i < n; ++i)
		{
		cv::Point featurePoint((int)shape2D.at<double>(i), (int)shape2D.at<double>(i +n));

		// A rough heuristic for drawn point size
		int thickness = 1.0;
		int thickness_2 = 1.0;

		int next_point = i + 1;
		if(i == 7)
		next_point = 0;
		if(i == 19)
		next_point = 8;
		if(i == 27)
		next_point = 20;

		cv::Point nextFeaturePoint((int)shape2D.at<double>(next_point), (int)shape2D.at<double>(next_point+n));
		if( i < 8 || i > 19)
		cv::line(img, featurePoint, nextFeaturePoint, cv::Scalar(255, 0, 0), thickness_2);
		else
		cv::line(img, featurePoint, nextFeaturePoint, cv::Scalar(0, 0, 255), thickness_2);

		//cv::circle(img, featurePoint, 1, Scalar(0,255,0), thickness);
		//cv::circle(img, featurePoint, 1, Scalar(0,0,255), thickness_2);


		}
		}
		else if(n == 6)
		{
		for( int i = 0; i < n; ++i)
		{
		cv::Point featurePoint((int)shape2D.at<double>(i), (int)shape2D.at<double>(i +n));

		// A rough heuristic for drawn point size
		int thickness = 1.0;
		int thickness_2 = 1.0;

		//cv::circle(img, featurePoint, 1, Scalar(0,255,0), thickness);
		//cv::circle(img, featurePoint, 1, Scalar(0,0,255), thickness_2);

		int next_point = i + 1;
		if(i == 5)
		next_point = 0;

		cv::Point nextFeaturePoint((int)shape2D.at<double>(next_point), (int)shape2D.at<double>(next_point+n));
		cv::line(img, featurePoint, nextFeaturePoint, cv::Scalar(255, 0, 0), thickness_2);
		}
		}
		*/

		////Compute Eye aspect ration for eyes
		//double right_ear = compute_EAR(righteye);
		//double left_ear = compute_EAR(lefteye);

		////if the avarage eye aspect ratio of lef and right eye less than 0.2, the status is sleeping.
		//if ((right_ear + left_ear) / 2 < 0.31)
		//	close_current = true;
		////cv::putText(img, "CLOSE", cv::Point(50, 300), CV_FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 1.7);
		//else
		//	close_current = false;
		//	//cv::putText(img, "NOT_CLOSE", cv::Point(50, 300), CV_FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 255, 255), 1.7);


		//cv::putText(graf_image, "Eyes:", cv::Point(350, 270), CV_FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 0), 1.7);
		//cv::putText(graf_image, close_current ? "CLOSE" : "OPEN", cv::Point(520, 270),
		//	CV_FONT_HERSHEY_DUPLEX, 0.6, close_current ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0), 1.7);

		////горизонталь на лице - глаза
		//if (close_current)
		//{
		//	cv::line(img, featurePointHorizontLeft, nextFeaturePointHorizontLeft, cv::Scalar(255, 255, 255), /*thickness_2*/10, CV_AA, draw_shiftbits);
		//	cv::line(img, featurePointHorizontRight, nextFeaturePointHorizontRight, cv::Scalar(255, 255, 255), /*thickness_2*/10, CV_AA, draw_shiftbits);
		//}
		//else
		//{
		//	cv::Point centerLeft((featurePointHorizontLeft.x + nextFeaturePointHorizontLeft.x) / 2, featurePointHorizontLeft.y);
		//	cv::circle(img, centerLeft, 150, cv::Scalar(0, 0, 255), 1, CV_AA, draw_shiftbits);

		//	cv::Point centerRight((featurePointHorizontRight.x + nextFeaturePointHorizontRight.x) / 2, featurePointHorizontRight.y);
		//	cv::circle(img, centerRight, 150, cv::Scalar(0, 0, 255), 1, CV_AA, draw_shiftbits);
		//}





		righteye.clear();
		lefteye.clear();

	}

	// Drawing landmarks on a face image
	void Draw(cv::Mat img, const cv::Mat_<double>& shape2D)
	{

		int n;

		if (shape2D.cols == 2)
		{
			n = shape2D.rows;
		}
		else if (shape2D.cols == 1)
		{
			n = shape2D.rows / 2;
		}

		for (int i = 0; i < n; ++i)
		{
			cv::Point featurePoint;
			if (shape2D.cols == 1)
			{
				featurePoint = cv::Point((int)shape2D.at<double>(i), (int)shape2D.at<double>(i + n));
			}
			else
			{
				featurePoint = cv::Point((int)shape2D.at<double>(i, 0), (int)shape2D.at<double>(i, 1));
			}
			int thickness = (int)std::ceil(5.0* ((double)img.cols) / 640.0);
			int thickness_2 = (int)std::ceil(1.5* ((double)img.cols) / 640.0);

			//cv::circle(img, featurePoint, 1, cv::Scalar(0,0,255), thickness);
			//cv::circle(img, featurePoint, 1, cv::Scalar(255,0,0), thickness_2);

		}

	}

	
	// Drawing detected landmarks on a face image
	void Draw(cv::Mat img, cv::Mat graf_image, const CLNF& clnf_model, cv::Rect rectFace, bool time_logs_refresh, char change_coiff_eye_distance, std::string mac, std::string uid)
	{

		int idx = clnf_model.patch_experts.GetViewIdx(clnf_model.params_global, 0);

		// Because we only draw visible points, need to find which points patch experts consider visible at a certain orientation
		if (clnf_model.patch_experts.visibilities.size() > 0)
		{
			//if (start)
			//{
			//	//запоминаем какакой был квадрат лица, для расчета пропорций. Квадрат может меняться или увеличиваться в зависимости от нахождения водителя.
			//	rect_first = clnf_model.GetBoundingBox();
			//}
			//else
			//	rect_current = clnf_model.GetBoundingBox();

			cv::Rect rectT = clnf_model.GetBoundingBox();
			//clnf_model.pdm.CalcBoundingBox(rectT, clnf_model.params_global, clnf_model.params_local);


			Draw(img, graf_image, clnf_model.detected_landmarks, clnf_model.patch_experts.visibilities[0][idx], /*rectFace*/rectT, time_logs_refresh, change_coiff_eye_distance, mac, uid);

			//// If the model has hierarchical updates draw those too
			//for (size_t i = 0; i < clnf_model.hierarchical_models.size(); ++i)
			//{
			//	if (clnf_model.hierarchical_models[i].pdm.NumberOfPoints() != clnf_model.hierarchical_mapping[i].size())
			//	{
			//		Draw(img, clnf_model.hierarchical_models[i]);
			//	}
			//}
		}
	}

	void DrawLandmarks(cv::Mat img, std::vector<cv::Point> landmarks)
	{
		for (cv::Point p : landmarks)
		{
			// A rough heuristic for drawn point size
			int thickness = (int)std::ceil(5.0* ((double)img.cols) / 640.0);
			int thickness_2 = (int)std::ceil(1.5* ((double)img.cols) / 640.0);

			cv::circle(img, p, 1, cv::Scalar(0, 0, 255), thickness);
			cv::circle(img, p, 1, cv::Scalar(255, 0, 0), thickness_2);
		}

	}

	//===========================================================================
	// Angle representation conversion helpers
	//===========================================================================

	// Using the XYZ convention R = Rx * Ry * Rz, left-handed positive sign
	cv::Matx33d Euler2RotationMatrix(const cv::Vec3d& eulerAngles)
	{
		cv::Matx33d rotation_matrix;

		double s1 = sin(eulerAngles[0]);
		double s2 = sin(eulerAngles[1]);
		double s3 = sin(eulerAngles[2]);

		double c1 = cos(eulerAngles[0]);
		double c2 = cos(eulerAngles[1]);
		double c3 = cos(eulerAngles[2]);

		rotation_matrix(0, 0) = c2 * c3;
		rotation_matrix(0, 1) = -c2 *s3;
		rotation_matrix(0, 2) = s2;
		rotation_matrix(1, 0) = c1 * s3 + c3 * s1 * s2;
		rotation_matrix(1, 1) = c1 * c3 - s1 * s2 * s3;
		rotation_matrix(1, 2) = -c2 * s1;
		rotation_matrix(2, 0) = s1 * s3 - c1 * c3 * s2;
		rotation_matrix(2, 1) = c3 * s1 + c1 * s2 * s3;
		rotation_matrix(2, 2) = c1 * c2;

		return rotation_matrix;
	}

	// Using the XYZ convention R = Rx * Ry * Rz, left-handed positive sign
	cv::Vec3d RotationMatrix2Euler(const cv::Matx33d& rotation_matrix)
	{
		double q0 = sqrt(1 + rotation_matrix(0, 0) + rotation_matrix(1, 1) + rotation_matrix(2, 2)) / 2.0;
		double q1 = (rotation_matrix(2, 1) - rotation_matrix(1, 2)) / (4.0*q0);
		double q2 = (rotation_matrix(0, 2) - rotation_matrix(2, 0)) / (4.0*q0);
		double q3 = (rotation_matrix(1, 0) - rotation_matrix(0, 1)) / (4.0*q0);

		double t1 = 2.0 * (q0*q2 + q1*q3);

		double yaw = asin(2.0 * (q0*q2 + q1*q3));
		double pitch = atan2(2.0 * (q0*q1 - q2*q3), q0*q0 - q1*q1 - q2*q2 + q3*q3);
		double roll = atan2(2.0 * (q0*q3 - q1*q2), q0*q0 + q1*q1 - q2*q2 - q3*q3);

		return cv::Vec3d(pitch, yaw, roll);
	}

	cv::Vec3d Euler2AxisAngle(const cv::Vec3d& euler)
	{
		cv::Matx33d rotMatrix = LandmarkDetector::Euler2RotationMatrix(euler);
		cv::Vec3d axis_angle;
		cv::Rodrigues(rotMatrix, axis_angle);
		return axis_angle;
	}

	cv::Vec3d AxisAngle2Euler(const cv::Vec3d& axis_angle)
	{
		cv::Matx33d rotation_matrix;
		cv::Rodrigues(axis_angle, rotation_matrix);
		return RotationMatrix2Euler(rotation_matrix);
	}

	cv::Matx33d AxisAngle2RotationMatrix(const cv::Vec3d& axis_angle)
	{
		cv::Matx33d rotation_matrix;
		cv::Rodrigues(axis_angle, rotation_matrix);
		return rotation_matrix;
	}

	cv::Vec3d RotationMatrix2AxisAngle(const cv::Matx33d& rotation_matrix)
	{
		cv::Vec3d axis_angle;
		cv::Rodrigues(rotation_matrix, axis_angle);
		return axis_angle;
	}

	//===========================================================================

	//============================================================================
	// Face detection helpers
	//============================================================================
	bool DetectFaces(std::vector<cv::Rect_<double> >& o_regions, const cv::Mat_<uchar>& intensity)
	{
		cv::CascadeClassifier classifier("classifiers/haarcascade_frontalface_alt.xml");
		if (classifier.empty())
		{
			std::cout << "Couldn't load the Haar cascade classifier" << std::endl;
			return false;
		}
		else
		{
			return DetectFaces(o_regions, intensity, classifier);
		}

	}

	bool DetectFaces(std::vector<cv::Rect_<double> >& o_regions, const cv::Mat_<uchar>& intensity, cv::CascadeClassifier& classifier)
	{

		std::vector<cv::Rect> face_detections;
		classifier.detectMultiScale(intensity, face_detections, 1.2, 2, 0, cv::Size(50, 50), cv::Size(intensity.size().width * 0.8, intensity.size().height * 0.8));

		/*std::cout << "==========================================================" << std::endl;
		std::cout << "test" << std::endl;
		std::cout << "==========================================================" << std::endl;*/

		// Convert from int bounding box do a double one with corrections
		o_regions.resize(face_detections.size());

		for (size_t face = 0; face < o_regions.size(); ++face)
		{
			// OpenCV is overgenerous with face size and y location is off
			// CLNF detector expects the bounding box to encompass from eyebrow to chin in y, and from cheeck outline to cheeck outline in x, so we need to compensate

			// The scalings were learned using the Face Detections on LFPW, Helen, AFW and iBUG datasets, using ground truth and detections from openCV

			// Correct for scale
			o_regions[face].width = face_detections[face].width * 0.8924;
			o_regions[face].height = face_detections[face].height * 0.8676;

			// Move the face slightly to the right (as the width was made smaller)
			o_regions[face].x = face_detections[face].x + 0.0578 * face_detections[face].width;
			// Shift face down as OpenCV Haar Cascade detects the forehead as well, and we're not interested
			o_regions[face].y = face_detections[face].y + face_detections[face].height * 0.2166;


		}
		return o_regions.size() > 0;
	}

	bool DetectSingleFace(cv::Rect_<double>& o_region, const cv::Mat_<uchar>& intensity_image, cv::CascadeClassifier& classifier, cv::Point preference)
	{
		// The tracker can return multiple faces
		std::vector<cv::Rect_<double> > face_detections;

		bool detect_success = LandmarkDetector::DetectFaces(face_detections, intensity_image, classifier);

		if (detect_success)
		{

			bool use_preferred = (preference.x != -1) && (preference.y != -1);

			if (face_detections.size() > 1)
			{
				// keep the closest one if preference point not set
				double best = -1;
				int bestIndex = -1;
				for (size_t i = 0; i < face_detections.size(); ++i)
				{
					double dist;
					bool better;

					if (use_preferred)
					{
						dist = sqrt((preference.x) * (face_detections[i].width / 2 + face_detections[i].x) +
							(preference.y) * (face_detections[i].height / 2 + face_detections[i].y));
						better = dist < best;
					}
					else
					{
						dist = face_detections[i].width;
						better = face_detections[i].width > best;
					}

					// Pick a closest face to preffered point or the biggest face
					if (i == 0 || better)
					{
						bestIndex = i;
						best = dist;
					}
				}

				o_region = face_detections[bestIndex];

			}
			else
			{
				o_region = face_detections[0];
			}

		}
		else
		{
			// if not detected
			o_region = cv::Rect_<double>(0, 0, 0, 0);
		}
		return detect_success;
	}

	//bool DetectFacesHOG(vector<cv::Rect_<double> >& o_regions, const cv::Mat_<uchar>& intensity, std::vector<double>& confidences)
	//{
	//    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();

	//    return DetectFacesHOG(o_regions, intensity, detector, confidences);

	//}

	//bool DetectFacesHOG(vector<cv::Rect_<double> >& o_regions, const cv::Mat_<uchar>& intensity, dlib::frontal_face_detector& detector, std::vector<double>& o_confidences)
	//{

	//    cv::Mat_<uchar> upsampled_intensity;

	//    double scaling = 1.3;

	//    cv::resize(intensity, upsampled_intensity, cv::Size((int)(intensity.cols * scaling), (int)(intensity.rows * scaling)));

	//    dlib::cv_image<uchar> cv_grayscale(upsampled_intensity);

	//    std::vector<dlib::full_detection> face_detections;
	//    detector(cv_grayscale, face_detections, -0.2);

	//    // Convert from int bounding box do a double one with corrections
	//    o_regions.resize(face_detections.size());
	//    o_confidences.resize(face_detections.size());

	//    for( size_t face = 0; face < o_regions.size(); ++face)
	//    {
	//        // CLNF expects the bounding box to encompass from eyebrow to chin in y, and from cheeck outline to cheeck outline in x, so we need to compensate

	//        // The scalings were learned using the Face Detections on LFPW and Helen using ground truth and detections from the HOG detector

	//        // Move the face slightly to the right (as the width was made smaller)
	//        o_regions[face].x = (face_detections[face].rect.get_rect().tl_corner().x() + 0.0389 * face_detections[face].rect.get_rect().width())/scaling;
	//        // Shift face down as OpenCV Haar Cascade detects the forehead as well, and we're not interested
	//        o_regions[face].y = (face_detections[face].rect.get_rect().tl_corner().y() + 0.1278 * face_detections[face].rect.get_rect().height())/scaling;

	//        // Correct for scale
	//        o_regions[face].width = (face_detections[face].rect.get_rect().width() * 0.9611)/scaling;
	//        o_regions[face].height = (face_detections[face].rect.get_rect().height() * 0.9388)/scaling;

	//        o_confidences[face] = face_detections[face].detection_confidence;


	//    }
	//    return o_regions.size() > 0;
	//}

	//bool DetectSingleFaceHOG(cv::Rect_<double>& o_region, const cv::Mat_<uchar>& intensity_img, dlib::frontal_face_detector& detector, double& confidence, cv::Point preference)
	//{
	//    // The tracker can return multiple faces
	//    vector<cv::Rect_<double> > face_detections;
	//    vector<double> confidences;

	//    bool detect_success = LandmarkDetector::DetectFacesHOG(face_detections, intensity_img, detector, confidences);

	//    if(detect_success)
	//    {

	//        bool use_preferred = (preference.x != -1) && (preference.y != -1);

	//        // keep the most confident one or the one closest to preference point if set
	//        double best_so_far;
	//        if(use_preferred)
	//        {
	//            best_so_far = sqrt((preference.x - (face_detections[0].width/2 + face_detections[0].x)) * (preference.x - (face_detections[0].width/2 + face_detections[0].x)) +
	//                               (preference.y - (face_detections[0].height/2 + face_detections[0].y)) * (preference.y - (face_detections[0].height/2 + face_detections[0].y)));
	//        }
	//        else
	//        {
	//            best_so_far = confidences[0];
	//        }
	//        int bestIndex = 0;

	//        for( size_t i = 1; i < face_detections.size(); ++i)
	//        {

	//            double dist;
	//            bool better;

	//            if(use_preferred)
	//            {
	//                dist = sqrt((preference.x - (face_detections[0].width/2 + face_detections[0].x)) * (preference.x - (face_detections[0].width/2 + face_detections[0].x)) +
	//                               (preference.y - (face_detections[0].height/2 + face_detections[0].y)) * (preference.y - (face_detections[0].height/2 + face_detections[0].y)));
	//                better = dist < best_so_far;
	//            }
	//            else
	//            {
	//                dist = confidences[i];
	//                better = dist > best_so_far;
	//            }

	//            // Pick a closest face
	//            if(better)
	//            {
	//                best_so_far = dist;
	//                bestIndex = i;
	//            }
	//        }

	//        o_region = face_detections[bestIndex];
	//        confidence = confidences[bestIndex];
	//    }
	//    else
	//    {
	//        // if not detected
	//        o_region = cv::Rect_<double>(0,0,0,0);
	//        // A completely unreliable detection (shouldn't really matter what is returned here)
	//        confidence = -2;
	//    }
	//    return detect_success;
	//}

	//============================================================================
	// Matrix reading functionality
	//============================================================================

	// Reading in a matrix from a stream
	void ReadMat(std::ifstream& stream, cv::Mat &output_mat)
	{
		// Read in the number of rows, columns and the data type
		int row, col, type;

		stream >> row >> col >> type;

		output_mat = cv::Mat(row, col, type);

		switch (output_mat.type())
		{
		case CV_64FC1:
		{
			cv::MatIterator_<double> begin_it = output_mat.begin<double>();
			cv::MatIterator_<double> end_it = output_mat.end<double>();

			while (begin_it != end_it)
			{
				stream >> *begin_it++;
			}
		}
		break;
		case CV_32FC1:
		{
			cv::MatIterator_<float> begin_it = output_mat.begin<float>();
			cv::MatIterator_<float> end_it = output_mat.end<float>();

			while (begin_it != end_it)
			{
				stream >> *begin_it++;
			}
		}
		break;
		case CV_32SC1:
		{
			cv::MatIterator_<int> begin_it = output_mat.begin<int>();
			cv::MatIterator_<int> end_it = output_mat.end<int>();
			while (begin_it != end_it)
			{
				stream >> *begin_it++;
			}
		}
		break;
		case CV_8UC1:
		{
			cv::MatIterator_<uchar> begin_it = output_mat.begin<uchar>();
			cv::MatIterator_<uchar> end_it = output_mat.end<uchar>();
			while (begin_it != end_it)
			{
				stream >> *begin_it++;
			}
		}
		break;
		default:
			printf("ERROR(%s,%d) : Unsupported Matrix type %d!\n", __FILE__, __LINE__, output_mat.type()); abort();


		}
	}

	void ReadMatBin(std::ifstream& stream, cv::Mat &output_mat)
	{
		// Read in the number of rows, columns and the data type
		int row, col, type;

		stream.read((char*)&row, 4);
		stream.read((char*)&col, 4);
		stream.read((char*)&type, 4);

		output_mat = cv::Mat(row, col, type);
		int size = output_mat.rows * output_mat.cols * output_mat.elemSize();
		stream.read((char *)output_mat.data, size);

	}

	// Skipping lines that start with # (together with empty lines)
	void SkipComments(std::ifstream& stream)
	{
		while (stream.peek() == '#' || stream.peek() == '\n' || stream.peek() == ' ' || stream.peek() == '\r')
		{
			std::string skipped;
			std::getline(stream, skipped);
		}
	}

}
