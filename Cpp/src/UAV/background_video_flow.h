#include <video_flow.h>
#include <ctime>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

template <typename VideoSourceType>
class BackgroundVideoFlow
{
	typedef unsigned int		UInt;
	typedef unsigned char		UChar;

	// State
	float				x_;
	float				y_;
	bool				loop_;

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
	UInt				wait_delay;
	UInt				resize_cols;
	UInt				resize_rows;
	UInt				median_filter_bandwith;
	UChar				a_min; 
	UChar				b_min;
	UChar				l_max;

	UInt				min_size;
	UInt				min_count;
	float				scale;

	UChar				refiner_threshold;
	UChar				refiner_flatten;

	float				process_position_variance;
	float				process_speed_variance;

	std::string			cross_cascade_name;

public:
	BackgroundVideoFlow() :
		camera_(nullptr),
		x_(0.f),
		y_(0.f),
		loop_(true),
		wait_delay(5),
		resize_cols(640),
		resize_rows(480),
		median_filter_bandwith(7),
		a_min(135),
		b_min(122),
		l_max(200),
		min_size(16),
		min_count(34),
		scale(1.2f),
		refiner_threshold(50),
		refiner_flatten(91),
		process_position_variance(40.f),
		process_speed_variance(1.f),
		cross_cascade_name("Z:\\Cpp\\UAV\\Cpp\\res\\cascade_5.xml")
	{
	}

