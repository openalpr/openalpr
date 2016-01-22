************************
Compiling
************************

Windows
=============

Using Precompiled Binaries
---------------------------

Precompiled binaries are available for Windows in both 32 and 64-bit format.  You can find the latest binaries on the `GitHub releases page <https://github.com/openalpr/openalpr/releases>`_.  This is the simplest option for installing OpenALPR onto a Windows machine.  Included in the package are the Visual Studio 2015 runtime drivers.  You must install this first in order to use the OpenALPR software.


Compiling OpenALPR via Build Script
------------------------------------

`OpenALPR Windows Build Scripts <https://github.com/peters/openalpr-windows>`_


Compile OpenALPR and Dependencies Manually
--------------------------------------------

`Video Tutorial <http://youtu.be/ooPln41Q6iM>`_

* Ensure you have a version of Visual Studio that is at least 2008 or above
* Download and install `cmake for windows <http://www.cmake.org/cmake/resources/software.html>`_
* Download `OpenCV for Windows <http://opencv.org/>`_ (3.0.0 at this time)
* Download `Tesseract OCR source code and VS2008 project files <https://code.google.com/p/tesseract-ocr/downloads/list>`_ (3.04 at this time)
* Download `Leptonica vs2008 development package <https://code.google.com/p/leptonica/downloads/list>`_ (Tesseract requirement)
* Download `OpenALPR source code <https://github.com/openalpr/openalpr>`_ from GitHub
* Create a libraries directory and put opencv and Tesseract into them
* Compile OpenCV.  `cd` into the opencv directory and type cmake . to create the VisualStudio project.
* Compile Tesseract.  Tesseract requires that you point it to the leptonica headers and the library binaries before it will compile. Note that tesseract needs to be compiled as a LIBRARY.  Make sure also that your compile mode matches for each of the projects (e.g., Release vs Debug).
* Update the CMakeLists.txt file in the OpenALPR src directory to point to the folders for your Tesseract and OpenCV libraries
* Depending on the versions of the libraries, you may need to tweak the linked libraries in here as well (e.g., liblept168 may be liblept173 at some point in the future).  These correspond to the library files for Leptonica and Tesseract.
* CD into the openalpr src directory and type "cmake ."
* Open the Visual Studio solution and compile.
* If all goes well, there should be an "alpr" executable.  In order to execute it, you will need a number of DLLs from OpenCV.  simply finding them in the OpenCV directories and copying them over to your executable should do the trick.



Linux
=============

OpenALPR compiles on many flavors of Linux.  However, the software is officially supported on Ubuntu 14.04 64-bit.

Using Precompiled Binaries
---------------------------

Precompiled binaries are available for Ubuntu 14.04 64-bit OS.  This is the simplest option for installing OpenALPR onto a Linux machine.  The precompiled binaries are managed via APT, so upgrades will automatically be installed when you use the *apt-get update && apt-get upgrade* command.  To install the precompiled binaries, run the following commands on your Ubuntu machine.

.. code-block:: bash 

    wget -O - http://deb.openalpr.com/openalpr.gpg.key | sudo apt-key add -
    echo "deb http://deb.openalpr.com/master/ trusty main" | sudo tee /etc/apt/sources.list.d/openalpr.list
    sudo apt-get update
    sudo apt-get install openalpr openalpr-daemon openalpr-utils libopenalpr-dev

Test the library

.. code-block:: bash 

    # Test US plates
    wget http://plates.openalpr.com/ea7the.jpg
    alpr -c us ea7the.jpg

    # Test European plates
    wget http://plates.openalpr.com/h786poj.jpg
    alpr -c eu h786poj.jpg

Compiling OpenALPR
-------------------

.. code-block:: bash 

    # Install prerequisites
    sudo apt-get install libopencv-dev libtesseract-dev git cmake build-essential libleptonica-dev
    sudo apt-get install liblog4cplus-dev libcurl3-dev

    # If using the daemon, install beanstalkd
    sudo apt-get install beanstalkd

    # Clone the latest code from GitHub
    git clone https://github.com/openalpr/openalpr.git

    # Setup the build directory
    cd openalpr/src
    mkdir build
    cd build

    # setup the compile environment
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_INSTALL_SYSCONFDIR:PATH=/etc ..

    # compile the library
    make

    # Install the binaries/libraries to your local system (prefix is /usr)
    sudo make install

    # Test the library
    wget http://plates.openalpr.com/h786poj.jpg -O lp.jpg
    alpr lp.jpg


