#pragma once

/*____________________________________________________
**
**	Title:			Cross Detector Library
**	Author:			ECP Hexa Team - Gabriel Huang
**	Date:		    2014
**  License:        MIT
**____________________________________________________
*/

#include <iostream>
#include <vector>
#include <deque>
#include <string>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/video/video.hpp>

/**
 *  \class Preprocess
 *  \brief Preprocesses raw input frames to binary mask
 *
 *  This class preprocesses raw input frames
 *  Basically, it converts them to L*a*b color space
 *  resizes them, and returns a binary mask.
 */
class Preprocess
{
    // Typedefs
    typedef unsigned char			UChar;
    typedef unsigned int			UInt;
    typedef std::vector<cv::Mat>	CvMatVtr;

    // Parameters
private:
    UInt			median_filter_bandwith_;
    UChar			l_max_;
    UChar			a_min_;
    UChar			b_min_;
    UInt			rows_;
    UInt			cols_;
    bool			debug_;
	float			low_;
	float			high_;

    // Buffers
private:
    cv::Mat			resized_src_;
    cv::Mat			lab_;
    cv::Mat			mask_l_;
    cv::Mat			mask_a_;
    cv::Mat			mask_b_;
    cv::Mat			mask_;
    CvMatVtr		channels_;

    // Getters / Setters
public:
    Preprocess();
    Preprocess& set_a_min(UChar x);
    Preprocess& set_b_min(UChar x);
    Preprocess& set_l_max(UChar x);
    Preprocess& set_median_filter_bandwith(UInt x);
    Preprocess& set_resize(UInt rows, UInt cols);
    Preprocess& set_debug(bool flag);
	Preprocess& set_low(UChar x);
	Preprocess& set_high(UChar x);
private:
	Preprocess(const Preprocess&);

    // Methods
public:
    Preprocess& get(const cv::Mat& src, cv::Mat& dst);
	Preprocess& get_soft(const cv::Mat& src, cv::Mat& dst);
};

/**
 *  \class Detect
 *  \brief Detects cross occurences
 *  
 *  This class takes a preprocessed frame
 *  and returns a pixel position (x, y) corresponding
 *  to the most probable position of a cross in the image.
 */
class Detect
{
    typedef unsigned int		UInt;

    // Data
private:
    cv::CascadeClassifier		classifier_;

    // Parameters
private:
    UInt						min_rows_;
    UInt						min_cols_;
    UInt						min_count_;
    float						scale_;
    bool						verbose_;

    // Getters
public:
    Detect();
    Detect& set_min_rows(UInt x);
    Detect& set_min_cols(UInt x);
    Detect& set_min_count(UInt x);
    Detect& set_scale(float x);
    Detect& set_verbose(bool flag);
    Detect& set_classifier(const std::string& cascade_xml_file);
    Detect& set_classifier(cv::CascadeClassifier& cascade_classifier);
private:
	Detect(const Detect&);

    // Methods
public:
    Detect& detect(cv::Mat& frame, std::vector<cv::Rect>& occurences);
    Detect& detect(const cv::Mat& frame, int& x, int& y);
};

class Refiner
{
	// Parameters
private:
	std::vector<float>	reduce_factors_;
	float				fraction_;
	float				flatten_limit_;

	// GSetters
public:
	Refiner();
	Refiner& set_threshold(float fraction);
	Refiner& set_reduction(const std::vector<float> factors);
	Refiner& set_flatten_limit(float limit);
private:
	Refiner(const Refiner&);

	// Inlines
public:
	inline float flatten(float x)
	{
		if(x <= flatten_limit_)
		{
			return 0.f;
		}
		else
		{
			return (x - flatten_limit_) / (1.f - flatten_limit_);
		}
	}

	// Methods
public:
	float crossness(const cv::Mat& processed_frame);
	bool get_best(const cv::Mat& processed_frame, const std::vector<cv::Rect>& occurences, cv::Point& center);
};

class Kalman
{
	// Parameters
	float				max_no_measure_;
	float				process_position_variance_;
	float				process_speed_variance_;

	// Data
	cv::KalmanFilter	kf_;
	cv::Mat				prediction_;
	float				no_measure_;

	// Ctor & GSetters
public:
	Kalman();
	cv::KalmanFilter& kf();
	Kalman& set_process_position_variance(float x);
	Kalman& set_process_speed_variance(float x);

private:
	Kalman(const Kalman&);
	void recalculate_process_variance_();

	// Methods
public:
	Kalman& advance(float dt);
	Kalman& update(const cv::Mat& measurement);
	cv::Mat guess() const;
	bool still_tracking() const;
	float no_measure() const;
};