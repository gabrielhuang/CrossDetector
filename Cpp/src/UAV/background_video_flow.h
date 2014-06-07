#pragma once

#include <video_flow.h>
#include <ctime>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <utilities.h>

const int wait_delay = 5;

template<typename VideoSourceType>
class BackgroundVideoFlow : public Subject<BackgroundVideoFlow<VideoSourceType> >
{
	// Typedefs
	typedef unsigned int		UInt;
	typedef unsigned char		UChar;

public: // fast hack
	// State
	float				x_;
	float				y_;
	bool				loop_;
	cv::Mat				raw_frame_;
	cv::Mat				processed_frame_;

	// Computers
	Preprocess			preprocessor_;
	Detect				detector_;
	Refiner				refiner_;
	Kalman				kalman_;

	// Thread
	boost::thread*		thread_;
	boost::mutex /*mutable*/		mutex_;

	// Source
	VideoSourceType*	camera_;

	// Parameters
public:
	UInt				resize_cols;
	UInt				resize_rows;
	UInt				median_filter_bandwith;
	UChar				a_min; 
	UChar				b_min;
	UChar				l_max;
	UChar				a_cut;
	UChar				a_sat;

	bool				has_visual_;

	UInt				min_size;
	UInt				min_count;
	float				scale;

	UChar				refiner_threshold;

	float				process_position_variance;
	float				process_speed_variance;

	std::string			cross_cascade_name;

public:
	BackgroundVideoFlow() :
		camera_(nullptr),
		x_(0.f),
		y_(0.f),
		loop_(true),
		resize_cols(720),
		resize_rows(576),
		median_filter_bandwith(5),
		a_min(135),
		b_min(122),
		l_max(200),
		min_size(16),
		min_count(34),
		scale(1.2f),
		refiner_threshold(50),
		a_cut(117),
		a_sat(170),
		has_visual_(false),
		process_position_variance(40.f),
		process_speed_variance(1.f),
		thread_(nullptr),
		cross_cascade_name("..\\res\\cascade_5.xml")
	{
	}

	void init(bool multi_threaded = false)
	{
        preprocessor_.set_l_max(l_max).set_a_min(a_min).set_b_min(b_min);
		preprocessor_.set_low(a_cut).set_high(a_sat);
        preprocessor_.set_resize(resize_rows, resize_cols);
        preprocessor_.set_median_filter_bandwith(median_filter_bandwith);
        preprocessor_.set_debug(false);

        detector_.set_classifier(cross_cascade_name);
        detector_.set_min_cols(min_size).set_min_rows(min_size);
        detector_.set_min_count(min_count);
        detector_.set_scale(scale);
        detector_.set_verbose(true);

		refiner_.set_threshold(float(refiner_threshold) / 100.f);

		kalman_.set_process_position_variance(process_position_variance);
		kalman_.set_process_speed_variance(process_speed_variance);
		float last_prediction_date = float(clock()) / float(CLOCKS_PER_SEC);

		loop_ = true;

		if(multi_threaded)
		{
			thread_ = new boost::thread(&BackgroundVideoFlow::loop, this);
		}
		else
		{
			loop();
		}
	}

	~BackgroundVideoFlow()
	{
		{
			boost::mutex::scoped_lock lock(mutex_);
			loop_ = false;
		}
		if(thread_)
		{
			thread_->join();
			delete thread_;
		}
		if(camera_)
		{
			delete camera_;
		}
	}

	inline bool has_new()
	{
		boost::mutex::scoped_lock lock(mutex_);
		return has_visual_;
	}

	// Getters
	float x()
	{
		boost::mutex::scoped_lock lock(mutex_);
		return x_;
	}

	float y()
	{
		boost::mutex::scoped_lock lock(mutex_);
		return y_;
	}

