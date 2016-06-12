.. _alprd:

************************
OpenALPR Agent (alprd)
************************

The OpenALPR daemon allows you to monitor a camera stream for license plate numbers in the background.  Alprd runs as a daemon process on Linux.  The plate numbers can be streamed to another server (via HTTP posts) or can be consumed programmatically via a beanstalkd queue.

Architecture
=============

Alprd operates as follows:
  1. The image stream is constantly pulled from the IP camera via MJPEG over HTTP
  2. alprd processes the stream as fast as it can looking for plate images.  The daemon automatically skips frames to stay in-sync with clock time.
  3. When one or more plates are detected, the information is written to a local beanstalkd queue (tube name is alprd) as JSON data.
  4. Optionally, alprd will also save the image as a jpeg and save it to a configurable location.
  5. Optionally, alprd also runs a separate process that drains the beanstalkd queue and uploads data to a remote HTTP server via POST.

Alprd can be used in two modes:
  1. Recognition results are streamed to an HTTP server
  2. Recognition results can be read from the beanstalkd queue

::

    +------------------+                     +-------------+         
    |                  |  MJPEG       POST   |             |         
    |  Network Camera  | <---+      +------> | HTTP Server |         
    |                  |     |      |        |             |         
    +------------------+     |      |        +-------------+         
                             |      |                                
                             |      |                                
                             |      |                                
                     +-------+------+                                
                     |              |                                
                     | alprd server |                                
                     |              |                                
                     +---------+----+------------+                   
                               |                 |                   
                               | Beastalkd queue |                   
                               |                 |                   
                               +-----------------+                   


The diagram above shows alprd being used to stream data to another HTTP server.  alprd is configured with a remote HTTP address.  As plates are identified, the server sends the JSON data to the remote HTTP server.  The beanstalkd queue and the alprd process are colocated on the same server.

::

    +------------------+                                         
    |                  |  MJPEG                                  
    |  Network Camera  | <---+                                   
    |                  |     |                                   
    +------------------+     |                                   
                             |                       +----------+
                             |                       |Processing|
                             |                       +----+-----+
                     +-------+------+                     |      
                     |              |                     |      
                     | alprd server |                     |      
                     |              |                     |      
                     +---------+----+------------+        |      
                               |                 |        |      
                               | Beastalkd queue | <------+      
                               |                 |               
                               +-----------------+               


The diagram above shows alprd being used without the HTTP server.  In this case, a beanstalkd consumer can be used to drain the results from the beanstalkd queue.  The beanstalkd tube name is "alprd."  Beanstalkd consumers can be written in any language, and can be colocated on the alprd server or located elsewhere.

Open Source Agent
====================

Configuration
-------------

.. code-block:: ini

    [daemon]

    ; Declare each stream on a separate line
    ; each unique stream should be defined as stream = [url]
    
    stream = http://10.1.2.3/camera1/stream.mjpeg
    stream = http://10.1.2.5/camera2/stream.mjpeg
    
    site_id = headquarters-usa 
    
    store_plates = 1
    store_plates_location = /var/www/html/plates/
    
    ; upload address is the destination to POST to
    upload_data = 0
    upload_address = http://localhost:9000/alpr/push/

alprd needs at least one "stream" defined.  This is just the URL for the mjpeg stream.  You may use multiple streams on one server -- each stream spawns a separate process that attempts to use a full CPU core.

The site-id will be stored along with the JSON plate results.  This is especially useful if you have multiple servers and need to keep track of where the results are coming from.  Additionally, each result will contain a camera ID (numbered 1 to n) based on the order of your "stream" statements in the alprd.conf file


Results
---------
The following is an example of the JSON results.  These results are initially stored in the beanstalkd queue, and then optionally sent in an HTTP post.

