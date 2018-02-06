//  ============================================================
//
//  Program: ArdCore Drum Sample Player
//
//  Description:
//
//    This is a very lo-fi drum sample player for 8 sounds.
//
//  I/O Usage:
//    Knob A0: Sound selector (summed with input A2)
//    Knob A1: Pitch (summed with input A3). Center value is normal pitch.
//    Analog In A2: Sound selection (summed with value of knob A0)
//    Analog In A3: VC Pitch (summed with value of knob A1)
//    Digital Out D0: Not used
//    Digital Out D1: Not used
//    Clock In: Trigger sound
//    Analog Out: Audio out
//
//    This sketch was generated by the drum sketch
//    generator, programmed by Alfonso Alba (fac)
//    E-mail: shadowfac@hotmail.com
//
//  ============================================================
//
//  License:
//
//  This software is licensed under the Creative Commons
//  Attribution-NonCommercial license. This license allows you
//  to tweak and build upon the code for non-commercial purposes,
//  without the requirement to license derivative works on the
//  same terms. If you wish to use this (or derived) work for
//  commercial work, please contact 20 Objects LLC at our website
//  (www.20objects.com).
//
//  For more information on the Creative Commons CC BY-NC license,
//  visit http://creativecommons.org/licenses/
//
//  ================= start of global section ==================


// Here we include the sample data files, which were automatically
// generated by a custom C program. In the future, I may release
// an executable version of this program so you can make your
// own sample banks

#include <SPI.h>
#include <DAC_MCP49xx.h>
DAC_MCP49xx dac(DAC_MCP49xx::MCP4921, 10);

#include <avr/pgmspace.h>

#include "pitch_table.h"
#include "sample0.h"
#include "sample1.h"
#include "sample2.h"
#include "sample3.h"
#include "sample4.h"
#include "sample5.h"
#include "sample6.h"
#include "sample7.h"

//  constants related to the Arduino Nano pin use
#define clkIn      2    // the digital (clock) input
#define digPin0    4    // the digital output pin D0
#define digPin1    5    // the digital output pin D1
#define digPin2    6    // the digital output pin D2
#define digPin3    7    // the digital output pin D3

//  constants related to the sample player
#define NUM_SAMPLES      8      // number of samples stored in flash RAM
#define MAX_SAMPLE_SIZE  1632   // maximum length of each sound

//  This array stores pointers to sample data in flash RAM
prog_uchar *sample_data[NUM_SAMPLES] = {
  sample0_data, sample1_data, sample2_data, sample3_data,
  sample4_data, sample5_data, sample6_data, sample7_data
};

//  This one stores the size of each sample
short sample_size[NUM_SAMPLES] = {
  sample0_size, sample1_size, sample2_size, sample3_size,
  sample4_size, sample5_size, sample6_size, sample7_size
};

//  This one stores the samplerate of each sound in samples per microsecond
float sample_fs_micro[NUM_SAMPLES] = {
  sample0_fs_micro, sample1_fs_micro, sample2_fs_micro, sample3_fs_micro,
  sample4_fs_micro, sample5_fs_micro, sample6_fs_micro, sample7_fs_micro
};


//  This is the SRAM buffer where the sound being played is stored
unsigned char sample[MAX_SAMPLE_SIZE];

//  Some global variables
char curSample = -1;    // current sample number (0 to NUM_SAMPLES-1)
short curSize = 0;      // size in samples of current sound
long curFSmicro = 0;   // sample rate of current sound
float envStart = 0;     // offset where decay starts (in samples)

short curVal0 = 0;      // stores the value of analog input 0
short curVal1 = 0;      // stores the value of analog input 1
short curVal2 = 0;      // stores the value of analog input 2
short curVal3 = 0;      // stores the value of analog input 3
unsigned long curTime = 0;  // current time in microseconds
unsigned long trigTime = 0; // time at which the sound was triggered
char trigState = 0;         // indicates if a sound is being played

short digi0Counter = 0;
short digi1Counter = 0;
short digi2Counter = 0;
short digi3Counter = 0;

short digi0Trigger = 2;
short digi1Trigger = 4;

//  variables for interrupt handling of the clock input
volatile char clkState = LOW;


// This function updates the sample buffer when a new sound is selected
// and also computes the playback samplerate according to the pitch
// Inputs: n = new sample number, pitch = pitch factor
void UpdateSample(int n, long pitch) {
  // limit n according to the maximum number of stored sounds
  if (n >= NUM_SAMPLES) n = NUM_SAMPLES - 1;

  // check if we really need to load a new waveform
  if (n != curSample) {
    // samples stored in flash RAM may be larger than the available buffer
    // so we must crop the sample when needed
    curSize = (sample_size[n] < MAX_SAMPLE_SIZE) ? sample_size[n] : MAX_SAMPLE_SIZE;

    // copy the sample in flash RAM to the buffer in SRAM
    memcpy_P(sample, sample_data[n], curSize);

    // update current sample number
    curSample = n;
  }

  // compute the playback samplerate taking the pitch factor into account;
  // this is specified in samples per microsecond
  curFSmicro = (long)(sample_fs_micro[curSample] * pitch);
}


