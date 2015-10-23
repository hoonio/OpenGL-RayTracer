//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_
//
//	OpenGL/ GLUT 'starter' code for writing your own ray-tracer.
//	
//	OPEN GL TRANSFORMATIONS REVIEW:
//		OpenGL transforms all vertices first by GL_MODELVIEW matrix and then by
//		the GL_PROJECTION matrix,and then by the 'viewport' transformation to
//		find the pixel addresses for display on screen. These matrices act as
//		converters from 'model' space to 'world' space to 'eye' space:
//
//		model space --GL_MODELVIEW ---> world space --GL_PROJECTION--> eye space
//		(vertices)  --GL_MODELVIEW --->  (ground)   --GL_PROJECTION--> (film)
//		
//		Note that we're used to thinking of *everything* in world space: both
//		WHAT we view and the camera that views them. The behavior of MODELVIEW
//		matrix makes sense to most people--it transforms basic shapes to their 
//		world-space positions.  But the GL_PROJECTION is usually confusing; this
//		matrix transforms away from world-space and into eye-space coords. The 
//		origin of eye-space is the cameras position, the -Z axis in eye-space is
//		the camera's direction of view.  The eye space x,y coords are the on-
//		screen x,y directions in the 2D image the camera makes (openGL's 
//		'viewport' then adjusts these to make pixel addresses).
//		--------------------Thus-------------------------
//		The GL_PROJECTION matrix changes all world-space coordinates of each 
//		point so they are measured using those eye-space camera coords.
//		-------------------------------------------------
//			It is dangerous to think of 'GL_PROJECTION as the 'camera-
//		position-setting' matrix, because you are probably thinking of the 
//		*INVERSE* of GL_PROJECTION.  Remember, the camera is at the origin of
//		eye space; if we want to find the camera position in world space, we
//		must transform that eye-space origin BACKWARDS through GL_PROJECTION
//		to get back to world-space coordinates!
//
//	WHAT THIS PROGRAM DOES:
//	  Draws 'world-space' axes as 3 colored lines: +x=RED, +y=GREEN, +z = BLUE
//	  Draws 'model-space' axes as 3 lines: +x = YELLOW, +y = CYAN, +z = PURPLE
//			(and a gray wire-frame teapot aligned with the model-space axes)
//		--MOUSE left-click/drag applies x,y glRotation() to GL_PROJECTION 
//		--MOUSE right-click/drag applies x,y glRotation() to GL_MODELVIEW
//		--ARROW keys applies x,y glTranslate() to GL_MODELVIEW
//		-- 'R' key to reset GL_MODELVIEW matrix to initial values.
//		-- 'r' key to reset GL_PROJECTION matrix to initial values.
//		-- 'm' or 'M' key: print out the GL_ MODELVIEW & GL_PROJECTION matrices
//		-- 'Q' key to quit.
//
//	To compile this under Microsoft Visual Studio (VC++ 6.0 or later) create
//		a new Project, Win32 Console Application, and make an 'empty' project.
//		Then add this file as 'source file', be sure you have GLUT installed on
//		your machine.
//
//  If you don't have it, search the web for 'GLUT' --Marc Kilgard's
//		elegant little library that makes OpenGL platform-independent.  GLUT
//		uses 'callback' functions--you tell GLUT the names of your functions,
//		then it calls them whenever users do something that requires a response
//		from your program; for example, they moved the mouse, they resized a
//		window or uncovered it so that it must be re-drawn.
//
//  for CS 351,2 Northwestern University,Jack Tumblin, jet@cs.northwestern.edu
//
//	12/11/2004 - J. Tumblin--Created.
//	01/01/2005 - J. Tumblin--Added teapot; changed camera position to (0,3,5),
//								and field-of-view to 20 degrees.
//						   --Two viewports; left == openGL, right == Ray Trace
//	01/11/2005 - J. Tumblin--Added 'm' command, moved camera to (0,0,8).
//  02/08/2006 - J. Tumblin--Added crude CCamera, CSphere, CRay classes, and
//						     showed how to set CCamera to match openGL.
//==============================================================================
#include <math.h>							// for sin(), cos(), tan(), etc.
#include <stdlib.h>							// for all non-core C routines.
#include <stdio.h>							// for printf(), scanf(), etc.
#include <iostream>							// for cout, cin, etc.
#include <assert.h>							// for error checking by ASSERT().
#include <GLUT/glut.h>						// Mark Kilgard's GLUT library.
											// (Error here? be sure you have
											// installed GLUT library
											// What's that? ask Google...
#include "CTransRot.h"						// Class to hold the current amount
											//	of rotate/translate for mouse.
#include "raytrace.h"						// Ray-tracing classes.

using namespace std;						// (allow both ANSI & non-ANSI C++)