	void init()
	{
        preprocessor_.set_l_max(l_max).set_a_min(a_min).set_b_min(b_min);
        preprocessor_.set_resize(resize_rows, resize_cols);
        preprocessor_.set_median_filter_bandwith(median_filter_bandwith);
        preprocessor_.set_debug(false);

        detector_.set_classifier(cross_cascade_name);
        detector_.set_min_cols(min_size).set_min_rows(min_size);
        detector_.set_min_count(min_count);
        detector_.set_scale(scale);
        detector_.set_verbose(true);

		refiner_.set_threshold(float(refiner_threshold) / 100.f);
		refiner_.set_flatten_limit(float(refiner_flatten) / 100.f);

		kalman_.set_process_position_variance(process_position_variance);
		kalman_.set_process_speed_variance(process_speed_variance);
		float last_prediction_date = float(clock()) / float(CLOCKS_PER_SEC);

		loop_ = true;
		thread_ = new boost::thread(&BackgroundVideoFlow::loop, this);
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
		cv::Mat raw_frame, processed_frame;
		while(true)
		{
			// Verify loop_ is true
			{
				boost::mutex::scoped_lock lock(mutex_);
				if(!loop_)
				{
					break;
				}
			}

			// Get frame
			*camera_ >> raw_frame;
			if(raw_frame.empty())
            {
                cv::waitKey(wait_delay);
            }

            // Preprocess
            preprocessor_.get_soft(raw_frame, processed_frame);
			//preprocessor_.get(raw_frame, processed_hard);
            float ratio_x = float(raw_frame.cols) / processed_frame.cols;
            float ratio_y = float(raw_frame.rows) / processed_frame.rows;

            // Detect
            std::vector<cv::Rect> occurences;
            detector_.detect(processed_frame, occurences);
			std::cout << "[BGVF] " << "Detected " << occurences.size() << std::endl;

			// Show Detections
            for (UInt i = 0; i < UInt(occurences.size()); ++i)
            {
				float cness = refiner_.flatten(refiner_.crossness(processed_frame(occurences[i])));
				std::cout << "[BGVF] " << "Crossness for occurence " << i << " = " << cness << std::endl;
				
                cv::Rect adjusted_rect(int(ratio_x * occurences[i].x),
                    int(ratio_y * occurences[i].y),
                    int(ratio_x * occurences[i].width),
                    int(ratio_y * occurences[i].height));
                cv::rectangle(raw_frame, adjusted_rect, cv::Scalar(cness * 255.f, 0.f, 0.f, 3));
            }

			// Kalman Update by dt
			float new_prediction_date = float(clock()) / float(CLOCKS_PER_SEC);
			kalman_.advance(new_prediction_date - last_prediction_date);

			// Get most probable center
			cv::Point center;
			bool actually_found = refiner_.get_best(processed_frame, occurences, center);
			if(actually_found)
			{
				// Draw Refined Center
				cv::Point adjusted_center(int(ratio_x * float(center.x)), int(ratio_y * float(center.y)));
				cv::circle(raw_frame, adjusted_center, 32, cv::Scalar(0.f, 0.f, 255.f, 0), CV_FILLED);
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
				cv::rectangle(raw_frame, cv::Rect(0, 0, raw_frame.cols - 1, raw_frame.rows - 1), 
						      actually_found ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 255), 20);
				cv::Point predicted_center = cv::Point(int(prediction.at<float>(0)), int(prediction.at<float>(1)));
				cv::Point adjusted_predicted_center(int(ratio_x * float(predicted_center.x)), int(ratio_y * float(predicted_center.y)));
				cv::circle(raw_frame, adjusted_predicted_center, 20, cv::Scalar(0.f, 255.f, 0, 0), CV_FILLED);
				
				// Update x and y
				{
					boost::mutex::scoped_lock lock(mutex_);
					x_ = predicted_center.x / float(processed_frame.cols) - 1.f;
					y_ = predicted_center.y / float(processed_frame.rows) - 1.f;
				}
			}

            // Show frames
            cv::imshow("Raw Frame", raw_frame);
			cv::imshow("Processed Soft", processed_frame);

            // Delay & Tune
            char key = cv::waitKey(wait_delay);
            switch (key)
            {
            case 'C': min_count += 4; std::cout << "[BGVF] " << "min_count = " << min_count << std::endl;
                detector_.set_min_count(min_count); break;
            case 'c': min_count = std::max(1, int(min_count) - 5); std::cout << "[BGVF] " << "min_count = " << min_count << std::endl;
                detector_.set_min_count(min_count); break;
            case 'S': scale += 0.05f; std::cout << "[BGVF] " << "scale = " << scale << std::endl;
                detector_.set_scale(scale); break;
            case 's': scale = std::max(1.f, scale - 0.05f); std::cout << "[BGVF] " << "min_count = " << scale << std::endl;
                detector_.set_scale(scale); break;
            case 'A': a_min += 10; std::cout << "[BGVF] " << "a_min = " << int(a_min) << std::endl;
                preprocessor_.set_a_min(a_min); break;
            case 'a': a_min -= 10; std::cout << "[BGVF] " << "a_min = " << int(a_min) << std::endl;
                preprocessor_.set_a_min(a_min); break;
            case 'B': std::cout << "[BGVF] " << "b_min = " << int(++b_min) << std::endl;
                preprocessor_.set_b_min(b_min); break;
            case 'b': std::cout << "[BGVF] " << "b_min = " << int(--b_min) << std::endl;
                preprocessor_.set_b_min(b_min); break;
			case 'T' : refiner_threshold += (refiner_threshold < 99);
				std::cout << "[BGVF] " << "RefinerThreshold = " << float(refiner_threshold) * 0.01f << std::endl;
				refiner_.set_threshold(float(refiner_threshold) * 0.01f); break;
			case 't' : refiner_threshold -= (refiner_threshold > 0);
				std::cout << "[BGVF] " << "RefinerThreshold = " << float(refiner_threshold) * 0.01f << std::endl;
                refiner_.set_threshold(float(refiner_threshold) * 0.01f); break;
			case 'F' : refiner_flatten += (refiner_flatten < 99);
				std::cout << "[BGVF] " << "RefinerFlatten = " << float(refiner_flatten) * 0.01f << std::endl;
				refiner_.set_flatten_limit(float(refiner_flatten) * 0.01f); break;
			case 'f' : refiner_flatten -= (refiner_flatten > 0);
				std::cout << "[BGVF] " << "RefinerFlatten = " << float(refiner_flatten) * 0.01f << std::endl;
                refiner_.set_flatten_limit(float(refiner_flatten) * 0.01f); break;
            }

		}
	}
};