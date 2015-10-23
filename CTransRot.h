//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_
//
//	Mouse- and keyboard-controlled class for rigid-body transforms used in
//	OpenGL/GLUT projects.
//
//	01/06/2005 J. Tumblin -- Created.

#include <GLUT/glut.h>
#include <math.h>

class CTransRot
//==============================================================================
// Record / accumulate offset amounts and rotation amounts from mouse & keyboard
{
public:
int m_x,m_y;					// last mouse-dragging position we examined.

double	pos[3];					// cumulative position offset
double	rot[3];					// cumulative rotation about x,y, and z axes
int		isDragging;				// TRUE if user is holding down the mouse button
								// that affects our value(s); else FALSE.


~CTransRot(void);				// default destructor
 CTransRot(void);				// default constructor
void reset(void);				// reset everything to zero.
void rotDragTo(int xNew, int yNew);	// Find rotations from mouse dragging.
void transDragTo(int xNew, int yNew); // Find translations from mouse dragging.
void applyMatrix(void);			// apply translations, rotations to openGL.
};