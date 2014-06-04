/*____________________________________________________
**
**	Title:			Cross Detector Library
**	Author:			ECP Hexa Team - Gabriel Huang
**	Date:		    2014
**  License:        MIT
**____________________________________________________
*/

#include <video_flow.h>

using namespace std;

typedef unsigned char UChar;
typedef unsigned int UInt;

Preprocess::Preprocess() :
a_min_(140),
b_min_(125),
l_max_(255),
rows_(240),
cols_(320),
median_filter_bandwith_(5),
channels_(3, cv::Mat()),
debug_(false)
{
}

Preprocess& Preprocess::set_a_min(UChar x)
{
    a_min_ = x;
    return *this;
}

Preprocess& Preprocess::set_b_min(UChar x)
{
    b_min_ = x;
    return *this;
}

Preprocess& Preprocess::set_l_max(UChar x)
{
    l_max_ = x;
    return *this;
}

Preprocess& Preprocess::set_median_filter_bandwith(UInt x)
{
    median_filter_bandwith_ = (x / 2) * 2 + 1;
    return *this;
}

Preprocess& Preprocess::set_resize(UInt rows, UInt cols)
{
    rows_ = rows;
    cols_ = cols;
    return *this;
}

Preprocess& Preprocess::set_debug(bool flag)
{
    debug_ = flag;
    return *this;
}

Preprocess& Preprocess::get(const cv::Mat& src, cv::Mat& dst)
{
    // Resize
    cv::resize(src, resized_src_, cv::Size(cols_, rows_));

    // Split to L*a*b
    cv::cvtColor(resized_src_, lab_, CV_BGR2Lab);
    cv::split(lab_, channels_);

    // Threshold
    mask_l_ = channels_[0] <= l_max_;
    mask_a_ = channels_[1] >= a_min_;
    mask_b_ = channels_[2] >= b_min_;
    mask_ = mask_l_ & mask_a_ & mask_b_;

    // MedianBlur (to remove small noise)
    if (median_filter_bandwith_ > 1)
    {
        cv::medianBlur(mask_, mask_, median_filter_bandwith_);
    }

    // Return thresholded
    dst = mask_;

    // Debug - Display 3 channels
    if (debug_)
    {
        cv::imshow("Resized BGR", resized_src_);
        cv::imshow("L Channel", channels_[0]);
        cv::imshow("l Mask", mask_l_);
        cv::imshow("a Mask", mask_a_);
        cv::imshow("b Mask", mask_b_);
        cv::imshow("Final Mask = l & a & b", mask_);
    }
    return *this;
}

namespace
{
	inline float soft(float L, float a, float b, 
			   float l_max, float a_min, float b_min)
	{
		float sigma = 30.f;
		float low = 135.f;
		float high = 170.f;
		float level = 0.f;
		if(L <= l_max && a >= 135 && b >= b_min)
		{
			if (a > high) return 255.f;
			else return level + (a - low) / (high - low) * (255.f - level);
			//return 255.f * max(0.f, 1.f / (1.f + expf(-(a - 120.f) / sigma)));
		}
		else
		{
			return 0.f;
		}
	}
}

Preprocess& Preprocess::get_soft(const cv::Mat& src, cv::Mat& dst)
{
    // Resize
    cv::resize(src, resized_src_, cv::Size(cols_, rows_));

    // Split to L*a*b
    cv::cvtColor(resized_src_, lab_, CV_BGR2Lab);
    cv::split(lab_, channels_);

    // Threshold
	dst = cv::Mat(rows_, cols_, CV_32FC1);
	for(UInt row = 0; row < rows_; ++row)
	{
		for(UInt col = 0; col < cols_; ++col)
		{
			dst.at<float>(row, col) = soft(channels_[0].at<UChar>(row, col), 
										   channels_[1].at<UChar>(row, col), 
										   channels_[2].at<UChar>(row, col), 
										   l_max_, a_min_, b_min_);
		}
	}

	dst.convertTo(dst, CV_8UC1);

    // MedianBlur (to remove small noise)
    if (median_filter_bandwith_ > 1)
    {
        cv::medianBlur(dst, dst, median_filter_bandwith_);
    }

    return *this;
}



Detect::Detect() :
    min_rows_(64),
    min_cols_(64),
    scale_(1.1f),
    min_count_(3),
    verbose_(false)
{
}

Detect& Detect::set_min_rows(UInt x)
{
    min_rows_ = x;
    return *this;
}

