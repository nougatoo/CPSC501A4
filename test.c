#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

float* data;
float* dryData;
float* irData;
float* resultData;
/*  CONSTANTS  ***************************************************************/
#define PI                3.14159265358979

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
}

int loadWave(char* filename)
{
	FILE* in = fopen(filename, "rb");

	if (in != NULL)
	{		
		printf("Reading %s...\n",filename);

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
			scaledSample = (float) sample/32767;
			data[i++] = scaledSample;
			//printf("%lf ", scaledSample);
			//printf("%lf ", data[i-1]);
			sample = 0;			
			scaledSample = 0;
		}
		
		fclose(in);
		printf("Closing %s...\n",filename);
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
		printf("Writing %s...\n",filename);

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
		
		for(int i=0; i<sampleCount; ++i)
		{			
			float MAX_VAL = 32767.f;
			
			
			//scale
			if(resultData[i] > 1)
				resultData[i] = 1;
			else if(resultData[i] < -1)
				resultData[i] = -1;	

			resultData[i] *= MAX_VAL;
			
			//write to file
			short sample = (short) resultData[i];
			fwrite(&sample, 1, bytesPerSample, out);
		}		
		
		//clean up
		free(resultData);
		fclose(out);
		printf("Closing %s...\n",filename);
	}
	else
	{
		printf("Can't open file\n");
		return 0;
	}
	return 1;
}


int main(int argc, char* argv[])
{
	//char* filename = argv[1];
	
	char* filename = "GuitarDry.wav";
	if(loadWave(filename))
		print();
		
	dryData = data;
	
	dryDataSize = subChunk2Size;
	
	filename = "l960taj_mahal_deep6.wav";
	if(loadWave(filename))
		print();
		
	irData = data;
	irDataSize = subChunk2Size;
	
	sizeOfResult = (sizeOfResult/2) -1;
	printf("%d ", sizeOfResult);

	resultData = (float*) malloc(sizeof(float) * sizeOfResult);
	printf("%d ", dryDataSize);
	printf("%d ", irDataSize);

	//result data = 
	convolve(dryData, dryDataSize/2, irData, irDataSize/2, resultData, sizeOfResult);
	
	saveWave("out.wav");
	free(data);
}