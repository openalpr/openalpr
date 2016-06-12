.. _web_server:

***********************
OpenALPR Web Server 
***********************

`*` Requires Commercial License

Requirements
================

OpenALPR monitors video streams in real-time to gather all license plates seen by your cameras. This data is browsable, searchable, and can trigger alerts. The Web server can be available either:

  #. In the cloud
  #. Installed On-Premises within your network

If you haven't seen the `live demo <http://www.openalpr.com/demo-cloud.html>`_, check it out! It shows a realistic example of the type of information you will soon be collecting from your cameras.

In order to get started, you will need:

  1. An IP camera capable of serving MJPEG or H264 video
  2. A computer (Intel i5 or better) with network access to the IP camera.  This will be your OpenALPR Agent.

First, configure your IP camera to capture the area that you wish to monitor. The camera must be capable of capturing a clear image of the license plate in order for OpenALPR to properly identify the numbers. You may want to experiment with different angles, optical zoom levels, and resolutions to get the best image quality. A straight-on shot of the license plate is best, but OpenALPR can work with shots at an angle if necessary.

Once your camera is setup, make sure that it has an IP address and that you can connect to the MJPEG or H264 stream. Some cameras support arguments in the URL to control resolution, frame-rate, etc. The Firefox web browser has the best MJPEG support among major browsers. Type in the MJPEG or H264 stream URL to Firefox and you should be able to clearly see your video. Some users reduce the frames per second (fps) of their video feed in order to reduce the stream bandwidth.  Fifteen frames per second is usually more than sufficient to capture passing vehicles from a fixed-camera.

Architecture
=============

.. image:: images/webserver_architecture.png
    :scale: 100%
    :alt: OpenALPR Web Server Architecture


As depicted in the diagram above, the OpenALPR agent reads a video stream from your IP camera, processes it, and uploads plate metadata to the OpenALPR web server. The agent also stores all of the plate images on a rolling buffer in its hard drive.

There is a constant stream of data flowing between the camera and the agent as well as between the agent and the cloud.  The data sent to the cloud is relatively low-bandwidth because it contains text metadata describing the license plates, and not the images.  OpenALPR Cloud does not store your plate images, these are downloaded directly from the agent when you select a plate to view from the web server.

Installation
===============================

The OpenALPR web server and the :ref:`OpenALPR agent <alprd>`  is installed as either a Virtual Machine (VM) or natively on 64-bit Ubuntu Linux 14.04. 

We recommend installing natively on Linux, since this provides higher performance and excellent reliability.  Running a VM on Windows is recommended for evaluations.



Linux Installation
----------------------

Download the Ubuntu 14.04 64-bit install DVD image and burn to a DVD:

  - http://releases.ubuntu.com/14.04/ubuntu-14.04.4-desktop-amd64.iso  

Follow this installation guide to install Ubuntu 14.04 64-bit:

  - http://howtoubuntu.org/how-to-install-ubuntu-14-04-trusty-tahr

Run the following command from the terminal:

.. code-block:: bash

    bash <(curl http://deb.openalpr.com/install)


.. image:: images/linux-install.png
    :scale: 100%
    :alt: OpenALPR VM installation step 4

- Choose the options for what you wish to install.  

  - If you want to connect the agent to the OpenALPR Cloud, just choose "install agent". 
  - If you wish to host the data on the VM (On-Premises configuration) choose "install webserver"

Virtual Machine Installation
---------------------------------

The OpenALPR Virtual Machine (VM) installs in a few minutes and can run on any operating system (e.g., Windows, Linux, Mac OS X).  Installing a VM on a datacenter-class server (such as VMware ESXi or Citrix XenServer) is considered production-grade.  Installing on a desktop system (e.g., VirtualBox on Windows 10) works well for evaluation, but we recommend installing it directly on Linux for longer-term usage.

The OpenALPR VM comes pre-installed with the OpenALPR agent.  The on-premesis web server can also be installed on this VM as well if you wish.  When the web-server is installed, then all OpenALPR processing and storage is maintained inside the VM and no data is sent to the cloud.

- Start downloading the `latest OpenALPR Virtual Machine <http://deb.openalpr.com/downloads/openalpr.ova>`_.
- Download and install the `VirtualBox installer <https://www.virtualbox.org/wiki/Downloads>`_ for your operating system. The OpenALPR VM will also work with other hypervisors such as VMware or Xen if you prefer to use one of those.  We are recommendeding VirtualBox because it is free.
- Open VirtualBox and choose File → Import Appliance
- Choose the openalpr.ova file downloaded in step #1
- Accept the default memory and CPU settings and click "Import"

.. image:: images/webserver_vminstall1.png
    :scale: 100%
    :alt: OpenALPR VM installation step 1

- Select the openalpr-agent-vm and click "Start"

- The VM should boot up quickly and provide you with a login prompt. Login with the default credentials: 
    - Username: admin
    - Password: admin

- You should see a menu like the one below. Use the up/down, tab, and enter keys to navigate the menu.

.. image:: images/webserver_vminstall2.png
    :scale: 100%
    :alt: OpenALPR VM installation step 2

- First setup the network by choosing **Network** → **eth0**.
    - Select either DHCP or static depending on your LAN configuration
    - Select Network → Test and make sure that you can successfully ping www.google.com

.. image:: images/webserver_vminstall3.png
    :scale: 100%
    :alt: OpenALPR VM installation step 3

- Select **Upgrade** from the main menu. The OpenALPR software is updated more frequently than the VM, there may be updates available.
  
- Optionally, select **Password** from the main menu to change your password to something more secure.

- Next select **Register** from the main menu.  Choose where you wish to send data.  You can send data to the OpenALPR Cloud, host the data on the OpenALPR Web
  server on the same VM, or send the data to another server.

.. image:: images/webserver_vmregister1.png
    :scale: 100%
    :alt: OpenALPR VM Registration step 1

- Type in your credentials (e-mail address and password) and select **OK**

.. image:: images/webserver_vmregister2.png
    :scale: 100%
    :alt: OpenALPR VM Registration step 2

- If registration is successful, you will see a success message. 

.. image:: images/webserver_vmregister2.png
    :scale: 100%
    :alt: OpenALPR VM Registration step 2

- The rest of the configuration is managed via the OpenALPR web server.  Login to the OpenALPR web server to configure your agent.

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

- Lastly, if you scroll to the bottom of the page you can watch the agent status.  At this point you should see **Video FPS** and other information indicating that video is being pulled from the camera and license plates are being recognized.  Now that the agent is configured, it will continue collecting data from the configured video streams.  If the agent is rebooted, the OpenALPR agent will automatically start.  If the camera goes down and comes back, or the network is down temporarily, the agent will retry until connectivity is restored.  All results are queued, so no data is lost in the event of an outage.

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