#define JT_TITLE	"CS352 RayTrace Starter"// Display window's title bar:
#define JT_WIDTH	384						// initial window half-width,
#define JT_HEIGHT	384						// initial window height in pixels.
#define JT_XPOS		  0						// initial window position
#define JT_YPOS		256
#define JT_ZNEAR	1.0						// near, far clipping planes for
#define JT_ZFAR		50.0					// the OpenGL 3D camera.

//====================
//
//	Function Prototypes  (these belong in a '.h' file if we have one)
//	
//====================

void glut_init(int *argc, char **argv);	// GLUT initialization
void ogl_init();						// OpenGL initialization

					// GLUT callback functions we will register. Called when:
void display(void);						// GLUT says re-draw the display window.
void reshape(int w, int h);				// GLUT says window was re-sized to w,h
void keyboard(unsigned char key, int x, int y);	//GLUT says user pressed a key
void keySpecial(int key, int x, int y);	// GLUT says user pressed a 'special'key
void mouseMove(int xpos,int ypos);		// GLUT says user moved the mouse to..
void mouseClik(int buttonID,int upDown,int xpos, int ypos);		
										// GLUT says user click/dragged mouse.
void myTrace(void);						// Do one scanline of ray-tracing; 
										// (called as the GLUT idle task)

//===================
//
// GLOBAL VARIABLES (bad idea!)
//
//====================
CTransRot setModel;			// User's changes to initial GL_MODELVIEW matrix
CTransRot setProj;			// User's changes to initial GL_PROJECTION matrix
int viewportW,viewportH;	// current size of left, right viewports, in pixels
CCamera   myCam;			// Our camera object.
CSphere   mySun;			// Our first sphere to trace.
CSphere   Chrome;			// Chrome sphere
CImageBuf myPic;			// Our ray-traced image to display.

//Used for printing:
double pModelMat[16];		// temp storage for openGL modelview and projection
double pProjMat[16];		//	matrices used for current on-screen display.

int main(int argc, char** argv)
//------------------------------------------------------------------------------
{
	printf("OpenGL on left, Ray Tracing on Right.\n");
	printf("-------------------------------------\n\n");
	printf("GL_PROJECTION: rotate   --Left Mouse Drag\n");
	printf("               print    -- 'm' key\n");
	printf("               reset    -- 'r' key    \n\n");

	printf("GL_MODELVIEW:  rotate   --Right Mouse Drag\n");
	printf("               translate--Arrow Keys\n");
	printf("               print    -- 'M' key\n");
	printf("               reset    -- 'R' key\n\n");

	printf("Ray Tracing:   start    --'t' or 'T' key\n");
	printf("               clear    --'c' or 'C' key\n\n");

	printf("Quit:  'Q','q' or 'ESC' key\n\n");

	glut_init(&argc, argv);				// First initialize GLUT,
	ogl_init();							// Then initialize any non-default 
										// states we want in openGL,

	glutMainLoop();
	// Then give program control to GLUT.  This is an infinite loop, and from
	// within it GLUT will call the 'callback' functions below as needed.
	return 0;							// orderly exit.
}

//=====================
//
//  Other Function Bodies
//
//=====================

void glut_init(int *argc, char **argv)
//------------------------------------------------------------------------------
// A handy place to put all the GLUT library's initial settings; note that we
// 'registered' all the function names for the callbacks we want GLUT to use.
{
	
	glutInit(argc, argv);				// GLUT's own internal initializations.

							// single buffered display, 
							//  RGB color model, use Z-buffering (depth buffer)
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(JT_WIDTH*2, JT_HEIGHT);	// set display-window size
	glutInitWindowPosition(JT_XPOS, JT_XPOS);	// position,
	glutCreateWindow(JT_TITLE);					// then create it.

	// Register GLUT Callback function names. (these names aren't very creative)
	glutDisplayFunc(display);			// 'display'  callback:  display();
	glutKeyboardFunc(keyboard);			// 'keyboard' callback:  keyboard(); 
	glutSpecialFunc(keySpecial);		// 'special'keys callback: keyspecial()
	glutReshapeFunc(reshape);			// 'reshape'  callback:  reshape();
	glutMouseFunc(mouseClik);			// callbacks for mouse click & move
	glutMotionFunc(mouseMove);		
//	glutIdleFunc(idle);					// 'idle'	  callback:  idle(); 
// No, not here! WE DON'T NEED IDLE() unless we have lots of background work
// to do. Instead, 'register' it when you have work to do, and 'un-register'
// it (use glutIdleFunc(NULL);) when that work is done. For ray tracing, we
// register idle() when user presses the 'T' key to start tracing, compute
// just a few rays per idle() call, and un-register idle() when the image is
// finished.
}


