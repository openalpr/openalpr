
.. _language_bindings:

********************
OpenALPR API
********************


OpenALPR is available as a C/C++ library and has bindings in C#, Java, and Python.  Additionally, the software can be used as a "black box" that can process video streams and make the data available to another system (using any programming language).

C/C++
=================

First, download or compile the OpenALPR library onto your target platform.  Make sure that the software runs by testing it using the alpr command-line executable.  Pre-compiled binaries are available for 32/64-bit Windows and Ubuntu Linux.

1. Add "alpr.h" as an include file to your project.
2. Include the openalpr.dll (Windows) or libopenalpr.so (Unix) file with your binaries
3. Include all other required shared libraries
4. Put the openalpr.conf and runtime_data directory in the same location as the binaries.  Alternatively, you can specify the location of the runtime_data in openalpr.conf or directly in the code.  

Below is a simple usage example of using the OpenALPR library:

.. code-block:: c++ 
   :linenos:

    // Initialize the library using United States style license plates.  
    // You can use other countries/regions as well (for example: "eu", "au", or "kr")
    alpr::Alpr openalpr("us", "/path/to/openalpr.conf");

    // Optionally specify the top N possible plates to return (with confidences).  Default is 10
    openalpr.setTopN(20);

    // Optionally, provide the library with a region for pattern matching.  This improves accuracy by 
    // comparing the plate text with the regional pattern.
    openalpr.setDefaultRegion("md");

    // Make sure the library loaded before continuing.  
    // For example, it could fail if the config/runtime_data is not found
    if (openalpr.isLoaded() == false)
    {
        std::cerr << "Error loading OpenALPR" << std::endl;
        return 1;
    }

    // Recognize an image file.  You could alternatively provide the image bytes in-memory.
    alpr::AlprResults results = openalpr.recognize("/path/to/image.jpg");

    // Iterate through the results.  There may be multiple plates in an image, 
    // and each plate return sthe top N candidates.
    for (int i = 0; i < results.plates.size(); i++)
    {
      alpr::AlprPlateResult plate = results.plates[i];
      std::cout << "plate" << i << ": " << plate.topNPlates.size() << " results" << std::endl;
      
        for (int k = 0; k < plate.topNPlates.size(); k++)
        {
          alpr::AlprPlate candidate = plate.topNPlates[k];
          std::cout << "    - " << candidate.characters << "\t confidence: " << candidate.overall_confidence;
          std::cout << "\t pattern_match: " << candidate.matches_template << std::endl;
        }
    }


C# and VB.NET
====================

.. code-block:: c#
   :linenos:

    using openalprnet;

    var alpr = new AlprNet("us", "/path/to/openalpr.conf", "/path/to/runtime_data");
    if (!alpr.IsLoaded())
    {
        Console.WriteLine("OpenAlpr failed to load!");
        return;
    }
    // Optionally apply pattern matching for a particular region
    alpr.DefaultRegion = "md";

    var results = alpr.Recognize("/path/to/image.jpg");

    foreach (var result in results.Plates)
    {
        Console.WriteLine("Plate {0}: {1} result(s)", i++, result.TopNPlates.Count);
        Console.WriteLine("  Processing Time: {0} msec(s)", result.ProcessingTimeMs);
        foreach (var plate in result.TopNPlates)
        {
            Console.WriteLine("  - {0}\t Confidence: {1}\tMatches Template: {2}", plate.Characters,
                              plate.OverallConfidence, plate.MatchesTemplate);
        }
    }




Python
====================

.. code-block:: python
   :linenos:

    from openalpr import Alpr

    alpr = Alpr("us", "/path/to/openalpr.conf", "/path/to/runtime_data")
    if not alpr.is_loaded():
        print("Error loading OpenALPR")
        sys.exit(1)
        
    alpr.set_top_n(20)
    alpr.set_default_region("md")

    results = alpr.recognize_file("/path/to/image.jpg")

    i = 0
    for plate in results['results']:
        i += 1
        print("Plate #%d" % i)
        print("   %12s %12s" % ("Plate", "Confidence"))
        for candidate in plate['candidates']:
            prefix = "-"
            if candidate['matches_template']:
                prefix = "*"

            print("  %s %12s%12f" % (prefix, candidate['plate'], candidate['confidence']))

    # Call when completely done to release memory
    alpr.unload()



Java
====================

