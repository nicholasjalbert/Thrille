/*
Copyright (c) 2005 Konstantin Isakov. Based on the original libbzip2 sources,
part of bzip2, version 1.0.2, Copyright (c) 1996-2002 Julian R Seward.
See LICENSE file for details.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "note.h"
#include "detectht.h"
#include "bzlib.h"

#ifdef ERR1
#define malloc thrilleMallocC
void * thrilleMallocC(size_t);
#endif

#ifndef HT_DETECTION
#warning Dont know how to detect hyperthreading on this system, please implement it to counter the need to specify it explicitly
#endif

#define BUFSZ 4096

static pthread_mutex_t inChunksMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t inChunksAvailableCondition = PTHREAD_COND_INITIALIZER;
static pthread_cond_t inChunksFreeCondition = PTHREAD_COND_INITIALIZER;
/* Number of chunks currently available to be allocated and put to the
inChunks ring buffer */
static int inChunksFree;
static bz_stream ** inChunks;
static int inChunksCount; /* Total number of elements in the inChunks buffer */
static volatile int inChunksHead = 0; 
static volatile int inChunksTail = -1; /* -1 means that the buffer is empty */ 

/* The output chunks, organized in a ring buffer fasion. The tail chunk is the
one to be written to stdout next, when it is ready. The number of chunks is set
to number of threads * 2 + 1. When a thread finishes compression, it checks if
it finished the compression of a chunk which is to be written next. If so,
it triggers a flush condition.
Note that the chunk is present when its data pointer is non-NULL.
*/

static pthread_mutex_t outChunksMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t outChunksAvailableCondition = PTHREAD_COND_INITIALIZER;
static pthread_cond_t outChunksFlushCondition = PTHREAD_COND_INITIALIZER;
static bz_stream ** outChunks;
static int outChunksCount;
static volatile int outChunksHead = 0; 
static volatile int outChunksTail = -1; /* -1 means that the buffer is empty */ 

static pthread_mutex_t inOutChunksAllocationMutex = PTHREAD_MUTEX_INITIALIZER;
FILE * nullout = NULL;


/* Wraps around a negative index within the ring buffer */
int wrapChunkIndex( int idx )
{
  if ( idx < 0 )
    return outChunksCount + idx;
  else
    return idx;
}

static size_t blockSize100k = 9;

void * allocMem( size_t size )
{
  void * result = malloc( size );

  if ( result == NULL )
  {
    note( 0, "Failed to allocate %u bytes of memory\n", size );
    exit( 1 );
  }

  return result;
}

/* The compressing threads' function */
void * threadFunction()
{
  int outChunk;
  bz_stream * strm;

  for( ; ; )
  {
    /* Take the mutex to make the in-out chunk acquisition operation atomic */
#ifdef ERR1
#else
    pthread_mutex_lock( &inOutChunksAllocationMutex );
#endif

    /* Get the next input chunk from the list of available */
    pthread_mutex_lock( &inChunksMutex );

    if ( inChunksTail == -1 )
    {
      note( 2, "THREAD INPUT STARVATION_____________\n" );
      pthread_cond_wait( &inChunksAvailableCondition, &inChunksMutex );
    }

    strm = inChunks[ inChunksTail ];

    ++inChunksTail;

    if ( inChunksTail == inChunksCount )
      inChunksTail = 0;

    if ( inChunksTail == inChunksHead )
      inChunksTail = -1;

    pthread_mutex_unlock( &inChunksMutex );

    /* Allocate the out chunk */

    pthread_mutex_lock( &outChunksMutex );

    if ( outChunksHead == outChunksTail )
    {
      note( 2, "THREAD OUTPUT STARVATION.............\n" );
      /* No free chunk available, wait for one */
      pthread_cond_wait( &outChunksAvailableCondition, &outChunksMutex );
    }

    if ( outChunksTail == -1 )
      outChunksTail = outChunksHead;

    note( 2, "allocating outchunk %d\n", outChunksHead );

    outChunk = outChunksHead++;

    if ( outChunksHead == outChunksCount )
      outChunksHead = 0;

    pthread_mutex_unlock( &outChunksMutex );
#ifdef ERR1
#else
    pthread_mutex_unlock( &inOutChunksAllocationMutex );
#endif

    /* Now perform the blocksort */

    BZ2_bzCompressDoBlocksort( strm );

    /* Great, now indicate that the in chunk is free and make the
    out chunk available */

    pthread_mutex_lock( &inChunksMutex );

    ++inChunksFree;

    pthread_cond_signal( &inChunksFreeCondition );

    pthread_mutex_unlock( &inChunksMutex );

    pthread_mutex_lock( &outChunksMutex );

    outChunks[ outChunk ] = strm;

    note( 2, "finished outchunk %d, tail is %d\n", outChunk, outChunksTail );

    if ( outChunk == outChunksTail ) {
        pthread_cond_signal( &outChunksFlushCondition );
    }
    pthread_mutex_unlock( &outChunksMutex );
  }
}