void ogl_init()
//------------------------------------------------------------------------------
// A handy place to put all the OpenGL initial settings-- remember, you only 
// to change things if you don't like openGL's default settings.
{
glClearColor(0.0, 0.0, 0.0, 0.0);       // Display-screen-clearing color;
										// acts as 'background color'
glColor3f(0.0, 0.0, 0.0);               // Select current color for drawing
glShadeModel(GL_SMOOTH);                // Choose 'flat shading' model 
										// instead of smooth, so we see facets

glEnable(GL_DEPTH_TEST);				// glDisable(GL_LIGHTING);
										// No lighting needed

GLfloat light_position[] = { myCam.lightPosition.row[0], myCam.lightPosition.row[1], myCam.lightPosition.row[2], 1.0 };
GLfloat light_position2[] = { 0.5, 2.0, 0.0, 1.0 };
GLfloat lm_ambient[] = { 0.2, 0.2, 0.2, 1.0 };

glEnable(GL_COLOR_MATERIAL);
glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE); 

glLightfv(GL_LIGHT0, GL_POSITION, light_position);
glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lm_ambient);
//glLightfv(GL_LIGHT0, GL_POSITION, light_position2);
//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lm_ambient);

glEnable(GL_LIGHTING);
glEnable(GL_LIGHT0);
}


