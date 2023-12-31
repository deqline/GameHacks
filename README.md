# GameHacks
A collection of 3 game hacks I developped in the past.

This repository is provided for educational purposes ( i.e to understand how game hacks work ) and for my personal portofolio. 

## Features
### CSGO 
A simple internal hack which finds the current game entity list (players). Tested in training.

### Deceit
An attempt to make an internal ESP cheat using DirectX on Deceit. 

Couldn't find the correct ViewMatrix so draws the ESP but not at the correct screen coordinates (see [image](https://i.ibb.co/sqCv8T9/279c710c-8b9e-4798-b035-ff71242f8de7-1.jpg))

### Fallguys

See [Demo](https://www.youtube.com/watch?v=_hc6_Ih4LdQ&t=3s)

A fully working external FallGuys hack with a player properties modifier.

This hack use a kernel driver that hooks an api in the DirectX driver (dxgkrnl.sys), such that, the usermode application can communicate with the driver without being detected by EasyAntiCheat.

The usermode executable was built by me at the time and the kernel driver code was inspired by this [video series](https://www.youtube.com/watch?v=KNGr4m99PTU&list=PLdm1osymZj-fYsZM4MZ4n9Wz0QdAASHrD);

## Building
For all of these projects you will need `Visual Studio Community` in order to open the solution file (`.sln`) inside each project folder.

Each project should be built in `x64`.

The CSGO and Deceit projects will compile to a `.dll` file which will have to be manually injected in the game process.

The Fallguys project will compile to a `.sys` (driver) that will have to be manually loaded on the system and an `.exe` file that will allow you to 
modify certain player values once the kernel driver is loaded.
