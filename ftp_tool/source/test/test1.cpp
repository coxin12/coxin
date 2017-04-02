#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "ftplib_mt.h"

int main( int argc, char* argv[])
{
    netbuf *ftp1_handle;
    netbuf *ftp2_handle;
    netbuf *data1_handle;
    netbuf *data2_handle;
    fprintf(stderr, "\n1\n\n");
    FtpConnect( "10.200.1.12", &ftp1_handle, 1);
    fprintf(stderr, "\n2\n\n");
    FtpLogin("etl", "cxbi1234", ftp1_handle);
    fprintf(stderr, "\n3\n\n");
    FtpConnect( "10.200.1.13", &ftp2_handle, 1);
    fprintf(stderr, "\n4\n\n");
    FtpLogin("etl", "cxbi1234", ftp2_handle);
    fprintf(stderr, "\n5\n\n");
    FtpAccess("/app/etl/czx/test/filetran1.log", FTPLIB_FILE_READ ,FTPLIB_IMAGE, ftp1_handle,&data1_handle); 
    fprintf(stderr, "\n6\n\n");
    FtpAccess("/app/etl/czx/test/filetran1.log", FTPLIB_FILE_READ ,FTPLIB_IMAGE, ftp1_handle,&data1_handle); 
    fprintf(stderr, "\n7\n\n");
    FtpAccess("/app/etl/czx/test/filetran1.log", FTPLIB_FILE_WRITE ,FTPLIB_IMAGE, ftp2_handle,&data2_handle);
    fprintf(stderr, "\n8\n\n");
    long l;
    long w;
    char sBuf[BUFSIZ];
    memset( sBuf, 0, BUFSIZ);
    while((l = FtpRead(sBuf, BUFSIZ, data1_handle) ) > 0)
    {
        w = FtpWrite(sBuf, l, data2_handle); 
        if( w < l )
        {
            fprintf( stderr, "FtpWrite:%d\n", w);
            break;
        }
        memset( sBuf, 0, BUFSIZ);
    }
    fprintf(stderr, "\n9\n\n");
    FtpClose( data1_handle );
    FtpClose( data2_handle );

    int size;
    fprintf(stderr, "\n10\n\n");
    FtpSize( "/app/etl/czx/test/filetran1.log", &size, FTPLIB_IMAGE, ftp2_handle);
    fprintf( stderr, "size:%d\n", size);
    return 0;
}