Compile OpenALPR and all Dependencies
--------------------------------------

1. Make sure that dependencies and required tools are installed

  * sudo apt-get install libpng12-dev libjpeg62-dev libtiff4-dev zlib1g-dev
  * sudo apt-get install build-essential
  * sudo apt-get install autoconf automake libtool
  * sudo apt-get install git-core
  * sudo apt-get install cmake

2. install opencv (tutorial) 

  * http://docs.opencv.org/2.4/doc/tutorials/introduction/linux_install/linux_install.html

3. download and install leptonica and tesseract-ocr

  * tesseract-ocr requires leptonica and at least one language package.  
  * http://www.leptonica.org/source/leptonica-1.70.tar.gz
  * https://tesseract-ocr.googlecode.com/files/tesseract-ocr-3.02.02.tar.gz
  * https://tesseract-ocr.googlecode.com/files/tesseract-ocr-3.02.eng.tar.gz
  * move the downloaded tarballs to some directory. I will assume that they are located at /usr/local/src/openalpr/. 
 
4. unpack the tarballs: 

  * tar xf /usr/local/src/openalpr/tesseract-ocr-3.02.02.tar.gz 
  * tar xf /usr/local/src/openalpr/tesseract-ocr-3.02.02.eng.tar.gz
  * tar xf /usr/local/src/openalpr/leptonica-1.70.tar.gz
 
5. compile leptonica:

  * cd  /usr/local/src/openalpr/leptonica-1.70/
  * ./configure --prefix=/usr/local
  * make
  * make install
 
6. compile tesseract:

  * cd /usr/local/src/openalpr/tesseract-ocr/
  * ./autogen.sh
  * ./configure
  * make
  * sudo make install
  * sudo ldconfig

7. clone the openalpr repo to /usr/local/src/openalpr/ directory

  * cd /usr/local/src/openalpr/
  * git clone https://github.com/openalpr/openalpr.git

8. update CMakeLists.txt compile openalpr

  * cd /usr/local/src/openalpr/openalpr/
  * gedit CMakeLists.txt &
  * SET(OpenCV_DIR "/usr/local/lib")
  * SET(Tesseract_DIR "/usr/local/src/openalpr/tesseract-ocr")
  * cmake ./
  * make

Note: For Tesseract 3.04 the source files can be downloaded from the main svn branch or https://drive.google.com/folderview?id=0B7l10Bj_LprhQnpSRkpGMGV2eE0&usp=sharing#list. 


Mac OS X
=========

Instructions for compiling on OS X, tested on OS X 10.9.5 (Mavericks).

Using Homebrew
---------------

  * brew tap homebrew/science
  * brew install openalpr

Compiling OpenALPR Manually
----------------------------

.. code-block:: bash 

    # Clone the latest code from GitHub
    git clone https://github.com/openalpr/openalpr.git

    # Setup the build directory
    cd openalpr/src
    mkdir build
    cd build

    # setup the compile environment
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_INSTALL_SYSCONFDIR:PATH=/etc ..

    # compile the library
    make

    # Install the binaries/libraries to your local system (prefix is /usr)
    sudo make install

    # Test the library
    wget http://easy-clan.com/ski/pics/license_plate.JPG -O lp.jpg
    alpr lp.jpg

Mobile (iOS and Android)
==============================

The OpenALPR library compiles on Android and iOS.  Example reference apps are available:

  - `Android <https://github.com/sujaybhowmick/OpenAlprDroidApp>`_
  - `iOS <https://github.com/twelve17/openalpr-ios>`_

Docker
=============

OpenALPR supports containerization inside Docker.  It uses Ubuntu 14.04 as a base image, and installs all the software using pre-compiled binaries.  Download the OpenALPR DockerFile and run the following commands to build it:

.. code-block:: bash

    # Build docker image
    docker build -t openalpr https://github.com/openalpr/openalpr.git

    # Download test image
    wget http://plates.openalpr.com/h786poj.jpg

    # Run alpr on image
    docker run -it --rm -v $(pwd):/data:ro openalpr -c eu h786poj.jpg