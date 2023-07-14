# CSRT-Descaled

## openCV CSRT Tracker C++ implementation

This implementation is tested on openCV 4.6 and 4.7 versions.

An example usage

## Building and Running




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
