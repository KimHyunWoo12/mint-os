#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define BYTESOFSECTOR  512

int AdjustInSectorSize( int iFd, int iSourceSize );
void WriteKernelInformation( int iTargetFd, int iKernelSectorCount, int iKernel32SectorCount );
int CopyFile( int iSourceFd, int iTargetFd );

int main(int argc, char* argv[])
{
    int iSourceFd;
	int iTargetFd;
    int iBootLoaderSize1;
    int iBootLoaderSize2;
    int iKernel32SectorCount;
	int iKernel64SectorCount;
    int iSourceSize;
        
    if( argc < 5 )
    {
        fprintf( stderr, "[ERROR] ImageMaker.exe BootLoader1.bin BootLoader2.bin Kernel32.bin Kernel64.bin\n" );
        exit( -1 );
    }
    
    if( ( iTargetFd = open( "Disk.img", O_RDWR | O_CREAT |  O_TRUNC , S_IREAD | S_IWRITE ) ) == -1 )
    {
        fprintf( stderr , "[ERROR] Disk.img open fail.\n" );
        exit( -1 );
    }

    printf( "[INFO] Copy boot loader1 to image file\n" );
    if( ( iSourceFd = open( argv[ 1 ], O_RDONLY ) ) == -1 )
    {
        fprintf( stderr, "[ERROR] %s open fail\n", argv[ 1 ] );
        exit( -1 );
    }

    iSourceSize = CopyFile( iSourceFd, iTargetFd );
    close( iSourceFd );

    iBootLoaderSize1 = AdjustInSectorSize( iTargetFd , iSourceSize );
    printf( "[INFO] %s size = [%d] and sector count = [%d]\n",
            argv[ 1 ], iSourceSize, iBootLoaderSize1 );

    printf( "[INFO] Copy boot loader2 to image file\n" );
    if( ( iSourceFd = open( argv[ 2 ], O_RDONLY) ) == -1 )
    {
        fprintf( stderr, "[ERROR] %s open fail\n", argv[ 2 ] );
        exit( -1 );
    }

    iSourceSize = CopyFile( iSourceFd, iTargetFd );
    close( iSourceFd );
    
    iBootLoaderSize2 = AdjustInSectorSize( iTargetFd , iSourceSize );
    printf( "[INFO] %s size = [%d] and sector count = [%d]\n",
            argv[ 2 ], iSourceSize, iBootLoaderSize2 );

    printf( "[INFO] Copy protected mode kernel to image file\n" );
    if( ( iSourceFd = open( argv[ 3 ], O_RDONLY) ) == -1 )
    {
        fprintf( stderr, "[ERROR] %s open fail\n", argv[ 3 ] );
        exit( -1 );
    }

    iSourceSize = CopyFile( iSourceFd, iTargetFd );
    close( iSourceFd );
    
    iKernel32SectorCount = AdjustInSectorSize( iTargetFd, iSourceSize );
    printf( "[INFO] %s size = [%d] and sector count = [%d]\n",
                argv[ 3 ], iSourceSize, iKernel32SectorCount );

	printf( "[INFO] Copy IA-32e mode kernel to image file\n");
	if( ( iSourceFd = open( argv[ 4 ], O_RDONLY) ) == -1)
	{
		fprintf( stderr, "[ERROR] %s open fail\n", argv[4]);
		exit(-1);
	}

	iSourceSize = CopyFile( iSourceFd, iTargetFd);
	close(iSourceFd);

	iKernel64SectorCount = AdjustInSectorSize(iTargetFd, iSourceSize);
	printf("[INFO] %s size = [%d] and sector count = [%d]\n", argv[ 4 ], iSourceSize, iKernel64SectorCount);

    printf( "[INFO] Start to write kernel information\n" );    
    WriteKernelInformation( iTargetFd, iKernel32SectorCount + iKernel64SectorCount, iKernel32SectorCount );
    printf( "[INFO] Image file create complete\n" );

    close( iTargetFd );
    return 0;
}

int AdjustInSectorSize( int iFd, int iSourceSize )
{
    int i;
    int iAdjustSizeToSector;
    char cCh;
    int iSectorCount;

    iAdjustSizeToSector = iSourceSize % BYTESOFSECTOR;
    cCh = 0x00;
    
    if( iAdjustSizeToSector != 0 )
    {
        iAdjustSizeToSector = 512 - iAdjustSizeToSector;
        printf( "[INFO] File size [%u] and fill [%u] byte\n", iSourceSize, 
            iAdjustSizeToSector );
        for( i = 0 ; i < iAdjustSizeToSector ; i++ )
        {
            write( iFd , &cCh , 1 );
        }
    }
    else
    {
        printf( "[INFO] File size is aligned 512 byte\n" );
    }
    
    iSectorCount = ( iSourceSize + iAdjustSizeToSector ) / BYTESOFSECTOR;
    return iSectorCount;
}

void WriteKernelInformation( int iTargetFd, int iTotalKernelSectorCount, int iKernel32SectorCount )
{
    unsigned short usData;
    long lPosition;
    
    lPosition = lseek( iTargetFd,517, SEEK_SET );
    if( lPosition == -1 )
    {
        fprintf( stderr, "lseek fail. Return value = %ld, errno = %d, %d\n", 
            lPosition, errno, SEEK_SET );
        exit( -1 );
    }

    usData = ( unsigned short ) iTotalKernelSectorCount;
    write( iTargetFd, &usData, 2 );
	usData = ( unsigned short ) iKernel32SectorCount;
	write( iTargetFd, &usData, 2 );

    printf( "[INFO] Total sector count except boot loader [%d]\n", iTotalKernelSectorCount );
	printf( "[INFO] Total sector count protected mode kernel [%d]\n", iKernel32SectorCount );
}

int CopyFile( int iSourceFd, int iTargetFd )
{
    int iSourceFileSize;
    int iRead;
    int iWrite;
    char vcBuffer[ BYTESOFSECTOR ];

    iSourceFileSize = 0;
    while( 1 )
    {
        iRead   = read( iSourceFd, vcBuffer, sizeof( vcBuffer ) );
        iWrite  = write( iTargetFd, vcBuffer, iRead );

        if( iRead != iWrite )
        {
            fprintf( stderr, "[ERROR] iRead != iWrite.. \n" );
            exit(-1);
        }
        iSourceFileSize += iRead;
        
        if( iRead != sizeof( vcBuffer ) )
        {
            break;
        }
    }
    return iSourceFileSize;
} 
