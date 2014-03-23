/*
  Cadnza
  
  This software is for demonstration purposes only. Use at your own risk.
  
  This program demonstrates how to play a neat song named "Cadnza" that
  can be found in William Barden Junior's book "TRS-80 Color Computer
  and MC-10 Programs". The song was composed by Anthony (Craig) Verbeck.
  The book is now out of print, but was originally available at Radio Shack
  (26-3195). See http://williambardenjr.com for more information about
  the author.
  
  For this project, I connected an Arduino Uno to the speaker in a
  RadioShack Electronics Learning Lab (28-280) via a 470ohm resistor.
  First, connect the Arduino GND to the Learning Lab (LL). Connect Arduino
  pin 6 to one end of the 470ohm resistor. Connect the other end of the
  resistor to LL terminal 68. Connect LL terminal 69 to the LL GND.
  The sound will be a little faint - you can try boosting it with
  the transitors or LM386 included with the kit.
 */

#include "pitches.h"

/** Pin to use to drive the speaker */
static const int tonePin = 6;

/**
 * Data taken almost directly from the Cadnza program.
 */
static const byte data[] = {
  // Data
  125, 147, 170, 147,
  140, 159, 176, 159,
  147, 170, 185, 170,
  159, 176, 193, 176,
  78, 117, 140, 117,
  89, 125, 147, 125,
  108, 140, 159, 140,
  193, 204, 216, 204,
  
  // Melody
  204, 200, 193, 185,
  159, 170, 176, 185,
  204, 200, 193, 204,
  210, 204, 200, 216,
  218, 216, 210, 204,
  200, 204, 210, 216,
  210, 204, 200, 193,
  204, 200, 193, 185,
  
  // C Part
  176, 147, 125, 108,
  89, 125, 140, 147,
  159, 176, 170, 193,
  185, 200, 210, 216,
  218, 227, 232, 231,
  227, 218, 223, 210,
  204, 193, 185, 176,
  170, 147, 125, 89,
  
  0
};

/**
 * This maps the Color Computer sound command frequency
 * values to Arduino values. Non-maps are keps at zero.
 * Figuring out the math that could replace this table
 * is left as an exercise for the reader.
 */
