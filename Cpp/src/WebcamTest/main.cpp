/*____________________________________________________
**
**	Title:			Live Cross Detector Demo
**	Author:			ECP Hexa Team - Gabriel Huang
**	Date:		    2014
**  License:        MIT
**____________________________________________________
*/

#include <video_flow.h>
#include <ctime>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    typedef unsigned int UInt;
    typedef unsigned char UChar;

    // Parameters
    UInt    wait_delay = 5; // (ms)

    UInt	resize_cols = 640;//320;
    UInt	resize_rows = 480;//240;
    UInt	median_filter_bandwith = 7;
    UChar	a_min = 135; // set to 145 for stronger filter (partly removes skin color)
    UChar	b_min = 122; // set to 130 for stronger filter (partly removes skin color)
    UChar   l_max = 200;

    UInt	min_size = 16;
    UInt	min_count = 34;
    float	scale = 1.2f;

	UChar	refiner_threshold = 50;
	UChar	refiner_flatten = 91;

    string cross_cascade_name = "Z:\\Cpp\\UAV\\Cpp\\res\\cascade_5.xml";

    try
    {
        // Video Input
        cv::Mat raw_frame;
        cv::Mat processed_frame;
		cv::Mat processed_hard;
        cv::VideoCapture camera;
		
        // Parse args
        if (argc == 2)
        {
            cross_cascade_name = argv[1];
        }
        cout << "Selected cascade : " << cross_cascade_name << endl;

        cout << "Reading from camera 0" << endl;
        camera = cv::VideoCapture(0);
        if (!camera.isOpened()) {
            throw std::runtime_error("Error opening video capture\n");
        }

        // Preprocessor
        Preprocess preprocessor;
        preprocessor.set_l_max(l_max).set_a_min(a_min).set_b_min(b_min);
        preprocessor.set_resize(resize_rows, resize_cols);
        preprocessor.set_median_filter_bandwith(median_filter_bandwith);
        preprocessor.set_debug(false);

        // Detector
        Detect detector;
        detector.set_classifier(cross_cascade_name);
        detector.set_min_cols(min_size).set_min_rows(min_size);
        detector.set_min_count(min_count);
        detector.set_scale(scale);
        detector.set_verbose(true);

		// Crossness (refiner)
		Refiner refiner;
		refiner.set_threshold(float(refiner_threshold) / 100.f);
		refiner.set_flatten_limit(float(refiner_flatten) / 100.f);

		// Kalman filtering
		//float last_prediction_date = float(clock()) / float(CLOCKS_PER_SEC);
		//float last_detection_date = 0.f;
		//float max_misdetection_time = 3.f; // seconds
		//bool cross_mode = false;
		Kalman kalman;
		//float x = 500.f;
		kalman.set_process_position_variance(100.f);
		kalman.set_process_speed_variance(1.f);
		float last_prediction_date = float(clock()) / float(CLOCKS_PER_SEC);

        while (true)
        {
            // Get frame
            camera >> raw_frame;
            if(raw_frame.empty())
            {
                continue;
            }

            // Preprocess
            preprocessor.get_soft(raw_frame, processed_frame);
			//preprocessor.get(raw_frame, processed_hard);
            float ratio_x = float(raw_frame.cols) / processed_frame.cols;
            float ratio_y = float(raw_frame.rows) / processed_frame.rows;

            // Detect
            vector<cv::Rect> occurences;
            detector.detect(processed_frame, occurences);
			cout << endl << "Detected " << occurences.size() << endl;

			// Show Detections
            for (UInt i = 0; i < UInt(occurences.size()); ++i)
            {
				float cness = refiner.flatten(refiner.crossness(processed_frame(occurences[i])));
				cout << "Crossness for occurence " << i << " = " << cness << endl;
				
                cv::Rect adjusted_rect(int(ratio_x * occurences[i].x),
                    int(ratio_y * occurences[i].y),
                    int(ratio_x * occurences[i].width),
                    int(ratio_y * occurences[i].height));
                cv::rectangle(raw_frame, adjusted_rect, cv::Scalar(cness * 255.f, 0.f, 0.f, 3));
            }

			// Kalman Update by dt
			float new_prediction_date = float(clock()) / float(CLOCKS_PER_SEC);
			kalman.advance(new_prediction_date - last_prediction_date);

			// Get most probable center
			cv::Point center;
			bool actually_found = refiner.get_best(processed_frame, occurences, center);
			if(actually_found)
			{
				// Draw Refined Center
				cv::Point adjusted_center(int(ratio_x * float(center.x)), int(ratio_y * float(center.y)));
				cv::circle(raw_frame, adjusted_center, 16, cv::Scalar(0.f, 255.f, 0.f, 0), CV_FILLED);
				cout << "Cross @ " << adjusted_center << endl;

				// Kalman Correct
				cv::Mat_<float> measurement(2,1);
				measurement(0) = center.x;
				measurement(1) = center.y;
				kalman.update(measurement);
			}

			// Kalman final prediction
			cv::Mat prediction = kalman.guess();
			last_prediction_date = new_prediction_date;

			// Draw frame if in cross mode : red if actually detected, yellow if prediction
			if(kalman.still_tracking())
			{
				cv::rectangle(raw_frame, cv::Rect(0, 0, raw_frame.cols - 1, raw_frame.rows - 1), 
						      actually_found ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 255), 20);
				cv::Point predicted_center = cv::Point(prediction.at<float>(0),prediction.at<float>(1));
				cv::Point adjusted_predicted_center(int(ratio_x * float(predicted_center.x)), int(ratio_y * float(predicted_center.y)));
				cv::circle(raw_frame, adjusted_predicted_center, 28, cv::Scalar(255.f, 0.f, 255.f, 0), CV_FILLED);
			}

            // Show frames
            cv::imshow("Raw Frame", raw_frame);
			cv::imshow("Processed Soft", processed_frame);

            // Delay & Tune
            char key = cv::waitKey(wait_delay);
            switch (key)
            {
            case 'C': min_count += 4; cout << "min_count = " << min_count << endl;
                detector.set_min_count(min_count); break;
            case 'c': min_count = max(1, int(min_count) - 5); cout << "min_count = " << min_count << endl;
                detector.set_min_count(min_count); break;
            case 'S': scale += 0.05f; cout << "scale = " << scale << endl;
                detector.set_scale(scale); break;
            case 's': scale = max(1.f, scale - 0.05f); cout << "min_count = " << scale << endl;
                detector.set_scale(scale); break;
            case 'A': a_min += 10; cout << "a_min = " << int(a_min) << endl;
                preprocessor.set_a_min(a_min); break;
            case 'a': a_min -= 10; cout << "a_min = " << int(a_min) << endl;
                preprocessor.set_a_min(a_min); break;
            case 'B': cout << "b_min = " << int(++b_min) << endl;
                preprocessor.set_b_min(b_min); break;
            case 'b': cout << "b_min = " << int(--b_min) << endl;
                preprocessor.set_b_min(b_min); break;
			case 'T' : refiner_threshold += (refiner_threshold < 99);
				cout << "RefinerThreshold = " << float(refiner_threshold) * 0.01f << endl;
				refiner.set_threshold(float(refiner_threshold) * 0.01f); break;
			case 't' : refiner_threshold -= (refiner_threshold > 0);
				cout << "RefinerThreshold = " << float(refiner_threshold) * 0.01f << endl;
                refiner.set_threshold(float(refiner_threshold) * 0.01f); break;
			case 'F' : refiner_flatten += (refiner_flatten < 99);
				cout << "RefinerFlatten = " << float(refiner_flatten) * 0.01f << endl;
				refiner.set_flatten_limit(float(refiner_flatten) * 0.01f); break;
			case 'f' : refiner_flatten -= (refiner_flatten > 0);
				cout << "RefinerFlatten = " << float(refiner_flatten) * 0.01f << endl;
                refiner.set_flatten_limit(float(refiner_flatten) * 0.01f); break;
            }
        }
    }
    catch (const std::runtime_error& msg)
    {
        cout << "Exception : " << msg.what() << endl;
    }

    cout << "Done" << endl;
    cin.get();
    return 0;
}