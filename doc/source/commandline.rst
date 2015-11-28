.. _alpr_command_line:

************************
Command Line Utility
************************


The OpenALPR Command Line Interface (CLI) utility is a great way to quickly test ALPR against images, videos, or webcams.  It is not recommended for sophisticated integration, since each time the CLI utility loads, it takes a number of seconds to initialize all of the OpenALPR recognition data.

Usage
------

::

       alpr  [-c <country_code>] [--config <config_file>] [-n <topN>] [--seek
             <integer_ms>] [-p <pattern code>] [--motion] [--clock] [-d] [-j]
             [--] [--version] [-h] <> ...


    Where: 

       -c <country_code>,  --country <country_code>
         Country code to identify (either us for USA or eu for Europe). 
         Default=us

       --config <config_file>
         Path to the openalpr.conf file

       -n <topN>,  --topn <topN>
         Max number of possible plate numbers to return.  Default=10

       --seek <integer_ms>
         Seek to the specied millisecond in a video file. Default=0

       -p <pattern code>,  --pattern <pattern code>
         Attempt to match the plate number against a plate pattern (e.g., md
         for Maryland, ca for California)

       --motion
         Use motion detection on video file or stream.  Default=off

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

       <>  (accepted multiple times)
         (required)  Image containing license plates


Examples
-----------

This  command  will  attempt to recognize number plates in the /source/image.jpg image using the European-style recognition data.  The config
file is not provided on the CLI, so it will use the value in the environment variable 'OPENALPR_CONFIG_FILE'  if  provided,  or  the  default
location.

::

    $ alpr -c eu /source/image.jpg

This command will attempt to recognize number plates in the /source/image.png image using the default USA-style recognition data.  The config
file is not provided on the CLI, so it will read the configuration data from /tmp/openalpr.conf

::

    $ alpr --config /tmp/openalpr.conf /source/image.png

This command will attempt to recognize number plates in all jpeg images in the current directory image using the USA-style recognition data.

::

    $ alpr -c us *.jpg

This command reads data from an input video (/source/video.mp4) and outputs recognition data as JSON.

::

    $ alpr -j /source/video.mp4

This command processes a list of image files provided in /source/imagefilelist.txt and writes JSON results to /out/recognitionresults.txt.

::

    $ alpr -j stdin < /source/imagefilelist.txt > /out/recognitionresults.txt

This command processes video from your webcam.  You can also use /dev/video0, /dev/video1, etc.  if you have multiple webcams.

::

    $ alpr webcam



