/*
AME 262 Final Project -- Auto-panner
Author: Lindsey Deng
Support input file with the following extensions: .wav, .aif, .aiff 
The input file has to be mono.
This program uses low frequency oscillator(LFOs) to pan the input file.
This program outputs a stereo audio file with processed panning. 
The user can specify the width, rate, phase, and type of panning.
Compile(MacOS M1): gcc autopan.c breakpoints.c -o autopan -Iinclude -Llib -lsndfile
Sample runs:
./autopan Salinas.wav Salinas_sine.wav 0.75 1 3 sine
Adapted from sfpan.c by Minglun Lee
constpower function written by Richard Dobson
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>      // for sin, cos, atan, sqrt
#include <sndfile.h>   
#include <breakpoints.h>
#include<time.h>

#define NFRAMES (1024)  // block size: number of frames per block


//function prototypes
void print_sfinfo(const SF_INFO * sfinfo); // print sfinfo to the console
int  sf_extension(const char * filename);  // determine the major format with the file extension (e.g. wav, aif, aiff).


// for command line arguments
enum{ARG_PROGNAME,ARG_INFILE,ARG_OUTFILE,ARG_WIDTH,ARG_RATE,ARG_PHASE,ARG_TYPE,ARG_NARGS};

//for indices for panning types
enum{SINE,SQUARE,SAWTOOTH,TRIANGLE,RANDOM};

//command line arguments for panning types
char *panning_types[] = {"sine","square","sawtooth","triangle","random"};

typedef struct panamps{
    double left;          // amp to the left channel
    double right;         // amp to the right channel
} PANAMPS;                // panning amplitudes

PANAMPS constpower(double position); // constant power function


int main (int argc, char * argv [])
{
   char * infilename;         // input file name
   char * outfilename;        // output file name
   SNDFILE * infile = NULL;   // input sound file pointer
   SNDFILE * outfile = NULL;  // output sound file pointer
   SF_INFO sfinfo;            // sound file info
   int outfile_major_type;    // output major type in hex
   long readcount;            // no. of samples read
   float * inbuffer = NULL;   // buffer for input file
   float * outbuffer = NULL;  // buffer for output file
   FILE * fp = NULL;          // for breakpoint file
   BREAKPOINT * points = NULL;    // breakpoint structure
   //double  sampletime = 0.0;  // sample time from t = 0
   //double  timeincr;          // time increment = 1/SR
   double  width;            // width of panning
   double  rate;              // rate of panning in Hz
   double  phase;             // phase of panning in radius per sample
   double duration;           // duration of the audio file
   int     panning_type;      // panning type
   double lfo_freq;           // frequency of the LFO
   double lfo_dur;            // duration of the LFO
   double lfo_phase;          // phase of the LFO 
   double lfo_amp;            // amplitude of the LFO
   double sampletime;         // sample time from t = 0
   double timeincr;           // time increment = 1/SR
   PANAMPS panamps;           // panning amplitudes
   srand(time(NULL));         // seed for random number generator



   //input validation
    if(argc != ARG_NARGS)
    {
        printf("--------------------WELCOME TO AUTO-PANNER--------------------\n");
        printf("Auto-panner: Automatically pan your audio file!\n");
        printf("Usage: %s infile outfile width rate phase type\n" , argv[ARG_PROGNAME]);
        printf("infile: input file name\n");
        printf("outfile: output file name\n");
        printf("width: amplitude of the LFO: (0.0 - 1.0)\n");
        printf("rate: rate of the LFO in Hz: (0.0 - 10.0)\n");
        printf("phase: phase of the LFO in radians: (0.0 - 2*pi)\n");
        printf("type: panning type: sine, square,sawtooth, triangle,random\n");
        printf("--------------------------------------------------------------\n");
        return 1;
    }

    infilename = argv[ARG_INFILE];
    outfilename = argv[ARG_OUTFILE];


    // check if the input file name and output file name are the same
    if(strcmp(infilename, outfilename) == 0)
    {
        printf("Error: input file name and output file name cannot be the same.\n");
        return 1;
    }

    // validate the width
   width = atof(argv[ARG_WIDTH]);
    if(width < 0.5 ||width > 1.0)
    {
        printf("Error: amount must be between 0.5 and 1.0.\n");
        return 1;
    }

    // validate the rate
    rate = atof(argv[ARG_RATE]);
    if(rate < 0.0 || rate > 10)
    {
        printf("Error: rate must be between 0.0 and 10.0\n");
        return 1;
    }

    // validate the phase
    phase = atof(argv[ARG_PHASE]);
    if(phase < 0.0 || phase > 2*M_PI)
    {
        printf("Error: phase must be between 0.0 and 2*pi.\n");
        return 1;
    }

    // validate the panning type
    panning_type = -1;
    for(int i = 0; i < 5; i++)
    {
        if(strcmp(argv[ARG_TYPE], panning_types[i]) == 0)
        {
            panning_type = i;
            break;
        }
    }

    if(panning_type == -1)
    {
        printf("Error: panning type must be sine, sawtooth, or random.\n");
        return 1;
    }

    //open the sound file for reading
    memset(&sfinfo, 0, sizeof(sfinfo)); // clear the SF_INFO struct
    if((infile = sf_open(infilename, SFM_READ, &sfinfo)) == NULL)
    {
       printf("Not able to open input file %s.\n", infilename) ;
       puts(sf_strerror (NULL));
       free(points);
        fclose(fp);    // close the breakpoint file
        return 1;
    }

    //calcuate duration of the sound file
    duration = (double)sfinfo.frames / (double)sfinfo.samplerate; 
    lfo_freq = rate;
    lfo_dur = duration;
    lfo_phase = phase;
    lfo_amp = width;
    double num_samples = lfo_dur * sfinfo.samplerate;

    double period = 1.0 /(double) lfo_freq;
    double phase_offset = phase * period;
    
    //sine lfo
    if(panning_type == SINE)
    {
        FILE *file = fopen("panpos.txt", "w");
        if (file == NULL) {
        printf("Error: unable to open file\n");
        return 1;
         }
        for(int i=0; i<num_samples; i+=1000)
        {
            double time = (double)i / (double)sfinfo.samplerate;
            double value = lfo_amp * sin(2*M_PI*lfo_freq*time + lfo_phase);
            fprintf(file, "%f %f\n", time, value);
        }

        fclose(file);
    }

    //square lfo
    if(panning_type == SQUARE)
    {
        FILE *file = fopen("panpos.txt", "w");
        if (file == NULL) {
        printf("Error: unable to open file\n");
        return 1;
         }
        for(int i=0; i<num_samples; i+=1000)
        {
            double time = (double)i / (double)sfinfo.samplerate;
            double value = fmod(time + phase_offset, period) / period < 0.5 ? lfo_amp : -lfo_amp; 
            fprintf(file, "%f %f\n", time, value);
        }

        fclose(file);
    }

    //sawtooth lfo
    if(panning_type ==SAWTOOTH)
    {
        FILE *file = fopen("panpos.txt", "w");
        if (file == NULL) {
        printf("Error: unable to open file\n");
        return 1;
     }

      for (int i = 0; i < num_samples; i+=1000) {
        double time = (double)i / (double)sfinfo.samplerate ;
        double value = (2.0 * lfo_amp / period) * (fmod(time + phase_offset, period) - 0.5 * period);  // calculate sawtooth value
        fprintf(file, "%lf %lf\n", time, value);
       }

         fclose(file);

    }

    //triangle lfo
    if(panning_type == TRIANGLE)
    {
        FILE *file = fopen("panpos.txt", "w");
        if (file == NULL) {
        printf("Error: unable to open file\n");
        return 1;
     }

      for (int i = 0; i < num_samples; i+=1000) {
        double time = (double)i / (double)sfinfo.samplerate;
        double value = (2*lfo_amp/M_PI)*asin(sin(2*M_PI*lfo_freq*time + phase_offset));
        fprintf(file, "%lf %lf\n", time, value);
       }

         fclose(file);

    }

    //random lfo
    if(panning_type == RANDOM)
    {
        FILE *file = fopen("panpos.txt", "w");
        if (file == NULL) {
        printf("Error: unable to open file\n");
        return 1;
     }

      for (int i = 0; i < num_samples; i+=1000) {
        double time = (double)i / sfinfo.samplerate;
        double value = ((double)rand() / RAND_MAX) * (2.0 * lfo_amp) - lfo_amp;
        fprintf(file, "%lf %lf\n", time, value);
       }

         fclose(file);

    }

    //open the generated breakpoint file for reading
    if((fp = fopen("panpos.txt", "r")) == NULL)
    {
        printf("Not able to open breakpoint file.\n");
        free(points);
        sf_close(infile);
        return 1;
    }
    unsigned long size = 0;
    points = get_breakpoints(fp, &size);
    if(points == NULL){
        printf("Error: No breakpoints read.\n");
        fclose(fp);
        return 1;
    }
    if(size < 2){
        printf("Error: at least two breakpoints required\n");
        free(points);
        fclose(fp);
        return 1;
    }
    /* we require breakpoints to start from 0 */
    if(points[0].time != 0.0){
        printf("Error in breakpoint data: first time must be 0.0\n");
        free(points);
        fclose(fp);
        return 1;
    }

     memset(&sfinfo, 0, sizeof (sfinfo));  // clear sfinfo

        /* Open input sound file  for reading &
     fill sound file information with sfinfo. */
    if((infile = sf_open(infilename, SFM_READ, &sfinfo)) == NULL)
    {
        printf("Not able to open input file %s.\n", infilename) ;
        puts(sf_strerror (NULL));
        free(points);
        fclose(fp);    // close the breakpoint file
        return 1;
    }
    
    if(sfinfo.channels != 1){
        printf("Error: Input file is not mono!\n");
        fclose(fp);
        free(points);
        sf_close(infile);
        return 1;
    }


    timeincr = 1.0/sfinfo.samplerate;     // sample time increment
    
    inbuffer = (float *)malloc(NFRAMES * sizeof(float)); // used to save a block of samples
    outbuffer = (float *)malloc(2 * NFRAMES * sizeof(float)); // for stereo
    outfile_major_type = sf_extension(outfilename); // return outfile major type in hex
    if(outfile_major_type == -1){
        printf("The outfile extension is not .wav, .aif, or .aiff\n");
        free(inbuffer);
        free(outbuffer);
        free(points);
        sf_close(infile);
        return 1;
    }
    
    sfinfo.channels = 2; // stereo for output file

     if(!sf_format_check(&sfinfo))  // check sfinfo for outfile
    {
        printf ("Invalid encoding\n") ;
        free(inbuffer);
        free(outbuffer);
        free(points);
        sf_close(infile) ;
        return 1;
    }
    
    // open a sound file for writing with sfinfo
    if((outfile = sf_open(outfilename, SFM_WRITE, &sfinfo)) == NULL)
    {
        printf("Not able to open output file %s.\n", outfilename) ;
        puts(sf_strerror (NULL));
        free(inbuffer);
        free(outbuffer);
        free(points);
        sf_close(infile) ;
        return 1 ;
    }

    //processing autopanning 
        while ((readcount = sf_read_float(infile, inbuffer, NFRAMES)) > 0){
        double stereopos;  
        
        for(int i = 0, out_i = 0; i < readcount; i++){
            // get the stereo position at the current sample time
            stereopos = val_at_brktime(points, size, sampletime); 
            panamps = constpower(stereopos);
            outbuffer[out_i++] = (float)(inbuffer[i] * panamps.left);
            outbuffer[out_i++] = (float)(inbuffer[i] * panamps.right);
            sampletime += timeincr;
        }
        sf_write_float(outfile, outbuffer, 2 * readcount) ;
    }    // read block by block until the end of the sound file

      /* clean up */
    free(inbuffer);
    free(outbuffer);
    free(points);
    sf_close(infile) ;   // close input sound file
    sf_close(outfile) ;  // close output text file
    
    return 0 ;

}

