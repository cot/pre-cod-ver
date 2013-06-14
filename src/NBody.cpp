/*
 * NBody.cpp
 *
 *  Created on: Aug 10, 2012
 *      Author: Antoine Grondin
 */

#include "NBody.h"
#include "VectorMath.h"
#include "unistd.h"
#include "time.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mpi.h"

using std::cout;
using std::cerr;
using std::endl;

///////////////////////////////////////////////////////////////////////
// Setup

int main( int argc, char **argv ) {

	int rank;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	printGreetings();

	if( TEST ) {
		if( !testVectorMath() ) {
			cerr << "Test VectorMath failed.";
			return -2;
		}

		cout << "All test OK" << endl;
	}

	setupWindow( argc, argv );
//	printf("petit passage 01 !\n");
	registerCallbacks();
	// Initialize GLEW
	GLenum anError = glewInit();

	// Not spawned process
	if(rank==0) {
		if( anError != 0 ) {
			fprintf( stderr, "GLEW Error: %s\n",
					glewGetErrorString( anError ) );

			if( D ) {
				cerr << " done" << endl;
			}

			return 1;
		}

		setupRenderContext();
		glutMainLoop();
	}
	MPI_Finalize();

	return 0;
}

void setupWindow( int argc, char **argv ) {
	gltSetWorkingDirectory( argv[0] );
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitWindowSize( APP_WIDTH, APP_HEIGHT );
	glutCreateWindow( APP_NAME );
	//glutIgnoreKeyRepeat(true);
	glutFullScreen();
}

