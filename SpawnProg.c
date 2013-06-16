#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mpi.h"

#include "src/Constants.h"

#define deltat 1
#define NITER 1

int main(int argc, char *argv[]){

	int i,j, iter;
	char *_coordX, *_coordY, *_coordZ;
	char *_masse;
	double *bufX, *bufY, *bufZ;
	float  *masse;
	double uX, uY, uZ;
	double _sqrt, res0;
	double _fX, _fY, _fZ;

        MPI_File fh;
        MPI_Status status;
        MPI_Request request;

	MPI_Init(&argc,&argv);

	bufX=(double *)calloc(BODY_COUNT,sizeof(double));
        _coordX = (char *)malloc(10);
	strcpy(_coordX,"_coordX");

	bufY=(double *)calloc(BODY_COUNT,sizeof(double));
        _coordY = (char *)malloc(10);
	strcpy(_coordY,"_coordY");

	bufZ=(double *)calloc(BODY_COUNT,sizeof(double));
        _coordZ = (char *)malloc(10);
	strcpy(_coordZ,"_coordZ");

	masse=(float *)calloc(BODY_COUNT,sizeof(float));
        _masse = (char *)malloc(10);
	strcpy(_masse,"_masse");

	/* Reading the data structure */
	MPI_File_open(MPI_COMM_SELF, _coordX, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
	MPI_File_set_view(fh, 0, MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
	MPI_File_iread(fh, bufX, BODY_COUNT, MPI_DOUBLE, &request);
	MPI_Wait( &request, &status );
	MPI_File_close(&fh);

	MPI_File_open(MPI_COMM_SELF, _coordY, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
	MPI_File_set_view(fh, 0, MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
	MPI_File_iread(fh, bufY, BODY_COUNT, MPI_DOUBLE, &request);
	MPI_Wait( &request, &status );
	MPI_File_close(&fh);

	MPI_File_open(MPI_COMM_SELF, _coordZ, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
	MPI_File_set_view(fh, 0, MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
	MPI_File_iread(fh, bufZ, BODY_COUNT, MPI_DOUBLE, &request);
	MPI_Wait( &request, &status );
	MPI_File_close(&fh);

	MPI_File_open(MPI_COMM_SELF, _masse, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
	MPI_File_set_view(fh, 0, MPI_FLOAT, MPI_FLOAT, "native", MPI_INFO_NULL);
	MPI_File_iread(fh, masse, BODY_COUNT, MPI_FLOAT, &request);
	MPI_Wait( &request, &status );
	MPI_File_close(&fh);

	res0 = 0.0;
	for(i=0;i<BODY_COUNT;i++) {
		_fX = 0.0;
		_fY = 0.0;
		_fZ = 0.0;
		for(j=0;j<BODY_COUNT;j++) {
			if(j!=i) {
				uX = bufX[i] - bufX[j] ;
				uY = bufY[i] - bufY[j] ;
				uZ = bufZ[i] - bufZ[j] ;

				res0 = uX * uX ;
				res0+= uY * uY ;
				res0+= uZ * uZ ;
				_sqrt = sqrt(res0);
				res0 = (double) 1.0 / _sqrt ;

				_fX+= res0 * res0 * res0 * uX * masse[j] ;
				_fY+= res0 * res0 * res0 * uY * masse[j] ;
				_fZ+= res0 * res0 * res0 * uZ * masse[j] ;
			}
		}
		printf("_fX = %g bufX[5] = %g  bufX[6] = %g\n",_fX,bufX[5],bufX[6]);
/*
		bufX[i] = bufX[i] - deltat * deltat * _fX * 1e7;
		bufY[i] = bufY[i] - deltat * deltat * _fY * 1e7;
		bufZ[i] = bufZ[i] - deltat * deltat * _fZ * 1e7;
*/
		bufX[i] = bufX[i] - deltat * deltat * _fX * 1e8 ;
		bufY[i] = bufY[i] - deltat * deltat * _fY * 1e8 ;
		bufZ[i] = bufZ[i] - deltat * deltat * _fZ * 1e8 ;
	}

	/* Writing the data structure */
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

	MPI_Finalize();

	/* Memory free */
	free(_coordX);
	free(bufX);
	free(bufY);
	free(bufZ);
	return EXIT_SUCCESS;

}
