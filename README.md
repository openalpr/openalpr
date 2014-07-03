openalpr
========

OpenALPR is an open source *Automatic License Plate Recognition* library written in C++.  The library analyzes images and identifies license plates.  The output is the text representation of any license plate characters found in the image.

Check out a live online demo here: http://www.openalpr.com/demo.html

User Guide
-----------


OpenALPR includes a command line utility.  Simply typing "alpr [image file path]" is enough to get started recognizing license plate images.

For example, the following output is created by analyzing this image:
![Plate Image](http://www.openalpr.com/images/demoscreenshots/plate3.png "Input image")


The library is told that this is a Missouri (MO) license plate which validates the plate letters against a regional template.

```
user@linux:~/openalpr$ alpr ./samplecar.png -t mo -r ~/openalpr/runtime_dir/

plate0: top 10 results -- Processing Time = 58.1879ms.
    - PE3R2X     confidence: 88.9371	 template_match: 1
    - PE32X      confidence: 78.1385	 template_match: 0
    - PE3R2      confidence: 77.5444	 template_match: 0
    - PE3R2Y     confidence: 76.1448	 template_match: 1
    - P63R2X     confidence: 72.9016	 template_match: 0
    - FE3R2X     confidence: 72.1147	 template_match: 1
    - PE32       confidence: 66.7458	 template_match: 0
    - PE32Y      confidence: 65.3462	 template_match: 0
    - P632X      confidence: 62.1031	 template_match: 0
    - P63R2      confidence: 61.5089	 template_match: 0

```

Detailed command line usage:

```
user@linux:~/openalpr$ alpr --help

USAGE:

   alpr  [-t <region code>] [-r <runtime_dir>] [-n <topN>]
         [--seek <integer_ms>] [-c <country_code>]
         [--clock] [-d] [-j] [--] [--version] [-h]
         <image_file_path>


Where:

   -t <region code>,  --template_region <region code>
     Attempt to match the plate number against a region template (e.g., md
     for Maryland, ca for California)

   -r <runtime_dir>,  --runtime_dir <runtime_dir>
     Path to the OpenAlpr runtime data directory

   -n <topN>,  --topn <topN>
     Max number of possible plate numbers to return.  Default=10

   --seek <integer_ms>
     Seek to the specied millisecond in a video file. Default=0

   -c <country_code>,  --country <country_code>
     Country code to identify (either us for USA or eu for Europe).
     Default=us

   --clock
     Measure/print the total time to process image and all plates.
     Default=off

   -d,  --detect_region
     Attempt to detect the region of the plate image.  Default=off

   -j,  --json
     Output recognition results in JSON format.  Default=off

   --,  --ignore_rest
     Ignores the rest of the labeled arguments following this flag.

   --version
     Displays version information and exits.

   -h,  --help
     Displays usage information and exits.

   <image_file_path>
     (required)  Image containing license plates


   OpenAlpr Command Line Utility
```


Compiling
-----------

OpenALPR compiles and runs on Linux, Mac OSX and Windows.

OpenALPR requires the following additional libraries:

    - Tesseract OCR v3.0.3 (https://code.google.com/p/tesseract-ocr/)
    - OpenCV v2.4.8+ (http://opencv.org/)

After cloning this GitHub repository, you should download and extract Tesseract and OpenCV source code into their own directories.  Compile both libraries.

Please follow these detailed compilation guides for your respective operating system:

* [Windows] (https://github.com/openalpr/openalpr/wiki/Compilation-instructions-(Windows))
* [Ubuntu Linux] (https://github.com/openalpr/openalpr/wiki/Compilation-instructions-(Ubuntu-Linux))

If all went well, there should be an executable named *alpr* along with *libopenalpr-static.a* and *libopenalpr.so* that can be linked into your project.


Questions
---------
Please post questions or comments to the Google group list: https://groups.google.com/forum/#!forum/openalpr


Contributions
-------------
Code contributions are not the only way to help out.  Do you have a large library of license plate images?  Do you have time to "tag" plate images in an input image?  If so, please add a note to the forum.

Donations
---------
OpenALPR provides bounties for open issues.  Other developers can fix/enhance the library and claim those bounties.  The best way to donate to the project is to add a bounty to one or more of the open issues on BountyShare

[![Bountysource](https://www.bountysource.com/badge/team?team_id=830&style=bounties_received)](https://www.bountysource.com/teams/openalpr/issues?utm_source=OpenALPR&utm_medium=shield&utm_campaign=bounties_received)


License
-------

Affero GPLv3
http://www.gnu.org/licenses/agpl-3.0.html