/*
print_sfinfo() is used to print the sound file information
*/
void print_sfinfo(const SF_INFO * sfinfo){
    int majortype = sfinfo->format & SF_FORMAT_TYPEMASK; // find the major type
    int subtype = sfinfo->format & SF_FORMAT_SUBMASK;    // find the subtype
    int major_count;  // number of major types
    int sub_count;    // number of subtypes
    SF_FORMAT_INFO formatTemp;
    memset(&formatTemp, 0, sizeof(formatTemp));   // clear the SF_FORMWT_INFO struct
    
    // full format
    printf(" format: 0x%08x\n", sfinfo->format);    // print the full format
    
    /*** major type ***/
    sf_command(NULL, SFC_GET_FORMAT_MAJOR_COUNT, &major_count,sizeof(int));
    for(int i=0; i<major_count; i++) {
        formatTemp.format = i;
        sf_command (NULL, SFC_GET_FORMAT_MAJOR, &formatTemp, sizeof(formatTemp));
        if (majortype == formatTemp.format) {
            printf(" %s (extension \"%s\")\n", formatTemp.name, formatTemp.extension);
            break; // find the matching format
        }
    }
    
    /*** subtype ***/
    memset(&formatTemp, 0, sizeof(formatTemp));
    sf_command(NULL, SFC_GET_FORMAT_SUBTYPE_COUNT, &sub_count,sizeof(int));
    for(int i=0; i<sub_count; i++) {
        formatTemp.format = i;
        sf_command (NULL, SFC_GET_FORMAT_SUBTYPE, &formatTemp, sizeof(formatTemp));
        if (subtype == formatTemp.format) {
            printf(" %s\n", formatTemp.name);
            // extension is not available for subtypes
            break; //find the matching format
        }
    }
}

