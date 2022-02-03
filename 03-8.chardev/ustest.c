#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"

char dev[ 80 ] = "/dev/";

int prepare() {
	int df;
	if( ( df = open( dev, O_RDWR ) ) < 0 )
		printf( "open device error: %m\n" );
	return df;
}

void test( int df ) {
	int buf;
	int res;
	printf( "------------------------------------\n" );
	do {
		if( ( res = read( df, &buf, sizeof(buf) ) ) > 0 ) {
			printf( "read %d bytes: %d\n", res, buf );
		}
		else if( res < 0 )
			printf( "read device error: %m\n" );
		else
			printf( "read end of stream\n" );
		read(df,NULL,1);
	} while ( res > 0 );
	printf( "------------------------------------\n" );
	}

int main( int argc, char *argv[] ) {
	strcat( dev, DEVNAME );
	int df1, df2;
	// разные дескрипторы одного устройства
	df1 = prepare();
	df2 = prepare();
	test( df1 );
	test( df2 );
	close( df1 );
	close( df2);
	return EXIT_SUCCESS;
};

