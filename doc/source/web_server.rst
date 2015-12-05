.. _web_server:

***********************
OpenALPR Web Server 
***********************

`*` Requires Commercial License

Requirements
================

OpenALPR Cloud collects metadata about license plates seen by your cameras. This data is browsable, searchable, and can trigger alerts. The Web server can be available either:

  #. In the cloud
  #. Installed On-Premises within your network

If you haven't seen the `live demo <http://www.openalpr.com/demo-cloud.html>`_, check it out! It shows a realistic example of the type of information you will soon be collecting from your cameras.

In order to get started, you will need:

  1. An IP camera capable of serving MJPEG or H264 video
  2. An OpenALPR agent installed on your network

First, configure your IP camera to capture the area that you wish to monitor. Generally, you'll want the video to be able to capture a clear image of the license plate in order for OpenALPR to properly identify the numbers. You may want to experiment with different optical zoom levels and resolutions to get the best image quality. A straight-on shot of the license plate is best, but OpenALPR can work with shots at an angle.

Once your camera is setup, make sure that it has an IP address and that you can connect to the MJPEG or H264 stream. Some cameras support arguments in the URL to control resolution, frame-rate, etc. The Firefox web browser has the best MJPEG support among major browsers. Type in the MJPEG or H264 stream URL to Firefox and you should be able to clearly see your video. Some users reduce the frames per second (fps) of their video feed in order to reduce the stream bandwidth. 

Architecture
=============

.. image:: images/webserver_architecture.png
    :scale: 100%
    :alt: OpenALPR Web Server Architecture


As depicted in the diagram above, the OpenALPR agent reads a video stream from your IP camera, processes it, and uploads plate metadata to the OpenALPR cloud. The agent also stores all of the plate images on a rolling buffer in its hard drive.

There is a constant stream of data flowing between the camera and the agent as well as between the agent and the cloud. When you log into the OpenALPR Cloud web portal to view your data, that information is retrieved on-demand. OpenALPR Cloud does not store your plate images, these are downloaded directly from the agent when you select a plate to view.


Web Server Installation
===============================

On an Ubuntu 14.04 64-bit server:

Add the OpenALPR GPG key and setup the OpenALPR deb repository

.. code-block:: bash

    wget -O - http://deb.openalpr.com/openalpr.gpg.key | sudo apt-key add -
    echo "deb http://deb.openalpr.com/web_server/ openalpr-web main" | sudo tee /etc/apt/sources.list.d/openalpr-web.list

    sudo apt-get update && sudo apt-get -y install openalpr-web

You will be prompted to create a MySQL database password.  Type in a value that is secure and that you will remember.

Configure OpenALPR by typing:

.. code-block:: bash

    openalpr-web-createdb

You will be prompted for the MySQL database password.  Type in the same value entered above.

Next, you will need to create an administrative user account.  Type in an e-mail address and a password for this new user.  You will use this to login to the web interface once it's started

Connect to the web interface by opening a browser and going to: *http://[System_IP_address]*

If you don't already know the system IP address, it can be found by typing ifconfig.

Login with the user e-mail and password that you just created.

Click on the "Getting Started" link and make note of your "Company ID."  You will need this to configure the agents and to setup API integrations.  This value should be kept secure and should be treated like a password. 



Agent Installation
===================

The :ref:`OpenALPR agent <alprd>`  is installed as either a Virtual Machine (VM) or a Debian install for 64-bit Ubuntu Linux. The VM installs in a matter of minutes and can run on any operating system (e.g., Windows, Linux, Mac OS X). The Debian installer for 64-bit Ubuntu Linux is a more advanced install option.

Virtual Machine Install
-------------------------

- Start downloading the `latest OpenALPR Virtual Machine <http://deb.openalpr.com/downloads/openalpr.ova>`_.
- Download and install the `VirtualBox installer <https://www.virtualbox.org/wiki/Downloads>`_ for your operating system. The OpenALPR VM will also work with other hypervisors such as VMware, Xen, or HyperV if you prefer to use one of those.
- Open VirtualBox and choose File → Import Appliance
- Choose the openalpr.ova file downloaded in step #1
- Accept the default memory and CPU settings and click "Import"