/*
 Determine the file major type with the file extension (e.g. wav, aif, aiff).
 Return the major type format in hex.
 */
int sf_extension(const char * filename){
    int filename_len = strlen(filename); // entire filename length (include the extension)
    if(strcmp((filename + (filename_len - 4)), ".wav") == 0){
        return SF_FORMAT_WAV;     // wav file type in hex
    }
    else if(strcmp((filename + (filename_len - 4)), ".aif") == 0
        || strcmp((filename + (filename_len - 5)), ".aiff") == 0){
        return SF_FORMAT_AIFF;   // aiff or aif file type in hex
    }
    else {
        return -1;               // extension is not wav, aiff, or aif
    }
}

PANAMPS constpower(double position)
{
    PANAMPS amps;  // amplitudes for left & right channels
    const double  piovr2    = 4.0 * atan(1.0) * 0.5;    /* pi/2: 1/4 cycle of a sinusoid */
    const double  root2ovr2 = sqrt(2.0) * 0.5;         /* sqrt(2)/2: 1/4 amplitude of a sinusoid */
    double thispos = position * piovr2;                    /* scale position to fit the pi/2 range */
    double angle = thispos * 0.5;                         /* each channel uses a 1/4 of a cycle */
    
    amps.left    = root2ovr2 * (cos(angle) - sin(angle));
    amps.right    = root2ovr2 * (cos(angle) + sin(angle));
    return amps;
}
