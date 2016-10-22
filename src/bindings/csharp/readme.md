OpenALPR C# Binding
======================

This .NET binding enables OpenALPR to be used directly from C#.  The binding uses the libopenalprc.dll produced from the native OpenALPR project.  

Code samples are available in the AlprNetTest and AlprNetGui project.  You must compile the program to match the native library version of OpenALPR.  Usually this is 64-bit.

To install, you must include all native DLLs along with the openalpr.conf and runtime_data directory in a location accessible by the .NET program.  

Troubleshooting
-----------------

The most common issue is that the native OpenALPR DLLs are not available to .NET.  The "Dependency Walker" program is very helpful for spotting missing DLLs.  Load the libopenalprc.dll into your project to detect if all the other native DLLs are available.
