//
//  copy data from WAVE file audio and dump to javascript Array
//
//  by evan raskob e.raskob@rave.ac.uk using code from http://code.google.com/p/musicg/
//
//  for 206: Tangible Media 2014 (BA Hons Design Interactions)
//

import java.io.*;


boolean fileChosen = false;
PrintWriter output, outputRMS;
String outFileName = "sound.js";



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

      while (currentIndex < amplitudes.length)
      {
        if (currentIndex % 80 == 0) output.println();
        
        // convert data to float
        //float data = float(amplitudes[currentIndex]) / float(Short.MAX_VALUE);
        
        float data = (float)amplitudes[currentIndex];
        output.print( nf(data,1,4) );
        
        
        if (currentIndex < (amplitudes.length-1))
        {
          output.print( ",");
        }
        currentIndex++; // next index
      }
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
