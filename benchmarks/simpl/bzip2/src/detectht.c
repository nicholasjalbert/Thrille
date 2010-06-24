#include <stdio.h>
#include <string.h>
#include "note.h"
#include "detectht.h"

#ifdef __linux__

static void noHt( void )
{
  note( 1, "No hyperthreading detected.\n" );
}

static char cpuInfoFile[] = "/proc/cpuinfo";
static char siblingsString[] = "siblings\t: ";
static char cpuCoresString[] = "cpu cores\t: ";

int isHtPresent( void )
{
  char buf[ 4096 ];
  char * str;
  FILE * f;

  f = fopen( cpuInfoFile, "r" );

  if ( f == NULL )
  {
    note( 1, "Failed to open %s, assuming there is no hyperthreading\n",
             cpuInfoFile );

    return 0;
  }

  buf[ fread( buf, 1, sizeof( buf ) - 1, f ) ] = 0;

  fclose( f );

  if ( ( str = strstr( buf, siblingsString ) ) )
  {
    int siblings;
    int cores;

    if ( sscanf( str + strlen( siblingsString ), "%d", & siblings ) != 1 )
    {
      note( 1, "Failed to read the number of siblings in %s, assuming no ht\n",
               cpuInfoFile );
      return 0;
    }

    str = strstr( buf, cpuCoresString );

    if ( str == NULL )
    {
      note( 1, "No cpu cores string found in %s, but it should be there, assuming no ht\n",
               cpuInfoFile );

      return 0;
    }

    if ( sscanf( str + strlen( cpuCoresString ), "%d", & cores ) != 1 )
    {
      note( 1, "Failed to read the number of cores in %s, assuming no ht\n",
               cpuInfoFile );
      return 0;
    }

    switch( siblings / cores )
    {
      case 1:
        noHt();
        return 0;

      case 2:
        note( 1, "Hyperthreading detected.\n" );
        return 1;

      default:
        note( 1, "Strange number of siblings (%d) in %s, but assuming ht is present\n",
                 siblings / cores, cpuInfoFile );

        return 1;
    }
  }
  else
  {
    noHt();
    return 0;
  }
}
#else
int isHtPresent( void )
{
  note( 1, "Assuming that hyperthreading is not present.\n" );
  return 0;
}
#endif