.. code-block:: json

    {
      "uuid": "f11e0acc-6aaf-4817-9299-9e6773043b8e",
      "camera_id": 1,
      "site_id": "headquarters",
      "img_width": 640,
      "img_height": 480,
      "epoch_time": 1402161050,
      "processing_time_ms": 138.669163,
      "results": [
        {
          "plate": "S11FRE",
          "confidence": 77.130661,
          "matches_template": 0,
          "region": "",
          "region_confidence": 0,
          "coordinates": [
            {
              "x": 218,
              "y": 342
            },
            {
              "x": 407,
              "y": 325
            },
            {
              "x": 407,
              "y": 413
            },
            {
              "x": 218,
              "y": 431
            }
          ],
          "candidates": [
            {
              "plate": "S11FRE",
              "confidence": 77.130661,
              "matches_template": 0
            },
            {
              "plate": "S11ERE",
              "confidence": 75.496307,
              "matches_template": 0
            },
            {
              "plate": "S11RE",
              "confidence": 75.440361,
              "matches_template": 0
            },
            {
              "plate": "S11CRE",
              "confidence": 75.340179,
              "matches_template": 0
            },
            {
              "plate": "S11FHE",
              "confidence": 75.240974,
              "matches_template": 0
            },
            {
              "plate": "S11EHE",
              "confidence": 73.606621,
              "matches_template": 0
            },
            {
              "plate": "S11HE",
              "confidence": 73.550682,
              "matches_template": 0
            },
            {
              "plate": "S11CHE",
              "confidence": 73.450493,
              "matches_template": 0
            },
            {
              "plate": "S11FBE",
              "confidence": 71.782944,
              "matches_template": 0
            },
            {
              "plate": "S11FE",
              "confidence": 71.762756,
              "matches_template": 0
            }
          ]
        },
        {
          "plate": "EJLESSIE",
          "epoch_time": 1402158050,
          "confidence": 78.167984,
          "matches_template": 0,
          "region": "",
          "region_confidence": 0,
          "processing_time_ms": 51.650604,
          "coordinates": [
            {
              "x": 226,
              "y": 369
            },
            {
              "x": 348,
              "y": 348
            },
            {
              "x": 355,
              "y": 406
            },
            {
              "x": 231,
              "y": 429
            }
          ],
          "candidates": [
            {
              "plate": "EJLESSIE",
              "confidence": 78.167984,
              "matches_template": 0
            },
            {
              "plate": "EDLESSIE",
              "confidence": 77.61319,
              "matches_template": 0
            }
          ]
        }
      ]
    }


Commercial Agent
====================

`*` Requires Commercial License

The commercial OpenALPR Agent has a number of :ref:`enhancements <commercial_enhancements>`.  With these enhancements, the commercial agent is significantly faster when analyzing video streams.

Installation
--------------

On an Ubuntu 14.04 64-bit server:

Add the OpenALPR GPG key and setup the OpenALPR deb repository

.. code-block:: bash

    wget -O - http://deb.openalpr.com/openalpr.gpg.key | sudo apt-key add -
    echo "deb http://deb.openalpr.com/commercial/ trusty main" | sudo tee /etc/apt/sources.list.d/openalpr.list

    sudo apt-get update && sudo apt-get -y install openalpr openalpr-daemon 

Edit the configuration file

.. code-block:: bash

    sudo nano /etc/openalpr/alprd.conf

