
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define STR_CLOSE   "close"
#define STR_QUIT    "quit"

//***************************************************************************
// log messages

#define LOG_ERROR               0       // errors
#define LOG_INFO                1       // information and notifications
#define LOG_DEBUG               2       // debug messages

// debug flag
int g_debug = LOG_INFO;
sem_t *g_sem_mutex = nullptr;

void log_msg( int t_log_level, const char *t_form, ... )
{
    const char *out_fmt[] = {
            "ERR: (%d-%s) %s\n",
            "INF: %s\n",
            "DEB: %s\n" };

    if ( t_log_level && t_log_level > g_debug ) return;

    char l_buf[ 1024 ];
    va_list l_arg;
    va_start( l_arg, t_form );
    vsprintf( l_buf, t_form, l_arg );
    va_end( l_arg );

    switch ( t_log_level )
    {
    case LOG_INFO:
    case LOG_DEBUG:
        fprintf( stdout, out_fmt[ t_log_level ], l_buf );
        break;

    case LOG_ERROR:
        fprintf( stderr, out_fmt[ t_log_level ], errno, strerror( errno ), l_buf );
        break;
    }
}

//***************************************************************************
// help

FILE* f;

struct ThreadPar
{
    int id;
    int numberin;
};


void *demo_thread( void *t_par )
{
    ThreadPar *l_ppar = ( ThreadPar * ) t_par;
    int id = l_ppar->id;
    int numbering = l_ppar->numberin;

    char filename[100];
    sprintf(filename, "output%d.txt", id);

    FILE* fOut = fopen(filename, "w");

    char buf[100];




    char output[1000];
    int i = 0;
    char temp[100];

    output[0] = '\0';

     sem_wait( g_sem_mutex );

    fseek(f, 0, 0);
    while (fgets(buf, sizeof(buf), f) != 0) //gets line
    {
       // printf("%s", buf);
        sprintf(temp, "%d. %s", i * numbering, buf);
        strcat(output,  temp);
        i++;
    }

    fwrite(output, sizeof(char), strlen(output), fOut);

    // sleep(numbering * 4);

    // fseek(fOut, 0, SEEK_SET);
    fprintf(fOut, "\nDokončeno počítání řádků: %d", i);
    fprintf(fOut, "Počet znaků: %ld", ftell(f));

    fclose(fOut);

    if ( sem_post( g_sem_mutex ) < 0 )
        {
           // log_msg( LOG_ERROR, "Unable to unlock critical section!" );
          
        }

    pthread_exit( ( void * ) ( ( intptr_t ) - id ) );
}

//***************************************************************************

int main( int t_narg, char **t_args )
{

     f = fopen("test.txt", "r");

     g_sem_mutex = sem_open( "/testSem", O_RDWR | O_CREAT, 0660, 1 );
    
    pthread_t thread_id[ 3 ];  
    void* thread_status[ 3 ];  


    for ( int i = 0; i < 3; i++ )
    {
        // ThreadPar* par = (ThreadPar*)malloc(sizeof(ThreadPar));
        // par->id = i;
        // par->numberin = i + 1;

        ThreadPar par;
        par.id = i;
        par.numberin = i + 1;

        int err = pthread_create( &thread_id[ i ], nullptr, demo_thread, &par);
        if ( err )
            log_msg( LOG_INFO, "Unable to create thread %d.", i );
        else
            log_msg( LOG_DEBUG, "Thread %d created - system id 0x%X.", i, thread_id[ i ] );
    }

    
    
   for ( int i = 0; i < 3; i++ )
   {
        pthread_join( thread_id[ i ], &thread_status[ i ] ); 
        // printf("Dokončeno %d", i);
   }



    //    if (stat(argv[1], &fileStat) < 0) {
    //     perror("stat");
    //     return 1;
    // }

   sem_unlink( "/testSem" );

    return 0;
}