openalpr++
========

OpenALPR++ is fork of OpenALPR (*Automatic License Plate Recognition*) library. In addition to basic functionality provided by openalpr, openalpr++ provides few more internal and intermediate information as a json file.


User Guide
-----------
Please check [OpenALPR](https://github.com/openalpr/openalpr) repository for complete user manual.

```
alpr resources/eu-sweden-licenseplate.png -c eu --debug --config config/openalpr-plusplus.conf | grep JSON > resources/out.json 
cat resources/out.json | grep DEBUG1 > resources/1.json 
cat resources/out.json | grep DEBUG2 > resources/2.json 
sed -e s/DEBUG1_JSON://g -i resources/1.json
sed -e s/DEBUG2_JSON://g -i resources/2.json
firefox resources/1.json resources/2.json

```

Sample JSON outputs [1](resources/1.json),[2](resources/2.json) for the following license plate from [wikipedia](https://en.wikipedia.org/wiki/Vehicle_registration_plates_of_Europe):

![This is an image](resources/eu-sweden-licenseplate.png)

#### Compiling

Tested on Ubuntu 18.04:

```
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    curl \
    git \
    libcurl3-dev \
    libleptonica-dev \
    liblog4cplus-dev \
    libopencv-dev \
    libtesseract-dev \
    wget

git clone https://github.com/ebadi/openalpr-plusplus.git
cd openalpr/src
mkdir build
cd build


cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_INSTALL_SYSCONFDIR:PATH=/etc ..  ;
make -j4
sudo make install
```

in case of the following segfault : `tesseract::LTRResultIterator::WordFontAttributes(bool*, bool*, bool*, bool*, bool*, bool*, int*, int*) const () from /usr/lib/x86_64-linux-gnu/libtesseract.so.4`

```
sudo add-apt-repository ppa:alex-p/tesseract-ocr
sudo apt-get update
sudo apt-get upgrade
```



License
-------

Affero GPLv3
http://www.gnu.org/licenses/agpl-3.0.html