Detect& Detect::set_min_cols(UInt x)
{
    min_cols_ = x;
    return *this;
}

Detect& Detect::set_min_count(UInt x)
{
    min_count_ = x;
    return *this;
}

Detect& Detect::set_scale(float x)
{
    scale_ = x;
    return *this;
}

Detect& Detect::set_verbose(bool flag)
{
    verbose_ = flag;
    return *this;
}

Detect& Detect::set_classifier(const string& cascade_xml_file)
{
    try
    {
        classifier_.load(cascade_xml_file);
    }
    catch (...)
    {
        throw std::runtime_error("Error loading cross cascade");
    }
    return *this;
}

Detect& Detect::set_classifier(cv::CascadeClassifier& cascade_classifier)
{
    classifier_ = cascade_classifier;
    return *this;
}

Detect& Detect::detect(cv::Mat& frame, std::vector<cv::Rect>& occurences)
{
    // Detect crosses
    classifier_.detectMultiScale(frame, occurences, scale_, min_count_, 0, cv::Size(min_cols_, min_rows_), cv::Size(480,480));

    // Debug : Show occurences
    if (verbose_)
    {
    //    for (vector<cv::Rect>::iterator occurence = occurences.begin();
			 //occurence != occurences.end();
			 //++occurence)
    //    {
    //        cv::Point center(occurence->x + occurence->width / 2, occurence->y + occurence->height / 2);
    //        cv::rectangle(frame, *occurence, cv::Scalar(120), 3);
    //    }
        cout << "CascadeCount : " << occurences.size() << endl;
    }
    return *this;
}




Refiner::Refiner() :
	fraction_(0.75f),
	flatten_limit_(0.92f)
{
	// Order increasingly
	reduce_factors_.push_back(0.74f);
	reduce_factors_.push_back(0.80f);
	reduce_factors_.push_back(0.68f);
	reduce_factors_.push_back(0.86f);
	reduce_factors_.push_back(0.64f);
}

Refiner& Refiner::set_threshold(float fraction)
{
	fraction_ = fraction;
	return *this;
}

Refiner& Refiner::set_flatten_limit(float limit)
{
	if(limit > 0.995f || limit < 0.f)
	{
		throw std::runtime_error("set_flatten_limit : out of limits");
	}
	flatten_limit_ = limit;
	return *this;
}

Refiner& Refiner::set_reduction(const std::vector<float> factors)
{
	reduce_factors_ = factors;
	return *this;
}

namespace
{
	inline int part(int length, const cv::Size& size)
	{
		if(length < size.width) return 0;
		if(length < size.width + size.height - 1) return 1;
		if(length < 2 * size.width + size.height - 2) return 2;
		return 3;
	}

	enum PartRelation { k_ADJACENT, k_OPPOSITE, k_SAME };

	inline int adjacent(int part_1, int part_2, cv::Point& corner, const cv::Size& size)
	{
		int x = abs(part_1 - part_2);
		if(x == 0)
		{
			return k_SAME;
		}
		else if(x == 2)
		{
			return k_OPPOSITE;
		}
		else if(x == 1)
		{
			int min_part = min(part_1, part_2);
			if(min_part == 0)		corner.x = size.width - 1, corner.y = 0;
			else if(min_part == 1)	corner.x = size.width - 1, corner.y = size.height - 1;
			else					corner.x = 0,			   corner.y = size.height - 1;
			return k_ADJACENT;
		}
		else
		{
			corner.x = corner.y = 0;
			return k_ADJACENT;
		}
	}

	inline cv::Point to_point(int length, const cv::Size& size)
	{
		switch(part(length, size))
		{
		case 0:		return cv::Point(length, 0);
		case 1:		return cv::Point(size.width - 1, length - size.width - 1);
		case 2:		return cv::Point(2 * size.width + size.height - length - 3, size.height - 1);
		default:	return cv::Point(0, 2 * (size.width + size.height) - length - 4);
		}
	}
}