void reshape(int w, int h)
//------------------------------------------------------------------------------
// GLUT 'reshape' Callback. Called when user resizes the window from its current
// shape to one with width w, height h.
// We usually initialize (or re-initialize)openGL's 'GL_PROJECTION' matrix here.
{
GLdouble cAspect;					// Camera aspect ratio: image width/height
GLdouble cTop,cBot,cLeft,cRight;	// Camera clipping planes for view frustum.
Vec4 cVRP,cVPN,cVUP;				// Camera's view ref pt;, viewplane normal,
									// and view up vector.
Vec4 cLookAt;						// The point the camera looks towards.

	// set viewport for OpenGL drawing in left half of window
	viewportW = w/2;					// global vars (YUK!) save the size
	viewportH = h;						// for use in drawing right half...
	glViewport(0, 0, viewportW, viewportH);
	cAspect = (GLdouble)viewportW / (GLdouble)viewportH;
	
	// Set the projection matrix  
	glMatrixMode(GL_PROJECTION);		// Select the Projection matrix,

//===CHOOSE A CAMERA:===
/*
//== THIS ONE ===-------------------------------------
	glLoadIdentity();		// (clear out any previous camera settings)
	gluOrtho2D(0, viewportW, 0, viewportH);	
							// orthographic projection(left,right,bottom,top): 
							// using (0,w/2,0,h) maps x,y world space to screen
							// space in pixel units; w/2 uses just the left
							// half of our window.
//==OR THIS ONE===------------------------------------
	glLoadIdentity();		// (Clear out any previous camera settings)
	gluPerspective(20.0,	// Set camera's vertical field-of-view  (y-axis)
							// measured in degrees, and set the display image
		cAspect,			// (width/height) and finally
		JT_ZNEAR,JT_ZFAR);	// set near, far clipping planes.
							// if GL_PERSPECTIVE matrix was identity before we
							// called gluPerspective, then we're at the world
							// space origin, but looking in the (world space)
							// -Z direction. 
							// (if current matrix is NOT identity, then the
							// current matrix M is pre-multiplied by the matrix
							// 'T' spec'd by gluPerspective: new matrix is MT).

	//-------------------------------------------
	// REMEMBER, all vertices are first multipled by the GL_MODELVIEW matrix,
	// and then by the GL_PROJECTION matrix before the 'viewport' maps them
	// to the display window.
	//-------------------------------------------
	// REMEMBER when you call glTranslate() or glRotate() in openGL, existing 
	// GL_PROJECTION or GL_MODELVIEW is ***PRE_MULTIPLIED*** by the specified
	// translate or rotate matrix to make the new GL_PROJECTION or GL_MODELVIEW
	// matrix!  This is *NOT* intuitive!
	//--------------------------------------------
	//		THUS, if we call glTranslatef(0,-3,-5) now, (after calling 
	//		gluPerspective, not before) we tranlate the world-space 
	//	origin to (0,-3,-5) *BEFORE* we apply the camera matrix that turns the
	//  translated world into a picture.  Lets do it:

		glTranslatef(0.0f, -3.0f, -5.0f);

	//		This gives you the same picture you'd get if you'd translated the
	// camera to the world-space location (0,+3,+5). Confusing, isn't it?!
	// Here's a good way to think of it; 
	//		1) if the GL_PERSPECTIVE matrix is set to the identity matrix, then
	//			eye space is the SAME as world space; the camera is at the 
	//			origin of both 'world' and 'eye' space, and looking in the 
	//			-Z direction.  
	//		2) The glTranslate(0,-3,-5) above converts world space coords to 
	//			eye-space coords.: he world space origin is at 0,-3,-5 in eye 
	//			space, and all other world-space points got this offset too.
	// The INVERSE transform (e.g. glTranslate(0,+3,+5) converts eye-space
	// coords to world space.  The camera is always the origin of eye-space, so 
	// if we transform the eye-space origin to world space, we find the camera's
	// world-space position is 0,+3,+5.

		glRotatef(30.0f, 0.0f, 1.0f, 0.0f);

	//		Similarly, if we next call glRotationf(30.0,0,1,0) (e.g. rotate by
	// 30 degrees around the y axis) the current contents of the GL_PROJECTION
	// matrix is again **PRE-MULTIPLIED** by the new rotation matrix we made.  
	// Any point in world space is rotated  FIRST(about the world-space origin)
	// then translated to make eye-space coordinates (where the camera is at
	// the origin and looking down the -Z axis).  
	// Just as before, the INVERSE transform (eye-space-to-world space) tells
	// us the camera position in world space. Take the origin of eye space
	// (e.g. the camera position) translate(0,+3,+5) so now camera is at 0,3,5
	// and still looking in -Z direction towards origin. Next, rotate about
	// the Y axis by -30 degrees, causing the camera to swing around from the
	// Z axis towards the -X axis. 


	glLoadIdentity();			// (Clear out any previous camera settings)
	gluPerspective(				// Set camera's internal parameters: 
		20.0,					// vertical (y-axis) field-of-view in degrees,
		cAspect,				// display image aspect ratio (width/height),
								// (use w/2 so we can show TWO viewports)
		JT_ZNEAR,JT_ZFAR);		// near,far clipping planes for camera depth.
	// Now position the camera: move

	gluLookAt( 0.0, 0.0, 8.0,	// VRP: eyepoint x,y,z position in world space.
			   0.0, 0.0, 0.0,	// 'look-at' point--we're looking at origin.
								// (VPN = look-at-point - VRP)
			   0.0, 1.0, 0.0);	// VUP: view 'up' vector; set 'y' as up...
	//*** SURPRISE****
	// the matrix made by gluLookAt() *POST-MULTIPLIES* the current matrix,
	// unlike the glRotate() and glTranslate() functions.  Why? because it
	// specifies **in world space** where we want to put the camera, and
	// **in world space** where we want the camera to look.

	// Puzzle: What would happen now if you called 'glTranslate(0,0,-6)?
	// can you explain what happens if you then call 'glRotate(30f,0,1,0)?
*/

//==OR THIS ONE===------------------------------------
	// Specifying our OpenGL camera matrix using glFrustum() and glLookAt() 
	// uses terms that are very useful for ray-tracing. We choose the camera's 
	// internal settings (its optics ) with glFrustum(), then choose all 
	// external settings (position, aiming direction) with glLookAt().
	// Read OpenGL Red Book, Chapter 3 "Viewing" to see that we can define a 
	// perspective camera's 'optics' very simply by the clipping planes that 
	// define how much of 3D space our camera will turn into an image. These 
	// clipping planes are named (left,right,top,bottom,znear,zfar). We
	// can specify each of them with a single number if we place the camera's 
	// center of projection (the 'eyepoint') at the origin, and define our 
	// viewing direction as the -Z axis. (Recall from CS351 that we look in the
	// -Z direction so that our camera can use right-handed coordinates).  
	// Define the plane where z = -znear = -1 as the 'film plane' of our camera,
	// then choose a rectangle on that plane to define exactly where the camera
	// looks to frame' its image.  For znear=1, the corners of this 
	//	"image rectangle" are located here:
	//		(cLeft,cTop,-1)	(cRight,cTop,-1)	
	//		(cLeft,cBot,-1) (cRight,cBot,-1)
	// Use glFrustum() to make an openGL camera matrix from these corner values.
	// To make a ray tracing camera that matches it exactly, we simply:
	//	--dice this image rectangle into pixels (are they little squares?)
	//  --shoot rays from the eyepoint (the origin of this coord. system)
	//		through each pixel's location on the image rectangle, and find the
	//		color of what those rays hit in the scene. That color=pixel color.

							// Define the perspective camera:
							// center of projection is at origin,
							// the viewing direction aimed down the -Z axis,
							// The 2-D image we make is on the z= -znear plane.
							// Following OpenGL red book, Chapter 3:
	cAspect = (GLdouble)viewportW/ (GLdouble)viewportH;		
							// image rectangles' ratio of width/height should
							// match the display window width/height ratio
							// to guarantee square pixel spacing.
	cLeft =  cAspect*(-0.25 / JT_ZNEAR),// left,right side clipping planes;
	cRight = cAspect*( 0.25 / JT_ZNEAR),// scale by 'aspect' so that our pixels 
							// stay square in the image rectangle, and divide by
							// JT_ZNEAR just in case znear != 1.0.
	cTop =  0.25 / JT_ZNEAR;// top, bottom side clipping planes: choose them
	cBot = -0.25 / JT_ZNEAR;// for a fixed vertical field of view.  Define the
							// vertical field of view as 'fovy' degrees, then:
							// tan(fovy/2) = top/znear;  if top/znear = 0.5, 
							// then fovy/2 = arctan(0.5) = 26.56 degrees 
	// Now we've defined all the camera's clipping planes.
	glLoadIdentity();		// (Clear out any previous camera settings)
	glFrustum(				// Make a perspective camera matrix by setting 
		cLeft,    cRight,	// the left, right clipping planes (+/-x limits) 
		cBot,     cTop,		// the top, bottom clipping planes,(+/-y limits)
		JT_ZNEAR, JT_ZFAR);	// th near, far clipping planes. (+/-z limits)

	// Set my ray-tracing camera's internal parameters too:
	myCam.SetFrustum(cLeft,cRight,cBot,cTop, viewportW, viewportH);


	// We've set the camera's lens; now position this camera in world space:
	cVRP.put(0.0, 0.0, 5.0, 1.0);	// VRP; set world-space eyepoint position:
	cLookAt.put(0.0, 0.0, 0.0, 1.0);// the point our camera looks at.
	cVPN.sub(cVRP,cLookAt);		// VPN: vector from 'lookAt' point to camera
	cVPN.norm(cVPN);			// (normalize it).
	cVUP.put(0.0, 1.0, 0.0, 0.0);	// let y axis be up direction\

	// For OpenGL camera:
	gluLookAt(				// VRP: eyepoint x,y,z position in world space.
		cVRP.row[0], cVRP.row[1], cVRP.row[2],	
							// 'look-at' point--we're looking at origin.
		cLookAt.row[0],cLookAt.row[1],cLookAt.row[2],	
								// (note: VPN = VRP - look-at-point)
							// VUP: view 'up' vector; set 'y' as up...
		cVUP.row[0], cVUP.row[1], cVUP.row[2]);
	//*** SURPRISE****
	// the matrix made by gluLookAt() *POST-MULTIPLIES* the current matrix,
	// unlike the glRotate() and glTranslate() functions.  Why? because it
	// specifies **in world space** where we want to put the camera, and
	// **in world space** where we want the camera to look.

	// For Ray Tracing Camera:
	myCam.lookAt(cVRP,cVPN,cVUP);	// use the same view-setting vectors:

	myPic.sizer(viewportW, viewportH);	// quarter-sized buffer.
	myPic.clear(0.9,0.4,0.1);			// set an orange color in the
										// empty buffer.

	//====END OF YOUR CHOICES FOR CAMERAS==============

	// Initialize the modelview matrix to do nothing:
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();					// Set it to 'do nothing'.
}

