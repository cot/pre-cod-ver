/*
 * NBody.h
 *
 *  Created on: Aug 10, 2012
 *      Author: Antoine Grondin
 */

#ifndef NBODY_H_
#define NBODY_H_

#include <iostream>
#include <string>
#include <GL/glew.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/freeglut.h>
#endif

// Project libs
#include <GLTools.h>
#include <GLFrustum.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <GLShaderManager.h>
#include <StopWatch.h>
#include <math3d.h>
// Local
#include "Constants.h"
#include "VectorMath.h"

const static bool D = DEBUG;

// Variables
// - Matrices and shaders
static GLShaderManager sShaderManager;
static GLFrustum sViewFrustrum;
static GLMatrixStack sProjectionMatrixStack;
static GLMatrixStack sModelViewMatrixStack;
static GLGeometryTransform sTransformPipeline;

////////////////////////////////////////////////////////////////////////
// MODELS
////////////////////////////////////////////////////////////////////////
static GLFrame sCameraFrame;

const static GLclampf sBackgroundColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
const static M3DVector4f sMainLightPos = { 0.0f, 10.0f, 5.0f, 1.0f };

static GLTriangleBatch sBodyBatch[BODY_COUNT];
static GLFrame sBodyFrames[BODY_COUNT];
static GLfloat sBodyRadius[BODY_COUNT];
static GLclampf sBodyColors[BODY_COUNT][4];
static Position3D sBodyPosition[BODY_COUNT];
static Velocity3D sBodyVelocity[BODY_COUNT];
static Acceleration3D sBodyAcceleration[BODY_COUNT];
static GLfloat sBodyMass[BODY_COUNT];


///////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////

// Setup
void setupWindow( int argc, char **argv );
void registerCallbacks();
void setupRenderContext();
void setupBodies();

// Callbacks
static void onChangeSize( int aNewWidth, int aNewHeight );
static void onRenderScene();
static void onMouseEvent( int key, int x, int y, int something );

// Drawing
static void drawBodies( CStopWatch *timeKeeper,
                        M3DVector4f *lightPosition );

// Physics
static void updatePhysics( float deltaT );
static void updateAcceleration( int bodyIndex );
static void updateVelocity( int bodyIndex, float deltaT );
static void updatePosition( int bodyIndex, float deltaT );

// Misc
inline void printGreetings() {
   std::cout << APP_NAME << " "
             << APP_VERSION << std::endl;
   std::cout << APP_COPYRIGHT
             << ", licensed under "
             << APP_LICENSE
             << std::endl;
   std::cout << "Contact me at "
             << APP_CONTACT
             << std::endl;
}

#endif
