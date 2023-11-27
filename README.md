# GameHacks
A collection of 3 game hacks I developped in the past.

Keep in mind this was in the beginning of my programming journey so it might contain some code from elsewhere and it shouldn't be used as a good coding reference.

This repository is provided for educational purposes ( i.e to understand how game hacks work ) and for my personal portofolio. 

## Features
### CSGO 
A simple internal hack which finds the current's game entity list (players). Tested in training.

### Deceit
An attempt to make an internal ESP cheat using DirectX on Deceit. 

Couldn't find the correct ViewMatrix so doesn't work.

### Fallguys
A fully working external FallGuys hack with a player properties modifier.

This hack use a kernel driver that hooks an api in the DirectX driver (dxgkrnl.sys), such that the usermode application can communicate with the driver without being detected by EasyAntiCheat.

The usermode was built by me at the time and the kernel mode part of this cheat was inspired by this [video series](https://www.youtube.com/watch?v=KNGr4m99PTU&list=PLdm1osymZj-fYsZM4MZ4n9Wz0QdAASHrD);

## Building
For all of these projects you will need "Visual Studio Community" in order to open the solution file ".sln" of the project and to build it in x64.

The CSGO and Deceit projects will compile to a `.exe`, Fallguys will compile to a `.sys` that will have to be manually loaded on the system and an `.exe` file that will allow you to 
modify certain player values once it established its communication with the kernel driver.
