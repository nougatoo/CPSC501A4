/******************************************************************
Brandon Brien
10079883

Code is based off of the reading and writing file given in 
tutorials.

******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "fft.c"

char chunkID[4];
int chunkSize;
char format[4];

char subChunk1ID[4];
int subChunk1Size;
short audioFormat;
short numChannels;
int sampleRate;
int byteRate;
short blockAlign;
short bitsPerSample;

int channelSize;
char subChunk2ID[4];
int subChunk2Size;

int dryDataSize;
int irDataSize;
int sizeOfResult;
int scaler;

float* data;
float* dryData;
float* irData;
float* resultData;

/*  CONSTANTS  ***************************************************************/

/*  Test tone frequency in Hz  */
#define FREQUENCY         440.0

/*  Test tone duration in seconds  */
#define DURATION          2.0				

/*  Standard sample rate in Hz  */
#define SAMPLE_RATE       44100.0

/*  Standard sample size in bits  */
#define BITS_PER_SAMPLE   16

/*  Standard sample size in bytes  */		
#define BYTES_PER_SAMPLE  (BITS_PER_SAMPLE/8)

/*  FUNCTION PROTOTYPES  *****************************************************/
void createTestTone(double frequency, double duration, char *filename);
void writeWaveFileHeader(int numberSamples, double outputRate, FILE *outputFile);
size_t fwriteIntLSB(int data, FILE *stream);
size_t fwriteShortLSB(short int data, FILE *stream);

void print()
{
	/*
	printf("\n============= HEADER INFO =============\n", chunkID);
	printf(" chunkID:%s\n", chunkID);
	printf(" chunkSize:%d\n", chunkSize);
	printf(" format:%s\n", format);
	printf(" subChunk1ID:%s\n", subChunk1ID);
	printf(" subChunk1Size:%d\n", subChunk1Size);
	printf(" audioFormat:%d\n", audioFormat);
	printf(" numChannels:%d\n", numChannels);
	printf(" sampleRate:%d\n", sampleRate);
	printf(" byteRate:%d\n", byteRate);
	printf(" blockAlign:%d\n", blockAlign);
	printf(" bitsPerSample:%d\n", bitsPerSample);
	printf(" subChunk2ID:%s\n", subChunk2ID);
	printf(" subChunk2Size:%d\n", subChunk2Size);
	*/
}

int loadWave(char* filename)
{
	FILE* in = fopen(filename, "rb");

	if (in != NULL)
	{		
		//printf("Reading %s...\n",filename);

		fread(chunkID, 1, 4, in);
		fread(&chunkSize, 1, 4, in);
		fread(format, 1, 4, in);

		//sub chunk 1
		fread(subChunk1ID, 1, 4, in);
		fread(&subChunk1Size, 1, 4, in);
		fread(&audioFormat, 1, 2, in);
		fread(&numChannels, 1, 2, in);
		fread(&sampleRate, 1, 4, in);
		fread(&byteRate, 1, 4, in);
		fread(&blockAlign, 1, 2, in);
		fread(&bitsPerSample, 1, 2, in);		
		
		//read extra bytes
		if(subChunk1Size == 18)
		{
			short empty;
			fread(&empty, 1, 2, in);		
		}
		//sub chunk 2
		fread(subChunk2ID, 1, 4, in);
		fread(&subChunk2Size, 1, 4, in);
		
		sizeOfResult = sizeOfResult + subChunk2Size;

		//read data		
		int bytesPerSample = bitsPerSample/8;
		int numSamples = subChunk2Size / bytesPerSample;
		data = (float*) malloc(sizeof(float) * numSamples);
		
		//fread(data, 1, bytesPerSample*numSamples, in);
		
		int i=0;
		short sample=0;
		float scaledSample;
		while(fread(&sample, 1, bytesPerSample, in) == bytesPerSample)
		{	
			scaledSample = (float) sample/32767/scaler;
			data[i++] = scaledSample;
			//printf("%lf ", scaledSample);
			//printf("%lf ", data[i-1]);
			sample = 0;			
			scaledSample = 0;
		}
		
		fclose(in);
		//printf("Closing %s...\n",filename);
	}
	else
	{
		printf("Can't open file\n");
		return 0;
	}
	return 1;
}
/*****************************************************************************
*
*    Function:     convolve
*
*    Description:  Convolves two signals, producing an output signal.
*                  The convolution is done in the time domain using the
*                  "Input Side Algorithm" (see Smith, p. 112-115).
*
*    Parameters:   x[] is the signal to be convolved
*                  N is the number of samples in the vector x[]
*                  h[] is the impulse response, which is convolved with x[]
*                  M is the number of samples in the vector h[]
*                  y[] is the output signal, the result of the convolution
*                  P is the number of samples in the vector y[].  P must
*                       equal N + M - 1
*
*****************************************************************************/

