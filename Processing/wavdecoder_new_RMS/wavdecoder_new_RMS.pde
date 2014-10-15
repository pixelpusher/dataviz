//
//  calculate RMS from WAVE file audio and dump to javascript Array
//
//  by evan raskob e.raskob@rave.ac.uk using code from http://code.google.com/p/musicg/
//
//  for 206: Tangible Media 2014 (BA Hons Design Interactions)
//


import java.io.*;


boolean fileChosen = false;
PrintWriter output, outputRMS;
String outFileName = "sound_rms.js";



void setup()
{
  size(400, 300);
  background(0);
  fill(200);
  text("hit space", 10, 20);
  output = createWriter(outFileName);
}

void draw()
{
}

void keyReleased()
{
  if (key==' ') 
    if (!fileChosen) 
    {
      fileChosen = true;
      background(0, 200, 0);
      selectInput("Select a file to process:", "fileSelected");
    }
}


void fileSelected(File selection) 
{
  if (selection == null) 
  {
    println("Window was closed or the user hit cancel.");
  } else 
  {
    println("User selected " + selection.getAbsolutePath());

    try
    {
      // Open the wav file specified as the first argument
      Wave wavFile = new Wave(selection.getAbsolutePath());

      //short[] amplitudes = wavFile.getSampleAmplitudes();
      NormalizedSampleAmplitudes nsa = new NormalizedSampleAmplitudes(wavFile);
      double[] amplitudes = nsa.getNormalizedAmplitudes();
      
      println("found " + amplitudes.length + " samples");

      // start writing to file
      output.print( "var audioData = [" );
      
      int currentIndex = 0;
      int RMSSize = 4410; // 1/10th of a second  CHANGEME!!!!!  Remember that 44100 is 1 sec

      while (currentIndex < amplitudes.length)
      {
        int samplesLeft = amplitudes.length - currentIndex;
        if (samplesLeft < RMSSize) break; // stop loop!
        
        int RMSIndex = 0;
        float RMSSum = 0;
        
        while (RMSIndex < RMSSize)
        {
          // convert data to float
          float data = (float)amplitudes[currentIndex];
          RMSSum += data*data; // add square of data to sum
          currentIndex++; 
          RMSIndex++;
        }
        
        // find average value - could also scale logarithmically
        float RMSAve = RMSSum / RMSSize;
        
        // convert float to formatted string with 4 decimal places
        output.print( nf(RMSAve,1,4) );
        
        output.print( ",");
        
        
      }
      // close javascript array variable
      output.print( "];");
    }
    catch (Exception e)
    {
      System.err.println(e);
    }
    
    output.flush(); // Writes the remaining data to the file
    output.close(); // Finishes the file
    exit(); // Stops the program
  }
}
