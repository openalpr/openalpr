openalpr
========

OpenALPR is an open source *Automatic License Plate Recognition* library written in C++ with bindings in C#, Java, Node.js, Go, and Python.  The library analyzes images and video streams to identify license plates.  The output is the text representation of any license plate characters.

Check out a live online demo here: http://www.openalpr.com/demo-image.html

User Guide
-----------


OpenALPR includes a command line utility.  Simply typing `alpr [image file path]` is enough to get started recognizing license plate images.

For example, the following output is created by analyzing this image:

<img src="https://images.cdn.circlesix.co/image/1/640/0/uploads/posts/2021/07/b22c2b504ab94949ea0df2a8789d0727.jpg" width="200">

```
user@linux:~/openalpr$ wget https://images.cdn.circlesix.co/image/1/640/0/uploads/posts/2021/07/b22c2b504ab94949ea0df2a8789d0727.jpg
user@linux:~/openalpr$ alpr -c gb ./b22c2b504ab94949ea0df2a8789d0727.jpg

plate0: 10 results
  - DH05PLH   confidence: 90.4896
  - DHO5PLH   confidence: 83.3287
  - DHD5PLH   confidence: 82.5504
  - 0H05PLH   confidence: 81.9928
  - BH05PLH   confidence: 79.4239
  - DHG5PLH   confidence: 79.0606
  - DM05PLH   confidence: 79.0074
  - DH05PLM   confidence: 78.603
  - H05PLH    confidence: 78.3127
  - DH5PLH    confidence: 77.0984
```

Detailed command line usage:

```
user@linux:~/openalpr$ alpr --help

USAGE: 

   alpr  [-c <country_code>] [--config <config_file>] [-n <topN>] [--seek
         <integer_ms>] [-p <pattern code>] [--clock] [-d] [-j] [--]
         [--version] [-h] <image_file_path>


Where: 

   -c <country_code>,  --country <country_code>
     Country code to identify (either us for USA or eu for Europe). 
     Default=us

   --config <config_file>
     Path to the openalpr.conf file

   -n <topN>,  --topn <topN>
     Max number of possible plate numbers to return.  Default=10

   --seek <integer_ms>
     Seek to the specified millisecond in a video file. Default=0

   -p <pattern code>,  --pattern <pattern code>
     Attempt to match the plate number against a plate pattern (e.g., md
     for Maryland, ca for California)

   --clock
     Measure/print the total time to process image and all plates. 
     Default=off

   -d,  --detect_region
     Attempt to detect the region of the plate image.  [Experimental] 
     Default=off

   -j,  --json
     Output recognition results in JSON format.  Default=off

   --,  --ignore_rest
     Ignores the rest of the labeled arguments following this flag.

   --version
     Displays version information and exits.

   -h,  --help
     Displays usage information and exits.

   <image_file_path>
     Image containing license plates


   OpenAlpr Command Line Utility

```


Binaries
----------

Pre-compiled Windows binaries can be downloaded on the [releases page](https://github.com/openalpr/openalpr/releases)

Install OpenALPR on Ubuntu 16.04 with the following commands:

    sudo apt-get update && sudo apt-get install -y openalpr openalpr-daemon openalpr-utils libopenalpr-dev

Documentation
---------------

Detailed documentation is available at [doc.openalpr.com](http://doc.openalpr.com/)

Integrating the Library
-----------------------

OpenALPR is written in C++ and has bindings in C#, Python, Node.js, Go, and Java.  Please see this guide for examples showing how to run OpenALPR in your application: http://doc.openalpr.com/bindings.html

Compiling
-----------

[![Build Status](https://travis-ci.org/openalpr/openalpr.svg?branch=master)](https://travis-ci.org/openalpr/openalpr)

OpenALPR compiles and runs on Linux, Mac OSX and Windows.

OpenALPR requires the following additional libraries:

    - Tesseract OCR v3.0.4 (https://github.com/tesseract-ocr/tesseract)
    - OpenCV v2.4.8+ (http://opencv.org/)

After cloning this GitHub repository, you should download and extract Tesseract and OpenCV source code into their own directories.  Compile both libraries.

Please follow these detailed compilation guides for your respective operating system:

* [Windows](https://github.com/openalpr/openalpr/wiki/Compilation-instructions-(Windows))
* [Ubuntu Linux](https://github.com/openalpr/openalpr/wiki/Compilation-instructions-(Ubuntu-Linux))
* [OS X](https://github.com/openalpr/openalpr/wiki/Compilation-instructions-(OS-X))
* [Android Library](https://github.com/SandroMachado/openalpr-android)
* [Android Application Sample](https://github.com/sujaybhowmick/OpenAlprDroidApp)
* [iOS](https://github.com/twelve17/openalpr-ios)
* [iOS React Native](https://github.com/cardash/react-native-openalpr)
* [Xamarin](https://github.com/kevinjpetersen/openalpr-xamarin)

If all went well, there should be an executable named *alpr* along with *libopenalpr-static.a* and *libopenalpr.so* that can be linked into your project.

Docker
------

``` shell
# Build docker image
docker build -t openalpr https://github.com/openalpr/openalpr.git
# Download test image
wget http://plates.openalpr.com/h786poj.jpg
# Run alpr on image
docker run -it --rm -v $(pwd):/data:ro openalpr -c eu h786poj.jpg
```

Questions
---------
Please post questions or comments to the Google group list: https://groups.google.com/forum/#!forum/openalpr


Contributions
-------------
Improvements to the OpenALPR library are always welcome.  Please review the [OpenALPR design description](https://github.com/openalpr/openalpr/wiki/OpenALPR-Design) and get started.

Code contributions are not the only way to help out.  Do you have a large library of license plate images?  If so, please upload your data to the anonymous FTP located at upload.openalpr.com.  Do you have time to "tag" plate images in an input image or help in other ways?  Please let everyone know by posting a note in the forum.


License
-------

Affero GPLv3
http://www.gnu.org/licenses/agpl-3.0.html

Commercial-friendly licensing available.  Contact: info@openalpr.com