.. image:: images/webserver_vminstall1.png
    :scale: 100%
    :alt: OpenALPR VM installation step 1

- Select the openalpr-daemon-vm and click "Start"
- The VM should boot up quickly and provide you with a login prompt. Login with the default credentials: root/openalpr
- You should see a menu like the one below. Use the up/down, tab, and enter keys to navigate the menu.

.. image:: images/webserver_vminstall2.png
    :scale: 100%
    :alt: OpenALPR VM installation step 2

- First setup the network by choosing Network → eth0.
    - Select either DHCP or static depending on your LAN configuration
    - Select Network → Test and make sure that you can successfully ping www.google.com

.. image:: images/webserver_vminstall3.png
    :scale: 100%
    :alt: OpenALPR VM installation step 3

- Select "Upgrade" from the main menu. The OpenALPR software is updated more frequently than the VM, there may be updates available.
- Optionally, select "Password" from the main menu to change your password to something more secure.
- Choose "Configure" from the main menu to configure the OpenALPR agent.
    - Add your company ID to the company_id parameter. For example, it may read: company_id = ca5e6e0f-4988-4cbb-ba7a-45226b8126d1
    - Choose an appropriate Site ID. Use letters, numbers, and dashes (no spaces or special characters). For example "company-hq" or "leesburg-office-park"
    - Configure your country. You should use "us" for US-style plates (12 inches by 6 inches) or "eu" for European plates (520mm by 110mm)
    - Configure at least one camera stream. This is the MJPEG or H264 URL for your IP camera. Each camera stream should be on one line that starts with stream =. There are a few examples in the config, but they are prefaced with semicolons (which comments them out). Make sure your stream entries do not have a semicolon in front.
    - Use the "tab" key to select OK and press enter to select it.

.. image:: images/webserver_vminstall4.png
    :scale: 100%
    :alt: OpenALPR VM installation step 4

- The menu now monitors the latest output from the OpenALPR process. If there were any problems with the configuration, it will tell you at this point. Otherwise, you'll see output indicating that OpenALPR is actively processing your video stream and uploading the results to the OpenALPR Cloud.

Debian install for 64-bit Ubuntu Linux
---------------------------------------

Alternatively, you may prefer to install the OpenALPR agent directly into an Ubuntu Linux server. These steps are not required if you installed the Virtual Machine referenced above.

First install a copy of 64-bit Ubuntu Linux server and gain console access.

From the terminal:

.. code-block:: bash

    # Install the OpenALPR repository GPG key
    wget -O - http://deb.openalpr.com/openalpr.gpg.key | sudo apt-key add -

    # Add the OpenALPR repository to your server
    echo "deb http://deb.openalpr.com/commercial/ openalpr main" | sudo tee /etc/apt/sources.list.d/openalpr.list

    # Install the OpenALPR software
    sudo apt-get update
    sudo apt-get install openalpr openalpr-daemon openalpr-utils libopenalpr-dev
                        
Edit the configuration file /etc/openalpr/alprd.conf

Configure the company_id, site_id, country, and stream values as described in the Virtual Machine section. Make sure that the value for upload_data is set to 1 and that the upload_address setting is configured to http://[on_premises_webserver]/push

.. code-block:: bash

    # Restart the alprd process
    sudo /etc/init.d/openalprd-daemon start

    # Tail the logs to see if the daemon is running successfully
    tail -f /var/log/alpr.log
                    
If all goes well, the log should show that the video stream is being processed and plates are being identified and uploaded. Once a plate is uploaded it should show up on the OpenALPR Cloud dashboard after a few seconds.


Web Services API
------------------

The Web Services API can be used to query your On-Premises server for data.  The API is documented `here <https://anypoint.mulesoft.com/apiplatform/openalpr/#/portals/apis/21174/versions/22584/pages/35526>`_