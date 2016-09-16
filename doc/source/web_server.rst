.. _web_server:

***********************
OpenALPR Web Server 
***********************

`*` Requires Commercial License

Requirements
================

OpenALPR monitors video streams in real-time to gather all license plates seen by your cameras. This data is browsable, searchable, and can trigger alerts. The Web server is available either:

  #. In the cloud
  #. Installed On-Premises within your network

If you haven't seen the `live demo <http://www.openalpr.com/demo-cloud.html>`_, check it out! It shows a realistic example of the type of information you will soon be collecting from your cameras.

In order to get started, you will need:

  1. An IP camera capable of serving MJPEG or H264 video
  2. A computer (Intel i5 or better) with network access to the IP camera.  This will be your OpenALPR Agent.

First, configure your IP camera to capture the area that you wish to monitor. The camera must be capable of capturing a clear image of the license plate in order for OpenALPR to properly identify the numbers. You may want to experiment with different angles, optical zoom levels, and resolutions to get the best image quality. A straight-on shot of the license plate is best, but that is usually not possible, so OpenALPR can work with shots at an angle.  Try to angle the camera so that the plate is clearly visible, and the vehicle is seen for as long as possible.

Once your camera is setup, make sure that it has an IP address and that you can connect to the MJPEG or H264 stream.  A useful, free utility for testing the video URL is `VLC Media Player <http://www.videolan.org/>`_.  You can test your video URL by selecting File -> Open Media Stream.  Type in the MJPEG or H264 stream URL and you should be able to clearly see your video. 

Some cameras support arguments in the URL to control resolution, frame-rate, etc. You may reduce the frames per second (fps) of the video feed in order to reduce the stream bandwidth.  Fifteen frames per second is usually more than sufficient to capture passing vehicles from a fixed-camera.  The resolution also should not be too high.  A resolution of 720p is generally sufficient for capturing license plates as long as the plate characters are legible.  Higher resolution often results in longer processing time without a gain in accuracy.

Architecture
=============

The OpenALPR agent reads a video stream from your IP camera, processes it, and uploads plate metadata to the OpenALPR web server. The agent also stores all of the plate images in a rolling buffer on the hard drive.

There is a constant stream of data flowing between the camera and the agent as well as between the agent and the web server.  The data sent to the cloud is relatively low-bandwidth because it contains text metadata describing the license plates.  The OpenALPR Web Server does not store your plate images, these are downloaded directly from the agent when you select a plate to view from the web server.

OpenALPR Agent
===============================

The OpenALPR Agent is a service that runs as a background task on your PC.  The agent analyzes video streams from one or more IP cameras and finds the license plates for the vehicles that pass by the camera.  We recommend a dedicated PC for the agent due to the amount of CPU used during processing; however, it can be installed on any machine.  The plate numbers may be kept completely within your network (on the On-Premise web server), or sent to the OpenALPR Cloud.

  * If you wish to use the OpenALPR Cloud to store your data, first `sign-up for an account <https://cloud.openalpr.com/account/register>`_. 
  * If you wish to use the On-Premises web server, request an evaluation key from `info@openalpr.com <mailto:info@openalpr.com?subject=Requesting%20OpenALPR%20Evaluation%20Key>`_ and install the Web Server using the Linux Installer instructions below.

Windows Installer
------------------

  1. Download the `OpenALPR Windows Installer <http://deb.openalpr.com/windows/openalpr-latest.exe>`_
  2. Install the program onto your PC
  3. Start the "Configure OpenALPR" program after the install completes
  4. Depending on how you wish to use the OpenALPR agent, you may choose one of four radio buttons

  .. image:: images/agent-windows-config.png
      :scale: 100%
      :alt: Windows Agent Configuration


  a. **OpenALPR Cloud** - Type in the E-mail address and password that you used to sign-up for the OpenALPR Cloud Service
  b. **OpenALPR On-Premises Web Server** -- Type in the URL of the on-premises web server and the e-mail address / password of the master account
  c. **Generic HTTP URL** -- Used for :ref:`Integrating other applications with the OpenALPR Agent <alprd>`.
  d. **Local Queue** -- Another method for :ref:`Integrating other applications with the OpenALPR Agent <alprd>`.

  You have now successfully connected the OpenALPR Agent with the OpenALPR web server.  All configuration / management is performed centrally on the OpenALPR web server.  The next step is to :ref:`configure the agent and add video streams to monitor <agent_configuration>`


