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

#define ITER 100

using std::cout;
using std::cerr;
using std::endl;

///////////////////////////////////////////////////////////////////////
// Setup

int main( int argc, char **argv ) {

	int rank;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	iter = 0;

	if( TEST ) {
		if( !testVectorMath() ) {
			cerr << "Test VectorMath failed.";
			return -2;
		}
		cout << "All test OK" << endl;
	}

	setupWindow( argc, argv );
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
	glutCreateWindow( "n body" );
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
	char _masse[10];
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
	strcpy(_masse,"_masse");

        for(i=0;i<BODY_COUNT;i++) {
		seek = 500.0;
		a 			= rand();
		b 			= fmod(a,seek);
		a 			= rand();
		c 			= -fmod(a,seek);
		sBodyRadius[i] 		= fmod(a,10) + 1; /* between 1 and 100 */
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
			sBodyPosition[i].x = bufX[i];
		a 			= rand();
		b 			= fmod(a,seek);
                a                       = rand();
                c                       = -fmod(a,seek);
                randval                 = (double) (b / seek) + (double) (c / seek);
//		bufY[i]                 = 300 * randval; /* between -100 and 100 */
		bufY[i] 		= 3000 * randval / seek; /* between 0 and 300 */
			sBodyPosition[i].y = bufY[i];
		a 			= rand();
		b 			= fmod(a,seek);
		a 			= rand();
		c 			= -fmod(a,seek);
//		randval                 = (double) (b / seek) + (double) (c / seek);
//		bufZ[i]			= 100 * randval; /* between -100 and 100 */
		randval 		= b + 2 * c ;
		bufZ[i] 		= 10.0 * randval; /* between -1000 and 500 */
			sBodyPosition[i].z = bufZ[i];
		sBodyVelocity[i].x 	= 1.0;
		sBodyVelocity[i].y 	= 1.0;
		sBodyVelocity[i].z 	= 0.0;
		sBodyAcceleration[i].x 	= 0.0;
		sBodyAcceleration[i].y 	= 0.0;
		sBodyAcceleration[i].z 	= 0.0;
		a			= rand();
		sBodyMass[i] 		= (float) (10 * fmodf(a,3) + 10);
        }
	sBodyMass[0]            = 1e9;

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

        MPI_File_open(MPI_COMM_SELF, _masse, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
        MPI_File_set_view(fh, 0, MPI_FLOAT, MPI_FLOAT, "native", MPI_INFO_NULL);
        MPI_File_iwrite(fh, sBodyMass, BODY_COUNT, MPI_FLOAT, &request);
        MPI_Wait( &request, &status );
        MPI_File_close(&fh);

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
		/* CHANGEMENT DE DIMENSION DES PARTICULES A L'AFFICHAGE */
		gltMakeSphere( sBodyBatch[i], sBodyRadius[i], 30, 2 );
		//gltMakeSphere( sBodyBatch[i], 5, 30, 50 );
		/* FIN DE CHANGEMENT DE DIMENSION DES PARTICULES A L'AFFICHAGE */
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
	char _masse[10];
        double *bufX;
        double *bufY;
        double *bufZ;
	double *masse;

        MPI_File fh;
        MPI_Status status;
        MPI_Request request;
	MPI_Comm parentcomm, intercomm;

	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
	MPI_Comm_get_parent( &parentcomm );

//	printf("petit passage 03 !\n");

        bufX  = (double *)calloc(BODY_COUNT,sizeof(double));
        bufY  = (double *)calloc(BODY_COUNT,sizeof(double));
        bufZ  = (double *)calloc(BODY_COUNT,sizeof(double));
        masse = (double *)calloc(BODY_COUNT,sizeof(double));
        strcpy(_coordX,"_coordX");
        strcpy(_coordY,"_coordY");
        strcpy(_coordZ,"_coordZ");
        strcpy(_masse,"_masse");
	strcpy(_SpawnProg,"SpawnProg");

	/* Spawn process */
	if (parentcomm == MPI_COMM_NULL) {
		/* Create nprocs more processes to write data evaluation update */
		MPI_Comm_spawn(_SpawnProg, MPI_ARGV_NULL, nprocs - 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercomm, MPI_ERRCODES_IGNORE); 
		iter++;
		printf("get off %i \n",iter);
	}

	/* Data Evaluation update on root */
	for(i=0;i<BODY_COUNT;i++) {
		/* Update X */
		MPI_File_open(MPI_COMM_SELF, _coordX, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL,&fh);
		MPI_File_set_view(fh, 0, MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
		MPI_File_iread(fh, &sBodyPosition[0].x, BODY_COUNT, MPI_DOUBLE, &request);
		MPI_Wait( &request, &status );
		MPI_File_close(&fh);
		/* Update Y */
		MPI_File_open(MPI_COMM_SELF, _coordY, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL,&fh);
		MPI_File_set_view(fh, 0, MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
		MPI_File_iread(fh, &sBodyPosition[0].y, BODY_COUNT, MPI_DOUBLE, &request);
		MPI_Wait( &request, &status );
		MPI_File_close(&fh);
		/* Update Z */
		MPI_File_open(MPI_COMM_SELF, _coordZ, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL,&fh);
		MPI_File_set_view(fh, 0, MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
		MPI_File_iread(fh, &sBodyPosition[0].z, BODY_COUNT, MPI_DOUBLE, &request);
		MPI_Wait( &request, &status );
		MPI_File_close(&fh);
		/* Update Masse */
		MPI_File_open(MPI_COMM_SELF, _masse, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
		MPI_File_set_view(fh, 0, MPI_FLOAT, MPI_FLOAT, "native", MPI_INFO_NULL);
		MPI_File_iwrite(fh, sBodyMass, BODY_COUNT, MPI_FLOAT, &request);
		MPI_Wait( &request, &status );
		MPI_File_close(&fh);
		//printf("in boucle iterative : masse[%i] = %f \n",i,masse[i]);
	}
	/* BOUCLE ITERATIVE */
//	if(iter < ITER) {
		if(rank == 0) {
			// Call the drawing functions
			drawBodies( &timeKeeper, &lightEyePos );
			// Switch the buffers to bring the drawing on screen
			glutSwapBuffers();
			glutPostRedisplay();  /* Redisplay to reply all the render view */
			iter++;
		}
//	}
//	else {
//	
//	}
}

///////////////////////////////////////////////////////////////////////
// Drawing

void drawBodies( CStopWatch *timeKeeper, M3DVector4f *lightPosition ) {
	// compute displacement and new vectors
	static float previousTime = 0.0f;
	float currentTime = timeKeeper->GetElapsedSeconds();
	previousTime = currentTime;

	for( int i = 0; i < BODY_COUNT; i++ ) {
		// Save
		sModelViewMatrixStack.PushMatrix();
		/* update position with regard to new values */
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