float Refiner::crossness(const cv::Mat& frame)
{
	deque<int> snake;
	deque<int> starts, stops;

	vector<float>::const_iterator reduce_factor;
	cv::Rect reduced_box;
	UChar threshold = 0;
	cv::Mat reduced_frame;

	for(reduce_factor = reduce_factors_.begin(); 
		reduce_factor != reduce_factors_.end(); 
		++reduce_factor)
	{
		float center_x = 0.5f * frame.cols;
		float center_y = 0.5f * frame.rows;
		int reduced_width = int(*reduce_factor * float(frame.cols));
		int reduced_height = int(*reduce_factor * float(frame.cols));

		reduced_box = cv::Rect(int(0.5f * (1 - *reduce_factor) * frame.cols),
			int(0.5f * (1 - *reduce_factor) * frame.rows),
			int(*reduce_factor * frame.cols),
			int(*reduce_factor * frame.rows));
		reduced_frame = frame(reduced_box);
		UChar max_intensity = 0;

		for(int col = 0; col < reduced_frame.cols; ++col)
		{
			for(int row = 0; row < reduced_frame.rows; ++row)
			{
				UChar intensity = reduced_frame.at<UChar>(row, col);

				if(intensity > max_intensity)
				{
					max_intensity = intensity;
				}
			}
		}

		threshold = UChar(fraction_ * float(max_intensity));
		//cv::imshow("reduced_frame ", reduced_frame);

		// Count
		{
			for(int col = 0; col < reduced_frame.cols; ++col)
			{
				snake.push_back(reduced_frame.at<UChar>(0, col) > threshold);
			}

			for(int row = 1; row < reduced_frame.rows; ++row)
			{
				snake.push_back(reduced_frame.at<UChar>(row, reduced_frame.cols - 1) > threshold);
			}

			for(int col = reduced_frame.cols - 2; col >= 0; --col)
			{
				snake.push_back(reduced_frame.at<UChar>(reduced_frame.rows - 1, col) > threshold);
			}

			for(int row = reduced_frame.rows - 2; row > 0; --row)
			{
				snake.push_back(reduced_frame.at<UChar>(row, 0) > threshold);
			}

			int in_red = snake.back();

			for(int i = 0; i < int(snake.size()); ++i)
			{
				if(!in_red && snake[i])
				{
					starts.push_back(i);
				}
				else if(in_red && !snake[i])
				{
					stops.push_back(i);
				}
				in_red = snake[i];
			}

			if(starts.size() == 4 || stops.size() == 4)
			{
				break;
			}
			else
			{
				snake.resize(0);
			}
		}
	}

	// Now snake contains border of *occurence
	// at size *reduce_factor
	if(!snake.size()) // Very probably not a cross
	{
		return 0.f;
	}

	// Generate a Cross primitive, then compare

	// If snake.back() is red (true)
	// the connected order will be

	// starts[0]<->stops[1]
	// starts[1]<->stops[2] 
	// starts[2]<->stops[3] 
	// starts[3]<->stops[0] 

	// otherwise :

	// starts[0]<->stops[0]
	// starts[1]<->stops[1] 
	// starts[2]<->stops[2] 
	// starts[3]<->stops[3]

	// THIS will get all to the *second* case
	if(snake.back())
	{
		stops.push_back(stops.front());
		stops.pop_front();
	}

	// Fill basic rectangles
	cv::Mat primitive(reduced_box.height, reduced_box.width, CV_8UC1, cv::Scalar(0));
	for(int i = 0; i < 2; ++i)
	{
		cv::Point points[] = 
		{ 
			to_point(starts[i],		reduced_box.size()),
			to_point(stops[i],		reduced_box.size()),
			to_point(starts[2 + i], reduced_box.size()),
			to_point(stops[2 + i],	reduced_box.size())
		};
		cv::fillConvexPoly(primitive, points, 4, cv::Scalar(255)); 
	}

	// Fill corners if needed
	for(int i = 0; i < 4; ++i)
	{
		int part_start	= part(starts[i], reduced_box.size());
		int part_stop	= part(stops[i], reduced_box.size());
		cv::Point corner;
		int adjacency = adjacent(part_start, part_stop, corner, reduced_box.size());
		if(adjacency == k_ADJACENT)
		{
			cv::Point points[] = 
			{ 
				to_point(starts[i],		reduced_box.size()),
				to_point(stops[i],		reduced_box.size()),
				corner,
			};
			cv::fillConvexPoly(primitive, points, 3, cv::Scalar(255)); 
		}
		else if(adjacency == k_OPPOSITE)
		{
			return 0.f;
		}
	}

	//cv::imshow("Generated primitive " , primitive);
	//cv::imshow("Thresholded image" , thresholded);
	//cv::waitKey(10);
	
	// Calculate crossness
	int crossness = 0;
	for(int row = 0; row < reduced_box.height; ++row)
	{
		for(int col = 0; col < reduced_box.width; ++col)
		{
			crossness += (primitive.at<UChar>(row, col) > 125) == (reduced_frame.at<UChar>(row, col) > threshold);
		}
	}

	return float(crossness) / float(reduced_box.area());
}

