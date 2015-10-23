//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_
//
//	bmp.h: Header file for read/write an image in Windows BMP file format
//
// Ankit Mohan, 3/2/2003   - Got out of my OLD game code
// Ankit Mohan, 10/07/2003 - Removed text/font related functions.
// Jack Tumblin 11/10/2004 - Formatted, revised comments

#include <stdio.h>
#include <stdlib.h>
#include <GLUT/glut.h>
#include <string.h>
#include <math.h>			// for sine, cosine fcns

struct BITMAP_FILEHEADER 
//------------------------------------------------------------------------------
// Copied from a Windows header file that defines BMP format
{ 
	unsigned long	bfSize; 
	unsigned short	bfReserved1; 
	unsigned short	bfReserved2; 
	unsigned long	bfOffBits; 
};

struct BITMAP_INFOHEADER
//------------------------------------------------------------------------------
// Copied from a Windows header file that defines BMP format
{
	unsigned long  biSize; 
	unsigned long   biWidth; 
	unsigned long   biHeight; 
	unsigned short   biPlanes; 
	unsigned short   biBitCount; 
	unsigned long  biCompression; 
	unsigned long  biSizeImage; 
	unsigned long   biXPelsPerMeter; 
	unsigned long   biYPelsPerMeter; 
	unsigned long  biClrUsed; 
	unsigned long  biClrImportant; 
};

struct RGB_TRIPLE 
//------------------------------------------------------------------------------
// Copied from a Windows header file that defines BMP format
{
	unsigned char	rgbtBlue; 
	unsigned char	rgbtGreen; 
	unsigned char	rgbtRed; 
};


int load_BMP(char *name, unsigned char **image, int *height, int *width);
//Loads a BMP file and returns it, with height and width.
//Allocation for the file is done in this function. Memory should be 
 //freed by the caller. 
 //The image data is returned in a 1-dimensional array that contains the pixel
 //RGBA values one from the top down to the bottom row (each row from left to
 //right). The alpha value is set to 255. 
 //
 //returns -1 if file not found
 //        -2 if compressed BMP (cannot load)
 //        -3 if not 24 bit BMP (cannot load)
 //
 //Sample code:
 //
 //int w, h;
 //unsigned char *img;
 //load_BMP("filename.bmp", &img, &h, &w);
 //


int save_BMP(char *name, unsigned char *image, int width, int height);
 //Save a BMP file.
 // 
 //The image data is expected in a 1-dimensional array that contains the pixel
 //RGBA values one from the bottom up to the top row (each row from left to
 //right). The alpha value is ignored. 
 //
 //returns -1 cannot write to file
 //         0 on success
 ///

int makeZone(unsigned char **image, int *width, int *height);
// Make a zone-plate text signal.