static const int coco2Arduino[] = {
    // 0->31
    0, 0, 0, 0, 0, NOTE_F3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, NOTE_FS3, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    
    // 32->63
    NOTE_G3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, NOTE_GS3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, NOTE_A3, 0, 0, 0, 0, 0,

    // 64->95
    0, 0, 0, 0, 0, NOTE_AS3, 0, 0,
    0, 0, 0, 0, 0, 0, NOTE_B3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, NOTE_C4, 0, 0, 0, 0, 0, 0,

    // 96->127
    0, 0, 0, NOTE_CS4, 0, 0, 0, 0,
    0, 0, 0, 0, NOTE_D4, 0, 0, 0,
    0, 0, 0, 0, 0, NOTE_DS4, 0, 0,
    0, 0, 0, 0, 0, NOTE_E4, 0, 0,

    // 128->159
    0, 0, 0, 0, 0, NOTE_F4, 0, 0,
    0, 0, 0, 0, NOTE_FS4, 0, 0, 0,
    0, 0, 0, NOTE_G4, 0, 0, 0, 0,
    0, NOTE_GS4, 0, 0, 0, 0, 0, NOTE_A4,
    
    // 160->191
    0, 0, 0, 0, 0, NOTE_AS4, 0, 0,
    0, 0, NOTE_B4, 0, 0, 0, 0, 0,
    NOTE_C5, 0, 0, 0, NOTE_CS5, 0, 0, 0,
    0, NOTE_D5, 0, 0, 0, NOTE_DS5, 0, 0,
    
    // 192->223
    0, NOTE_E5, 0, 0, 0, NOTE_F5, 0, 0,
    NOTE_FS5, 0, 0, 0, NOTE_G5, 0, 0, NOTE_GS5,
    0, 0, NOTE_A5, 0, 0, NOTE_AS5, 0, 0,
    NOTE_B5, 0, NOTE_C6, 0, 0, NOTE_CS6, 0, NOTE_D6,

    // 224>255
    0, NOTE_DS6, 0, NOTE_E6, 0, NOTE_F6, 0, NOTE_FS6,
    NOTE_G6, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/**
 * Mimics the Color Computer sound command.
 * @param frequency[in] frequency on [0, 255]
 * @param duration[in] duration on [0, 255]
 * @param stopPlaying[in] whether or not to stop playing after completing.
 *        true values behave more like the Color Computer, but false
 *        results in smoother sound transitions.
 */
static void sound(int frequency, int duration, bool stopPlaying = true) {
  tone(tonePin, coco2Arduino[frequency]);
  delay(duration * 75);
  if (stopPlaying)
    noTone(tonePin);
}

/** chord data */
static byte cd[8][4];

/** melody */
static byte md[200];

/**
 * This function sets up the chord and melody
 * arrays.
 */
static void readArrays() {
  int dataIndex = 0;
  for(int ii=0; ii<8; ii++) {
    for(int jj=0; jj<4; jj++, dataIndex++) {
      cd[ii][jj] = data[dataIndex];
    }
  }
  
  int ii = 0;
  do {
    md[ii] = data[dataIndex++];
  } while(md[ii++] > 0);
}

/**
 * Plays a chord.
 *
 * @param s[in] speed
 * @param n[in] number of times to play chord
 * @param c[in] chord to play on [0, 7]
 */
static void playChord(int n, int s, int c) {
  for(int n1 = 0; n1<n; n1++)
    for(int n2 = 0; n2<4; n2++)
      sound(cd[c][n2], s, false);
}

/**
 * Plays a melody and chord
 * Plays the melody as a first note
 * Uses the same parameters as playChord with the
 * addition of nt which is the note of the melody
 * to start play.
 *
 * @param s[in] speed
 * @param n[in] number of times to play chord
 * @param c[in] chord to play on [0, 7]
 * @param nt[in/out] note in melody to start play on [0, 199]
 */
static void playMelodyAndChord(int n, int s, int c, int &nt) {
  if (md[nt] <= 0) return;
  for(int n1 = 0; n1<n; n1++) {
    for(int n2 = 0; n2<4; n2++) {
      sound(md[nt], s); nt++;
      sound(cd[c][n2], s, false);
      if (md[nt] <= 0) return;
    }
  }
}

/** Setup does not have anything to do */
void setup() {
  // read arrays
  readArrays();
}


/** Each loop plays the song once */
void loop() {
  // Start playing the song
  int s = 4, n = 4, c = 0;
  playChord(n, s, c);
  c = 5; playChord(n, s, c);
  c = 4, n = 2; playChord(n, s, c);
  sound(78, 4, false); sound(125, 4, false);
  sound(147, 4, false); sound(159, 4, false);
  c = 2, n = 1; playChord(n, s, c);
  
  // Counterpoint
  n = 4;
  c = 0, s = 2; int nt = 0; playMelodyAndChord(n, s, c, nt);
  c = 5, nt = 0; playMelodyAndChord(n, s, c, nt);
  c = 0, nt = 0; playMelodyAndChord(n, s, c, nt);
  c = 5, nt = 0; playMelodyAndChord(n, s, c, nt);
  c = 4, n = 2, nt = 7; playMelodyAndChord(n, s, c, nt);
  sound(78, 2, false); sound(170, 2, false);
  sound(125, 2, false); sound(193, 2, false);
  sound(147, 2, false); sound(204, 2, false);
  sound(159, 2, false); sound(210, 2, false);
  nt = 7, c = 2, n = 1; playMelodyAndChord(n, s, c, nt);
  s = 1;
  n = 8, nt = 0, c = 0;
  playMelodyAndChord(n, s, c, nt);
  c = 5; playMelodyAndChord(n, s, c, nt);
  nt = 0;
  c = 0; playMelodyAndChord(n, s, c, nt);
  c = 5; playMelodyAndChord(n, s, c, nt);
  nt = 0;
  c = 4; playMelodyAndChord(n, s, c, nt);
  nt = 0;
  playMelodyAndChord(n, s, c, nt);
  c = 0, n = 4, nt = 0; playMelodyAndChord(n, s, c, nt);
  c = 3, n = 2; playMelodyAndChord(n, s, c, nt);
  c = 4; playMelodyAndChord(n, s, c, nt);
  c = 0; n = 4, nt = 0; playMelodyAndChord(n, s, c, nt);
  c = 3; n = 2; playMelodyAndChord(n, s, c, nt);
  c = 4; playMelodyAndChord(n, s, c, nt);
  c = 5; playMelodyAndChord(n, s, c, nt);
  c = 6; playMelodyAndChord(n, s, c, nt);
  nt = 0;
  c = 7, n = 8; playMelodyAndChord(n, s, c, nt);
  nt = 0;
  s = 8, n = 1; playMelodyAndChord(n, s, c, nt);
  sound(125, 32);
 
  delay(10000);
}
