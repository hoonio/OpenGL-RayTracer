//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_
//
// bmp.cpp: Simple BMP file loader
//
// Function to load a BMP file etc.
//
// Ankit Mohan, 3/2/2003 - Got out of my OLD game code
// Ankit Mohan, 10/07/2003 - Removed text/font related functions.
// Jack Tumblin,11/10/2004 - formatted, revised comments.

#include "bmp.h"


int load_BMP(char *name, unsigned char **image, int *width, int *height)
//------------------------------------------------------------------------------
// Loads a BMP file and returns it, with height and width.
// Allocation for the file is done in this function. Memory should be 
// freed by the caller. 
// The image data is returned in a 1-dimensional array that contains the pixel
// RGBA values one from the bottom up to the top row (each row from left to
// right). The alpha value is set to 255. 
//
// returns -1 if file not found
//         -2 if compressed BMP (cannot load)
//         -3 if not 24 bit BMP (cannot load)
//
// Sample code--load a BMP image as an openGL texture map:
//
// int w, h;
// unsigned char **img;
//     loadBMP("filename.bmp", &img, &h, &w);
//
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, 
//													GL_UNSIGNED_BYTE, img);
//
{
FILE *bmpFile;
BITMAP_FILEHEADER bmpFileHeader;
BITMAP_INFOHEADER bmpInfoHeader;
struct RGB_TRIPLE rgb;
unsigned long i, j, offset;
unsigned short type;
int padding;
unsigned char scratch[10];

	bmpFile = fopen(name, "rb");
	if (!bmpFile)
		return -1;

	// first read the first two bytes (type) 
	fread(&type, 2, 1, bmpFile);

	fread(&bmpFileHeader, sizeof(bmpFileHeader), 1, bmpFile); // Read File Header 
	fread(&bmpInfoHeader, sizeof(bmpInfoHeader), 1, bmpFile); // Read Info Header 
    
    if (bmpInfoHeader.biCompression)
		return -2; // file was stored with compression--I don't know how to read it. 

    if (bmpInfoHeader.biBitCount != 24)
		return -3; // file is not 24 bits/pixel;  I don't support palette-based images.

    *image = (unsigned char *)malloc(bmpInfoHeader.biWidth*bmpInfoHeader.biHeight*4);
    *height = bmpInfoHeader.biHeight;
    *width = bmpInfoHeader.biWidth;
    offset=0;

	// number of bytes in each line of a bmp file is a multiple of 4.
	// provide adequate padding
	padding = (4 - (*width * 3) % 4) % 4;

    for (j=0; j<bmpInfoHeader.biHeight; j++)
	{
		for (i=0; i<bmpInfoHeader.biWidth; i++) 
		{
			fread(&rgb, sizeof(rgb), 1, bmpFile);

			(*image)[offset] = rgb.rgbtRed;
	        offset++;

		    (*image)[offset] = rgb.rgbtGreen;
	        offset++;

		    (*image)[offset] = rgb.rgbtBlue;
			offset++;

			(*image)[offset] = 255;
		    offset++;
		}
		fread(scratch, padding, 1, bmpFile); // read the padding and ignore it 
	}

	fclose(bmpFile);

	return 0;
}