void display(void)
//------------------------------------------------------------------------------
// GLUT 'display' Callback. GLUT calls this fcn when it needs you to redraw 
// the dislay window's contents. Your program should never call 'display()',
// because it will confuse GLUT--instead, call glutPostRedisplay() if you need
// to trigger a redrawing of the screen.
{

        // set viewport to the left-side half of the window:
        glViewport(0, 0, (GLsizei) viewportW, (GLsizei) viewportH);
        // Clear the frame-buffer 
        glClearColor(0.0, 0.0, 0.0, 0.0);        // Set OpenGL background color,
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        glEnable(GL_LIGHTING);
// =============================================================================
// START LEFT-SIDE DRAWING CODE HERE 
// =============================================================================
        glMatrixMode(GL_PROJECTION);        // select projection matrix,
        glPushMatrix();                                        // save current version, then
        setProj.applyMatrix();                        // apply results of mouse, keyboard to 
                                                                        // the GL_PROJECTION matrix.
        glGetDoublev(GL_PROJECTION_MATRIX,pProjMat);
                                                                        // and save the new matrix we make.

        // Draw model-space axes:
        glMatrixMode(GL_MODELVIEW);                // select the modelview matrix,
        glPushMatrix();                                        // save current version, then
        setModel.applyMatrix();                        // apply results of mouse, keyboard
        glGetDoublev(GL_MODELVIEW_MATRIX,pModelMat);        
                                                                        // and save the matrix we make.

GLfloat mat_ambient[] = { 0.3, 0.3, 0.3, 1.0 };
GLfloat mat_diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
GLfloat mat_specular[] = { 0.7, 0.7, 0.7, 1.0 };
glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
glMaterialf(GL_FRONT, GL_SHININESS, 100.0);
        glColor3f(0.78,0.89,0.82);                        // Set color to light grey, and draw a
        //glutWireTeapot(0.8);                        // Little Teapot(see OpenGL Red Book, pg 660)
        //glutSolidTeapot(0.8);                        // Little Teapot(see OpenGL Red Book, pg 660)
        glPushMatrix();
        glTranslatef(mySun.pos.row[0],mySun.pos.row[1],mySun.pos.row[2]);
        glutSolidSphere(mySun.radius,60,60);                // or a sphere with 22 lat/longitude lines
                                                                        // (GL_LIGHTING would make this look better)
        glPopMatrix();

        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);                                // draw axes in model-space.
                glColor3f ( 1.0, 1.0, 0.0);        // Yellow X axis
                glVertex3f( 0.0, 0.0, 0.0);        
                glVertex3f( 1.0, 0.0, 0.0);

                glColor3f ( 0.0, 1.0, 1.0);        // Cyan Y axis
                glVertex3f( 0.0, 0.0, 0.0);        
                glVertex3f( 0.0, 1.0, 0.0);

                glColor3f ( 1.0, 0.0, 1.0);        // Purple Z axis
                glVertex3f( 0.0, 0.0, 0.0);
                glVertex3f( 0.0, 0.0, 1.0);
        glEnd();

        // Draw axes in world-space:        
        glLoadIdentity();                        // wipe out current GL_MODELVIEW matrix so that
                                                                // model-space vertices become world-space
                                                                // vertices without change.
        
        glBegin(GL_LINES);                        // start drawing lines:
                glColor3f ( 1.0, 0.0, 0.0);        // Red X axis
                glVertex3f( 0.0, 0.0, 0.0);        
                glVertex3f( 1.0, 0.0, 0.0);

                glColor3f ( 0.0, 1.0, 0.0);        // Green Y axis
                glVertex3f( 0.0, 0.0, 0.0);        
                glVertex3f( 0.0, 1.0, 0.0);

                glColor3f ( 0.0, 0.0, 1.0);        // Blue Z axis
                glVertex3f( 0.0, 0.0, 0.0);
                glVertex3f( 0.0, 0.0, 1.0);

        glEnd();                                        // end drawing lines

		GLfloat chrome_ambient[] = { 0.3, 0.3, 0.3, 1.0 };
		GLfloat chrome_diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
		GLfloat chrome_specular[] = { 0.7, 0.7, 0.7, 1.0 };
		glMaterialfv(GL_FRONT, GL_AMBIENT, chrome_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, chrome_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, chrome_specular);
		glMaterialf(GL_FRONT, GL_SHININESS, 100.0);
        glColor3f(0.78,0.89,0.82);                        // Set color to light grey, and draw a

        glPushMatrix();
        glTranslatef(Chrome.pos.row[0],Chrome.pos.row[1],Chrome.pos.row[2]);
        glutSolidSphere(Chrome.radius,60,60);                // or a sphere with 22 lat/longitude lines
                                                                        // (GL_LIGHTING would make this look better)
        glPopMatrix();

        glPopMatrix();                                // restore original MODELVIEW matrix.
        glMatrixMode(GL_PROJECTION);// Restore the original GL_PROJECTION matrix
        glPopMatrix();
        // ================================================================================
        // END LEFT-SIDE DRAWING CODE HERE 
        // ================================================================================

        // set viewport to the right-side half of the window:
        glMatrixMode(GL_PROJECTION);// Save projection matrix used for left side,
        glPushMatrix();        
        glLoadIdentity();                        // clear it, and temporarily make a new one 
        glViewport((GLsizei) viewportW,0, (GLsizei)viewportW,(GLsizei) viewportH);
                                                                // to write right-half side of window using
        gluOrtho2D(0, viewportW/2, 0, viewportH);        
                                                        // orthographic projection(left,right,bottom,top): 
                                                        // where (0,w,0,h) maps x,y world space to screen
                                                        // space in pixel units.
        // ================================================================================
        // START RIGHT-SIDE DRAWING CODE HERE 
        // ================================================================================
                                                // Draw the contents of our 2D image buffer on-screen.        
                                                // It holds the latest ray-traced image.
        myPic.displayMe(0,0);

