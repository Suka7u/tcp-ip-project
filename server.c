/*
 * Auth: Tom Nguyen
 * Date: 12/06/2023 (Due: 12/07/2023)
 * Course: CSCI-3550 (Sec: 851)
 * Desc: Project01 Server Side, a program that writes bytes received from a socket
 */


#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<netdb.h>
#include<ctype.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<arpa/inet.h>


#define BUF_SIZE (100*1024*1024)
#define FILE_NAME_SIZE 64

/* declare global variables */
char outfile[FILE_NAME_SIZE];
int fd_out = -1;
int sockfd = -1;
int cl_sockfd;
char *inbuf = NULL;

/* declare function prototypes */
void cleanup(void);
void SIGINT_handler( int sig );

 /* SIGINT handler for the server */
void SIGINT_handler( int sig ) {

    /* issue a message to the user */
    fprintf( stderr, "server: Server Interrupted. Shutting down.\n" );

    /* code for cleaning up */
    cleanup();

    /* ends the program with failure */
    exit( EXIT_FAILURE );

} /* end SIGINT_handler() */

/* function for cleaning up */
void cleanup() {

    /* cleanup buffer */
    if ( inbuf != NULL ) {

        free( inbuf );
        inbuf = NULL;

    }

    /* cleanup socket of server */
    if ( sockfd > -1 ) {

        close( sockfd );
        sockfd = -1;

    }

    /* cleanup client socket */
    if( cl_sockfd > -1 ) {

        close( cl_sockfd );
        cl_sockfd = -1;

    }

    /* cleanup output file */
    if( fd_out > -1 ) {

        close( fd_out );
        fd_out = -1;

    }
} /* end cleanup */

int main ( int argc, char *argv[] ) {

    /* initialize and declare variables */
    const char *port_str = argv[1];
    int bytes_read;
    int bytes_written;
    int file_count = 1;
    int opval = 1;
    struct sockaddr_in sa;
    struct in_addr ia;
    struct sockaddr_in cl_sa;
    socklen_t cl_sa_size = sizeof( cl_sa );

    /* run if arguments are correct */
    if( argc == 2 ) {

        /* setup port value */
        unsigned short int port = (unsigned short int) atoi( port_str );

        /* error to check for privileged port */
        if(port >= 0 && port <= 1023) {

            fprintf( stderr, "server: ERROR: Port number is privileged.\n");
            exit( EXIT_FAILURE );
        } /* end if */

        /* signal handler for sigint */
        signal(SIGINT, SIGINT_handler);

        /* check to see if ip address works after setting it up */
        if ( inet_aton( "127.0.0.1", &ia ) == 0 ) {

            fprintf( stderr, "server: ERROR: Setting the IP address. \n" );
            cleanup();
            exit( EXIT_FAILURE );

        } /* end if */

        /* setup socket fields */
        sa.sin_family = AF_INET;
        sa.sin_addr = ia;
        sa.sin_port = htons( port );
        
        /* create a new socket */
        sockfd = socket( AF_INET, SOCK_STREAM, 0 );

        /* error for when the socket fails to be created */
        if( sockfd < 0) {

            fprintf( stderr, "server: ERROR: Failed to create socket.\n");
            cleanup();
            exit( EXIT_FAILURE );

        } /* end if */

        /* allows for socket renewal */
        if( setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &opval, sizeof( int ) ) != 0 ) {

            fprintf( stderr, "server: ERROR: setsockopt() failed.\n");
            cleanup();
            exit( EXIT_FAILURE );

        } /* end if */

        /* binds the listening socket to an address */
        if ( bind( sockfd, (struct sockaddr *) &sa, sizeof( sa ) ) != 0 ) {

            fprintf( stderr, "server: ERROR: Failed to bind socket.\n");
            cleanup();
            exit( EXIT_FAILURE );

        } /* end if */

        /* turn socket into listener socket */
        if ( listen( sockfd, 32 ) != 0 ) {

            fprintf( stderr, "server: ERROR: listen() failed.\n");
            cleanup();
            exit( EXIT_FAILURE );

        } /* end if */

        /* clear all bytes to 0 */
        memset( ( void *) &cl_sa, 0, sizeof( cl_sa ) );

        /* repeat until client side is finished */
        while(1) {

            /* allocate memory */
            inbuf = (void *) malloc( BUF_SIZE );

            /* error if no memory allocated */
            if( inbuf == NULL ) {
                
                fprintf( stderr, "server: ERROR: Failed to allocate memory.\n" );
                exit( EXIT_FAILURE );

            } /* end if */

            printf("server: Awaiting TCP connections over port %d...\n", port);

            /* connection request */
            cl_sockfd = accept( sockfd, (struct sockaddr *) &cl_sa, &cl_sa_size );
            printf("server: Receiving file...\n");

            /* connection is established */
            if( cl_sockfd > 0 ) {

                printf("server: Connection accepted!\n");
                /* reads data from socket */
                bytes_read = recv( cl_sockfd, (void *) inbuf, BUF_SIZE, MSG_WAITALL );

                /* error if data read fails */
                if( cl_sockfd > -1 ) {

                    printf("server: Connection closed.\n" );
                    close( cl_sockfd );
                    cl_sockfd = -1;

                } /* end if */

                /* do this if bytes are read */
                if( bytes_read >= 0 ) {

                    /* sets up dynamic file name */
                    sprintf(outfile, "file-%02d.dat", file_count);

                    /* opens a file to be written on */
                    fd_out = open( outfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );

                    /* error if empty */
                    if( fd_out < 0 ) {

                        fprintf( stderr, "server: ERROR: Unable to ceate %s.\n", outfile );
                        cleanup();
                        exit( EXIT_FAILURE );

                    }

                    /* write on the file */
                    bytes_written = write(fd_out, inbuf, bytes_read);
                    printf("server: Saving file: \"%s\"...\n", outfile);

                    /* error if writing fails */
                    if ( bytes_read != bytes_written ) {

                        fprintf( stderr, "server: ERROR: Unable to write %s\n", outfile);
                        cleanup();
                        exit( EXIT_FAILURE );

                    }

                    /* incrememnts file counter and closes the file */
                    file_count++;
                    close( fd_out );
                    fd_out = -1;
                    printf("Done.\n\n");

                } /* end if */
                else {
                    /* error if socket reading fails */
                    fprintf( stderr, "server: ERROR: Reading from socket.");
                    cleanup();
                    exit( EXIT_FAILURE );

                } /* end else */


            } /* end if */
            else {

                /* error if connection fails */
                fprintf( stderr, "server: ERROR: While attempting to accept a connection.\n" );
                cleanup();
                exit( EXIT_FAILURE );

            } /* end else */


        } /* end while */

    } /* end if */
    else {

        /* error that exits when not correct number of arguments */
        fprintf( stderr, "server: USAGE: <port>. Try again.\n");
        exit( EXIT_FAILURE );

    } /* end else */

    /* successfully exit program */
    exit( EXIT_SUCCESS );

} /* end main */