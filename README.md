# CSRT-Descaled
This implementation aims to use openCV CSRT tracker in C++ with higher fps values. For this purpose, it descales the frames and run inference on these frames in order to increase the fps if it is possible when fps is lower than the threshold.

## openCV CSRT Tracker C++ implementation

This implementation is tested on openCV 4.6 and 4.7 versions.\
\
\
It is specifically designed to be conveniently used in applications where the bbox coordinates and the tracking signal is recieved from outside of the program.

This program will print the parameter values, explained in the "Additional usage details" section.

Then, it will print the default openCV CSRT tracker parameters. If one wants to make experiments on this parameters, they can be easily changed.

Then, it will print the widht, height and FPS of the captured frames.

In run-time, the information messages will be printed when tracking is initialized and descaling operations are done.




### Building and Running
First, download the files
Then, go in to the project folder. Create a build directory and navigate to it.
```
mkdir build
cd build
```
After this, perform cmake and make commands.
```
cmake ..
make
```
Finally, you are ready to run the program.\
In the example usage below:\
\
The first parameter, 0.22 means the width of initial bbox is the %22 percent of the capture width.\
The second parameter, 0.3 means the height of initial bbox is the %30 percent of the capture height.\
The third parameter, 14 means if the mean FPS is lower than 14, the program will try to increase the FPS by descaling if possible.
```sh
./csrt_descaled_tracker 0.22 0.3 14
```
For additianol parser usage, the necessary details are given below: 

#### Additional usage details

This implementation takes additional arguments to classical openCV implementations. These parser arguments are:
+ The bbox width and bbox height. These are ratios and they have to be between 0 and 1. By default, bbox_w=0.22, bbox_h=0.3
+ The minimum FPS value. This indicates the program will try to increase the FPS by descaling if mean FPS is lower than this. By default, FPS_THRESHOLD=14
+ The minimum size of the downscaled frames. This makes the program stop descaling if it will cause the frames width or height to be lower than this value. By defalult, MIN_DOWNSCALED_FRAME_HEIGHT=50
+ The window number to calculate mean FPS. This indicates how many frames will be used to calculate the mean FPS. By default, MOVING_AVG_WINDOW=10.
+ The maximum descaling ratio. If descaling ratio becomes larger than this value, the descaling operation stops. It is recommended to use values of power of two such as 2,4 and 8. By default, MAX_DESCALE_RATIO=4
+ The maximum bbox size. Regardless of how large the bbox width and height ratio is, the actual width and height values of initial bbox in pixels can't become larger than this value. By default, MAX_DESCALE_RATIO=10000 (If a limitation is needed, this value may become handy.)

Possible parser usages:
```sh
./csrt_descaled_tracker <bbox_w> <bbox_h>
./csrt_descaled_tracker <bbox_w> <bbox_h> <FPS_THRESHOLD>
./csrt_descaled_tracker <bbox_w> <bbox_h> <FPS_THRESHOLD> <MIN_DOWNSCALED_FRAME_SIZE>
./csrt_descaled_tracker <bbox_w> <bbox_h> <FPS_THRESHOLD> <MIN_DOWNSCALED_FRAME_SIZE> <WINDOW_COUNT>
./csrt_descaled_tracker <bbox_w> <bbox_h> <FPS_THRESHOLD> <MIN_DOWNSCALED_FRAME_SIZE> <WINDOW_COUNT> <MAX_DESCALE_RATIO>
./csrt_descaled_tracker <bbox_w> <bbox_h> <FPS_THRESHOLD> <MIN_DOWNSCALED_FRAME_SIZE> <WINDOW_COUNT> <MAX_DESCALE_RATIO> <BBOX_MAX_SIZE>
```