// Once you get past the basics, you don't REALLY want to be doing ray
// tracing within the display() callback: display will HANG until you've made
// a new image (could be a long wait!), and a minor mouse click could destroy
// the image you just waited an hour to see. 
// Instead, we can make 'image buffer' that holds your ray-traced image as you
// create it. Then the 'display()' callback should simply display that buffer's 
// current contents on-screen without disturbing it, using the glDrawPixels() 
// command.
// You can then use the 'idle() callback function to do your ray-tracing into 
// the image buffer you made. You should also consider using a keystrokes 
// (e.g. 'T' for 'trace', R,S for run,stop) to control the ray-tracing, rather
// than start a new imag every time you change the openGL view or call 'display'.

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();                        // restore original GL_PROJECTION matrix.
// ================================================================================
// END RIGHT-SIDE DRAWING CODE HERE 
// ================================================================================

//        cout << "Screen ReDrawn" << endl;
        glutSwapBuffers();
        glFlush();        
}                                                                                                                                                                              

void keyboard(unsigned char key, int x, int y)
//------------------------------------------------------------------------------
// GLUT 'keyboard' Callback.  User pressed an alphanumeric keyboard key.
// ('special' keys such as return, function keys, arrow keys? keyboardspecial)
{

	switch(key) 
	{
		case 27: // Esc  
		case 'Q':
		case 'q':
			exit(0);		// Quit application  
			break;
		case 'r':			// reset GL_PROJECTION matrix
			setProj.reset();
			cout << "GL_PROJECTION matrix reset" << endl;
			break;
		case 'R':			// reset GL_MODELVIEW matrix
			setModel.reset();
			cout << "GL_MODELVIEW matrix reset" << endl;
			break;
		case 'm':			// Print out current OpenGL matrix contents
		case'M':
										// read the world->eye space matrix.
			printf("\n Current value of GL_PROJECTION matrix:\n");
			// (note: OpenGL stores matrices as a sequence of columns of 4)
			printf("%f \t %f \t %f \t %f\n", 
					pProjMat[0], pProjMat[4], pProjMat[8], pProjMat[12] );
			printf("%f \t %f \t %f \t %f\n", 
					pProjMat[1], pProjMat[5], pProjMat[9], pProjMat[13] );
			printf("%f \t %f \t %f \t %f\n", 
					pProjMat[2], pProjMat[6], pProjMat[10],pProjMat[14] );
			printf("%f \t %f \t %f \t %f\n", 
					pProjMat[3], pProjMat[7], pProjMat[11],pProjMat[15] );

			printf("\n Current value of GL_MODELVIEW matrix:\n");
			// (note: OpenGL stores matrices as a sequence of columns of 4)
			printf("%f \t %f \t %f \t %f\n", 
					pModelMat[0], pModelMat[4], pModelMat[8], pModelMat[12] );
			printf("%f \t %f \t %f \t %f\n", 
					pModelMat[1], pModelMat[5], pModelMat[9], pModelMat[13] );
			printf("%f \t %f \t %f \t %f\n", 
					pModelMat[2], pModelMat[6], pModelMat[10],pModelMat[14] );
			printf("%f \t %f \t %f \t %f\n", 
					pModelMat[3], pModelMat[7], pModelMat[11],pModelMat[15] );
			break;
		
		case 't':		// ray trace the scene using current camera settings
		case 'T':
			glutIdleFunc(myTrace);		// register 'myTrace()' so it gets
						// all un-used compute time.  We'll 'unregister' it
						// when the ray-tracing is done.
			myCam.xNow = 0;		// Go start of image:  myTrace() will make new
			myCam.yNow = 0;		// pixels until (xNow,yNow) = (xsize,ysize)
			break;
		case 'c':
		case 'C':		// clear the image buffer:
			myPic.clear(0.8, 0.5,0.3);	
			break;
		default:
			printf("unknown key.  Try arrow keys, R,r,m,t,c or q");
			break;
	}
	// We might have changed something. Force a re-display  
	glutPostRedisplay();
}