	void loop()
	{
		// Note, camera must ABSOLUTELY be initialized
		// in same thread when using (Free)GLUT
		std::cout << "[BGVF] Loading camera..." << std::endl;
		camera_ = new VideoSourceType(0);
		if(!camera_->isOpened())
		{
			throw std::runtime_error("Error opening video capture");
		}

		std::cout << "[BGVF] Started Loop" << std::endl;
		float last_prediction_date = float(clock()) / float(CLOCKS_PER_SEC);

		while(true)
		{
			// Verify loop_ is true
			{
				boost::mutex::scoped_lock lock(mutex_);
				has_visual_ = false;
				if(!loop_)
				{
					break;
				}
			}

			// Get frame
			*camera_ >> raw_frame_;
			if(raw_frame_.empty())
            {
                cv::waitKey(wait_delay);
            }

            // Preprocess
            preprocessor_.get_soft(raw_frame_, processed_frame_);
			//preprocessor_.get(raw_frame_, processed_hard);
            float ratio_x = float(raw_frame_.cols) / processed_frame_.cols;
            float ratio_y = float(raw_frame_.rows) / processed_frame_.rows;

            // Detect
            std::vector<cv::Rect> occurences;
            detector_.detect(processed_frame_, occurences);
			//std::cout << "[BGVF] " << "Detected " << occurences.size() << std::endl;

			// Show Detections
            for (UInt i = 0; i < UInt(occurences.size()); ++i)
            {
				float cness = refiner_.flatten(refiner_.crossness(processed_frame_(occurences[i])));
				//std::cout << "[BGVF] " << "Crossness for occurence " << i << " = " << cness << std::endl;
				
                cv::Rect adjusted_rect(int(ratio_x * occurences[i].x),
                    int(ratio_y * occurences[i].y),
                    int(ratio_x * occurences[i].width),
                    int(ratio_y * occurences[i].height));
                cv::rectangle(raw_frame_, adjusted_rect, cv::Scalar(cness * 255.f, 0.f, 0.f, 3));
            }

			// Kalman Update by dt
			float new_prediction_date = float(clock()) / float(CLOCKS_PER_SEC);
			kalman_.advance(new_prediction_date - last_prediction_date);

			// Get most probable center
			cv::Point center;
			bool actually_found = refiner_.get_best(processed_frame_, occurences, center);
			if(actually_found)
			{
				// Draw Refined Center
				cv::Point adjusted_center(int(ratio_x * float(center.x)), int(ratio_y * float(center.y)));
				cv::circle(raw_frame_, adjusted_center, 32, cv::Scalar(0.f, 0.f, 255.f, 0), CV_FILLED);
				std::cout << "[BGVF] " << "Cross @ " << adjusted_center << std::endl;

				// Kalman Correct
				cv::Mat_<float> measurement(2,1);
				measurement(0) = float(center.x);
				measurement(1) = float(center.y);
				kalman_.update(measurement);
			}

			// Kalman final prediction
			cv::Mat prediction = kalman_.guess();
			last_prediction_date = new_prediction_date;

			// Draw frame if in cross mode : red if actually detected, yellow if prediction
			if(kalman_.still_tracking())
			{
				cv::rectangle(raw_frame_, cv::Rect(0, 0, raw_frame_.cols - 1, raw_frame_.rows - 1), 
						      actually_found ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 255), 20);
				cv::Point predicted_center = cv::Point(int(prediction.at<float>(0)), int(prediction.at<float>(1)));
				cv::Point adjusted_predicted_center(int(ratio_x * float(predicted_center.x)), int(ratio_y * float(predicted_center.y)));
				cv::circle(raw_frame_, adjusted_predicted_center, 20, cv::Scalar(0.f, 255.f, 0, 0), CV_FILLED);
				
				// Update x and y
				{
					boost::mutex::scoped_lock lock(mutex_);
					x_ = predicted_center.x / float(processed_frame_.cols) - 1.f;
					y_ = predicted_center.y / float(processed_frame_.rows) - 1.f;
				}
			}

			// Notify
			has_visual_ = kalman_.still_tracking();
			notify();
		}
	}

	void get_raw_frame(cv::Mat& dst)
	{
		boost::mutex::scoped_lock lock(mutex_);
		dst = raw_frame_.clone();
	}

	void get_processed_frame(cv::Mat& dst)
	{
		boost::mutex::scoped_lock lock(mutex_);
		dst = processed_frame_.clone();
	}

	void set_loop(bool flag)
	{
		boost::mutex::scoped_lock lock(mutex_);
		loop_ = flag;
	}
};

template<typename VideoSourceType>
class Viewer : public Observer<BackgroundVideoFlow<VideoSourceType> >
{
	typedef BackgroundVideoFlow<VideoSourceType> SpecificBackgroundVideoFlow;

	bool		wait_;
	cv::Mat		raw_frame_;
	cv::Mat		processed_frame_;

public:
	Viewer(SpecificBackgroundVideoFlow* subject, bool wait = true) :
		Observer<SpecificBackgroundVideoFlow>(subject),
		wait_(wait)
	{
	}