/* This thread is only writing the results to stdout */
void * writerThread()
{
  void * buf = allocMem( blockSize100k * 101000 + 600 );
  size_t blockSize;
  bz_stream * strm;
  bz_stream_state_out savedState;
  int wasStateSaved = 0;

  pthread_mutex_lock( &outChunksMutex );

  for( ; ; )
  {
    if ( outChunksTail == -1 || ! outChunks[ outChunksTail ] )
      pthread_cond_wait( &outChunksFlushCondition, &outChunksMutex );

    pthread_mutex_unlock( &outChunksMutex );

    note( 2, "flushing outchunk %d\n", outChunksTail );

    strm = outChunks[ outChunksTail ];

    if ( wasStateSaved )
      BZ2_bzCompressRestoreOutputState( strm, &savedState );

    blockSize = BZ2_bzCompressStoreBlocksort( strm, buf, strm->avail_in );

    BZ2_bzCompressSaveOutputState( strm, &savedState );
    wasStateSaved = 1;

    BZ2_bzCompressEnd( strm );
#ifdef NICKEDIT
    if ( blockSize && fwrite( buf, blockSize, 1, nullout) != 1 )
    {
      note( 0, "Error writing to nullout\n" );
      exit( 1 );
    }

#else

    if ( blockSize && fwrite( buf, blockSize, 1, stdout ) != 1 )
    {
      note( 0, "Error writing to stdout\n" );
      exit( 1 );
    }

#endif

    free( strm );

    outChunks[ outChunksTail ] = NULL;

    /* Since we have output the tail chunk, increase the tail, notifying
    a thread in case it is waiting for a free chunk.*/
    pthread_mutex_lock( &outChunksMutex );

    ++outChunksTail;

    if ( outChunksTail == outChunksCount )
      outChunksTail = 0;

    pthread_cond_signal( &outChunksAvailableCondition );

    /* If we empty the ring buffer out, mark it as empty */
    if ( outChunksTail == outChunksHead )
      outChunksTail = -1;
  }
}

static char helpText[] =
"\nbzip2smp, a parallelizing bzip2 implementation, version 1.0, 2-Dec-2005\n\n"
#ifdef NICKEDIT
"Nick Usage: bzip2smp [-123456789] [-p#] [filein]\n\n"
#endif
"Usage: bzip2smp [-123456789v] [-p#] [--ht|--no-ht] [--help]\n\n"
"The data is read from standard input and the result is output to standard ouput.\n"
"No other modes are supported.\n\n"
"Use -123456789 to specify the bzip2 block size (900k by default).\n\n"
"Use -v to increase verbosity (may be specified more than once).\n\n"
"Use -p# to specify the number of threads to use (default is the number of CPUs\n"
"present in system).\n\n"
"Please note that the use of hyperthreading generally degrades performance due\n"
"to the increased cache miss. Use --ht to make the program halve the number of\n"
"CPUs returned by the system to account to the presence of hyperthreading. "

#ifdef HT_DETECTION
                                                                           "By\n"
"default, this is attempted to be autodetected. Use -v to get a clue on how\n"
"the detection worked for you. In case the misdetection occured, use --no-ht.\n"
"There is no need to bother with the --ht flags if you specify the number of\n"
"threads (-p#) explicitly.\n"

#else

                                                                           "To\n"
"check out the number of CPUs detected, use -v key. There is no need to bother\n"
"with the --ht flag if you specify the number of threads (-p#) explicitly.\n"

#endif
;

