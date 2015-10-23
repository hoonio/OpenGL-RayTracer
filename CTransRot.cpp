//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_
//
//	Mouse- and keyboard-controlled class for rigid-body transforms used in
//	OpenGL/GLUT projects.
//
//	01/06/2005 J. Tumblin -- Created.
//  02/07/2006 J. Tumblin -- mild improvements to ApplyMatrix(); added rotDragTo
//				and transDragTo functions.
//==============================================================================
// jt_transRot function bodies:

#include"CTransRot.h"
#define JT_INCR 1.0					// Degrees rotation per pixel of mouse move

CTransRot::~CTransRot(void)
//------------------------------------------------------------------------------
// Default destructor
{
}

CTransRot::CTransRot(void)
//------------------------------------------------------------------------------
// Default constructor
{
	reset();						// set all values to zero.
}

void CTransRot::reset(void)
//------------------------------------------------------------------------------
// Set all values to zero.
{
int i;

	for(i=0; i<3; i++)
	{
		pos[i] = 0.0; 
		rot[i] = 0.0;
	}
}

void CTransRot::rotDragTo(int xNew, int yNew)
//------------------------------------------------------------------------------
// Find rotations from mouse dragging. Mouse was dragged from m_x, m_y to
// xNew,yNew on-screen; apply additional rotation to our m_matrix.
{

	rot[0] += JT_INCR*(xNew - m_x);	// update the rotation amounts
	rot[1] += JT_INCR*(yNew - m_y);	//  
	m_x = xNew;						// update the start position for dragging.
	m_y = yNew;
}

void CTransRot::transDragTo(int xNew, int yNew)
//------------------------------------------------------------------------------
// Find translations from mouse dragging. Mouse was dragged from m_x,m_y to
// xNew,yNew on-screen; apply additional translation to our m_matrix.
{
	pos[0] += JT_INCR*(xNew - m_x);	// update the translation amounts
	pos[1] += JT_INCR*(yNew - m_y);	
	m_x = xNew;						// Update the start position for dragging.
	m_y = yNew;
}


void CTransRot::applyMatrix(void)
//------------------------------------------------------------------------------
// Apply rotations, then translations to the coordinate axes.  
// (Note OpenGL pre-multiplies matrices,
//   so commands appear to be in reverse order).
//							
{
double angl;					

	glTranslated(pos[0], pos[1], pos[2]);	// translate by mouse offset amount

	angl =rot[0]*rot[0] + rot[1]*rot[1];	// mouse displacement distance
	angl = sqrt(angl);

			// (HINT: want mouse acceleration? just drop the square root!)
	glRotated(angl, rot[1], rot[0], 0.0f);	// on-screen rotation: 
			// approximate a 'trackball' for rotation:
			// mouse drag in y direction rotates about x axis on-screen, and
			// mouse drag in x direction rotates about y axis on-screen.
}
#undef JT_INCR