void keySpecial(int key, int x, int y)
//------------------------------------------------------------------------------
// GLUT 'special' Callback.  User pressed an non-alphanumeric keyboard key, such
// as function keys, arrow keys, etc.
{
static double x_pos, y_pos;

	switch(key)	
	{
		case GLUT_KEY_LEFT:		// left arrow key
			setModel.pos[0] -= 0.1;
			break;
		case GLUT_KEY_RIGHT:	// right arrow key
			setModel.pos[0] += 0.1;
			break;
		case GLUT_KEY_DOWN:		// dn arrow key
			setModel.pos[1] -= 0.1;
			break;
		case GLUT_KEY_UP:		// up arrow key
			setModel.pos[1] += 0.1;
			break;
		default:
			break;
	}
//	printf("key=%d, setModel.x_pos=%f, setModel.y_pos=%f\n",
//							key,setModel.pos[0],setModel.pos[1]);
	// We might have changed something. Force a re-display  
	glutPostRedisplay();
}

void mouseClik(int buttonID,int upDown,int xpos, int ypos)
//------------------------------------------------------------------------------
// GLUT 'mouse' Callback.  User caused a click/unclick event with the mouse:
//     buttonID== 0 for left mouse button,
//			  (== 1 for middle mouse button?)
//			   == 2 for right mouse button;
//		upDown == 0 if mouse button was pressed down,
//			   == 1 if mouse button released.
//		xpos,ypos == position of mouse cursor, in pixel units within the window.
// *CAREFUL!* Microsoft puts origin at UPPER LEFT corner of the window.
{
	if(buttonID==0)				// if left mouse button,
	{
		if(upDown==0)			// on mouse press,
		{
			setProj.isDragging = 1;	// get set to record GL_PROJECTION changes.
			setProj.m_x = xpos;		// Dragging begins here.
			setProj.m_y = ypos;
		}
		else setProj.isDragging = 0;
	}
	else if(buttonID==2)		// if right mouse button,
	{
		if(upDown==0)
		{
			setModel.isDragging = 1;// get set to record GL_MODELVIEW changes.
			setModel.m_x = xpos;	// Dragging begins here.
			setModel.m_y = ypos;
		}
		else setModel.isDragging = 0;
	}
	else						// something else.
	{
		setProj.isDragging  = 0;	// default; DON'T change GL_PROJECTION
		setModel.isDragging = 0;	//					or  GL_MODELVIEW
	}

//	printf("clik: buttonID=%d, upDown=%d, xpos=%d, ypos%d\n",
//										buttonID,upDown, xpos,ypos);
}

