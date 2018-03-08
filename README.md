# settlers_online

This is a collection of tools for simulating combat in [The Settlers Online](https://www.thesettlersonline.com).
Currently, it consists of three major components:

- C++ header library describing combat mechanics (`settlers_online`).
- Native command-line version of the simulator (`black_marsh`) based on aforementioned C++ headers, that can be compiled on Linux, Mac, or Windows.
- Windows-only .NET GUI for the simulator (`LeytePond`) that calls the command-line application for actual simulations.


The compiled GUI version requres an x64 Windows system with .NET Framework 4.7, and the latest version of [Visual C++](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads). The main executable is [LeytePond.exe](./bin/LeytePond.exe). To run it, download the following folders:
- [bin](./bin/) folder, containing executables, binaries, etc.;
- [maps](./maps/) folder, containing description of units and maps;
- [faces](./faces/) folder, containing images for unit types;
- [skills](./skills/) folder, containing images for skills.


```
///////|////////////////////////|
///|//////"--,-------;\\\|\\|\\\\
//|///////    ______   \\\|\\\\|\
////////|    |      |   |\\\\\\\\
////// //   | o = 0 |   \\\ \\\\\
//||   ||   \       /    ||   |||
//||.".||||||||||||||||||||" .|||
///////////|////|||//|\\\\\\\\\\|
```
