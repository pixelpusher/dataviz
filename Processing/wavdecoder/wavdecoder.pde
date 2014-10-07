//
// modified by evan raskob e.raskob@rave.ac.uk from http://www.labbookpages.co.uk/audio/javaWavFiles.html
//
// 2014


import java.io.*;

boolean fileChosen = false;
PrintWriter output;
String outFileName = "sound.csv";


void setup()
{
  size(400,300);
  background(0);
  fill(200);
  text("hit space", 10,20);
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
      background(0,200,0);
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
      WavFile wavFile = WavFile.openWavFile(new File(selection.getAbsolutePath()));

      // Display information about the wav file
      wavFile.display();

      // Get the number of audio channels in the wav file
      int numChannels = wavFile.getNumChannels();

      // Create a buffer of 100 frames
      //double[] buffer = new double[100 * numChannels];
      
      println("found " + wavFile.getNumFrames() + " frames to read over " + numChannels + " channels");
      
      int passSize = 1000;
      long passes = ceil(wavFile.getNumFrames() / float(passSize));  // read 100 frames at a time
      long completePasses = 0; // just for checking...

      println("--doing " + passes + " passes of " + passSize);
      
      // only read first channel, in chunks
      double[] buffer = new double[passSize];

      long framesRead = 0, totalFramesRead = 0;
      int framesLeft = 0;
      double min = Double.MAX_VALUE;
      double max = Double.MIN_VALUE;


      do
      {
        // how many frames are left? (max of passSize)
        
        long frameDiff = (wavFile.getNumFrames() - totalFramesRead);
        if (frameDiff > passSize) 
        {
          framesLeft = passSize;
        }
        else
        {
          framesLeft = int(frameDiff); 
        }
        
        println("frames left " + framesLeft);
        
        // Read frames into buffer
        framesRead = wavFile.readFrames(buffer, framesLeft);
        
        totalFramesRead += framesRead; 

        // Loop through frames and look for minimum and maximum value
        for (int s=0; s<framesRead * numChannels; s+=numChannels)
        {

          // write to CSV - could be large!
          
          // this prints them into a single row
          //output.print(buffer[s]+",");
          
          // this prints into a single column
          output.println(buffer[s]);

          if (buffer[s] > max) max = buffer[s];
          if (buffer[s] < min) min = buffer[s];
        }
        if (framesRead > 0) completePasses++;
      }
      while (framesRead != 0);

      println("completed " + completePasses + " of " + passes);

      // Close the wavFile
      wavFile.close();
      output.flush(); // Writes the remaining data to the file
      output.close(); // Finishes the file

        // Output the minimum and maximum value to the file
      println("Min:" + min + " Max:" + max);
    }
    catch (Exception e)
    {
      System.err.println(e);
    }
    exit(); // Stops the program
  }
}