void mouseMove(int xpos,int ypos)
//------------------------------------------------------------------------------
// GLUT 'move' Callback.  User moved the mouse while pressing 1 or more of the
// mouse buttons.  xpos,ypos is the MS-Windows position of the mouse cursor in
// pixel units within the window.
// CAREFUL! MSoft puts origin at UPPER LEFT corner pixel of the window!
{

	if(setModel.isDragging==1)		// if we're dragging the left mouse,
	{								// increment the x,y rotation amounts.
		setModel.rotDragTo(xpos,ypos);
	}
	if(setProj.isDragging==1)		// if we're dragging theright mouse,
	{								// increment the x,y rotation amounts.
		setProj.rotDragTo(xpos,ypos);
	}
//	printf("move %d, %d\n", xpos,ypos);	// print what we did.

	// We might have changed something. Force a re-display  
	glutPostRedisplay();
}

void myTrace(void)
//------------------------------------------------------------------------------
// Perform some ray tracing as a background task.  This function is registered
// as the 'idle' function when users press the 't' or 'T' key, and it gets ALL
// the free CPU time until we finish an image and 'un-register' ourselves.
{
CRay eye;							// ray from eye into scene.
double dist, distChrome;
int k;
	// have we just finished an image?
	if(myCam.yNow >= myPic.ysize)
	{		// Yes!
		glutIdleFunc(NULL);		// un-register us; don't call us anymore when
								// the Windows system is idle.
		printf("Finished Ray Tracing!\n");
	}
	// No.  Let's trace some rays into our image buffer; do the rays for
	// the xNow,yNow pixels
	k=30;	// how many scanlines to try:
	while(myCam.yNow < myPic.ysize && k>0)
	{
		while(myCam.xNow < myPic.xsize)
		{
			myCam.makeEyeRay(eye, myCam.xNow, myCam.yNow); // make an eye ray.
			dist = mySun.hitBAD(eye);	// test it against our sphere:
			distChrome = Chrome.hitBAD(eye);	// test it against our sphere:
			if(dist > 0)				// did we hit our sphere?
			{
				myPic.put(myCam.xNow, myCam.yNow,
							0.7,dist,dist);	// Yes. Let distance set color.
			}
			else if(distChrome > 0)				// did we hit our sphere?
			{
				myPic.put(myCam.xNow, myCam.yNow,
							distChrome,0.7,distChrome);	// Yes. Let distance set color.
			}
			else
			{
				myPic.put(myCam.xNow, myCam.yNow, 0.2, 0.2, 0.2);	
				// No. set gray background
			}
			myCam.xNow++;				// go to the next pixel on the scanline.
		}
		// done with scanline; go to the next one.
		myCam.xNow=0;
		myCam.yNow++;
		k--;				// but stop after k more of them.
	}
	// Keep k small; we need to keep calls to myTrace() short so that the
	// Windows system won't bog down waiting for us. If there is nothing else
	// for Windows to do, it will return to the idle task (us) immediately.
	glutPostRedisplay();	// show what we've done.
}
	
/*
void idle(void)
//------------------------------------------------------------------------------
// GLUT 'idle' Callback. Called when OS has nothing else to do; a 'clock tick'.  
// Use 'idle' *ONLY IF* your program does anything that needs continual updates, even 
// when users are not pressing keys, then put code to do the updates here.
// If you need to redraw the screen after your update, don't forget to call
// glutPostRedisplay() too.
//			*** A COMMON MISTAKE TO AVOID: ***
// 'idle()' gets called VERY OFTEN.  If you register 'idle()' and leave the idle
// function empty, GLUT will waste most/all CPU time not otherwise used on
// useless calls to idle().  If idle() contains only glutPostRedisplay(), you
// will force GLUT to redraw the screen as often as possible--even if the 
// contents of the screen has not changed.  
// If your program ONLY changes screen contents when user moves,clicks, or drags
// the mouse, presses a key, etc., then you don't need idle() at all! 
// Instead, call glutPostRedisplay() at the end of each of the GLUT callbacks 
// that change the screen contents.  Then you'll update the screen only when 
// there is something new to show on it.
{

}
*/