Linux Installer
----------------------

Download the Ubuntu 16.04 64-bit install DVD image and burn to a DVD:

  - http://releases.ubuntu.com/16.04/ubuntu-16.04.1-desktop-amd64.iso  

Follow this installation guide to install Ubuntu 16.04 64-bit:

  - http://www.ubuntu.com/download/desktop/install-ubuntu-desktop

Run the following command from the terminal:

.. code-block:: bash

    bash <(curl http://deb.openalpr.com/install)


.. image:: images/linux-install.png
    :scale: 100%
    :alt: OpenALPR VM installation step 4

- Choose one of the following:  

  - To connect the agent to the OpenALPR Cloud, just choose "install agent". 
  - To host the data on your own web server (On-Premises configuration) choose "install webserver"


.. _agent_configuration:

Configuration
===============================

- Login to the OpenALPR Web Server.  

- Select Configuration -> Agents from the menu on the left-hand side of the page

- You should see your new agent on this screen.  Select "Configure" to setup the camera.

- Select **Add Stream** to connect your agent to the camera stream.

.. image:: images/webserver_vminstall5.png
    :scale: 100%
    :alt: OpenALPR VM installation step 4

- Select the model of IP camera you wish to connect to.  Fill in the IP address.  If the camera requires credentials, check the box and enter your camera's username and password.
- Click **Test**.  After a few seconds, you will see a window indicating whether the connection was successful or not.  If it was successful, click **Save Camera**.  Otherwise, try another option (such as H264 Alt1 or MJPEG) and click **Test** again until you succeed.

.. image:: images/webserver_vminstall-testsuccess.png
    :scale: 100%
    :alt: OpenALPR VM installation step 4

- Next, configure the **Agent Parameters**.  

  - Choose a sensible name for your **Site ID**.  This is usually the location of the agent system (e.g., headquarters, dallas-branch, warehouse3, etc.).  Each agent should be given a unique Site ID.
  - Choose the **country** where the camera is located.  US will recognize North American-style plates (12 inches x 6 inches).  EU will recognize European-style plates.  There is also support for other countries that have plates with different dimensions.
  - The number of **Processing Cores**  controls how much CPU is allocated to the LPR process.  The more processing cores you provide (up to the number of CPU cores on the system) the more frames per second (fps) you can process.  Higher fps generally contributes to better accuracy and capability to detect plates on faster moving vehicles.
  - **Disk Quota** controls how much space is reserved for storing vehicle and license plate images.  It operates as a rolling buffer, so once it runs out of space, the oldest images are removed.
  - **Pattern** should be country (in Europe) that the camera is located in.  In the US, OpenALPR uses a high-accuracy state detection algorithm to detect the state of origin, so it is better to leave the pattern set to "None" for recognition in the USA.

- Click **Update**.

.. image:: images/webserver_vminstall6.png
    :scale: 100%
    :alt: OpenALPR VM installation step 4

- Lastly, if you scroll to the bottom of the page you can watch the agent status.  You should now see **Video FPS** and other information indicating that video is being pulled from the camera and license plates are being recognized.  Now that the agent is configured, it will continue collecting data from the configured video streams.  If the agent is rebooted, the OpenALPR agent will automatically start.  If the camera goes down and comes back, or the network is down temporarily, the agent will retry until connectivity is restored.  All results are queued, so no data is lost in the event of an outage.

.. image:: images/webserver_vminstall7.png
    :scale: 100%
    :alt: OpenALPR VM installation step 4


Advanced Configuration
---------------------------------------

The OpenALPR Agent can also be configured manually by modifying the file in /etc/openalpr/alprd.conf.  This is an advanced option, and may be useful when managing dozens/hundreds of agents.  The default values with description is found here: /usr/share/openalpr/config/alprd.defaults.conf.  You may add any value into the alprd.conf file and restart the agent for the changes to be picked up.

Additional documentation on these configuration options is located in the :ref:`commercial_config_options`.

To restart services, run the  command:

.. code-block:: bash

    sudo service openalpr-daemon restart

To watch the OpenALPR logs, run the following command: 

    tail -f /var/log/alpr.log
                    



.. _web_services_api:

Web Services 
====================

The `Web Services API <api/>`_ can be used to programmatically query your On-Premises server for data.  The API is documented `here <api/>`_