//
// BeeView.cpp : Defines the entry point for the console application.
//
#define _ITERATOR_DEBUG_LEVEL 0
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#define ANIMOSITY 1
#define RIGHT_EYE 0xFF0000
#define LEFT_EYE  0x0088BB

#include "stdafx.h"
#include "windows.h"
#include <stdio.h>

#include <opencv2/opencv.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//#include <iostream>
//#include <fstream>
#include "GUI.h"      // GUI.cpp (mouse,keyboard,signals)

using namespace std;
using namespace cv;

#define showFrame(f) {cvResize(f,gSFrame);cvShowImage("video", gSFrame);}

// INVESTIGATE THE ENVIRONMENT
void showenv(char **e)
{int i=0;while(e[i]!=(char *)NULL){printf("[%s]\n", e[i]);i++;}}

CvCapture* capture  = NULL;
CvCapture* capture2 = NULL;
IplImage* gFrame = NULL;  // Main capture frame for live image
IplImage* gSFrame = NULL; // Sample frame for color capture
IplImage* gSnapshot = NULL; // Last picture taken
bool threeD = false;
int sequenceNum = 0;
int g_frameDelay = 100; // Minimally 10 frames per second

#include <process.h>
char *av[100];
char *ep[100];
void restart(void) { _execve("BeeView", av, ep); }

void cleanup(void) {
  if (capture != NULL) {
    cvReleaseCapture(&capture);
    capture = NULL;
  }
  if (capture2 != NULL) {
    cvReleaseCapture(&capture2);
    capture2 = NULL;
  }
}

void logerror(char *txt)
{
  char msg[100];
  sprintf(msg, "echo %s >>log.txt", msg);
  system(msg);
}

#include <signal.h>
void catcher(int signum)
{
  printf("caught %d\n",signum);
  loop = 0;
}

int
main(int argc, char **argv, char **envp)
{
  int i;
  for (i=0;i<100;i++) { av[i] = argv[i]; ep[i]=envp[i]; }
  setvbuf(stdout,NULL,_IONBF,0); // No buffering
  signal(SIGINT, catcher);       // cleanup if INTR
  system("C:/cygwin/bin/date");
  system("/bin/rm log.txt");
  // showenv(envp);

  capture = cvCaptureFromCAM(0);
  if(!capture){ printf("No primary capture device.\n"); exit(3); }
  if (threeD) {
    capture2 = cvCaptureFromCAM(1);
    if(!capture2) {
      printf("No secondary (left eye)capture device.\n"); exit(3);
    }
  }
	
  cvNamedWindow("video", CV_WINDOW_AUTOSIZE);
  int cxScreen = GetSystemMetrics(SM_CXSCREEN);
  int cyScreen = GetSystemMetrics(SM_CYSCREEN);
  //  printf("Screen is %d, %d\n", cxScreen, cyScreen);
  cvResizeWindow("video", cxScreen+50, cyScreen+130);
  cvMoveWindow( "video", -10, -30);
  int mouseParam = 5;
  cvSetMouseCallback("video", mouseHandler, &mouseParam);

  gFrame = cvQueryFrame(capture);
  gSFrame = cvCreateImage(cvSize(cxScreen+50,cyScreen+130), IPL_DEPTH_8U, 3);
  gSnapshot = cvQueryFrame(capture);
  while(loop) {
    gFrame = cvQueryFrame(capture);
    if(!gFrame) break;	// Quit if we cannot grab a frame.
    addWeighted( gFrame, 0.5, gSnapshot, 0.5, 0.0, gFrame); // Onion skinning
    showFrame(gFrame); /* Resize and display */
    checkKeyboard();
  }
  printf("cleanup after loop\n");
  cleanup();
  printf("after cleanup\n");
  _execve("BeeView", argv, envp);
  //  restart();
}

#ifdef ANIMOSITY
void usage(void)
{
   printf("animosity [3d]\n");
   printf("option 3d will produce anaglyph movies (use red-blue glasses)\n");
}

void merge(Mat img1, Mat img2)
{
   addWeighted( img1, 0.5, img2, 0.5, 0.0, img2);
}

void anaglyph(IplImage *img, int side)
{
  /*
  if (side == RIGHT_EYE) 
    color(img, RIGHT_EYE);
  else
    color(img, LEFT_EYE);
  */
}

void takePicture(void)
{
  char filename[32];
  sprintf(filename, "l/img_%4d.jpg", sequenceNum++);
  gSnapshot = cvQueryFrame(capture);
  if  (!gSnapshot)
    logerror("no capture on primary camera");
  else {
    if (threeD) {
      gFrame = cvQueryFrame(capture2);

      if (!gFrame) logerror("no capture on second camera");
      else { /* Do Anaglyph coloring */
	anaglyph(gSnapshot, RIGHT_EYE);
	anaglyph(gFrame, LEFT_EYE);
	imwrite(filename, gFrame, CV_IMWRITE_JPEG_QUALITY);
      }
    }
    imwrite(&filename[2], gSnapshot, CV_IMWRITE_JPEG_QUALITY);
  }
}

void playMovie(void)
{
IplImage* f = NULL;
int frameDelay;
char filename[32];

 if (threeD) {
   frameDelay = g_frameDelay/2;
 } else { 
   frameDelay = g_frameDelay;
 }
 for (int i=0; i<sequenceNum; i++) {
   sprintf(filename, "l/img_%4d.jpg", i);
   imread(&filename[2], f);  /* Right Eye Image */
   showFrame(f);
   usleep(frameDelay);
   if (threeD) {
     imread(filename, f);  /* Left Eye Image */
     showFrame(f);
     usleep(frameDelay);
   }
 }
}


void resetMovie(void)
{
  sequenceNum = 0;
  system("echo l/*.jpg *.jpg >>log.txt");
}

#endif
