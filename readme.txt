Background:
This project creates a REDHAWK wrapper for the dump1090 ADSB receiver.
Acts as a nice example waveform using REDHAWK and template for wrapping C code.  
Based on the Malcom Robb fork of dump1090. 

Install instructions: 
Import into your REDHAWK workspace, regenerate the component, build it and drag to SDR root. 
This project depends on RTL dongle, it has not been tested with other hardware.
The waveform's .xml file has been modified to connect the component to the  RTL device. 
In order to use the google maps based plane plotting, you will need to copy the dump1090-MR folder into a directory of your choosing. By default it goes in home/redhawk. If you choose a different location you will need to modify the HTMLPATH # define in dump1090.h
Also for this project we changed the default port for the web ui to 8081. Running concerent waveforms you may 
 
Running:
It is reccomended that your RTL Node be launched from the command line prior to instantiation (example): 
 nodeBooter -d /var/redhawk/sdr/dev/nodes/RTL_Node/DeviceManager.dcd.xml

The reason for this is that the code makes extensive use of printing to the console to display airplane tracks and is very ugly if you use the redhawk ide console. 


Open Web UI for interactive mode in a browser at: http://127.0.0.1:8081/


Known issues: 
Depends on RTL dongle, has not been tested with other hardware. 
Demonstration purposes only...use at your own risk. 
Need to add getters/setters for all properties. 
Need to check out modes other than aggressive/mode a/c and interactive. 
Have to manually install web component by copying dump1090-mr. 

Web UI currently integrated tightly with the WF code, It would be best to break the http server out to a separate entity. 