	void update()
	{
		subject_->get_raw_frame(raw_frame_);
		subject_->get_processed_frame(processed_frame_);
		cv::imshow("Raw", raw_frame_);
		cv::imshow("Processed", processed_frame_);
		if(wait_)
		{
			cv::waitKey(wait_delay);
		}
	}
};

template<typename VideoSourceType>
class Parametrizer : public Observer<BackgroundVideoFlow<VideoSourceType> >
{
	typedef BackgroundVideoFlow<VideoSourceType> SpecificBackgroundVideoFlow;
public:
	Parametrizer(SpecificBackgroundVideoFlow* subject) :
		Observer<SpecificBackgroundVideoFlow>(subject)
	{
	}

	void update()
	{
		// Delay & Tune
		switch(cv::waitKey(wait_delay))
		{
		case 'C': subject_->min_count += 4; std::cout << "[Parametrizer] " << "min_count = " << subject_->min_count << std::endl;
			subject_->detector_.set_min_count(subject_->min_count); break;
		case 'c': subject_->min_count = std::max(1, int(subject_->min_count) - 5); std::cout << "[Parametrizer] " << "min_count = " << subject_->min_count << std::endl;
			subject_->detector_.set_min_count(subject_->min_count); break;
		case 'S': subject_->scale += 0.05f; std::cout << "[Parametrizer] " << "scale = " << subject_->scale << std::endl;
			subject_->detector_.set_scale(subject_->scale); break;
		case 's': subject_->scale = std::max(1.f, subject_->scale - 0.05f); std::cout << "[Parametrizer] " << "min_count = " << subject_->scale << std::endl;
			subject_->detector_.set_scale(subject_->scale); break;
		case 'A': subject_->a_min += 10; std::cout << "[Parametrizer] " << "a_min = " << int(subject_->a_min) << std::endl;
			subject_->preprocessor_.set_a_min(subject_->a_min); break;
		case 'a': subject_->a_min -= 10; std::cout << "[Parametrizer] " << "a_min = " << int(subject_->a_min) << std::endl;
			subject_->preprocessor_.set_a_min(subject_->a_min); break;
		case 'B': std::cout << "[Parametrizer] " << "b_min = " << int(++subject_->b_min) << std::endl;
			subject_->preprocessor_.set_b_min(subject_->b_min); break;
		case 'b': std::cout << "[Parametrizer] " << "b_min = " << int(--subject_->b_min) << std::endl;
			subject_->preprocessor_.set_b_min(subject_->b_min); break;
		case 'T' : subject_->refiner_threshold += (subject_->refiner_threshold < 99);
			std::cout << "[Parametrizer] " << "RefinerThreshold = " << float(subject_->refiner_threshold) * 0.01f << std::endl;
			subject_->refiner_.set_threshold(float(subject_->refiner_threshold) * 0.01f); break;
		case 't' : subject_->refiner_threshold -= (subject_->refiner_threshold > 0);
			std::cout << "[Parametrizer] " << "RefinerThreshold = " << float(subject_->refiner_threshold) * 0.01f << std::endl;
			subject_->refiner_.set_threshold(float(subject_->refiner_threshold) * 0.01f); break;
		case 'L' :
			subject_->a_cut += subject_->a_cut < 255 ? 1 : 0;
			std::cout << "[Parametrizer] " << "L*a*b a_cut = " << int(subject_->a_cut) << std::endl;
			subject_->preprocessor_.set_low(subject_->a_cut); break;
		case 'l' :
			subject_->a_cut -= subject_->a_cut > 0 ? 1 : 0;
			std::cout << "[Parametrizer] " << "L*a*b a_cut = " <<  int(subject_->a_cut)  << std::endl;
			subject_->preprocessor_.set_low(subject_->a_cut); break;
		case 'H' :
			subject_->a_sat += subject_->a_sat < 255 ? 1 : 0;
			std::cout << "[Parametrizer] " << "L*a*b a_sat = " << int(subject_->a_sat) << std::endl;
			subject_->preprocessor_.set_high(subject_->a_sat); break;
		case 'h' :
			subject_->a_sat -= subject_->a_sat > 0 ? 1 : 0;
			std::cout << "[Parametrizer] " << "L*a*b a_sat = " << int(subject_->a_sat) << std::endl;
			subject_->preprocessor_.set_high(subject_->a_sat); break;
		}
	}
};