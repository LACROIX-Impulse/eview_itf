Generality {#mainpage}
=========

## Principle of eViewItf

The eCube is composed of several parts. Three main software are present:
-	eView
-	Customer application
-	eVision

eView is the LACROIX Impulse solution to handle the camera and display pipeline. It is a bare metal software running on a real time ARM Cortex R7 CPU.
The Customer application is an adaptative software that can be designed for any customer needs. It runs in a Linux OS on an ARM Cortex A53 CPU.
eVision is the LACROIX Impulse perception stack solution to apply detections, tracking or other perception algorithms on a camera frame received from eView. It also runs in a Linux OS on the ARM Cortex A53 CPU but can also use the hardware accelerators of the platform.
eViewItf is the link between eView and the Customer application. It allows the Customer application to easily use and control the video and display pipeline. Thanks to eView and eViewItf, the Customer application does not need to take care of the camera initialization, the frames receptions, the display management and all the things related to the cameras and the display. eView is made to handle all these things and eViewItf will control eView according to the Customer application requests.

@image html block_diagram.png "eCube block diagram"
@image latex block_diagram.png "eCube block diagram"

## eViewItf features

As presented above, eViewItf allows the Customer application to control eView. Therefore, the features of eViewItf are linked to eView.
eViewItf allows to use and control 8 cameras. These cameras can be of different types and can be connected to the eCube through the Fakra connectors. The cameras features are as follows:
-	Display the camera input on the screen connected to the eCube (can select one of the cameras input to be printed on the display).
-	Get the camera’s frames (to be able to process them).
-	Get or set a specific camera parameter (for instance the camera exposure).

A cropping can also be applied on the displayed camera input in order to only print on the screen a desired ROI.
In addition to the 8 cameras input, 8 streamers are also present. These streamers can be used to replay a previously recorded camera input, to display a modified frame or more generally to print whatever the user want to the display connected to the eCube.
Finally, eViewItf can also control eView to set a blender over the currently displayed frame. This blender can be used to overlay a user defined picture over the frames printed on the display. This feature is useful to print extra data on the display. For instance, it can be used to add the bounding boxes of a detection directly on the display while also printing the frames that comes from the camera. This way, both the real-time input frames and the detections results can be printed on screen. Another example could be to print some statistics or performances of the algorithm used to processed the received frames (real-time on display information). Two blender interfaces are available for convenience purposes.

All these features can be used through the eViewItf API. Presenting this API is the purpose of the next part.
