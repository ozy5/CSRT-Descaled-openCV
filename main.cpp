#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <sys/time.h>
using namespace std;
using namespace cv;

//function to convert timeval to desired format
double __get_us(struct timeval t) { return (t.tv_sec * 1000000 + t.tv_usec); }

//FPS lower-bound
int FPS_THRESHOLD = 14;

//The size of the window to calculate mean fps
int MOVING_AVG_WINDOW = 10;

//Lower-bound for width or height of descaled inference frame
int MIN_DOWNSCALED_FRAME_HEIGHT = 50;

//Maximum length of width and height of the bbox for tracking initialization
int BBOX_MAX_SIZE = 10000;

//Maximum descaling ratio. After the k value reaches this value, descaling for inference frame stop.
int MAX_DESCALE_RATIO = 4;

//value for descaling (This value is a place-holder. The actual initial k value is in the while loop.)
int k = 1;

//value for incrementing the descaling factor.
int k_increment;

//determine if downscaling will be used or not. If false, downscaling is off.
bool IS_DOWNSCALING_ON = true;

//initial normalized global values for the static bbox, make them volatile to be able to change them externally
volatile float bbox_x = 0.5;
volatile float bbox_y = 0.5;
volatile float bbox_w = 0.22;
volatile float bbox_h = 0.3;


//boolean value to update the static bbox using normalized global values for the static bbox, make it volatile to be able to change them with interrupts
volatile bool if_bbox_change = true;

//boolean value to check if tracking signal recieved, make it volatile to be able to change them with interrupts
volatile bool track = false;