void convolve(float x[], int N, float h[], int M, float y[], int P)
{
  int n, m;

  /*  Make sure the output buffer is the right size: P = N + M - 1  */
  if (P != (N + M - 1)) {
    printf("Output signal vector is the wrong size\n");
    printf("It is %-d, but should be %-d\n", P, (N + M - 1));
    printf("Aborting convolution\n");
    return;
  }

  /*  Clear the output buffer y[] to all zero values  */  
  for (n = 0; n < P; n++)
    y[n] = 0.0;

  /*  Do the convolution  */
  /*  Outer loop:  process each input value x[n] in turn  */
  for (n = 0; n < N; n++) {
    /*  Inner loop:  process x[n] with each sample of h[]  */
    for (m = 0; m < M; m++)
	{
      y[n+m] += x[n] * h[m];
	 }
	 //printf("Hi");
  }
}


int saveWave(char* filename)
{
	FILE* out = fopen(filename, "wb");

	if (out != NULL)
	{		
		//printf("Writing %s...\n",filename);

		fwrite(chunkID, 1, 4, out);
		fwrite(&chunkSize, 1, 4, out);
		fwrite(format, 1, 4, out);

		//sub chunk 1
		fwrite(subChunk1ID, 1, 4, out);
		fwrite(&subChunk1Size, 1, 4, out);
		fwrite(&audioFormat, 1, 2, out);
		fwrite(&numChannels, 1, 2, out);
		fwrite(&sampleRate, 1, 4, out);
		fwrite(&byteRate, 1, 4, out);
		fwrite(&blockAlign, 1, 2, out);
		fwrite(&bitsPerSample, 1, 2, out);		
		
		//read extra bytes
		if(subChunk1Size == 18)
		{
			short empty = 0;
			fwrite(&empty, 1, 2, out);		
		}
		
		//sub chunk 2
		fwrite(subChunk2ID, 1, 4, out);
		int bytesPerSample = bitsPerSample / 8;
		subChunk2Size = sizeOfResult*bytesPerSample;
		fwrite(&subChunk2Size, 1, 4, out);

		//read data		

		int sampleCount = sizeOfResult;
		//int sampleCount =  subChunk2Size / bytesPerSample;
		
		float temp;
		float MAX_VAL = 32767.f;
		
		for(int i=0; i<sampleCount; ++i)
		{						
			temp = resultData[i];

			if(temp > 1)
				temp = 1;
			else if(temp < -1)
				temp = -1;	
		
			temp *= MAX_VAL;
			
			//write to file
			short sample = (short) temp;
			fwrite(&sample, 1, bytesPerSample, out);
			
			i++;
			temp = resultData[i];
			if(temp > 1)
				temp = 1;
			else if(temp < -1)
				temp = -1;	
			temp *= MAX_VAL;
			//write to file
			sample = (short) temp;
			fwrite(&sample, 1, bytesPerSample, out);
			
			i++;
			temp = resultData[i];
			if(temp > 1)
				temp = 1;
			else if(temp < -1)
				temp = -1;	
			temp *= MAX_VAL;
			//write to file
			sample = (short) temp;
			fwrite(&sample, 1, bytesPerSample, out);

			i++;
			temp = resultData[i];
			if(temp > 1)
				temp = 1;
			else if(temp < -1)
				temp = -1;	
			temp *= MAX_VAL;
			//write to file
			sample = (short) temp;
			fwrite(&sample, 1, bytesPerSample, out);

			i++;
			temp = resultData[i];
			if(temp > 1)
				temp = 1;
			else if(temp < -1)
				temp = -1;	
			temp *= MAX_VAL;
			//write to file
			sample = (short) temp;
			fwrite(&sample, 1, bytesPerSample, out);

			i++;
			temp = resultData[i];
			if(temp > 1)
				temp = 1;
			else if(temp < -1)
				temp = -1;	
			temp *= MAX_VAL;
			//write to file
			sample = (short) temp;
			fwrite(&sample, 1, bytesPerSample, out);

			i++;
			temp = resultData[i];
			if(temp > 1)
				temp = 1;
			else if(temp < -1)
				temp = -1;	
			temp *= MAX_VAL;
			//write to file
			sample = (short) temp;
			fwrite(&sample, 1, bytesPerSample, out);

			i++;
			temp = resultData[i];
			if(temp > 1)
				temp = 1;
			else if(temp < -1)
				temp = -1;	
			temp *= MAX_VAL;
			//write to file
			sample = (short) temp;
			fwrite(&sample, 1, bytesPerSample, out);

			i++;
			temp = resultData[i];
			if(temp > 1)
				temp = 1;
			else if(temp < -1)
				temp = -1;	
			temp *= MAX_VAL;
			//write to file
			sample = (short) temp;
			fwrite(&sample, 1, bytesPerSample, out);

			i++;
			temp = resultData[i];
			if(temp > 1)
				temp = 1;
			else if(temp < -1)
				temp = -1;	
			temp *= MAX_VAL;
			//write to file
			sample = (short) temp;
			fwrite(&sample, 1, bytesPerSample, out);

			i++;
			temp = resultData[i];
			if(temp > 1)
				temp = 1;
			else if(temp < -1)
				temp = -1;	
			temp *= MAX_VAL;
			//write to file
			sample = (short) temp;
			fwrite(&sample, 1, bytesPerSample, out);			
			
		}		
		
		//clean up
		free(resultData);
		fclose(out);
		//printf("Closing %s...\n",filename);
	}
	else
	{
		printf("Can't open file\n");
		return 0;
	}
	return 1;
}

