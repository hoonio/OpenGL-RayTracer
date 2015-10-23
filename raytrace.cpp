//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_
//
//	Function implementations for Ray-tracing classes
//
//	01/20/2005 J. Tumblin -- Created.
//  02/06/2006 J. Tumblin -- added a crude, non-vector sphere traced at origin.
//							Start by cleaning up this code--use a vector class
//							as recommended by P. Shirley's book.

#include "raytrace.h"			// home-brewed vector/matrix classes.

//==============================
//
//  CCamera member functions
//
//==============================

CCamera::~CCamera(void)					
 //-----------------------------------------------------------------------------
 // default destructor
 {
	xsize = ysize = 0;
	xNow = yNow = 0;
	vrp.zero();  vrp.row[3] = 1.0;	// set to world space origin.
	vpn.zero();  vpn.row[2] = 1.0;	// look down -z axis; (vpn = +z axis)
	vup.zero();	 vup.row[1] = 1.0;	// make +y axis the 'up' direction.

	u.zero(); u.row[0] = 1.0;		// make u,v,n match world space x,y,z axes
	v.zero(); v.row[1] = 1.0;
	n.zero(); n.row[2] = 1.0;
 }

CCamera::CCamera(void)
//------------------------------------------------------------------------------
// default constructor
{

}

BOOL CCamera::SetFrustum(double nuLeft, double nuRight, 
						 double nuBot, double nuTop,
						 int nuXsize, int nuYsize)
//------------------------------------------------------------------------------
// Set the camera's internal parameters by setting its clipping planes;
// also set the number of pixels we'll make.
// (Note that we presume znear = 1.0 and zfar is unlimited).
{
	// for now, just copy.  You may want to do more later...
	iLeft = nuLeft;		
	iRight = nuRight;
	iTop = nuTop;
	iBot = nuBot;
	xsize = nuXsize;
	ysize = nuYsize;
	return(TRUE);
}

BOOL CCamera::lookAt(Vec4 nuVRP, Vec4 nuVPN, Vec4 nuVUP)
//------------------------------------------------------------------------------
// Set the camera's external parameters by setting nuVRP, its world-space center
// of projection; setting nuVPN, the camera's z-axis direction (camera looks 
// down -z), and its 'up' direction nuVUP.
// VRP = 'View Reference Point'; VPN = 'View Plane Normal'; VUP = View UP vector
{
	// Copy. Late you'll want to use these vrp,vpn,vup values to
	// compute the u,v,n vectors.
	vrp.copy(nuVRP);
	vpn.copy(nuVPN);
	vup.copy(nuVUP);
	return(FALSE);
}

BOOL CCamera::makeEyeRay(CRay& eyeRay, double xpt, double ypt)
//------------------------------------------------------------------------------
// Set the values of a given 'eyeRay' ray so that it starts at the camera's 
// center-of-projection in world space, and points in the direction of pixel 
// (x,y) on the image rectangle.  If x,y are integers, then the ray goes 
// through the pixel location. Return FALSE on error.
{
double xpos,ypos;			// ray's position within the image rectangle
double xfrac,yfrac;

// for now, ignore the vrp and the u,v,n vectors and assume they're the 
// origin and the x,y,z axes.
// later, you should use them to compute the eye rays.

	// If we divide the image rectangle into a grid of xsize,ysize little
	// squares, then the width, height of each square is:
	xfrac = (iRight - iLeft) / (double)xsize;
	yfrac = (iTop - iBot) / (double)ysize;
	// We want the x,y origin to start at the CENTER of the lower-left square:
	xpos = iLeft + 0.5*xfrac;
	ypos = iBot + 0.5*yfrac;
	// and we want to find the position in the rectangle for x,y:
	xpos += xpt*xfrac;
	ypos += ypt*yfrac;
	// The eye ray always begins at the camera's center of projection, so
	eyeRay.orig.zero();			// (for now, our camera fixed at the origin)
	eyeRay.orig.row[3] = 1.0;	//(orig is a point; set its w= 1.0).

	eyeRay.dir.row[0] = xpos;	// Set 'dir' to the point on our image rectangle
	eyeRay.dir.row[1] = ypos;
	eyeRay.dir.row[2] = -1.0;
	eyeRay.dir.row[3] = 1.0;	// (it's a point, so it's w is 1.0)
	eyeRay.dir.sub(eyeRay.dir,eyeRay.orig);	
								// Ray direction = pixel point - ray origin.
	eyeRay.dir.norm(eyeRay.dir);	// 
	// Now that we have the ray's direction, move ray's starting point to
	// the world-space camera location:
	eyeRay.orig.copy(vrp);
	return(TRUE);
}


//==============================
//
//  CRay member functions
//
//==============================
CRay::~CRay(void)					
 //-----------------------------------------------------------------------------
 // default destructor
 {
 }