bool Refiner::get_best(const cv::Mat& processed_frame, const std::vector<cv::Rect>& occurences, cv::Point& center)
{
	deque<float> scores;

	for(vector<cv::Rect>::const_iterator occurence = occurences.begin();
		occurence != occurences.end();
		++occurence)
	{
		float score = crossness(processed_frame(*occurence));
		scores.push_back(flatten(score));
	}

	deque<float>::iterator max_it = std::max_element(scores.begin(), scores.end());

	if(max_it == scores.end() || *max_it < std::numeric_limits<float>().epsilon())
	{
		// No cross
		return false;
	}

	// TODO Make it better
	int idx = std::distance(scores.begin(), max_it);
	cv::Rect solution = occurences[idx];
	center.x = solution.x + solution.width / 2;
	center.y = solution.y + solution.height / 2;
	return true;
}

Kalman::Kalman() :
	kf_(4, 2, 0),
	max_no_measure_(2.f),
	process_position_variance_(0.001f),
	process_speed_variance_(0.001f)
{
	// No measure time is infinite at first
	no_measure_ = max_no_measure_ + 1.f;

	// Transition Matrix, suppose dt = 0.1 seconds
	kf_.transitionMatrix = *(cv::Mat_<float>(4, 4) << 
		1,	0,	0.1,	0,   
		0,	1,	0,		0.1,  
		0,	0,	1,		0,  
		0,	0,	0,		1);

	// Init internal state (x, y, dx/dt, dy/dt)
	kf_.statePre.at<float>(0) = 120.f;
	kf_.statePre.at<float>(1) = 120.f;
	kf_.statePre.at<float>(2) = 0;
	kf_.statePre.at<float>(3) = 0;

	// Measure only (x,y)
	cv::setIdentity(kf_.measurementMatrix);

	// Process Noise, ~ N(0, diag(DeltaX, DeltaY, DeltaDxDt, DeltaDyDt)
	recalculate_process_variance_();

	// Measurement Noise, ~ N(0, 1)
	cv::setIdentity(kf_.measurementNoiseCov, cv::Scalar::all(1));

	// Anything, will be recalculated
	cv::setIdentity(kf_.errorCovPost, cv::Scalar::all(1));
}

cv::KalmanFilter& Kalman::kf()
{
	return kf_;
}

Kalman& Kalman::set_process_position_variance(float x)
{
	process_position_variance_ = x;
	recalculate_process_variance_();
	return *this;
}

Kalman& Kalman::set_process_speed_variance(float x)
{
	process_speed_variance_ = x;
	recalculate_process_variance_();
	return *this;
}

void Kalman::recalculate_process_variance_()
{
	kf_.processNoiseCov = *(cv::Mat_<float>(4, 4) << 
		process_position_variance_,	0,	0, 0, 
		0, process_position_variance_,	0, 0, 
		0, 0, process_speed_variance_, 0, 
		0, 0, 0, process_speed_variance_);
}

Kalman& Kalman::advance(float dt)
{
	kf_.transitionMatrix = *(cv::Mat_<float>(4, 4) <<	
		1,	0,	dt,	0,   
		0,	1,	0,	dt,  
		0,	0,	1,	0,  
		0,	0,	0,	1);
	float sqrDt = dt * dt;
	kf_.processNoiseCov = *(cv::Mat_<float>(4, 4) << 
		sqrDt * process_position_variance_,	0,	0, 0, 
		0, sqrDt * process_position_variance_,	0, 0, 
		0, 0, sqrDt * process_speed_variance_, 0, 
		0, 0, 0, sqrDt * process_speed_variance_);
	prediction_ = kf_.predict();
	no_measure_ += dt;
	return *this;
}

Kalman& Kalman::update(const cv::Mat& measurement)
{
	no_measure_ = 0.f;
	kf_.correct(measurement);
	prediction_ = kf_.predict();
	return *this;
}

cv::Mat Kalman::guess() const
{
	return prediction_;
}

bool Kalman::still_tracking() const
{
	return no_measure_ < max_no_measure_;
}

float Kalman::no_measure() const
{
	return no_measure_;
}