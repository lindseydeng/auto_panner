# auto_panner
Support input file with the following extensions: .wav, .aif, .aiff 
The input file has to be mono.
This program uses low frequency oscillator(LFOs) to pan the input file.
This program outputs a stereo audio file with processed panning. 
The user can specify the width, rate, phase, and type of panning.
Compile(MacOS M1): gcc autopan.c breakpoints.c -o autopan -Iinclude -Llib -lsndfile
Sample runs:
./autopan Salinas.wav Salinas_sine.wav 0.75 1 3 sine
Adapted from sfpan.c by Minglun Lee
