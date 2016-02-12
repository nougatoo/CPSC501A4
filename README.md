# What is it
Takes a dry recording of an instrument and an impulse response and produces an altered version of the dry recording.
For the dry recording, this repo include a 30 second clip of a guitar and for an impulse reponse it uses a big concert hall.
The output of the program makes it sound like the guitar is being played in the concert hall. A report of optimizations has been
included in the repo. 

## How it works
It works by using covolution of the individual bits of the digital signal. There are two version of the code, and optimized and 
unoptimzed version. The optimized version uses Fast-Fourier-Transformations to make this process extremely quick. The unoptimized
can take 10-15 minutes to run. 

## How to run
Just compile the the convolveOPT.c and run with the following command line arguments GuitarDry.wav BIG_HALL_E001_M2S.wav output.wav. Or you can listen to the original and new audio files that i've included. The sound on the new guitar is very quiet. Volume will need to be turned up.
