FROM ubuntu:22.04

# Install prerequisites
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    curl \
    git \
    libcurl4-openssl-dev \
    libleptonica-dev \
    liblog4cplus-dev \
    libopencv-dev \
    libtesseract-dev \
    wget && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Copy all data
COPY . /srv/openalpr

# Setup the build directory
RUN mkdir /srv/openalpr/src/build
WORKDIR /srv/openalpr/src/build

# Setup the compile environment
RUN cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_INSTALL_SYSCONFDIR:PATH=/etc .. && \
    make -j2 && \
    make install

WORKDIR /data

ENTRYPOINT ["alpr"]