//  ==================== start of setup() ======================

//  This setup routine should be used in any ArdCore sketch that
//  you choose to write; it sets up the pin usage, and sets up
//  initial state. Failure to properly set up the pin usage may
//  lead to damaging the Arduino hardware, or at the very least
//  cause your program to be unstable.

void setup()
{
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);

  // set up the digital outputs
  pinMode(digPin0, OUTPUT);
  digitalWrite(digPin0, LOW);
  pinMode(digPin1, OUTPUT);
  digitalWrite(digPin1, LOW);
  pinMode(digPin2, OUTPUT);
  digitalWrite(digPin3, LOW);
  pinMode(digPin3, OUTPUT);
  digitalWrite(digPin3, LOW);

  // Make sure some sample is loaded
  UpdateSample(0, 1);
  dacOutputFast(128);

  // set up an interrupt handler for the clock in. If you
  // aren't going to use clock input, you should probably
  // comment out this call.
  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr, RISING);
  
  dac.setBuffer(true);        //  Set FALSE for 5V vref.
  dac.setGain(2);             //  "1" for 5V vref. "2" for 2.5V vref.
  dac.setPortWrite(true);     //  Faster analog outs, but loses pin 7.	
  
}


//  This is the main loop

void loop()
{
  unsigned long deltaT = 0;;
  long p, s, n, out = 128;

  // check to see if the clock as been set
  if (clkState == HIGH) {

    digi0Counter++;
    digi1Counter++;
    digi2Counter++;
    digi3Counter++;
    
    if(digi0Counter >= digi0Trigger){
      digi0Counter=0;
      digitalWrite(digPin0, HIGH);    
    }
    
    if(digi1Counter >= digi1Trigger){
      digi1Counter=0;
      digitalWrite(digPin1, HIGH);    
    }
    
    if(digi2Counter >= map(analogRead(2),0,1014,1,12)){
      digi2Counter=0;
      digitalWrite(digPin2, HIGH);    
    }

    if(digi3Counter >= map(analogRead(3),0,1014,1,12)){
      digi3Counter=0;
      digitalWrite(digPin3, HIGH);    
    }    

    clkState = LOW;      // clock pulse acknowledged
    trigState = 1;       // a sound has been triggered
    trigTime = micros(); // remember starting time

    // Poll the analog inputs and update their values
    // This is only done at sample-start to save resources
    curVal0 = deJitter(analogRead(0), curVal0);
    curVal1 = deJitter(analogRead(1), curVal1);

    // Read pitch factor from lookup table
    p = pgm_read_dword_near(pitch_table + (curVal1));

    // Load new sample if necessary and update playback samplerate
    //UpdateSample((curVal0 + curVal2) >> 7, p);
    UpdateSample(map(curVal0, 0, 1023, 0, NUM_SAMPLES), p);

  }

  // Check if a sound is being played
  if (trigState) {
    // Obtain current time
    curTime = micros();

    // Obtain time since the sound was triggered;
    // a correction is made in the unlikely case the clock overflows
    deltaT = (curTime >= trigTime) ? (curTime - trigTime) : ((0xFFFFFFFF - trigTime + curTime) + 1);

    // Convert time to samples and separate integer from fractional part
    s = deltaT * curFSmicro;
    n = s >> 16;
    p = s & 0xFFFF;

    // Check if the sound hasn't reached its end
    if (n < curSize - 1) {
      // Compute output by linear interpolation and apply envelope
      out = ((65536 - p) * sample[n] + p * sample[n+1]) >> 16;
    } else {
      out = 128;
      trigState = 0;  // sample has ended
    }

    dacOutputFast(out);
  }
  
  digitalWrite(digPin0, LOW);
  digitalWrite(digPin1, LOW);
  digitalWrite(digPin2, LOW);
  digitalWrite(digPin3, LOW);
  
}


//  =================== convenience routines ===================

//  These routines are some things you will need to use for
//  various functions of the hardware. Explanations are provided
//  to help you know when to use them.

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
void isr()
{
  // Note: you don't want to spend a lot of time here, because
  // it interrupts the activity of the rest of your program.
  // In most cases, you just want to set a variable and get
  // out.
  clkState = HIGH;
}

//  dacOutput(long) - deal with the DAC output
//  ------------------------------------------
void dacOutput(long v)
{
  dac.outputA(v*4);
}


// This routine was taken from the Ardcore Oscillator example
// and it's supposed to be faster than dacOutput
void dacOutputFast(long v)
{
  dac.outputA(v*4);
}


//  deJitter(int, int) - smooth jitter input
//  ----------------------------------------
int deJitter(int v, int test)
{
  // this routine just make sure we have a significant value
  // change before we bother implementing it. This is useful
  // for cleaning up jittery analog inputs.
  if (abs(v - test) > 8) {
    return v;
  }
  return test;
}

//  ===================== end of program =======================