int main( int argc, char** argv)
{
    //parse the arguments
    if(argc == 3)
    {
        bbox_w = atof(argv[1]);
        bbox_h = atof(argv[2]);
    }
    else if(argc == 4)
    {
        bbox_w = atof(argv[1]);
        bbox_h = atof(argv[2]);
        FPS_THRESHOLD = atoi(argv[3]);
    }
    else if(argc == 5)
    {
        bbox_w = atof(argv[1]);
        bbox_h = atof(argv[2]);
        FPS_THRESHOLD = atoi(argv[3]);
        MIN_DOWNSCALED_FRAME_HEIGHT = atoi(argv[4]);
    }
    else if(argc == 6)
    {
        bbox_w = atof(argv[1]);
        bbox_h = atof(argv[2]);
        FPS_THRESHOLD = atoi(argv[3]);
        MIN_DOWNSCALED_FRAME_HEIGHT = atoi(argv[4]);
        MOVING_AVG_WINDOW = atoi(argv[5]);
    }
    else if(argc == 7)
    {
        bbox_w = atof(argv[1]);
        bbox_h = atof(argv[2]);
        FPS_THRESHOLD = atoi(argv[3]);
        MIN_DOWNSCALED_FRAME_HEIGHT = atoi(argv[4]);
        MOVING_AVG_WINDOW = atoi(argv[5]);
        MAX_DESCALE_RATIO = atoi(argv[6]);
    }
    else if(argc == 9)
    {
        bbox_w = atof(argv[1]);
        bbox_h = atof(argv[2]);
        FPS_THRESHOLD = atoi(argv[3]);
        MIN_DOWNSCALED_FRAME_HEIGHT = atoi(argv[4]);
        MOVING_AVG_WINDOW = atoi(argv[5]);
        MAX_DESCALE_RATIO = atoi(argv[6]);
        BBOX_MAX_SIZE = atoi(argv[7]);
    }
    else if(argc!=1)
    {
        cout << "Possible parser usages: ./csrt_descaled_tracker <bbox_w> <bbox_h> \n ./csrt_descaled_tracker <bbox_w> <bbox_h> <FPS_THRESHOLD> \n ./csrt_descaled_tracker <bbox_w> <bbox_h> <FPS_THRESHOLD> <MIN_DOWNSCALED_FRAME_SIZE> \n ./csrt_descaled_tracker <bbox_w> <bbox_h> <FPS_THRESHOLD> <MIN_DOWNSCALED_FRAME_SIZE> <WINDOW_COUNT>\n ./csrt_descaled_tracker <bbox_w> <bbox_h> <FPS_THRESHOLD> <MIN_DOWNSCALED_FRAME_SIZE> <WINDOW_COUNT> <MAX_DESCALE_RATIO> \n  ./csrt_descaled_tracker <bbox_w> <bbox_h> <FPS_THRESHOLD> <MIN_DOWNSCALED_FRAME_SIZE> <WINDOW_COUNT> <MAX_DESCALE_RATIO> <BBOX_MAX_SIZE> \n \n    " << endl;
        return -1;
    }



    //print the fps threshold and min downscaled frame height
    cout << "FPS_THRESHOLD: " << FPS_THRESHOLD << ", MIN_DOWNSCALED_FRAME_HEIGHT: " << MIN_DOWNSCALED_FRAME_HEIGHT << ", MOVING_WINDOW_AVERAGE: " << MOVING_AVG_WINDOW << endl << endl;





    //set the text attributes for FPS text
    Point top_left_fps(50,50);
    Scalar color_fps(118, 185, 0);

    //create time objects
    struct timeval start_time, stop_time;

    //create values for the actual static_bbox to display
    int bbox_x_px, bbox_y_px, bbox_w_px, bbox_h_px;

    //create rectangle objects for static_bbox, tracked_bbox and downscaled_bbox
    Rect static_bbox, tracked_bbox, downscaled_bbox, ROI;

    //create matricies for frames
    Mat frame, resized_frame;




    //create the parameters object for the CSRT tracker
    TrackerCSRT::Params params;

    //params.admm_iterations = 3;
    //params.number_of_scales = 30;
    //params.window_function = "hann"; //cheb (-), hann(++), kaiser(+)
    //params.num_hog_channels_used = 16;
    //params.template_size = BBOX_MAX_SIZE < 200 ? BBOX_MAX_SIZE : 200;
    //params.template_size = 150;
    //params.scale_model_max_area = 150;
    //params.scale_lr = 0.04;


    //print the parameters
    cout << "printing the parameters: " << endl;
    cout << "admm_iterations: " << params.admm_iterations     << endl;
    cout << "background_ratio: " << params.background_ratio     << endl;
    cout << "cheb_attenuation: " << params.cheb_attenuation     << endl;
    cout << "filter_lr: " << params.filter_lr     << endl;
    cout << "gsl_sigma: " << params.gsl_sigma     << endl;
    cout << "histogram_bins: " << params.histogram_bins     << endl;
    cout << "histogram_lr: " << params.histogram_lr     << endl;
    cout << "hog_clip: " << params.hog_clip     << endl;
    cout << "hog_orientations: " << params.hog_orientations     << endl;
    cout << "kaiser_alpha: " << params.kaiser_alpha     << endl;
    cout << "num_hog_channels_used: " << params.num_hog_channels_used     << endl;
    cout << "number_of_scales: " << params.number_of_scales     << endl;
    cout << "padding: " << params.padding     << endl;
    cout << "psr_threshold: " << params.psr_threshold     << endl;
    cout << "scale_lr: " << params.scale_lr     << endl;
    cout << "scale_model_max_area: " << params.scale_model_max_area     << endl;
    cout << "scale_sigma_factor: " << params.scale_sigma_factor     << endl;
    cout << "scale_step: " << params.scale_step     << endl;
    cout << "template_size: " << params.template_size     << endl;
    cout << "use_channel_weights: " << params.use_channel_weights     << endl;
    cout << "use_color_names: " << params.use_color_names     << endl;
    cout << "use_gray: " << params.use_gray     << endl;
    cout << "use_hog: " << params.use_hog     << endl;
    cout << "use_rgb: " << params.use_rgb     << endl;
    cout << "use_segmentation: " << params.use_segmentation     << endl;
    cout << "weights_l: " << params.weights_lr     << endl;
    cout << "window_function: " << params.window_function     << endl;
    cout << endl << endl << endl << endl;


    //create tracker object
    Ptr<Tracker> tracker = TrackerCSRT::create(params);

    //capture video


    //Change the capture option for capturing without gstreamer

    //VideoCapture cap(0);
    VideoCapture cap("v4l2src device=/dev/video0 ! image/jpeg ! jpegdec ! videoconvert ! appsink");


    //VideoCapture cap("v4l2src device=/dev/video0 ! image/jpeg ! jpegdec ! videoconvert ! appsink");
    //VideoCapture cap("/home/cuda/Desktop/cpp_csrt/car_sample.mp4");

    //if video couldn't captured, exit
    if (!cap.isOpened())
    {
        cout << "Error opening video file " << endl;
        return -1;
    }



    //get the capture dimensions
    int w = cap.get(CAP_PROP_FRAME_WIDTH);
    int h = cap.get(CAP_PROP_FRAME_HEIGHT);
    int org_fps = cap.get(CAP_PROP_FPS);

    //creat value for downscaled frame
    int downscaled_width = w / k;
    int downscaled_height = h / k;

    //create a size object for the downscaled frame
    Size downscaled_frame_size(downscaled_width, downscaled_height);

    //create an fps array for moving window average
    int fps_arr[MOVING_AVG_WINDOW], fps;

    //create a value t detect the pressed key
    int f;

    //create a mean_fps value
    float mean_fps = 0;
    int oldest_fps;

    //create a counter for the fps array
    int  counter=0;

    //boolean value to check if tracking ever initialized.
    bool if_initialized = false;

    //check if downscaling suitable
    bool downscaling_suitable = IS_DOWNSCALING_ON;

    //check if the mean_fps is the mean of last MOVING_AVG_WINDOW frames or not
    bool is_mean_fps_ready = false;


    //create a value for iterations
    int i;


    //create a value for rescaling the detected bbox if k changed
    float ratio;



    //print original dimensions
    cout << "original w: " << w << ",  original h: " << h << ", original fps: " << org_fps << endl << endl;



    //read the first frame
    cap >> frame;

    //if fail to read the frame, exit
    if ( frame.empty() ) 
    { 
        cout << "Error reading the first frame" << endl;
        return -1; 
    }

    //uncomment the lines below the select a region of interes with the first frame, set bbox coordinates according to the selected ROI and start tracking
    /*
    ROI = selectROI("Tracker", frame, true);

    cout << ROI.x << " " << ROI.y << " " << ROI.width << " " << ROI.height << endl;

    bbox_x = (ROI.x + ROI.width/2)/(float)w;
    bbox_y = (ROI.y + ROI.height/2)/(float)h;
    bbox_w = (ROI.width)/(float)w;
    bbox_h = (ROI.height)/(float)h;

    cout << bbox_x << " " << bbox_y << " " << bbox_w << " " << bbox_h << endl;

    track=true;
    */


    //set a named window
    namedWindow("Tracker", 1);


    //perform the tracking process
    printf("Start the tracking process, press ESC to quit, press t to track.\n\n");



    for( ;; )
    {
        //start the timer
        gettimeofday(&start_time, NULL);
        
        //detect key
        f = waitKey(1);

        //if esc is pressed, break
        if(f == 27) break;

        if(f==116) track = true;

        //if there is a change in the bbox coordinates, update the static_bbox and downscaled_bbox
        if(if_bbox_change)
        {
            //update the static_bbox pixel values
            bbox_x_px = bbox_x * w;
            bbox_y_px = bbox_y * h;
            bbox_w_px = bbox_w * w;
            bbox_h_px = bbox_h * h;

            //check if bbox width exceeded the pre-determined upper-bound, resize width if it did.
            if(bbox_w_px > BBOX_MAX_SIZE)
            {
                cout << "the given bbox width is larger than it's limit: " << BBOX_MAX_SIZE << endl;
                bbox_w_px = BBOX_MAX_SIZE;
            }

            //check if bbox height exceeded the pre-determined upper-bound, resize height if it did.
            if(bbox_h_px > BBOX_MAX_SIZE)
            {
                cout << "the given bbox height is larger than it's limit: " << BBOX_MAX_SIZE << endl << endl;
                bbox_h_px = BBOX_MAX_SIZE;
            }
            
            //update the static_bbox
            static_bbox = Rect(bbox_x_px - bbox_w_px/2 ,bbox_y_px - bbox_h_px/2, bbox_w_px, bbox_h_px);

            //update the downscaled_bbox
            downscaled_bbox = Rect((bbox_x_px - bbox_w_px/2)/k ,(bbox_y_px - bbox_h_px/2)/k, (bbox_w_px)/k, (bbox_h_px)/k);

            //set if_bbox_change to false
            if_bbox_change = false;
        }  


        //get frame from the video
        cap >> frame;

        //if fail to read the frame
        if ( frame.empty() ) 
        { 
            cout << "Error while reading the frame" << endl << endl;
            return -1; 
        }


        //resize the frame (descale for inference)
        resize(frame,resized_frame, downscaled_frame_size);

        

        //if track signal recieved, initialize tracking
        if(track)
        {
            //reset the k to 1
            k = 1;

            //set the new downscaled frame dimensions
            downscaled_width = w / k;
            downscaled_height = h / k;
            downscaled_frame_size = Size(downscaled_width, downscaled_height);

            //update the downscaled_bbox
            downscaled_bbox = Rect((bbox_x_px - bbox_w_px/2)/k ,(bbox_y_px - bbox_h_px/2)/k, (bbox_w_px)/k, (bbox_h_px)/k);
            downscaled_frame_size = Size(downscaled_width, downscaled_height);

            //resize the frame
            resize(frame,resized_frame, downscaled_frame_size);


            //initialize the tracker
            tracker->init(resized_frame, downscaled_bbox);

            //print the message
            cout << "Tracker initialized" << endl << endl;

            //set the track value to false again
            track = false;

            //set if_initialized to true
            if_initialized = true;

            //set if downscaling propery will be used.
            downscaling_suitable = IS_DOWNSCALING_ON;

            //reset the mean_fps value and the counter
            mean_fps = 0;
            counter = 0;

            //set is_mean_fps_ready to false
            is_mean_fps_ready = false;
        }






        // update the tracking result if tracking initialized
        if(if_initialized) 
        {
            //re-start the timer for calculating fps if tracking initialized
            gettimeofday(&start_time, NULL);
            //update the tracker
            tracker->update(resized_frame,tracked_bbox);

            //draw the tracked bbox
            rectangle( frame, Rect(tracked_bbox.x*k, tracked_bbox.y*k, tracked_bbox.width*k, tracked_bbox.height*k), Scalar( 255, 0, 0 ), 2, 1 );
        }
        



        // draw the static bbox
        rectangle( frame, static_bbox, Scalar( 0, 255, 0 ), 2, 1 );

        //end the timer
        gettimeofday(&stop_time, NULL);

        //calculate the fps
        fps = 1000000 / (__get_us(stop_time) - __get_us(start_time));

        //put the fps to the frame
        putText(frame, //target image
                "FPS:   " + to_string(fps), //text
                top_left_fps, //top-left position
                FONT_HERSHEY_COMPLEX,
                1.0,
                color_fps, //font color
                2);

        // show image with the tracked object
        imshow("Tracker",frame);

        //check if downscaling is suitable and tracking is initialized
        if (downscaling_suitable && if_initialized)
        {
            //calculate the mean fps in O(1) complexity
            if (is_mean_fps_ready)
            {
                oldest_fps = fps_arr[counter]; //get the oldest fps value in the array 
                mean_fps += (fps - oldest_fps) / (float)MOVING_AVG_WINDOW; //calculate the new mean fps value
            }
            else mean_fps += fps / (float)MOVING_AVG_WINDOW;

            //use the array in a circular way to keep track of the last 'MOVING_AVG_WINDOW' number of fps values.
            fps_arr[counter] = fps;

            if (is_mean_fps_ready) // if the moving window average fps array is full
            {

                //check if the mean fps is lower than the threshold
                if (mean_fps < FPS_THRESHOLD)
                {

                    k_increment = k;

                    //print the message
                    cout << "The mean FPS of the last " << MOVING_AVG_WINDOW << " frames was lower than threshold (" << FPS_THRESHOLD << " > " << mean_fps << ")\nTo increase the fps, increasing the downscaling factor k by " << k_increment << "\nThe change of k value is:  " << k << "---->" << k+k_increment << endl;

                    //set the new downscaling factor
                    k += k_increment;

                    //set the new downscaled frame dimensions
                    downscaled_width = w / k;
                    downscaled_height = h / k;
                    downscaled_frame_size = Size(downscaled_width, downscaled_height);


                    //if the inference frame is too small, stop the future downscalings
                    if((MIN_DOWNSCALED_FRAME_HEIGHT*(k/(float)(k-k_increment)) > downscaled_height) || (MIN_DOWNSCALED_FRAME_HEIGHT*(k/(float)(k-k_increment)) > downscaled_width) || k >= MAX_DESCALE_RATIO) downscaling_suitable = false;
                    

                    //update the downscaled_bbox
                    downscaled_bbox = Rect((bbox_x_px - bbox_w_px/2)/k ,(bbox_y_px - bbox_h_px/2)/k, (bbox_w_px)/k, (bbox_h_px)/k);
                    downscaled_frame_size = Size(downscaled_width, downscaled_height);

                    //resize the frame
                    resize(frame,resized_frame, downscaled_frame_size);

                    //print the new width and height for inference
                    cout << "Now, inferences are being made on frames with " << downscaled_width << "x" << downscaled_height << "resolution" << endl << endl;

                    //calculate the ratio
                    ratio = k/(float)(k-k_increment);                


                    //re-initialize the tracker with the new downscaling factor
                    tracker->init(resized_frame, Rect(tracked_bbox.x/ratio, tracked_bbox.y/ratio, tracked_bbox.width/ratio, tracked_bbox.height/ratio));

                    //reset the mean_fps value and the counter
                    mean_fps = 0;

                    //reset the counter
                    counter = 0;

                    //set is_mean_fps_ready to false
                    is_mean_fps_ready = false;
                }
                else //mean fps is greater than the threshold
                {
                    if(counter == MOVING_AVG_WINDOW - 1) //new counter is the last index of the array
                    {
                        is_mean_fps_ready = true; //set is_mean_fps_ready to true 
                        counter = 0; //reset the counter to the first index of the array
                    }
                    else counter++;
                }
            }
            else //the moving window average fps array is not full or downscaling is not suitable
            {
                if(counter == MOVING_AVG_WINDOW - 1) //new counter is the last index of the array
                {
                    is_mean_fps_ready = true; //set is_mean_fps_ready to true
                    counter = 0; //reset the counter to the first index of the array
                }
                else counter++; //increment the counter
            }
        }
    }
}
