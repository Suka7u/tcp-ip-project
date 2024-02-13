/*
 * Auth: Tom Nguyen
 * Date: 12/06/2023 (Due: 12/07/2023)
 * Course: CSCI-3550 (Sec: 851)
 * Desc: Project01 Client Side, a program that sends bytes read from a file through a socket
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

/* declare global variables */
int fd_in = -1;
char *outbuf = NULL;
int sockfd = -1;

/* declare function prototypes */
void cleanup( void );
void SIGINT_handler( int sig );


void cleanup( void ) {

    /* releases memory */
    if( outbuf != NULL ) {

        free( outbuf );
        outbuf = NULL;

    } /* end if */

    /* closes the socket */
    if( sockfd >= 0) {

        close( fd_in );
        fd_in = -1;

    } /* end if */

} /* end cleanup */


/* SIGINT handler for the client */
void SIGINT_handler( int sig ) {

    /* issue a message to the user */
    fprintf( stderr, "client: Client Interrupted. Shutting down.\n" );

    /* code for cleaning up */
    cleanup();

    /* ends the program with failure */
    exit( EXIT_FAILURE );

} /* end SIGINT_handler() */

 int main( int argc, char *argv[]) {

    /* initialize and declare variables */
    struct in_addr ia;
    struct sockaddr_in sa;
    const char *port_str = argv[2];
    unsigned short int port;
    int bytes_read;
    int i;
    int bytes_sent;

    /* do this if the argument count is correct */
    if (argc >= 4) {

        /* setup port */
        port = (unsigned short int) atoi( port_str );

        /* error if port number is privileged */
        if (port >= 0 && port <= 1023) {

            fprintf( stderr, "client: ERROR: Port number is privileged.\n");
            exit( EXIT_FAILURE);

        } /* end if */

        /* sets up the IP address and indicates if there is an error */
        if ( inet_aton( argv[1], &ia ) == 0 ) {

            fprintf( stderr, "client: ERROR: Setting the IP address. \n" );
            cleanup();
            exit( EXIT_FAILURE );

        } /* end if */

        /* fields for socket */
        sa.sin_family = AF_INET;
        sa.sin_addr = ia;
        sa.sin_port = htons( port );

        /* allocates space */
        outbuf = (char *) malloc( BUF_SIZE );

        /* if there is no memory allocated do this */
        if( outbuf == NULL ) {
            
            /* issues an error and closes the program with failure */
            fprintf( stderr, "ERROR: Could not allocate memory.\n");
            exit( EXIT_FAILURE );

        } /* end if */

        /* signal handler for sigint */
        signal( SIGINT, SIGINT_handler );

        /* loop until all files are sent */
        for (i = 3; i < argc; i++) {

            /* sets up socket */
            sockfd = socket( AF_INET, SOCK_STREAM, 0);

            /* error if socket is failed to be created */
            if( sockfd < 0 ) {

                fprintf( stderr, "client: ERROR: Failed to create socket.\n" );
                exit( EXIT_FAILURE );

            } /* end if */

            printf("client: Connecting to %s:%s...\n", argv[1], argv[2]);

            /* attempts to connect to socket */
            if( connect( sockfd, (struct sockaddr *) &sa, sizeof( sa ) ) != 0) {

                fprintf( stderr, "client: ERROR: connecting to %s:%s...\n", argv[1], argv[2] );
                cleanup();
                exit( EXIT_FAILURE );

            } /* end if */

            printf( "client: Success!\n" );

            printf( "client: Sending: \"%s\"...\n", argv[1] );

            /* opens input file */
            fd_in = open(argv[i], O_RDONLY);

            /* if open fails indicate an error */
            if( fd_in < 0 ) {

                fprintf( stderr, "client: ERROR: failed to open: %s\n\n", argv[i] );
                continue;

            } /* end if */

            /* reads from the file */
            bytes_read = read( fd_in, outbuf, BUF_SIZE );

            /* indicates there was a problem reading the file */
            if( bytes_read < 0 ) {

                fprintf( stderr, "client: Unable to read file: %s\n\n", argv[i] );

            } /* end if */

            /* attempts to send the stored bytes */
            bytes_sent = send( sockfd, (const void *) outbuf, bytes_read, 0);

            /* indicates if something goes wrong with sending */
            if( bytes_read != bytes_sent ) {

                fprintf( stderr, "client: ERROR: While sending data.\n" );

            } /* end if */

            /* close socket */
            if( sockfd > -1 ) {

                close( sockfd );
                sockfd = -1;

            } /* end if */

            /* cleans up and indicates the file transger is complete */
            cleanup();
            printf("client: Done. \n");

        } /* end for */

        /* indicates the program is finished and successful */
        printf( "client: File transfer(s) complete.\n" );
        printf( "client: Goodbye!\n" );
        cleanup();
        exit( EXIT_SUCCESS );

    } /* end if */
    else {

        /* error if the number of arguments is incorrect */
        fprintf( stderr, "client: USAGE: client <IP_addr> <port> <files>. Try again.\n" );
        exit( EXIT_FAILURE );

    } /* end else */

 }