Configure the following properties:

  - company_id (using the value found on the web server)
  - site_id (a unique text value that identifies this agent)
  - stream (The mjpeg URL for the camera stream.  Multiple streams are added as additional stream entries)
  - analysis_threads (number of CPU cores dedicated to OpenALPR processing)
  - upload_address (set this to http://[*Web_server_IP*]/push/)
  - store_plates_maxsize_mb (amount of space used for storing license plate images)

Test the daemon from the CLI to make sure your settings are correct.  Press CTRL+C once you see that the video stream is connected.

    alprd -f

If the MJPEG stream is configured correctly, the logs should show that the video is being retrieved

    DEBUG - Video FPS: 10

Generate a license key for the agent:

.. code-block:: bash

    cd ~
    wget http://deb.openalpr.com/register.py
    sudo openalpr-licenserequest --license_count 0 > /dev/null && sudo python register.py -u [Username] -p [Password] -c [Number of camera licenses]

The number of camera licenses corresponds to the maximum number of simultaneous video streams that this agent will support.

Each time you run this utility, it will transfer the requested number of licenses from our online registration server to your agent server. 

.. _commercial_config_options:

Commercial Configuration Options
-----------------------------------


The /etc/openalpr/alprd.conf file has additional properties that control the commercial-only features:

The commercial software will group similar plate numbers together and send a JSON post that contains the UUIDs of all of the plate images in the group.  These configuration files control the grouping sensitivity.

.. code-block:: ini

    plate_groups_enabled = 1
    plate_groups_time_delta_ms = 4500
    plate_groups_min_plates_to_group = 3


The commercial daemon will maintain a disk quota and will automatically clean up after itself.  Images stored to disk will be removed on a FIFO-basis.  These parameters control the maximum size on disk and the frequency of clean-up.

.. code-block:: ini

    store_plates_maxsize_mb = 8000
    store_plates_cleanup_interval_seconds = 120


The upload threads control the number of simultaneous data uploads.  Generally, the number of threads would only need to be increased if the web server is slow to respond.

.. code-block:: ini

    upload_threads = 2


Motion detection is an effective way to improve efficiency and get more plate reads from the available processors.  The ALPR processing will only analyze frames with movement, and will ignore areas of the image that have not changed.  This parameter controls whether motion detection is enabled (1) or disabled (0).

.. code-block:: ini

    motion_detection = 1


The ALPR processing occurs in parallel, each thread will consume an entire CPU core.  Allocating more CPUs for ALPR processing will linearly increase the number of plate reads per second

.. code-block:: ini

    analysis_threads = 2


With motion detection enabled, it's possible to buffer moving images and analyze them as CPU becomes available.  For example, if a car moves past the camera over 10 frames, and the CPU can only analyze 3 of those frames during that period, the buffer will allow you to analyze all 10 frames.  After the movement is complete and there is no other activity, the ALPR process will drain the buffer looking for license plates during the period of movement.  Increasing the buffer_size will increase the duration of this period, but will also consume more memory.  The camera_buffer_size unit is number of image frames.

.. code-block:: ini

        camera_buffer_size = 60    

    

Group Results
--------------

ALPR group results are sent in the following JSON format:

.. code-block:: json

    {
      "version": 1,
      "data_type": "alpr_group",
      "epoch_start": 1448326208789,
      "epoch_end": 1448326210046,
      "company_id": "44c9274f-8d57-432a-8cd7-8c991f22ead3",
      "site_id": "dimitar-video",
      "camera_id": 1414799424,
      "uuids": [
        "dimitar-video-cam1414799424-1448326208789",
        "dimitar-video-cam1414799424-1448326209041",
        "dimitar-video-cam1414799424-1448326209648",
        "dimitar-video-cam1414799424-1448326209975",
        "dimitar-video-cam1414799424-1448326210046"
      ],
      "best_plate": {
        "plate": "A13709",
        "confidence": 92.075203,
        "matches_template": 0,
        "plate_index": 0,
        "region": "mo",
        "region_confidence": 0,
        "processing_time_ms": 96.942459,
        "requested_topn": 10,
        "coordinates": [
          {
            "x": 204,
            "y": 134
          },
          {
            "x": 709,
            "y": 222
          },
          {
            "x": 658,
            "y": 500
          },
          {
            "x": 167,
            "y": 383
          }
        ],
        "candidates": [
          {
            "plate": "A13709",
            "confidence": 92.075203,
            "matches_template": 0
          },
          {
            "plate": "A137O9",
            "confidence": 83.32373,
            "matches_template": 0
          },
          {
            "plate": "A137Q9",
            "confidence": 83.001274,
            "matches_template": 0
          },
          {
            "plate": "A137D9",
            "confidence": 82.240067,
            "matches_template": 0
          },
          {
            "plate": "AI3709",
            "confidence": 80.574394,
            "matches_template": 0
          },
          {
            "plate": "A137U9",
            "confidence": 80.238014,
            "matches_template": 0
          },
          {
            "plate": "A137G9",
            "confidence": 78.271233,
            "matches_template": 0
          },
          {
            "plate": "A137B9",
            "confidence": 78.039688,
            "matches_template": 0
          },
          {
            "plate": "A3709",
            "confidence": 77.410858,
            "matches_template": 0
          },
          {
            "plate": "AI37O9",
            "confidence": 71.822922,
            "matches_template": 0
          }
        ]
      },
      "best_confidence": 92.075203,
      "best_uuid": "dimitar-video-cam1414799424-1448326208789",
      "best_plate_number": "A13709"
    }

Heartbeat
-------------

Every minute, the OpenALPR agent adds a heartbeat message to the queue.  The heartbeat provides general health and status information.  The format is as follows:

.. code-block:: json

    {
      "version": 1,
      "data_type": "heartbeat",
      "company_id": "xxxxxxxx-yyyy-yyyy-yyyy-zzzzzzzzzzzz",
      "site_id": "your-unique-sitename",
      "timestamp": 1453426302097,
      "system_uptime_seconds": 2595123,
      "daemon_uptime_seconds": 2594832,
      "internal_ip_address": "192.168.0.54",
      "cpu_cores": 2,
      "cpu_last_update": 1453426297878,
      "cpu_usage_percent": 18.579235,
      "disk_quota_total_bytes": 8000000000,
      "disk_quota_consumed_bytes": 0,
      "disk_quota_last_update": 1453408254586,
      "memory_consumed_bytes": 2083704832,
      "memory_last_update": 1453426297878,
      "memory_swapused_bytes": 0,
      "memory_swaptotal_bytes": 1608511488,
      "memory_total_bytes": 2099093504,
      "processing_threads_active": 1,
      "processing_threads_configured": 1,
      "beanstalk_queue_size": 0,
      "video_streams": [
        {
          "camera_id": 1630410444,
          "fps": 12,
          "is_streaming": true,
          "url": "rtsp://192.168.0.5:554/axis-media/media.amp?videocodec=h264&resolution=1280x720&compression=30&mirror=0&rotation=0&textposition=top&text=1&clock=1&date=0&overlayimage=0&fps=11&keyframe_interval=32&videobitrate=0&maxframesize=0",
          "last_update": 1453426302086
        }
      ]
    }

Web Services
-------------

The OpenALPR daemon exposes a simple web service for retrieving license plate images.  Each image is referenced by a UUID that is sent along with the JSON metadata.

Assuming that the daemon port is set to the default (8355), the full image is referenced using the following URL:

  - http://[*Agent_IP*]:8355/img/[*plate_event_uuid*].jpg

In some cases, you may prefer to just retrieve a cropped image around the license plate.  This would require significantly less bandwidth than downloading the entire source image.  The X and Y coordinates can be computed from the JSON metadata x/y coordinates of the license plate.  The x1/y1 coordinates reference the top-left of the license plate crop region, and the x2/y2 coordinates reference the bottom right.  For example, assuming the crop is located at (477,258), (632,297):

  - http://[*Agent_IP*]:8355/crop/[*plate_event_uuid*]?x1=477&y1=258&x2=632&y2=297

Additionally, the web server exposes a `web service API <api/>`_ for searching license plates and groups.  Detailed documentation is available in the :ref:`web server section <web_services_api>`

OpenALPR Agent Docker Container
----------------------------------

The OpenALPR Agent Docker container runs the OpenALPR agent within Docker given a license.conf and alprd.conf file that you supply.  The image is built on top of Ubuntu 14.04 64-bit.  When the container is launched, it runs both Beanstalkd and the alprd daemon.  The container exposes the following ports:

  1. Port 11300 - Beanstalkd
  2. Port 8355 - The OpenALPR daemon web service

The /var/lib/openalpr/plateimages/ folder is exposed as a volume that can be attached to the host file system.  This location will contain license plate images in a rolling buffer with a maximum size specified in alprd.conf

