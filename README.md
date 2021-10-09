# What is stm_send
stm_send is a tool to send and execute binary data to old 68k macs using the Diagnostic mode

# Diagnostic mode
While researching technical details about the Macintosh SE/30, I discovered on [mac68k.info](https://mac68k.info/wiki/display/mac68k/Diagnostic+Mode) that there is a "Diagnostic Mode" on some 68k Macintoshes. This mode is used by TechStep, a device for testing the various components of a Macintosh.

An easy way to access this mode is to use the mac's interrupt button (the “Programmer's Interrupt switch”) before the computer starts the OS.

Beyond the possibilities of testing the hardware of the machine, this mode has two very interesting elements:
- the "* D" command allowing to drop data, and therefore also code, in the memory of the target computer
- the "* G" command to launch the execution of the code on the target computer at a given address

These commands are also highlighted in the section "TechStep Requests CPUID on IIsi", on the page of mac68k.info, where the TechStep sends a small program to retrieve the CPUID of the target.

stm_send uses these commands to upload binary data in the memory of the mac and to execute it. The transfer being done in hexadecimal at 9600 baud is not very fast. The settings made on the serial port are specific to linux.

It would be possible to speed up the transfers by doing it in two stages: first send a program which puts the SCC at a higher speed before returning control to "Service Mode", then send the data.

# Usage
To upload the binary to the mac, after compilation, the procedure is as follows:
1. connect the serial port of the mac "Modem" to a computer
2. turn on the mac
3. when the mouse cursor appears press the interrupt button
4. run the stm_send program with apropriate parameters

# Future work
I see two interesting things that could emerge thanks to this method:
- create boot floppies with only one machine without OS
- install a basic OS on a SCSI disk without having the installation disks

# References
- [My Original post in French](https://www.scolan.net/utilisation-du-diagnostic-mode-sur-macintosh-se-30/) ([google translated to english](https://www-scolan-net.translate.goog/utilisation-du-diagnostic-mode-sur-macintosh-se-30/?_x_tr_sl=fr&_x_tr_tl=en))
- [Details about the Diagnostic mode on mac68k.info](https://mac68k.info/wiki/display/mac68k/Diagnostic+Mode) 
