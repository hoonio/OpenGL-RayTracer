//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_
//
//	Ray-tracing classes
//
//	01/20/2005 J. Tumblin -- Created.

#include <GLUT/glut.h>				// For openGL and GLUT 3D graphics
#include "my_vecmat.h"				// for vector classes


class CRay
//==============================================================================
// a ray in an unspecified coordinate system.
{
public:
	Vec4 orig;					// The starting point of the ray in x,y,z coords.
	Vec4 dir;					// The ray's direction vector.

	~CRay(void);				// Default destructor
	 CRay(void);				// Default constructor
};

class CCamera
//==============================================================================
// A perspective camera in its own 'camera' or 'eye' coordinate system: the
// center-of-projection is at the origin, the camera looks down the -Z axis,
// and the image plane is at z=-1.  Within this z= -1 plane we define an
// 'image rectangle' whose corners are at 
//	(iLeft, iTop, -1) (iRight, iTop, -1)
//	(iLeft, iBot, -1) (iRight, iBot, -1).
// Place your grid of pixels (sample locations) on this image rectangle.  To
// find the color of any point on the rectangle (not just at pixel locations)
// shoot a ray through that point from the origin, and find the color of what
// that ray hits in the scene.
{
private:
	// The camera's external parameters (sets camera position & aiming)
	Vec4 vrp;					// world-space view reference point; the
								// 'center of projection' for the camera.
	Vec4 vpn;					// the camera's 'view plane normal' vector;
								// In world-space, this vector points from vrp
								// AWAY from the the direction we're looking.
	Vec4 vup;					// the 'View Up Vector' in world space.

	// The camera's coordinate axes as defined in world space.
	Vec4 u,v,n;	

	// the camera's internal parameters (sets camera optics)
	double iLeft,iRight,iBot,iTop;	// camera's clipping plane limits that set
									// the image rectangle.
public:
	Vec4 lightPosition;
	int xsize,ysize;				// the number of pixels on that rectangle.

	int xNow,yNow;				// current pixel we're evaluating

public:
	CCamera(void);				// default constructor,
	~CCamera(void);				// default destructor.

	BOOL SetFrustum(double nuLeft, double nuRight, 
					double nuBot, double nuTop, int nuXsize, int nuYsize);
								// set camera's optics.
	BOOL lookAt(Vec4 nuVRP, Vec4 nuVPN, Vec4 nuVUP);
								// move, aim camera in world space.
	BOOL makeEyeRay(CRay& eyeRay);	
								// find ray integer pixels xNow,yNow on the image.
	BOOL makeEyeRay(CRay& eyeRay, double xpt, double ypt);
								// find ray at arbitrary point xpt,ypt on the
								// image (not necessarily integer positions)
};

class CSphere
//==============================================================================
// A sphere object for ray tracing.
{
//-----------Member data:
public:
	Vec4	pos;					// centerpoint position 
	double	radius;					// sphere radius
	//Phong lighting model
	Vec4	K_a, K_d, K_s;			// Ambient, Diffuse, Specular reflectance
	int		K_shiny;				// 'shiny-ness' factor; exponent for cosine
									// of angle between eye ray 
									// and reflected light ray
//-----------Member functions:
	~CSphere(void);					// default destructor
	 CSphere(void);					// default constructor
	void reset(void);				// reset everything to zero.
	double hitBAD(CRay& in);		// BAD Intersection test: returns distance
									// from ray to sphere silhouette;if ray 'in' hits 
									// our sphere, then return the distance 
									// from ray origin to the hit-point; else
									// return a negative number.

};

class CImageBuf 
//===============================================================================
// A floating-point RGB image buffer object to holds ray-traced results for
// display in openGL using the 'glDrawPixels' command.
// easy to display in openGL.
{
private:
	GLfloat *pBuffer;

public:
	int xsize,ysize;

	~CImageBuf(void);				// default destructor.
	CImageBuf(void);				// default constructor,
	GLfloat* getBufferPtr(void) {return pBuffer;};

	void sizer(int nuXsize, int nuYsize);
									// If buffer is any size except xsize,ysize,
									// then discard its contents and make a
									// new one of the right size.
	void kill(void);				// wipe out the buffer; discard its memory
									// and set size to zero.
	void clear(GLfloat r, GLfloat g, GLfloat b); // fill buffer with one color.
	void put(int x, int y, GLfloat* pRGB);	// Set the pixel at x,y to the 
											//RGB=( pRGB[0], pRGB[1], pRGB[2]).
	void put(int x, int y, GLfloat r, GLfloat g, GLfloat b); // put rgb at x,y
	void displayMe(int xorig, int yorig);
									// draw buffer with its lower left corner
									// at xorig,yorig.  (2D openGL display)
};