/**
	This function checks that two audio files are roughtly the same
 */
int compareOutputs(char* testFile1, char* testFile2)
{
	float* optimizedData;
	float* unoptimizedData;
	int successCount = 0;
	
	scaler = 1;
	
	/* Read Optimized Data */
	if(loadWave(testFile1))
		print();
	optimizedData = data;
	
	printf("\n%d ", subChunk2Size/2);
	
	/* Read File Data */
	if(loadWave(testFile2))
		print();
	unoptimizedData = data;
	
	printf("\n%d ", subChunk2Size/2);
	
	
	float temp1;
	float temp2;
		
	for(int i =0;i<subChunk2Size/2;i++)
	{
		temp1 = optimizedData[i]*32767;
		temp2 = unoptimizedData[i]*32767;
			
		//temp1 is between 95% and 105% of the temp2
		if( ((temp1<(temp2*1.05)) && (temp1>(temp2*0.95))) || (temp1 == temp2))
		{
			successCount++;
		}
		else if( (temp2<(temp1*1.05) && (temp2>(temp1*0.95))) || (temp1 == temp2))
		{
			successCount++;
		}
	}
	
	
	printf("\n%d ", successCount);
	
	return successCount;
}


int main(int argc, char* argv[])
{
	char* filename = argv[1];
	//Used to measure overall time from read to write
	clock_t totalTime = clock(); 
	clock_t end;
	clock_t iTime;
	float seconds;
	scaler = 30;
	
	clock_t readTime = clock(); 
	//char* filename = "GuitarDry.wav";
	if(loadWave(filename))
		print();
		
	dryData = data;	
	dryDataSize = subChunk2Size/2; //Size of the data (number of entries)  = chunk size /numSamples (2)
	
	end = clock();
	seconds = (float)(end - readTime) / CLOCKS_PER_SEC;
	printf("\nReading Dry Sound Time: %.4f(s)", seconds);
	
	
	/* 
		Creating the double array.
		The size needs to be a power of two, and it needs to be padded with zeros 
	*/

	double test;
	iTime = clock();	
	
	int sizeOfDubDry;
	int n;
	
	n= dryDataSize;
	
	n--;
	n = n | n>>1;	
	n = n | n>>2;
	n = n | n>>4;
	n = n | n>>8;
	n = n | n>>16;
	n++;
	
	sizeOfDubDry = n*2;

	double* dubDryData = (double*) malloc(sizeof(double) * sizeOfDubDry);
	//Traverse through the dry data float array
	int i;
	for(i = 0;i<dryDataSize;i++)
	{
		dubDryData[i] = (double) dryData[i];
	}
	
	i++;
	/* Adds a zero padding to the rest of the data  array */
	for(i;i<sizeOfDubDry;i++)
	{
		dubDryData[i] = 0;
	}
	

	
	/* ------------------------------------------------------------------------------------- */
	/* Impulse Response Stuff */
	
	filename = argv[2];
	readTime = clock(); 
	//filename = "BIG_HALL_E001_M2S.wav";
	if(loadWave(filename))
		print();
		
	irData = data;
	irDataSize = subChunk2Size/2; //Size of the data = chunk size /numSamples (2)

	end = clock();
	seconds = (float)(end - readTime) / CLOCKS_PER_SEC;
	printf("\nReading Impulse Response Time: %.4f(s)", seconds);
	

	/* 
		Creating the double array.
		The size needs to be a power of two, and it needs to be padded with zeros 
	*/
	
	double* dubIRData = (double*) malloc(sizeof(double)* sizeOfDubDry);
	//Traverse through the dry data float array
	int j;
	for(j = 0;j<irDataSize;j++)
	{
		dubIRData[j] = (double)  irData[j];
	}
	
	j++;
	/* Adds a zero padding to the rest of the data  array */
	for(j;j<sizeOfDubDry;j++)
	{
		dubIRData[j] = 0;
	}
	
	/* ------------------------------------------------------------------------------------- */
	/* Data array for the convolution to fill */
	sizeOfResult = sizeOfResult>>1;
	sizeOfResult--;
	
	
	
	/* 
		Fast convolution 
	*/
	clock_t convolutionTime = clock(); 
	
	/* Meastures FFT time for the dry data */
	clock_t fftTime = clock();
	four1(dubDryData-1, sizeOfDubDry/2, 1);
	end = clock();
	seconds = (float)(end - fftTime) / CLOCKS_PER_SEC;
	printf("\n\tFFT Dry Data Time: %.4f(s)", seconds);
	
	/* Measures FFT time for IR data */
	fftTime = clock();
	four1(dubIRData-1, sizeOfDubDry/2, 1);
	end = clock();
	seconds = (float)(end - fftTime) / CLOCKS_PER_SEC;
	printf("\n\tFFT Impulse Response Data Time: %.4f(s)", seconds);
	
	
	/* All the numbers seem to be fine up to here */
	double* resultDubData = (double*) malloc(sizeof(double)* sizeOfDubDry);

	
	/* Measures frequency domain covolution time */
	clock_t convolvTime = clock();
	
	//Now have the result data FFT
	for(int k = 0;k<sizeOfDubDry;k+=2)
	{
		//k = real
		//k+1 = imaginary 
		resultDubData[k] = (dubDryData[k]*dubIRData[k]) - (dubDryData[k+1]*dubIRData[k+1]);
		resultDubData[k+1] = (dubDryData[k+1]*dubIRData[k]) + (dubDryData[k]*dubIRData[k+1]) ;
	}
	
	end = clock();
	seconds = (float)(end - convolvTime) / CLOCKS_PER_SEC;
	printf("\n\tFrequency Domain Convolution Time: %.4f(s)", seconds);
	
	
	//Now have it back in time domain
	clock_t ifftTime = clock();
	four1(resultDubData-1, sizeOfDubDry/2, -1);
	end = clock();
	seconds = (float)(end - ifftTime) / CLOCKS_PER_SEC;
	printf("\n\tIFFT on convoluted data (frequency domain) Time: %.4f(s)", seconds);
	
	end = clock();
	seconds = (float)(end - convolutionTime) / CLOCKS_PER_SEC;
	printf("\nTime to finish FFT x2, IFFT x1 and Convolution: %.4f(s)", seconds);
	
	free(resultData);
	
	resultData = (float*) malloc(sizeof(float)* sizeOfDubDry);
	
	//Gets the resulting data back into a float array
	for(int k =0;k<sizeOfDubDry;k++)
	{
		resultData[k] = (float) (resultDubData[k]/32767.f);
	}
	
	//Now we can write it to the wav file
	convolutionTime = clock();
	saveWave(argv[3]);
	
	end = clock();
	seconds = (float)(end - convolutionTime) / CLOCKS_PER_SEC;
	printf("\nWriting Result To File Time: %.4f(s)", seconds);
	
	free(data);
		
	end = clock();
	seconds = (float)(end - totalTime) / CLOCKS_PER_SEC;
	printf("\n\nOverall Run time: %.4f(s)\n\n", seconds);
	
}