CRay::CRay(void)
//------------------------------------------------------------------------------
// default constructor
{
		orig.zero();			// make ray start at origin
		orig.row[3] = 1.0;		// it's a point, not a vector; set w to 1.0.
		dir.zero();
		dir.row[0]= 1.0;		// make ray point in +x direction.
}

//==============================
//
//  CSphere member functions
//
//==============================
CSphere::~CSphere(void)					
 //-----------------------------------------------------------------------------
 // default destructor
 {
 }

CSphere::CSphere(void)
//------------------------------------------------------------------------------
// default constructor
{
int i;

	radius = 0.5;
	pos.zero();					// put unit sphere at origin, but
	pos.row[0] = 0.3;			// offset it a little diagonally.
	pos.row[1] = 0.2;
	pos.row[2] = 0.4;
	pos.row[3] = 1.0;			// this is a point; set w=1.

	for(i=0; i<3; i++)
	{
		K_a.row[i] = 0.30;		// pick an ambient reflectance color,
		K_d.row[i] = 0.50;		// pick a diffuse reflectance color,
		K_s.row[i] = 0.70;		// pick a specular reflectance color,
	}
	K_shiny = 3;				// set its Phong 'shinyness' coefficient.

}

double CSphere::hitBAD(CRay& in)
//------------------------------------------------------------------------------
// a HACK intersection test: find how close the ray gets to the sphere's 
// silhouette: positive for rays that are WITHIN the silhouette, negative
// outside. **NOT** a good sphere intersection test!!! make a GOOD one
// instead that uses quadratics.
{
Vec4 toMe, toRay;
double tdist, r;

	// THIS IS A HACK, NOT A PROPER RAY/SPHERE INTERSECTION TEST!
	toMe.sub(pos,in.orig);		// find vector from eyepoint to sphere
	tdist = toMe.dot(toMe,in.dir);	// find dot-product with ray direction
	toRay.scale(in.dir,tdist);
	toRay.sub(toRay,toMe);
	r = toRay.norm(toRay);
	return (radius-r);			// positive if r is < radius...
}

//==============================
//
//  CImageBuf member functions
//
//==============================

CImageBuf::~CImageBuf(void)
//------------------------------------------------------------------------------
// Default destructor
{
	kill();						// destroy the buffer & its contents.
}

CImageBuf::CImageBuf(void)
//------------------------------------------------------------------------------
// Default constructor
{
	xsize = 0;					// empty and
	ysize = 0;
	pBuffer = NULL;				// pointing to nothing.
}


void CImageBuf::sizer(int nuXsize, int nuYsize)
//------------------------------------------------------------------------------
// Check the current buffer size. If it isn't nuXsize,nuYsize, discard the 
// buffer and make a new one of that size.
{
	if(nuXsize!=xsize || nuYsize!=ysize)		// is there a size mismatch?
	{
		kill();							// destroy the buffer, and
		pBuffer = new GLfloat[3*nuXsize*nuYsize];	// make a new one.
		xsize = nuXsize;
		ysize = nuYsize;

	}
}

void CImageBuf::kill(void)
//------------------------------------------------------------------------------
// Destroy the contents of the buffer and discard the memory it uses.
{
	if(xsize!=0 && ysize!=0 && pBuffer !=NULL)	// if we've got a buffer,
	{
		delete pBuffer;					// delete it's memory.
		pBuffer = NULL;					// Point to nothing.
	}
	xsize = 0;		
	ysize = 0;
}

void CImageBuf::clear(GLfloat r, GLfloat g, GLfloat b)
//------------------------------------------------------------------------------
// Fill the entire buffer with one color.
{
int i,imax;

	imax = xsize*ysize*3;				// # of pixels in buffer
	for(i=0; i<imax; i+=3)				// fill them all
	{
		pBuffer[i  ] = r;
		pBuffer[i+1] = g;
		pBuffer[i+2] = b;
	}
}

void CImageBuf::put(int x, int y, GLfloat* pRGB)
//------------------------------------------------------------------------------
// Set the pixel value at x,y to RGB = ( pRGB[0], pRGB[1], pRGB[2]).
{
int i;

	for(i=0; i<3; i++) pBuffer[(x + y*xsize)*3 + i] = pRGB[i];
}

void CImageBuf::put(int x, int y, GLfloat r, GLfloat g, GLfloat b)
//------------------------------------------------------------------------------
// Set the pixel value at x,y to r,g,b.
{
int i;

	i = (x + y*xsize)*3;
	pBuffer[i] =   r;
	pBuffer[i+1] = g;
	pBuffer[i+2] = b;
}

void CImageBuf::displayMe(int xorig, int yorig)
//------------------------------------------------------------------------------
// Display this buffer on-screen with its lower-left corner at xorig,yorig.
// (call this from your openGL 'display()' callback)
{
	glRasterPos2i(xorig, yorig);
	glDrawPixels(xsize,ysize,GL_RGB,GL_FLOAT,pBuffer);
}