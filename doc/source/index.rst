.. openalpr documentation master file, created by
   sphinx-quickstart on Sun Nov 15 23:25:01 2015.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

OpenALPR Documentation
====================================

Getting Started
------------------

OpenALPR is an open source Automatic License Plate Recognition library written in C++ with bindings in C#, Java, Node.js, and Python. The library analyzes images and video streams to identify license plates. The output is the text representation of any license plate characters.

The software can be used in many different ways.  For example, with OpenALPR you can:

  #. Recognize license plates from camera streams.  The results are :ref:`browsable, searchable and can trigger alerts <web_server>`.  The data repository can be in the cloud or stored entirely within your network on-premises. `*`
  #. Recognize license plates from camera streams and :ref:`send the results to your own application <alprd>`.
  #. :ref:`Process a video file <video_processing>` and store the license plate results in a CSV and SQLite database. `*`
  #. Analyze still images from the :ref:`command-line <alpr_command_line>`
  #. Integrate license plate recognition into your application :ref:`directly in-code (C/C++, C#, VB.NET, Java, Python, Node.js) <language_bindings>`
  #. Run OpenALPR as a :ref:`web service <alpr_web_service>`.  A JPG image is sent over HTTP POST, the OpenALPR web service responds with the license plate information in the image. `*`

`*` Requires  :ref:`OpenALPR Commercial License <commercial>`

Contents:

.. toctree::
   :maxdepth: 2

   compiling
   commandline
   bindings
   alprd
   web_server
   video_processing
   accuracy_improvements
   commercial


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