int save_BMP(char *name, unsigned char *image, int width, int height)
//------------------------------------------------------------------------------
// Save a BMP file.  The image data is expected in a 1-dimensional array that 
// contains the pixel RGBA values scanned from the bottom up to the top row,
// with each row scanned from left to right. The alpha value is ignored. 
// returns -1 on file writing error,
//          0 on success.
{
FILE *bmpFile;
BITMAP_FILEHEADER bmpFileHeader;
BITMAP_INFOHEADER bmpInfoHeader;
struct RGB_TRIPLE rgb;
unsigned long i, j, offset;
int padding;
unsigned char scratch[10];

	memset(scratch, 0, sizeof(scratch));

	bmpFile = fopen(name, "wb");
	if (!bmpFile)
		return -1;

	// first write the first two bytes (type) 
	fwrite("BM", 2, 1, bmpFile);

	// File header 
	memset(&bmpFileHeader, 0, sizeof(struct BITMAP_FILEHEADER));
	bmpFileHeader.bfSize = 2 + sizeof(struct BITMAP_FILEHEADER) + sizeof(struct BITMAP_INFOHEADER)
			+ width * height * 3;
	bmpFileHeader.bfOffBits = 2 + sizeof(struct BITMAP_FILEHEADER) + sizeof(struct BITMAP_INFOHEADER);
	fwrite(&bmpFileHeader, sizeof(bmpFileHeader), 1, bmpFile); // write it! 

	// Info header 
	memset(&bmpInfoHeader, 0, sizeof(struct BITMAP_INFOHEADER));
	bmpInfoHeader.biSize = 0x28;
	bmpInfoHeader.biWidth = width;
	bmpInfoHeader.biHeight = height;
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = 24;
	bmpInfoHeader.biCompression = 0;
	bmpInfoHeader.biSizeImage = width * height * 3;
	bmpInfoHeader.biXPelsPerMeter = 1;
	bmpInfoHeader.biYPelsPerMeter = 1;
	bmpInfoHeader.biClrUsed = 0;
	bmpInfoHeader.biClrImportant = 0;
	fwrite(&bmpInfoHeader, sizeof(bmpInfoHeader), 1, bmpFile); // write it! 

    offset=0;

	/// number of bytes in each line of a bmp file is a multiple of 4.
	 // provide adequate padding
	 ///
	padding = (4 - (width * 3) % 4) % 4;

    for (j=0; j<bmpInfoHeader.biHeight; j++) {
		for (i=0; i<bmpInfoHeader.biWidth; i++) {
			rgb.rgbtRed = image[offset];
	        offset++;
		    
			rgb.rgbtGreen = image[offset];
	        offset++;

		    rgb.rgbtBlue = image[offset];
			offset++;

		    offset++; // ignore alpha 

			fwrite(&rgb, sizeof(rgb), 1, bmpFile);
		}
		fwrite(scratch, padding, 1, bmpFile); // write the padding 
	}

	fclose(bmpFile);

	return 0;
}

int makeZone(unsigned char **image, int *width, int *height)
//------------------------------------------------------------------------------
// Make a 'zone plate' image to demo aliasing; use default size of 256 x 256
//
{
#define ZONE_SIZE 256		// Make sure this is a power of 2.

struct RGB_TRIPLE rgb;
long i, j, offset;
static int imax, jmax;		// 
double val, dist2,tmpx,tmpy;					// sine-wave value.

    *image = (unsigned char *)malloc(ZONE_SIZE * ZONE_SIZE * 4);
	imax = ZONE_SIZE;
	jmax = ZONE_SIZE;
    height = &jmax;		// point to these static vars (bad idea; I'm rushed).
    width = &imax;
    offset=0;

    for (j=0; j< ZONE_SIZE; j++) 
	{
		for (i=0; i<ZONE_SIZE; i++) 
		{
			tmpx = (double)(i-(ZONE_SIZE/2));		// How far from image center?
			tmpy = (double)(j-(ZONE_SIZE/2));
			dist2 = (tmpx*tmpx + tmpy*tmpy) / (ZONE_SIZE*ZONE_SIZE*2);
			val = 0.5 + 0.45*cos(dist2 * 1000.0);
			val *= 255.0;
			rgb.rgbtRed = (unsigned short)(val + 0.5);
			rgb.rgbtBlue = rgb.rgbtGreen = rgb.rgbtRed;

			(*image)[offset] = rgb.rgbtRed;
	        offset++;

		    (*image)[offset] = rgb.rgbtGreen;
	        offset++;

		    (*image)[offset] = rgb.rgbtBlue;
			offset++;

			(*image)[offset] = 255;
		    offset++;
		}
	}
	save_BMP("zone.bmp",*image, *width, *height);
	return 0;
}
