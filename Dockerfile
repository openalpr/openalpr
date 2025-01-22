# Dockerfile
FROM ubuntu:18.04

# Install prerequisites
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake \
    curl \
    git \
    libcurl3-dev \
    libleptonica-dev \
    liblog4cplus-dev \
    libopencv-dev \
    libtesseract-dev \
    python3 \
    python3-pip \
    wget

# Ensure pip is up-to-date
RUN pip3 install --upgrade pip

# Install MarkupSafe separately to avoid issues
RUN pip3 install markupsafe

# Install Python requirements
COPY requirements.txt /srv/openalpr/requirements.txt
RUN pip3 install -r /srv/openalpr/requirements.txt

# Copy all data
COPY . /srv/openalpr

# Setup the build directory
RUN mkdir /srv/openalpr/src/build
WORKDIR /srv/openalpr/src/build

# Setup the compile environment
RUN cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_INSTALL_SYSCONFDIR:PATH=/etc .. && \
    make -j2 && \
    make install

# Copy the Python server script
COPY alpr_server.py /srv/openalpr/api/alpr_server.py

# Set the working directory for the API
WORKDIR /srv/openalpr/api

# Expose the port for the HTTP server
EXPOSE 8000

# Start the HTTP server
CMD ["python3", "alpr_server.py"]
