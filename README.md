# FDL-3-Blaster

To update FDL-3 firmware you need the following:

[Sparkfun Microview Programmer](https://www.sparkfun.com/products/12924)
[XLoader](http://xloader.russemotto.com/)
Windows computer. (There are ways to do this on a mac but I have not found a quick and easy utility to do so)

-Remove the rear blaster cap and Microview. 
-Insert the Microview programmer into the USB port on your Windows computer. 
-Insert the Microview from your blaster into the programmer paying careful attention to orientation.
-The screen on your microview should point up with the USB connector facing left. There is a picture indication on the board as well.
-Run XLoader. 
-Choose your desired firmware hex file. 
-Select the ATmega328 option under device.
-Choose the com port of your USB programmer.
-Set baud rate to 115200.
-Click upload.
-You should see the lights under the screen flickering. Flashing can take 30 seconds or longer.
-When flashing is complete you will see the FDL-3 splash screen then the speed menu as usual.
-Remove the Microview from the programmer and put it back in your FDL-3.
-Replace the rear cap and you're good to go.

-It's not a bad idea to wipe your settings between each firmware update.
-To do this press and hold the control knob while turning on the blaster.
-The FDL-3 splash screen will show then the screen will go blank.
-When you release the knob you will be asked to press it again to clear your settings.
-If you wait 2 seconds, this message will go away and the blaster will boot as usual.
-If you click the knob in these two seconds the blaster will read "Clearing Settings" for 10-15 seconds.
-Once settings have been cleared your FDL-3 will beep a little melody and boot as usual with empty settings.
