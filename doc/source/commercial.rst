.. _commercial:

*************************
Commercial Enhancements
*************************

OpenALPR is commercially supported open source software.  OpenALPR is licensed under dual licenses to meet the needs of open source users as well as for-profit commercial entities.  The software may be used under the terms of the Affero `GNU Public License v3 (AGPL) <http://www.gnu.org/licenses/agpl-3.0.html>`_.  However, this license has strong copyleft requirements that most for-profit companies cannot use.  For this reason, we also offer the software under a commercial license.  Please contact info@openalpr.com for license pricing and terms.

The OpenALPR commercial license overrides the AGPL terms and allows OpenALPR to be used without copyleft requirements.  The software may then be used, integrated, and distributed in closed-source proprietary applications.

Additionally, there are a number of features and enhancements that are available exclusively to commercial customers.  

.. _commercial_enhancements:

  - Multi-core processing
  - Efficient processing via motion detection
  - License plate grouping
  - High-accuracy US state recognition
  - Support for H264 video streams
  - On-Premises web server
  - Video file processing

Multi-Core Processing
-----------------------

The OpenALPR agent utilizes multiple CPU cores in parallel to improve the analysis frame rate.  Faster processing allows OpenALPR to record number plates for vehicles at higher speeds and will also contribute to higher accuracy at lower speeds due to plate grouping.  The "analysis_threads" configuration property in alprd.conf controls the number of simmultaneous CPU cores used to process license plates.  Additionally, if a GPU is available (either via OpenCL or Nvidia CUDA) the agent can make use of this to accelerate license plate processing.  

Efficient Processing via Motion Detection
-------------------------------------------

Utilizing motion detection greatly increases the efficiency of the OpenALPR agent.  Rather than monitoring all pixels of every frame in a video, the software ignores areas in the video that have not changed (and therefore could not contain a license plate).  When motion is detected, only the portion where the vehicle is located will be analyzed.  

To provide the most possible reads, OpenALPR also utilizes a configurable image buffer.  When there is lots of motion detected, the video frames are placed into this buffer and processed.  Therefore, if the video has moments of inactivity, the CPU resources will remain utilized processing older video data in order to provide the most possible license plate reads.

License Plate Grouping
-----------------------

In a video stream, a single license plate is often seen many times as it travels past the camera.  For example, if the vehicle passes the camera over the course of 2 seconds at 30 frames per second, OpenALPR may recognize that same license plate 60 times.  The plate grouping feature tracks the license plate as it moves, and delivers a single result for the license plate number that is scored based on the number of recognitions.  Therefore, high-speed processing produces a highly accurate license plate number.

High-Accuracy US State Recognition
------------------------------------

This feature determines the US state for a given license plate.  For example, OpenALPR will differentiate a Maryland license plate versus one from California.  This also increases accuracy, since each state has a unique text pattern.  Knowing the originating state for the license plate allows OpenALPR to match the text results against the unique state pattern.

Support for H264 Video Streams
--------------------------------

The OpenALPR Commercial agent has more ubiquitous support for connecting to IP camera video streams.  In addition to MJPEG, OpenALPR also supports H264 over both RTSP and HTTP.

On-Premises Web Server
-------------------------

The commercial web server is a data repository for license plate information.  License plates is browsable, searchable, and triggers e-mail alerts for matching plate numbers.

Video File Processing
----------------------

OpenALPR has a utility that efficiently processes video files to produce a CSV output containing all the license plates found in the video stream.