int main( int argc, char *argv[] )
{
  int hyperthreading = -1;
  unsigned int threadsCount = 0;
  pthread_t dummy;
  char inputBuf[ BUFSZ ];
  char * inputBufPtr = inputBuf;
  size_t inputBufLeft = 0;
  bz_stream_state_bs savedState;
  int wasStateSaved = 0;
#ifdef NICKEDIT
  /*Nick adding read and write from files*/
  FILE * infile;
  if (argc < 4) {
      fprintf( stderr, helpText );
      return 1;
  }
#endif

  int x;

  /* Parse arguments */
  for( x = 1; x < argc; ++x )
  {
    if ( !strcmp( argv[ x ], "--help" ) )
    {
      fprintf( stderr, helpText );
      return 1;
    }

    if ( !strcmp( argv[ x ], "--ht" ) )
    {
      hyperthreading = 1;
      continue;
    }

    if ( !strcmp( argv[ x ], "--no-ht" ) )
    {
      hyperthreading = 0;
      continue;
    }

    if ( argv[ x ][ 0 ] == '-' )
    {
      char * p = argv[ x ] + 1;
      char c;

      do
      {
        c = *p++;

        if (  c >= '1' && c <= '9' )
        {
          blockSize100k = c - '1' + 1;
        }
        else
        if ( c == 'p' )
        {
          if ( sscanf( p, "%u", &threadsCount ) != 1 )
          {
            note( 0, "Error parsing the number of threads passed: %s.\n",
                  argv[ x ] + 2 );

            return 1;
          }
          c = 0; /* We don't support parsing past -p# */
        }
        else
        if ( c == 'v' )
        {
          ++verbosityLevel;
        }
        else
          break;
      } while( c );

      if ( !c )
        continue;
    }
#ifdef NICKEDIT
    break;
#endif
    note( 0, "Unrecognized option %s passed.\n", argv[ x ] );

    return 1;
  }

#ifdef NICKEDIT
#else
  if ( isatty ( fileno ( stdout ) ) )
  {
    note( 0, "Won't write compressed data to a terminal. Use --help to get help.\n" );
    return 1;
  }
#endif

  if ( !threadsCount )
  {
    x = sysconf( _SC_NPROCESSORS_ONLN );

    if ( x == -1 )
    {
      note( 0, "Failed to get the number of processors in the system: %s",
            strerror( errno ) );
  
      return 1;
    }

    threadsCount = x;

    if ( hyperthreading == -1 )
      hyperthreading = isHtPresent();

    if ( hyperthreading )
      threadsCount /= 2;
  
    note( 1, "CPUs detected: %d\n", threadsCount );
  }

  note( 1, "Threads to use: %d\n", threadsCount );

  /* Allocate the input chunks buffer. The actual input chunks are malloc()ed
  dynamically, since they are consumed randomly. The inChunksFree variable
  holds the number of chunks we can malloc(). */

  inChunksCount = threadsCount * 2;
  inChunksFree = inChunksCount;
  inChunks = ( bz_stream ** ) allocMem( sizeof( bz_stream * ) * inChunksCount );

  /* Ok, now allocate output chunks */

  outChunksCount = threadsCount * 2 + 1;
  outChunks = ( bz_stream ** ) allocMem( sizeof( bz_stream * ) *
                                         outChunksCount );

  for( x = outChunksCount; x--; )
    outChunks[ x ] = NULL;

  /* Output chunks allocated. */

  /* Create compression threads */
  for( x = 0; x < (int)threadsCount; ++x )
  {
    if ( pthread_create( &dummy, NULL, &threadFunction, NULL ) )
    {
      note( 0, "Failed to create compression thread(s).\n" );
      return 1;
    }
  }
  /* All compression threads started, and are waiting for some data to eat. */

  /* Start the writer thread */
  if ( pthread_create( &dummy, NULL, &writerThread, NULL ) )
  {
    note( 0, "Failed to create writer thread.\n" );
    return 1;
  }

  /* This main thread acts only as a data reader. The compression threads
  pick the data up and process it, and the writer thread writes down the
  results.*/
#ifdef NICKEDIT
    /*Nick adds file read and write*/
    printf("Opening file %s\n", argv[3]);
    infile = fopen(argv[3], "r");
    nullout = fopen("/dev/null", "w");
#endif

  for( ; ; )
  {
    bz_stream * strm = (bz_stream *)allocMem( sizeof( bz_stream ) );
    int lastBlock = 1;

    /* Get an input chunk to place input data to */
    
    pthread_mutex_lock( &inChunksMutex );
    if ( !inChunksFree )
    {
      /* No input chunks currently available, wait for one to become free */
      pthread_cond_wait( &inChunksFreeCondition, &inChunksMutex );
    }

    /* We just use it without advancing the head. Since we're the only one
    who store to the head, it is safe */
    inChunks[ inChunksHead ] = strm;

    pthread_mutex_unlock( &inChunksMutex );

    /* Init the compression */
    strm->bzalloc = NULL;
    strm->bzfree = NULL;
    strm->opaque = NULL;

    x = BZ2_bzCompressInit ( strm, blockSize100k, 0, 0, 1 );

    if ( x != BZ_OK )
    {
      note( 0, "bzip2 compress init returned error code %d\n", x );

      exit( 1 );
    }

    /* Any and all output data will be produced in a writer thread */
    strm->avail_out = 0;

    if ( wasStateSaved )
      BZ2_bzCompressRestoreState( strm, &savedState );


    /* Feed the compressor until it can't handle more */

    for( ; ; )
    {
      if ( inputBufLeft )
      {
        strm->next_in = inputBufPtr;
        strm->avail_in = inputBufLeft;
  
        x = BZ2_bzCompress( strm, BZ_RUN|BZ_STOP_BEFORE_BLOCKSORT );
  
        if ( x == BZ_STOPPED_BEFORE_BLOCKSORT )
        {
          /* The compressor can't handle no more. Leave the rest to the
          compression thread */
          inputBufLeft = strm->avail_in;
          inputBufPtr = strm->next_in;

          lastBlock = 0;
          break;
        }

        if ( x != BZ_RUN_OK )
        {
          note( 0, "bzip2 compressing routine (input) returned "
                   "error code %d\n", x );
          return 1;
        }
        else
          inputBufLeft = 0;
      }
      else
      {
#ifdef NICKEDIT
        inputBufLeft = fread( inputBuf, 1, sizeof( inputBuf ), infile);
#else
        inputBufLeft = fread( inputBuf, 1, sizeof( inputBuf ), stdin );
#endif

        if ( !inputBufLeft )
        {
          /* eof or error */

#ifdef NICKEDIT
          if ( feof( infile) )
#else
          if ( feof( stdin ) )
#endif
          {
            break;
          }
          else
          {
            /* 0 bytes read, and not on feof -- must be ferror */

#ifdef NICKEDIT
            note( 0, "error reading data from file\n" );
#else
            note( 0, "error reading data from stdin\n" );
#endif

            return 1;
          }
        }

        inputBufPtr = inputBuf;
      }
    }

    if ( !lastBlock )
    {
      /* Save the state for the future blocks */
      BZ2_bzCompressSaveStateBeforeBlocksort( strm, &savedState );
      wasStateSaved = 1;
    }
    else
    {
      /* Finish up the block, so it goes to the blocksort stage */
      strm->avail_in = 0;
      x = BZ2_bzCompress( strm, BZ_FINISH|BZ_STOP_BEFORE_BLOCKSORT );
      if ( x != BZ_STOPPED_BEFORE_BLOCKSORT )
      {
        note( 0, "Error: bzip2 compressing routine (finish) "
                 "returned error code %d\n", x );
        return 1;
      }
    }

    /* Save the info on whether more blocks will follow or not. This
    would be used to either run or finish the stream */
    /* We reuse avail_in. Way easier than putting our own superstruct. */
    strm->avail_in = lastBlock;

    /* We have made an input chunk on the input chunk ring buffer,
    now let's present it */

    pthread_mutex_lock( &inChunksMutex );

    if ( inChunksTail == -1 )
    {
      inChunksTail = inChunksHead;
      pthread_cond_signal( &inChunksAvailableCondition );
    }

    ++inChunksHead;

    if ( inChunksHead == inChunksCount )
      inChunksHead = 0;

    --inChunksFree;

    pthread_mutex_unlock( &inChunksMutex );

    if ( lastBlock )
      break;
  }

  /* Now that the data is all read, just wait until all in chunks are consumed
  and out chunks are written. */

  note( 2, "waiting for jobs to finish\n" );

  pthread_mutex_lock( &inChunksMutex );
  while ( inChunksFree != inChunksCount )
    pthread_cond_wait( &inChunksFreeCondition, &inChunksMutex );
  pthread_mutex_unlock( &inChunksMutex );

  pthread_mutex_lock( &outChunksMutex );
  while ( outChunksTail != -1 )
    pthread_cond_wait( &outChunksAvailableCondition, &outChunksMutex );
  pthread_mutex_unlock( &outChunksMutex );

  /* Done */

  /* No further semantic cleanup is required.
  Don't care freeing up the resources, the OS must do it anyway. */

  return 0;
}
