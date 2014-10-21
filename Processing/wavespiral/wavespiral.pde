//
//  calculate RMS from WAVE file audio and create 3D sculpture
//
//  copyright evan raskob 2014, all rights reserved (for now)
//  by evan raskob evanraskob@gmail.com using code from http://code.google.com/p/musicg/
//
// Draw a base spiral and offset it by the sound volume (RMS)
//
// TODO:
// * log scaling out output
// * make spirals by tempo so peaks/troughs line up
// * draw as round circles instead of blocks
// 


import java.io.*;
import toxi.geom.*;
import toxi.geom.mesh.*;
import toxi.math.*;
import toxi.volume.*;
import toxi.math.waves.*;
import toxi.processing.*;
import processing.opengl.*;

boolean fileChosen = false;
PrintWriter output, outputRMS;
float[] soundAmplitudes;
float[] rmsAmplitudes;
ToxiclibsSupport gfx;


TriangleMesh mesh;


void setup()
{
  size(1024, 768, P3D);
  background(0);

  fill(200);
  text("hit space", 10, 20);

  gfx=new ToxiclibsSupport(this);
  noLoop(); //turn off loop until needed

  mesh = new TriangleMesh();

  //createSpiral();
}




void createSpiral()
{
  int totalPoints = rmsAmplitudes.length;

  int turns = 5;
  int pointsPerTurn = rmsAmplitudes.length/turns;

  float distanceBetweenSpirals = 40;
  float spiralThickness = 30;  
  float spiralRadius = 100;


  Vec3D prevPoint = new Vec3D(0, 0, 0);
  Vec3D spiralPoint = new Vec3D(spiralRadius, 0, 0);

  Vec3D B0 = new Vec3D();
  Vec3D B0r = new Vec3D();
  Vec3D B1 = new Vec3D();
  Vec3D B1r = new Vec3D();
  Vec3D A0 = new Vec3D();
  Vec3D A0r = new Vec3D();
  Vec3D A1 = new Vec3D();
  Vec3D A1r = new Vec3D();

  int pointsRendered = 0;

  for (int turn=0; turn < turns; turn++)
  {
    float turnsProgress = float(turn)/turns;
    float turnsAngle = turnsProgress * TWO_PI;


    for (int turnIndex=0; turnIndex < pointsPerTurn; turnIndex++)
    {
      Vec3D prevPrevPoint = prevPoint;

      prevPoint = spiralPoint;

      float turnsInnerProgress = float(turnIndex)/pointsPerTurn;
      float turnsInnerAngle = turnsInnerProgress * TWO_PI;
      float x = cos( turnsInnerAngle ) * spiralRadius;
      float y = sin( turnsInnerAngle ) * spiralRadius;
      float z = (turn + turnsInnerProgress) * (spiralThickness+distanceBetweenSpirals);
      spiralPoint = new Vec3D(x, y, z);

      //float offset = ( sin( turnsInnerAngle ) + 2 ) * tubeRadius ;
      float offset = (rmsAmplitudes[pointsRendered]+0.05) * 60;
      float offsetX = offset * cos( turnsInnerAngle ); 
      float offsetY = offset * sin( turnsInnerAngle );

      Vec3D BB0 = A0;
      Vec3D BB0r = A0r;
      Vec3D BB1 = A1;
      Vec3D BB1r = A1r;

      A0  = spiralPoint.add( 0, 0, 0);
      A0r = spiralPoint.add( offsetX, offsetY, 0);

      A1  = spiralPoint.add( 0, 0, spiralThickness);
      A1r = spiralPoint.add( offsetX, offsetY, spiralThickness);



      B0  = prevPoint.add( 0, 0, 0);
      B0r = prevPoint.add( offsetX, offsetY, 0);

      B1  = prevPoint.add( 0, 0, spiralThickness);
      B1r = prevPoint.add( offsetX, offsetY, spiralThickness);

      // connect gaps
      if (pointsRendered > 0)
      {
        mesh.addFace( B1, B1r, BB1r);

        //top
        mesh.addFace( B1r, B0r, BB0r);
        mesh.addFace( BB0r, BB1r, B1r);

        mesh.addFace( B0r, B0, BB0r);
      } else {
        //add beginning
        mesh.addFace( A0, A1, A1r);
        mesh.addFace( A1r, A0r, A0);
      }

      //bottom
      mesh.addFace( A0, A1, B1);
      mesh.addFace( B1, B0, A0);

      // right side
      mesh.addFace( A1, B1, B1r);
      mesh.addFace( B1r, A1r, A1);

      // top side
      mesh.addFace( A1r, B1r, B0r);
      mesh.addFace( B0r, A0r, A1r);

      // left side
      mesh.addFace( A0r, B0r, B0);
      mesh.addFace( B0, A0, A0r);

      pointsRendered++;
    }
  }
  //end cap
  mesh.addFace( A0, A1, A1r);
  mesh.addFace( A1r, A0r, A0);

  mesh.computeFaceNormals();
  mesh.computeVertexNormals();

  loop(); // start drawing
}



void draw()
{
  background(0);
  fill(200, 0, 200, 100);
  stroke(255);

  camera(width - mouseX, height - mouseY, 400, 0, 0, 0, 0, 1, 0);
  lights();

  beginShape(TRIANGLES);
  gfx.mesh( mesh );
  endShape();
}

void keyReleased()
{
  if (key == 'a')
  {
    loop();
  } else if (key == 'z')
  {
    noLoop();
  } else if (key == 's')
  {
    mesh.saveAsSTL( "mesh.stl" );
    println("saved");
  } else if (key==' ') 
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
      soundAmplitudes = new float[amplitudes.length];

      int RMSSize = 44; // 1/100th of a second  CHANGEME!!!!!  Remember that 44100 is 1 sec
      rmsAmplitudes = new float[amplitudes.length/RMSSize];

      println("found " + amplitudes.length + " samples");

      println("calculating " + rmsAmplitudes.length + " samples");

      int currentIndex = 0;
      int rmsArrayIndex = 0;

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

        rmsAmplitudes[rmsArrayIndex++] = RMSAve;
        println("stored " + rmsArrayIndex + ":" + RMSAve);
      }
    }
    catch (Exception e)
    {
      System.err.println(e);
    }

    createSpiral();
  }
}