.. code-block:: java
   :linenos:

    import com.openalpr.jni.Alpr;
    import com.openalpr.jni.AlprPlate;
    import com.openalpr.jni.AlprPlateResult;
    import com.openalpr.jni.AlprResults;

    Alpr alpr = new Alpr("us", "/path/to/openalpr.conf", "/path/to/runtime_data");

    // Set top N candidates returned to 20
    alpr.setTopN(20);

    // Set pattern to Maryland
    alpr.setDefaultRegion("md");

    AlprResults results = alpr.recognize("/path/to/image.jpg");
    System.out.format("  %-15s%-8s\n", "Plate Number", "Confidence");
    for (AlprPlateResult result : results.getPlates())
    {
        for (AlprPlate plate : result.getTopNPlates()) {
            if (plate.isMatchesTemplate())
                System.out.print("  * ");
            else
                System.out.print("  - ");
            System.out.format("%-15s%-8f\n", plate.getCharacters(), plate.getOverallConfidence());
        }
    }

    // Make sure to call this to release memory
    alpr.unload();


Node.js
====================

A Node.js binding to OpenALPR is available here:
https://www.npmjs.com/package/node-openalpr

The source code is available here:
https://github.com/netPark/node-openalpr


.. _cloud_api:

Cloud API (Commercial)
=======================

The OpenALPR Cloud API is a web-based service that analyzes images for license plates as well as vehicle information such as make, model, and color.  
The Cloud API service is easy to integrate into your application via a web-based REST service.  You send image data to the OpenALPR API, we process the data, 
and return JSON data describing the license plate and vehicle.

Check out the online demo: http://www.openalpr.com/demo-image.html

Sign-Up
---------

When you're ready to get started, sign-up for an account at https://cloud.openalpr.com/

Once enrolled, you will automatically be assigned a free account that has a limited number of API credits per month.  Each time you use the service, you use one or more 
API credits.  You may enter your credit card information and upgrade your plan to give you access to more credits per month.

Integrate
----------

Because the OpenALPR Cloud API is REST-based, it works with any programming language on any operating system.  You can make API calls using whatever method
you prefer.

To make integration easier, the OpenALPR Cloud API also includes permissively licensed open source client libraries in a variety of languages.  
The GitHub repo is available here: https://github.com/openalpr/cloudapi

Check out the `REST API documentation <api/cloudapi.html>`_ for more detailed information about the REST service.  
This is generated from the `OpenALPR Cloud API Swagger definition <api/specs/cloudapi.yaml>`_


.. _alpr_web_service:

Docker-Based Web Service (Commercial)
======================================

The OpenALPR Library Docker container provides the OpenALPR image processing as a web service.  In this mode, images are sent to OpenALPR via HTTP POST, and OpenALPR responds with the metadata describing all license plates in the image.  This docker image exposes port 8080.

Requests into this service are sent as HTTP POST requests to:

  http://[*ip_address*]:8080/v1/identify/plate

The post should contain this parameter:

  image - A file containing a JPEG image

Results will be sent back in the following JSON format:


.. code-block:: json

    {
        "data_type": "alpr_results",
        "epoch_time": 1448299357883,
        "img_height": 480,
        "img_width": 640,
        "results": [
            {
                "plate": "AKS4329",
                "confidence": 86.457352,
                "region_confidence": 95,
                "region": "ga",
                "plate_index": 0,
                "processing_time_ms": 84.982811,
                "candidates": [
                    {
                        "matches_template": 0,
                        "plate": "AKS43Z9",
                        "confidence": 88.429092
                    },
                    {
                        "matches_template": 1,
                        "plate": "AKS4329",
                        "confidence": 86.457352
                    },
                    {
                        "matches_template": 0,
                        "plate": "AKS3Z9",
                        "confidence": 79.028625
                    },
                    {
                        "matches_template": 0,
                        "plate": "AKS329",
                        "confidence": 77.056877
                    }
                ],
                "coordinates": [
                    {
                        "y": 128,
                        "x": 286
                    },
                    {
                        "y": 129,
                        "x": 360
                    },
                    {
                        "y": 159,
                        "x": 360
                    },
                    {
                        "y": 157,
                        "x": 286
                    }
                ],
                "matches_template": 1,
                "requested_topn": 20
            }
        ],
        "version": 2,
        "processing_time_ms": 172.226624,
        "regions_of_interest": []
    }

OpenALPR Agent
====================

OpenALPR can also be configured as a "black box" that makes data available to other systems.  When configured in this mode, OpenALPR is installed as a Linux daemon, and is configured to monitor one or more MJPEG video streams.  It automatically processes the images and produces JSON data describing the license plates found int he images.  This data can either be pushed to another server (as an HTTP POST) or pulled from another server (via beanstalkd queue).

More information about the OpenALPR agent is available here: :ref:`alprd`