void registerCallbacks() {
        /* Add parallel data traitment */
        int i, j;
	double randval;
        int rank,nprocs;
        char _SpawnProg[10];

        char _coordX[10];
        char _coordY[10];
        char _coordZ[10];
	double *bufX, *bufY, *bufZ;

	double a,b,c,seek,randvalue;

        MPI_File fh;
        MPI_Status status;
        MPI_Request request;
        MPI_Comm parentcomm, intercomm;

        MPI_Comm_rank(MPI_COMM_WORLD,&rank);
        MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
        MPI_Comm_get_parent( &parentcomm );

	srand(time(NULL));

        bufX=(double *)calloc(BODY_COUNT,sizeof(double));
        bufY=(double *)calloc(BODY_COUNT,sizeof(double));
        bufZ=(double *)calloc(BODY_COUNT,sizeof(double));
        strcpy(_coordX,"_coordX");
        strcpy(_coordY,"_coordY");
        strcpy(_coordZ,"_coordZ");
        strcpy(_SpawnProg,"SpawnProg");

        for(i=0;i<BODY_COUNT;i++) {
		seek = 500.0;
		a 			= rand();
		b 			= fmod(a,seek);
		a 			= rand();
		c 			= -fmod(a,seek);
		sBodyRadius[i] 		= fmod(a,100) + 1; /* between 1 and 100 */
		for(j=0;j<4;j++) {
			a                       = rand();
			b                       = fmod(a,seek);
			sBodyColors[i][j] 	= 0.1 + (double) (b / seek) ; /* between 0.1 and 1 */
		}
		a 			= rand();
		b 			= fmod(a,seek);
		a 			= rand();
		c 			= -fmod(a,seek);
		randval 		= (double) (b / seek) + (double) (c / seek);
                bufX[i] 		= 100 * randval; /* between -100 and 100 */
		a 			= rand();
		b 			= fmod(a,seek);
                bufY[i] 		= 300 * randval / seek; /* between 0 and 300 */
		a 			= rand();
		b 			= fmod(a,seek);
		a 			= rand();
		c 			= -fmod(a,seek);
		randval 		= b + 2 * c ;
                bufZ[i] 		= randval; /* between -1000 and 500 */
		sBodyVelocity[i].x 	= 0.0;
		sBodyVelocity[i].y 	= 0.0;
		sBodyVelocity[i].z 	= 0.0;
		sBodyAcceleration[i].x 	= 0.0;
		sBodyAcceleration[i].y 	= 0.0;
		sBodyAcceleration[i].z 	= 0.0;
		sBodyMass[i] 		= rand() ;
        }

        /* Creation of data structures */
        MPI_File_open(MPI_COMM_SELF, _coordX, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
        MPI_File_set_view(fh, 0, MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
        MPI_File_iwrite(fh, bufX, BODY_COUNT, MPI_DOUBLE, &request);
        MPI_Wait( &request, &status );
        MPI_File_close(&fh);

        MPI_File_open(MPI_COMM_SELF, _coordY, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
        MPI_File_set_view(fh, 0, MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
        MPI_File_iwrite(fh, bufY, BODY_COUNT, MPI_DOUBLE, &request);
        MPI_Wait( &request, &status );
        MPI_File_close(&fh);

        MPI_File_open(MPI_COMM_SELF, _coordZ, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
        MPI_File_set_view(fh, 0, MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
        MPI_File_iwrite(fh, bufZ, BODY_COUNT, MPI_DOUBLE, &request);
        MPI_Wait( &request, &status );
        MPI_File_close(&fh);

//	printf("bufX[5] = %g\t bufY[5] = %g\t bufZ[5] = %g\n",bufX[5], bufY[5], bufZ[5]);
        for(i=0;i<BODY_COUNT;i++) {
		sBodyPosition[i].x = bufX[i];
		sBodyPosition[i].y = bufY[i];
		sBodyPosition[i].z = bufZ[i];
        }
//	printf("petit passage 02 !\n");
	glutReshapeFunc( onChangeSize );
	glutDisplayFunc( onRenderScene );
}

void setupRenderContext() {
	sShaderManager.InitializeStockShaders();
	glEnable( GL_DEPTH_TEST );
	setupBodies();
	glClearColor( sBackgroundColor[0],
			sBackgroundColor[1],
			sBackgroundColor[2],
			sBackgroundColor[3] );
	glEnable( GL_LINE_SMOOTH );
}

void setupBodies() {

	for( int i = 0; i < BODY_COUNT; i++ ) {
		gltMakeSphere( sBodyBatch[i], sBodyRadius[i], 30, 50 );
		sBodyFrames[i].SetOrigin( sBodyPosition[i].x,
				sBodyPosition[i].y,
				sBodyPosition[i].z );
	}
}

///////////////////////////////////////////////////////////////////////
// Callbacks

void onChangeSize( int aNewWidth, int aNewHeight ) {
	glViewport( 0, 0, aNewWidth, aNewHeight );
	sViewFrustrum.SetPerspective( APP_CAMERA_FOV,
			float( aNewWidth ) / float( aNewHeight ),
			APP_CAMERA_NEAR,
			APP_CAMERA_FAR );
	sProjectionMatrixStack.LoadMatrix(
			sViewFrustrum.GetProjectionMatrix() );
	sTransformPipeline.SetMatrixStacks( sModelViewMatrixStack,
			sProjectionMatrixStack );
}

void onRenderScene( void ) {
	// Clear the buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	static CStopWatch timeKeeper;
	// Place camera
	M3DMatrix44f mCamera;
	sCameraFrame.GetCameraMatrix( mCamera );
	sModelViewMatrixStack.PushMatrix( mCamera );
	// Transform the light position into eye coordinates
	M3DVector4f lightPos = { sMainLightPos[0],
		sMainLightPos[1],
		sMainLightPos[2],
		sMainLightPos[3]
	};
	M3DVector4f lightEyePos;
	m3dTransformVector4( lightEyePos, lightPos, mCamera );
/* Potential evaluation over all the bodies */
	/* Add parallel data traitment */
	int i;
	int rank,nprocs;
	char _SpawnProg[10];

        char _coordX[10];
        char _coordY[10];
        char _coordZ[10];
        double *bufX;
        double *bufY;
        double *bufZ;

        MPI_File fh;
        MPI_Status status;
        MPI_Request request;
	MPI_Comm parentcomm, intercomm;

	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
	MPI_Comm_get_parent( &parentcomm );

//	printf("petit passage 03 !\n");

        bufX = (double *)calloc(BODY_COUNT,sizeof(double));
        bufY = (double *)calloc(BODY_COUNT,sizeof(double));
        bufZ = (double *)calloc(BODY_COUNT,sizeof(double));
        strcpy(_coordX,"_coordX");
        strcpy(_coordY,"_coordY");
        strcpy(_coordZ,"_coordZ");
	strcpy(_SpawnProg,"SpawnProg");

	/* Spawn process */
	if (parentcomm == MPI_COMM_NULL) {
		/* Create nprocs more processes to write data evaluation update */
		MPI_Comm_spawn(_SpawnProg, MPI_ARGV_NULL, nprocs - 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercomm, MPI_ERRCODES_IGNORE); 
		//printf("I'm the parent.\n");
	}
	//else {
		//printf("I'm the spawned.\n");
	//}
	/* End update */

	/* Data Evaluation update on root */
	for(i=0;i<BODY_COUNT;i++) {
		/* Update X */
		MPI_File_open(MPI_COMM_SELF, _coordX, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL,&fh);
		MPI_File_set_view(fh, 0, MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
		MPI_File_iread(fh, bufX, BODY_COUNT, MPI_DOUBLE, &request);
		MPI_Wait( &request, &status );
		MPI_File_close(&fh);
		sBodyPosition[i].x = bufX[i];
		/* Update Y */
		MPI_File_open(MPI_COMM_SELF, _coordY, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL,&fh);
		MPI_File_set_view(fh, 0, MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
		MPI_File_iread(fh, bufY, BODY_COUNT, MPI_DOUBLE, &request);
		MPI_Wait( &request, &status );
		MPI_File_close(&fh);
		sBodyPosition[i].y = bufY[i];
		/* Update Z */
		MPI_File_open(MPI_COMM_SELF, _coordZ, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL,&fh);
		MPI_File_set_view(fh, 0, MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
		MPI_File_iread(fh, bufZ, BODY_COUNT, MPI_DOUBLE, &request);
		MPI_Wait( &request, &status );
		MPI_File_close(&fh);
		sBodyPosition[i].z = bufZ[i];
	}
	if(rank == 0) {
//		printf(" X = %g \t Y = %g \t Z = %g\n",sBodyPosition[5].x,sBodyPosition[5].y,sBodyPosition[5].z);
//		printf(" bufX = %g \t bufY = %g \t bufZ = %g\n",bufX[5],bufY[5],bufZ[5]);
//		printf("sBodyPosition[5].x = %g\t sBodyPosition[5].y = %g\t sBodyPosition[5].z = %g\n",sBodyPosition[5].x, sBodyPosition[5].y, sBodyPosition[5].z);
		/* End local update */

		// Call the drawing functions
		drawBodies( &timeKeeper, &lightEyePos );
		// Switch the buffers to bring the drawing on screen
		glutSwapBuffers();
		glutPostRedisplay();  /* Redisplay to reply all the render view */
	}
}

///////////////////////////////////////////////////////////////////////
// Drawing

void drawBodies( CStopWatch *timeKeeper, M3DVector4f *lightPosition ) {
	// compute displacement and new vectors
	static float previousTime = 0.0f;
	float currentTime = timeKeeper->GetElapsedSeconds();
	//	updatePhysics( currentTime - previousTime );
	previousTime = currentTime;
	
	for( int i = 0; i < BODY_COUNT; i++ ) {
		// Save
		sModelViewMatrixStack.PushMatrix();
		// update position with regard to new values
		sBodyFrames[i].SetOrigin( sBodyPosition[i].x,
				sBodyPosition[i].y,
				sBodyPosition[i].z );
		// draw
		sModelViewMatrixStack.MultMatrix( sBodyFrames[i] );
		sShaderManager.UseStockShader( GLT_SHADER_POINT_LIGHT_DIFF,
				sTransformPipeline.GetModelViewMatrix(),
				sTransformPipeline.GetProjectionMatrix(),
				lightPosition,
				sBodyColors[i] );
		sBodyBatch[i].Draw();
		// Restore
		sModelViewMatrixStack.PopMatrix();
	}
}

///////////////////////////////////////////////////////////////////////
// Physics

void updatePhysics( float deltaT ) {

	for( int i = 0; i < BODY_COUNT; i++ ) {
		updateAcceleration( i );
		updateVelocity( i, deltaT );
		updatePosition( i, deltaT );
	}
}

void updateAcceleration( int bodyIndex ) {
	Force3D netForce = { 0, 0, 0 };

	for( int i = 0; i < BODY_COUNT; i++ ) {
		if( i == bodyIndex ) {
			continue;
		}

		Force3D vectorForceToOther = {0, 0, 0};
		Force scalarForceBetween = forceNewtonianGravity3D(
				sBodyMass[bodyIndex],
				sBodyMass[i],
				sBodyPosition[bodyIndex],
				sBodyPosition[i] );
		direction( sBodyPosition[bodyIndex],
				sBodyPosition[i],
				vectorForceToOther );
		vectorForceToOther.x *= scalarForceBetween;
		vectorForceToOther.y *= scalarForceBetween;
		vectorForceToOther.z *= scalarForceBetween;
		netForce.x += vectorForceToOther.x;
		netForce.y += vectorForceToOther.y;
		netForce.z += vectorForceToOther.z;
	}

	sBodyAcceleration[bodyIndex] = computeAccel3D( sBodyMass[bodyIndex],
			netForce );
}

void updateVelocity( int bodyIndex, float deltaT ) {
	sBodyVelocity[bodyIndex] = computeVelo3D(
			sBodyAcceleration[bodyIndex],
			sBodyVelocity[bodyIndex],
			deltaT );
}

void updatePosition( int bodyIndex, float deltaT ) {
	sBodyPosition[bodyIndex] = computePos3D( sBodyVelocity[bodyIndex],
			sBodyPosition[bodyIndex],
			deltaT );
}

