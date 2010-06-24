#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <math.h>
#include <errno.h>
#include <sys/time.h>       /* struct timeval */
#include <unistd.h>
#include <getopt.h>

#ifdef ERR1
void thrilleAssertC(int);
#endif

#ifndef _RADIXSORT_H
#define _RADIXSORT_H


#ifndef _SWARM_MULTICORE_H
#define _SWARM_MULTICORE_H


#ifndef _SWARM_H
#define _SWARM_H

#define HAVE_DLFCN_H 1
#define HAVE_GETOPT_H 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIBRT 1
#define HAVE_LOG2 
#define HAVE_MEMORY_H 1
#define HAVE_SELECT 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_SYSCONF 1
#define HAVE_SYS_RESOURCE_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_UNISTD_H 1
#define PACKAGE "swarm"
#define PACKAGE_BUGREPORT ""
#define PACKAGE_NAME "swarm"
#define PACKAGE_STRING "swarm 1.1"
#define PACKAGE_TARNAME "swarm"
#define PACKAGE_VERSION "1.1"
#define STDC_HEADERS 1
#define TIME_WITH_SYS_TIME 1
#define VERSION "1.1"
#define _RAND 

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */


/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */



enum reduce_tag {MAX, MIN, SUM, PROD, LAND, BAND, LOR, BOR, LXOR, BXOR};

extern  FILE* SWARM_outfile;

#define DEFAULT_PRI (PRI_OTHER_MIN+PRI_OTHER_MAX)/2
#define pNULL       (NULL)
#define SWARM_done(x) SWARM_Cleanup(x); pthread_exit(pNULL); return 0;

#define pthread_mb_np() asm("mb;")

#define MYTHREAD     (ti->mythread)
#define THREADED     uthread_info_t *ti
#define TH           ti
#define on_one_thread if (MYTHREAD == 0)
#define on_thread(k)  if (MYTHREAD == (k))
#define on_one        on_one_thread
#define BIND_TH       0
#define ERR_TH        0
#define CACHELOG      7
#define NOSHARE(x)    ((x)<<CACHELOG)

#define THARGC       (ti->argc)
#define THARGV       (ti->argv)
#define EXENAME      (ti->argv[0])

#define THRAND       (ti->rand)

/* max and min are predefined under stdlib.h in Visual Studio */
#undef min		
#undef max
#define max(a,b)   ((a) > (b) ? (a) : (b))
#define min(a,b)   ((a) < (b) ? (a) : (b))

#ifdef HAVE_LIBSPRNG
#define THSPRNG     (ti->randgen)
#else
#if defined(SOLARIS)
#else
typedef struct {
  long *randtbl;
  long *fptr;
  long *rptr;
  long *state;
  int rand_type;
  int rand_deg;
  int rand_sep;
  long *end_ptr;
} rrandom_info_t;
#endif
#endif


#ifndef MAXTHREADS
extern int          MAXTHREADS;
#endif
extern  int 	THREADS;

struct thread_inf {
  int                 mythread;   /* Thread number */
  int                 argc;      
  char              **argv;
  long                  m1;        /* used in pardo */
  long                  m2;        /* used in pardo */
  long                 blk;        /* used in pardo */
#ifdef HAVE_LIBSPRNG
  int                *randgen;    /* SPRNG generator pointer */
#else
#if defined(SOLARIS)
  unsigned short      xi[3];      /* used in rrandom_th nrand48() */
#else
  rrandom_info_t      rand;       /* used in rrandom_th */
#endif
#endif
  long                rbs;        /* used in random_bit, random_bitstring */
  short               rc;         /* used in random_bit, random_count */
  int                 udata;      /* User data */
};

typedef struct thread_inf uthread_info_t;

typedef int reduce_t;

extern uthread_info_t *uthread_info;

#define SWARM_partition_loop_across_threads(i,first,last,inc) \
        SWARM_pardo((i),(first),(last),(inc))     
#define pardo(i,first,last,inc) \
        SWARM_pardo((i),(first),(last),(inc))     
#define SWARM_pardo(i,first,last,inc)                    \
        if (((first)==0)&&((last)==THREADS)) {          \
            ti->m1 = MYTHREAD;                          \
	    ti->m2 = ti->m1 + 1;                        \
	} else {                                        \
            ti->blk = ((last)-(first))/THREADS;         \
            if (ti->blk == 0) {    		        \
              ti->m1  = (first)+MYTHREAD;               \
              ti->m2  = (ti->m1) + 1;                   \
              if ((ti->m1) >= (last))                   \
                 ti->m1 = ti->m2;                       \
	    }                                           \
            else {                                      \
              ti->m1  = (ti->blk) * MYTHREAD + (first); \
	      if (MYTHREAD < THREADS-1)                 \
	          ti->m2 = (ti->m1)+(ti->blk);          \
	      else                                      \
	          ti->m2 = (last);                      \
            }                                           \
	}                                               \
        if ((inc)>1) {                            \
            while ((ti->m1-(first)) % (inc) > 0)  \
                ti->m1 += 1;                      \
        }                                         \
	for (i=ti->m1 ; i<ti->m2 ; i+=(inc))

#define task_do(x)  (MYTHREAD == ((x) % THREADS))

#if 1
#define SWARM_Barrier() SWARM_Barrier_sync(TH)
#else
#define SWARM_Barrier() SWARM_Barrier_tree(TH)
#endif
  
void   	 SWARM_Barrier_tree(THREADED);
void   	 SWARM_Barrier_sync(THREADED);
void  	 *SWARM_malloc(int bytes, THREADED);
void   	 *SWARM_malloc_l(long bytes, THREADED);
void   	 SWARM_free(void *, THREADED);

typedef pthread_mutex_t SWARM_mutex_t;
typedef pthread_mutexattr_t SWARM_mutexattr_t;
int    SWARM_mutex_init(SWARM_mutex_t **, const SWARM_mutexattr_t *, THREADED);
int    SWARM_mutex_destroy(SWARM_mutex_t *, THREADED);
#define SWARM_mutex_lock(m)    pthread_mutex_lock(m)
#define SWARM_mutex_trylock(m) pthread_mutex_trylock(m)
#define SWARM_mutex_unlock(m)  pthread_mutex_unlock(m)

int     SWARM_Bcast_i(int    myval, THREADED);
long    SWARM_Bcast_l(long   myval, THREADED);
double  SWARM_Bcast_d(double myval, THREADED);
char    SWARM_Bcast_c(char   myval, THREADED);
int     *SWARM_Bcast_ip(int    *myval, THREADED);
long    *SWARM_Bcast_lp(long   *myval, THREADED);
double  *SWARM_Bcast_dp(double *myval, THREADED);
char    *SWARM_Bcast_cp(char   *myval, THREADED);
int     SWARM_Reduce_i(int    myval, reduce_t op, THREADED);
long    SWARM_Reduce_l(long   myval, reduce_t op, THREADED);
double  SWARM_Reduce_d(double myval, reduce_t op, THREADED);
int     SWARM_Scan_i(int    myval, reduce_t op, THREADED);
long    SWARM_Scan_l(long   myval, reduce_t op, THREADED);
double  SWARM_Scan_d(double myval, reduce_t op, THREADED);

void  SWARM_Init(int*, char***);
void  SWARM_Run(void *);
void  SWARM_Finalize(void);
void  SWARM_Cleanup(THREADED);

void assert_malloc(void *ptr);
double  get_seconds(void);

#ifndef HAVE_GETTIMEOFDAY
	struct timezone 
	{
		int tz_minuteswest; 
		int tz_dsttime;
	};
/* 	struct timeval */
/* 	{ */
/* 		long tv_sec; */
/* 		long tv_usec; */
/* 	}; */
	void gettimeofday(struct timeval* p, struct timezone* tz /* IGNORED */);
#endif

#define errprnt(msg)    { fprintf(stderr,"%s: %s\n",EXENAME,msg); exit(1); }

  
#endif


typedef struct _SWARM_MULTICORE_barrier {
  pthread_mutex_t lock;
  int n_clients;
  int n_waiting;
  int phase;
  pthread_cond_t wait_cv;
} *_SWARM_MULTICORE_barrier_t;

_SWARM_MULTICORE_barrier_t _SWARM_MULTICORE_barrier_init(int n_clients);
void          _SWARM_MULTICORE_barrier_destroy(_SWARM_MULTICORE_barrier_t nbarrier);
void          _SWARM_MULTICORE_barrier_wait(_SWARM_MULTICORE_barrier_t nbarrier);

typedef struct _SWARM_MULTICORE_reduce_i_s {
  pthread_mutex_t lock;
  int n_clients;
  int n_waiting;
  int phase;
  int sum;
  int result;
  pthread_cond_t wait_cv;
} *_SWARM_MULTICORE_reduce_i_t;

_SWARM_MULTICORE_reduce_i_t _SWARM_MULTICORE_reduce_init_i(int n_clients);
void           _SWARM_MULTICORE_reduce_destroy_i(_SWARM_MULTICORE_reduce_i_t nbarrier);
int            _SWARM_MULTICORE_reduce_i(_SWARM_MULTICORE_reduce_i_t nbarrier, int val, reduce_t op);

typedef struct _SWARM_MULTICORE_reduce_l_s {
  pthread_mutex_t lock;
  int n_clients;
  int n_waiting;
  int phase;
  long sum;
  long result;
  pthread_cond_t wait_cv;
} *_SWARM_MULTICORE_reduce_l_t;

_SWARM_MULTICORE_reduce_l_t _SWARM_MULTICORE_reduce_init_l(int n_clients);
void           _SWARM_MULTICORE_reduce_destroy_l(_SWARM_MULTICORE_reduce_l_t nbarrier);
long           _SWARM_MULTICORE_reduce_l(_SWARM_MULTICORE_reduce_l_t nbarrier, long val, reduce_t op);

typedef struct _SWARM_MULTICORE_reduce_d_s {
  pthread_mutex_t lock;
  int n_clients;
  int n_waiting;
  int phase;
  double sum;
  double result;
  pthread_cond_t wait_cv;
} *_SWARM_MULTICORE_reduce_d_t;

_SWARM_MULTICORE_reduce_d_t _SWARM_MULTICORE_reduce_init_d(int n_clients);
void           _SWARM_MULTICORE_reduce_destroy_d(_SWARM_MULTICORE_reduce_d_t nbarrier);
double         _SWARM_MULTICORE_reduce_d(_SWARM_MULTICORE_reduce_d_t nbarrier, double val, reduce_t op);

typedef struct _SWARM_MULTICORE_scan_i_s {
  pthread_mutex_t lock;
  int n_clients;
  int n_waiting;
  int phase;
  int *result;
  pthread_cond_t wait_cv;
} *_SWARM_MULTICORE_scan_i_t;

_SWARM_MULTICORE_scan_i_t   _SWARM_MULTICORE_scan_init_i(int n_clients);
void           _SWARM_MULTICORE_scan_destroy_i(_SWARM_MULTICORE_scan_i_t nbarrier);
int            _SWARM_MULTICORE_scan_i(_SWARM_MULTICORE_scan_i_t nbarrier, int val, reduce_t op,int th_index);


typedef struct _SWARM_MULTICORE_scan_l_s {
  pthread_mutex_t lock;
  int n_clients;
  int n_waiting;
  int phase;
  long *result;
  pthread_cond_t wait_cv;
} *_SWARM_MULTICORE_scan_l_t;

_SWARM_MULTICORE_scan_l_t   _SWARM_MULTICORE_scan_init_l(int n_clients);
void           _SWARM_MULTICORE_scan_destroy_l(_SWARM_MULTICORE_scan_l_t nbarrier);
long           _SWARM_MULTICORE_scan_l(_SWARM_MULTICORE_scan_l_t nbarrier, long val, reduce_t op,int th_index);

typedef struct _SWARM_MULTICORE_scan_d_s {
  pthread_mutex_t lock;
  int n_clients;
  int n_waiting;
  int phase;
  double *result;
  pthread_cond_t wait_cv;
} *_SWARM_MULTICORE_scan_d_t;

_SWARM_MULTICORE_scan_d_t   _SWARM_MULTICORE_scan_init_d(int n_clients);
void           _SWARM_MULTICORE_scan_destroy_d(_SWARM_MULTICORE_scan_d_t nbarrier);
double         _SWARM_MULTICORE_scan_d(_SWARM_MULTICORE_scan_d_t nbarrier, double val, reduce_t op,int th_index);

typedef struct _SWARM_MULTICORE_spin_barrier {
  int n_clients;
  pthread_mutex_t lock;
  int n_waiting;
  int phase;
} *_SWARM_MULTICORE_spin_barrier_t;

_SWARM_MULTICORE_spin_barrier_t _SWARM_MULTICORE_spin_barrier_init(int n_clients);
void           _SWARM_MULTICORE_spin_barrier_destroy(_SWARM_MULTICORE_spin_barrier_t sbarrier);
void           _SWARM_MULTICORE_spin_barrier_wait(_SWARM_MULTICORE_spin_barrier_t sbarrier);

#endif


void countsort_swarm(long q,
		     int *lKey,
		     int *lSorted,
		     int R,
		     int bitOff, int m,
		     THREADED);

#define radixsort_swarm(a,b,c,d)   radixsort_swarm_s3(a,b,c,d)

void radixsort_swarm_s3(long q,
			int *lKeys,
			int *lSorted,
			THREADED);
void radixsort_swarm_s2(long q,
			int *lKeys,
			int *lSorted,
			THREADED);
void radixsort20_swarm_s1(long q,
			  int *lKeys,
			  int *lSorted,
			  THREADED);
void radixsort20_swarm_s2(long q,
			  int *lKeys,
			  int *lSorted,
			  THREADED);

void radixsort_check(long q,
		     int *lSorted);

#endif






#define bits(x,k,j) ((x>>k) & ~(~0<<j))

/****************************************************/
void countsort_swarm(long q,
		     int *lKey,
		     int *lSorted,
		     int R,
		     int bitOff, int m,
		     THREADED)
/****************************************************/
/* R (range)      must be a multiple of SMPS */
/* q (elems/proc) must be a multiple of SMPS */
{
    register int
	j,
	k,
        last, temp,
	offset;
    
    int *myHisto,
        *mhp,
        *mps,
        *psHisto,
        *allHisto;

    long x;

    myHisto  = (int *)SWARM_malloc(THREADS*R*sizeof(int), TH);
    psHisto  = (int *)SWARM_malloc(THREADS*R*sizeof(int), TH);

    mhp = myHisto + MYTHREAD*R;

    for (k=0 ; k<R ; k++)
      mhp[k] = 0;
    
    pardo(x, 0, q, 1)
      mhp[bits(lKey[x],bitOff,m)]++;

    SWARM_Barrier();

    pardo(k, 0, R, 1) {
      last = psHisto[k] = myHisto[k];
      for (j=1 ; j<THREADS ; j++) {
	temp = psHisto[j*R + k] = last + myHisto[j*R +  k];
	last = temp;
      }
    }

    allHisto = psHisto+(THREADS-1)*R;

    SWARM_Barrier();
    
    offset = 0;

    mps = psHisto + (MYTHREAD*R);
    for (k=0 ; k<R ; k++) {
      mhp[k]  = (mps[k] - mhp[k]) + offset;
      offset += allHisto[k];
    }

    SWARM_Barrier();
    
    pardo(x, 0, q, 1) {
      j = bits(lKey[x],bitOff,m);
      lSorted[mhp[j]] = lKey[x];
      mhp[j]++;
    }

    SWARM_Barrier();

    SWARM_free(psHisto, TH);
    SWARM_free(myHisto, TH);
}

/****************************************************/
void radixsort_check(long q,
		     int *lSorted)
/****************************************************/
{
  long i;

  for (i=1; i<q ; i++) 
    if (lSorted[i-1] > lSorted[i]) {
      fprintf(stderr,
	      "ERROR: q:%ld lSorted[%6ld] > lSorted[%6ld] (%6d,%6d)\n",
	      q,i-1,i,lSorted[i-1],lSorted[i]);
#ifdef ERR1
      thrilleAssertC(0);
#endif
    }
}

/****************************************************/
void radixsort_swarm_s3(long q,
			int *lKeys,
			int *lSorted,
			THREADED)
/****************************************************/
{
  int *lTemp;

  lTemp = (int *)SWARM_malloc_l(q*sizeof(int), TH);
			
  countsort_swarm(q, lKeys,   lSorted, (1<<11),  0, 11, TH);
  countsort_swarm(q, lSorted, lTemp,   (1<<11), 11, 11, TH);
  countsort_swarm(q, lTemp,   lSorted, (1<<10), 22, 10, TH);

  SWARM_free(lTemp, TH);
}

/****************************************************/
void radixsort_swarm_s2(long q,
			int *lKeys,
			int *lSorted,
			THREADED)
/****************************************************/
{
  int *lTemp;

  lTemp = (int *)SWARM_malloc_l(q*sizeof(int), TH);
			
  countsort_swarm(q, lKeys,   lTemp,   (1<<16),  0, 16, TH);
  countsort_swarm(q, lTemp,   lSorted, (1<<16), 16, 16, TH);

  SWARM_free(lTemp, TH);
}

/****************************************************/
void radixsort20_swarm_s1(long q,
			  int *lKeys,
			  int *lSorted,
			  THREADED)
/****************************************************/
{
  countsort_swarm(q, lKeys,   lSorted, (1<<20),  0, 20, TH);
}

/****************************************************/
void radixsort20_swarm_s2(long q,
			     int *lKeys,
			     int *lSorted,
			     THREADED)
/****************************************************/
{
  int *lTemp;

  lTemp = (int *)SWARM_malloc_l(q*sizeof(int), TH);
			
  countsort_swarm(q, lKeys,   lTemp,   (1<<10),  0, 10, TH);
  countsort_swarm(q, lTemp,   lSorted, (1<<10), 10, 10, TH);

  SWARM_free(lTemp, TH);
}


#ifndef _NAS_R_H
#define _NAS_R_H

#define _NAS_BITS 19
#define _NAS_SEED 314159265.00
#define _NAS_MULT 1220703125.00

double   find_my_seed( long kn,       /* my processor rank, 0<=kn<=num procs */
                       long np,       /* np = num procs                      */
                       long nn,       /* total num of ran numbers, all procs */
                       double s,      /* Ran num seed, for ex.: 314159265.00 */
                       double a );    /* Ran num gen mult, try 1220703125.00 */

void	create_seq( double seed, double a , int q, int *arr);

void	create_seq_swarm( double seed, double a , int q, int *arr, THREADED);

void	create_seq_random_swarm( double seed, double a , int q, int *arr, THREADED);

#endif


static double R23, R46, T23, T46;

/*-----------------------------------------------------------------------------
|
|     FUNCTION RANDLC (X, A)
|
|   This routine returns a uniform pseudorandom double precision number in the
|   range (0, 1) by using the linear congruential generator
|
|   x_{k+1} = a x_k  (mod 2^46)
|
|   where 0 < x_k < 2^46 and 0 < a < 2^46.  This scheme generates 2^44 numbers
|   before repeating.  The argument A is the same as 'a' in the above formula,
|   and X is the same as x_0.  A and X must be odd double precision integers
|   in the range (1, 2^46).  The returned value RANDLC is normalized to be
|   between 0 and 1, i.e. RANDLC = 2^(-46) * x_1.  X is updated to contain
|   the new seed x_1, so that subsequent calls to RANDLC using the same
|   arguments will generate a continuous sequence.
|
|   This routine should produce the same results on any computer with at least
|   48 mantissa bits in double precision floating point data.  On Cray systems,
|   double precision should be disabled.
|
|   David H. Bailey     October 26, 1990
|
|      IMPLICIT DOUBLE PRECISION (A-H, O-Z)
|      SAVE KS, R23, R46, T23, T46
|      DATA KS/0/
|
|   If this is the first call to RANDLC, compute R23 = 2 ^ -23, R46 = 2 ^ -46,
|   T23 = 2 ^ 23, and T46 = 2 ^ 46.  These are computed in loops, rather than
|   by merely using the ** operator, in order to insure that the results are
|   exact on all systems.  This code assumes that 0.5D0 is represented exactly.
|
-----------------------------------------------------------------------------*/

/*****************************************************************/
/*************           R  A  N  D  L  C             ************/
/*************                                        ************/
/*************    portable random number generator    ************/
/*****************************************************************/

static double	randlc(double *X, double *A)
{
      static int        KS;
      static double	R23, R46, T23, T46;
      double		T1, T2, T3, T4;
      double		A1;
      double		A2;
      double		X1;
      double		X2;
      double		Z;
      int     		i, j;

      if (KS == 0) 
      {
        R23 = 1.0;
        R46 = 1.0;
        T23 = 1.0;
        T46 = 1.0;
    
        for (i=1; i<=23; i++)
        {
          R23 = 0.50 * R23;
          T23 = 2.0 * T23;
        }
        for (i=1; i<=46; i++)
        {
          R46 = 0.50 * R46;
          T46 = 2.0 * T46;
        }
        KS = 1;
      }
/*
|   Break A into two parts such that A = 2^23 * A1 + A2 and set X = N.
*/
      T1 = R23 * *A;
      j  = (int)T1;
      A1 = j;
      A2 = *A - T23 * A1;
/*
|   Break X into two parts such that X = 2^23 * X1 + X2, compute
|   Z = A1 * X2 + A2 * X1  (mod 2^23), and then
|   X = 2^23 * Z + A2 * X2  (mod 2^46).
*/
      T1 = R23 * *X;
      j  = (int)T1;
      X1 = j;
      X2 = *X - T23 * X1;
      T1 = A1 * X2 + A2 * X1;
      
      j  = (int)(R23 * T1);
      T2 = j;
      Z = T1 - T23 * T2;
      T3 = T23 * Z + A2 * X2;
      j  = (int)(R46 * T3);
      T4 = j;
      *X = T3 - T46 * T4;
      return(R46 * *X);
} 

static void init_nas() {

  int i;
  
  R23 = 1.0;
  R46 = 1.0;
  T23 = 1.0;
  T46 = 1.0;
    
  for (i=1; i<=23; i++) {
    R23 = 0.50 * R23;
    T23 = 2.0 * T23;
  }
  for (i=1; i<=46; i++) {
    R46 = 0.50 * R46;
    T46 = 2.0 * T46;
  }
}


static double	randlc_swarm(double *X, double *A)
{
  double		T1, T2, T3, T4;
  double		A1;
  double		A2;
  double		X1;
  double		X2;
  double		Z;
  int     		j;

  
/*
|   Break A into two parts such that A = 2^23 * A1 + A2 and set X = N.
*/

  T1 = R23 * *A;
  j  = (int)T1;
  A1 = j;
  A2 = *A - T23 * A1;

/*
|   Break X into two parts such that X = 2^23 * X1 + X2, compute
|   Z = A1 * X2 + A2 * X1  (mod 2^23), and then
|   X = 2^23 * Z + A2 * X2  (mod 2^46).
*/
  T1 = R23 * *X;
  j  = (int)T1;
  X1 = j;
  X2 = *X - T23 * X1;
  T1 = A1 * X2 + A2 * X1;
      
  j  = (int)(R23 * T1);
  T2 = j;
  Z = T1 - T23 * T2;
  T3 = T23 * Z + A2 * X2;
  j  = (int)(R46 * T3);
  T4 = j;
  *X = T3 - T46 * T4;
  return(R46 * *X);
} 



/*****************************************************************/
/************   F  I  N  D  _  M  Y  _  S  E  E  D    ************/
/************                                         ************/
/************ returns parallel random number seq seed ************/
/*****************************************************************/

/*
 * Seek to create a random number sequence of total length nn residing
 * on np number of processors.  Each processor will therefore have a 
 * subsequence of length nn/np.  This routine returns that random 
 * number which is the first random number for the subsequence belonging
 * to processor rank kn, and which is used as seed for proc kn ran # gen.
 */

double   find_my_seed( long kn,       /* my processor rank, 0<=kn<=num procs */
                       long np,       /* np = num procs                      */
                       long nn,       /* total num of ran numbers, all procs */
                       double s,      /* Ran num seed, for ex.: 314159265.00 */
                       double a )     /* Ran num gen mult, try 1220703125.00 */
{

  long i;

  double t1,t2,an;
  long mq,nq,kk,ik;



      nq = nn / np;

      for( mq=0; nq>1; mq++,nq/=2 )
          ;

      t1 = a;

      for( i=1; i<=mq; i++ )
        t2 = randlc( &t1, &t1 );

      an = t1;

      kk = kn;
      t1 = s;
      t2 = an;

      for( i=1; i<=100; i++ )
      {
        ik = kk / 2;
        if( 2 * ik !=  kk ) 
            randlc( &t1, &t2 );
        if( ik == 0 ) 
            break;
        randlc( &t2, &t2 );
        kk = ik;
      }

      return( t1 );

}




/*****************************************************************/
/*************      C  R  E  A  T  E  _  S  E  Q      ************/
/*****************************************************************/

/* q = n/p */

void	create_seq( double seed, double a , int q, int *arr)
{
	double		x;
	register int    i, k;

        k = (1<<_NAS_BITS)/4;

	for (i=0; i<q; i++)
	{
	    x  = randlc(&seed, &a);
	    x += randlc(&seed, &a);
    	    x += randlc(&seed, &a);
	    x += randlc(&seed, &a);  

            arr[i] = (int)(k*x);
	}
}



void	create_seq_swarm( double seed, double a , int q, int *arr, THREADED)
{
	double		x;
	register int    i, k;

        k = (1<<_NAS_BITS)/4;

	on_one
	  init_nas();
	SWARM_Barrier();
	
	for (i=0; i<q; i++)
	{
	    x  = randlc_swarm(&seed, &a);
	    x += randlc_swarm(&seed, &a);
    	    x += randlc_swarm(&seed, &a);
	    x += randlc_swarm(&seed, &a);

            arr[i] = (int)(k*x);
	}
}


void	create_seq_random_swarm( double seed, double a , int q, int *arr,
				  THREADED)
{
	register int    i, k;

        k = 2147483648; /* (1<<31) */

	on_one
	  init_nas();
	SWARM_Barrier();
	
	for (i=0; i<q; i++)
	  arr[i] = (int)(k * randlc_swarm(&seed, &a));
}





void create_input_random_swarm(int myN, int *x, THREADED) {
  create_seq_random_swarm( 317*(MYTHREAD+17),
			    _NAS_MULT,   
			    myN,
			    x,
			    TH);   
}


void create_input_nas_swarm(int n, int *x, THREADED) {
  register int tsize, mynum, thtot;

  tsize = n / THREADS;
  mynum = MYTHREAD;
  thtot = THREADS;
  
  create_seq_swarm( find_my_seed( mynum,
				   thtot,
				   (n >> 2),
				   _NAS_SEED,    /* Random number gen seed */
				   _NAS_MULT),   /* Random number gen mult */
		     _NAS_MULT,                  /* Random number gen mult */
		     tsize,
		     x+(tsize*MYTHREAD),
		     TH);   

}



#define  DEBUG  1
#define  INFO   1

#define MAXLEN  80

#define MAXTHREADS_DEFAULT 64

#ifndef MAXTHREADS
int     MAXTHREADS = MAXTHREADS_DEFAULT;
#endif /* MAXTHREADS */
#define DEFAULT_THREADS 2
int     THREADS;
uthread_info_t *uthread_info;
static pthread_t     *spawn_thread;

static int    _swarm_init=0;

#define MAX_GATHER 2

static int    _SWARM_bcast_i;
static long   _SWARM_bcast_l;
static double _SWARM_bcast_d;
static char   _SWARM_bcast_c;
static int    *_SWARM_bcast_ip;
static long   *_SWARM_bcast_lp;
static double *_SWARM_bcast_dp;
static char   *_SWARM_bcast_cp;



static _SWARM_MULTICORE_barrier_t nbar;

int    SWARM_mutex_init(SWARM_mutex_t **mutex, const SWARM_mutexattr_t *attr, THREADED) 
{
  int r;
  r = 0;
  *mutex = (SWARM_mutex_t *)SWARM_malloc(sizeof(SWARM_mutex_t), TH);
  on_one_thread {
    r = pthread_mutex_init(*mutex, attr);
  }
  r = SWARM_Bcast_i(r, TH);
  return r;
}

int    SWARM_mutex_destroy(SWARM_mutex_t *mutex, THREADED) {
  int r;
  r = 0;
  SWARM_Barrier();
  on_one_thread {
    r = pthread_mutex_destroy(mutex);
    free (mutex);
  }
  r = SWARM_Bcast_i(r, TH);
  return r;
}

static void SWARM_Barrier_sync_init(void) {
  nbar = _SWARM_MULTICORE_barrier_init(THREADS);
}

static void SWARM_Barrier_sync_destroy(void) {
  _SWARM_MULTICORE_barrier_destroy(nbar);
}

void SWARM_Barrier_sync(THREADED) {
#if defined(SOLARIS)&&defined(THREADMAP)
  _solaris_report(TH);
#endif /* defined(SOLARIS)&&defined(THREADMAP) */
  _SWARM_MULTICORE_barrier_wait(nbar);
}

static volatile int up_buf[NOSHARE(MAXTHREADS_DEFAULT)][2];
static volatile int down_buf[NOSHARE(MAXTHREADS_DEFAULT)];

static void
SWARM_Barrier_tree_init(void) {
  int i;

  for (i=0 ; i<THREADS ; i++) 
    up_buf[NOSHARE(i)][0] = up_buf[NOSHARE(i)][1] = down_buf[NOSHARE(i)] = 0;
  return;
}

static void
SWARM_Barrier_tree_destroy(void) { return; }

static void
SWARM_Barrier_tree_up(THREADED) {
    
  register int myidx  = MYTHREAD;
  register int parent = (MYTHREAD - 1) / 2;
  register int odd_child = 2 * MYTHREAD + 1;
  register int even_child = 2 * MYTHREAD + 2;
  register int parity = MYTHREAD & 1;

  if (MYTHREAD == 0) {
    if (THREADS > 2) {
      while (up_buf[NOSHARE(myidx)][0] == 0 ||
	     up_buf[NOSHARE(myidx)][1] == 0) ;
    }
    else if (THREADS == 2) {
	while (up_buf[NOSHARE(myidx)][1] == 0) ;
    }
  } 
  else 
    if (odd_child >= THREADS) 
      up_buf[NOSHARE(parent)][parity]++;
    else 
      if (even_child >= THREADS) {
	while (up_buf[NOSHARE(myidx)][1] == 0) ;
	up_buf[NOSHARE(parent)][parity]++;
      } 
      else {
	while (up_buf[NOSHARE(myidx)][0] == 0 ||
	       up_buf[NOSHARE(myidx)][1] == 0) ;
	up_buf[NOSHARE(parent)][parity]++;
      }

  up_buf[NOSHARE(myidx)][0] = up_buf[NOSHARE(myidx)][1] = 0;
#ifdef SOLARIS
  sun_mb_mi_np();
#endif
  return;
}

static void
SWARM_Barrier_tree_down(THREADED) {
    
  register int myidx  = MYTHREAD;
  register int left = 2 * MYTHREAD + 1;
  register int right = 2 * MYTHREAD + 2;

  if (MYTHREAD != 0) 
    while (down_buf[NOSHARE(myidx)] == 0) ;
  
  if (left < THREADS)
    down_buf[NOSHARE(left)]++;
  if (right < THREADS)
    down_buf[NOSHARE(right)]++;

  down_buf[NOSHARE(myidx)] = 0;
#ifdef SOLARIS
  sun_mb_mi_np();
#endif
  return;
}

void
SWARM_Barrier_tree(THREADED) {
  SWARM_Barrier_tree_up(TH);
  SWARM_Barrier_tree_down(TH);
  return;
}

void   *SWARM_malloc(int bytes, THREADED) {
  void *ptr;
  ptr=NULL;
  on_one_thread {
    ptr = malloc(bytes);
    assert_malloc(ptr);
  }
  return(SWARM_Bcast_cp(ptr, TH));
}

void   *SWARM_malloc_l(long bytes, THREADED) {
  void *ptr;
  ptr=NULL;
  on_one_thread {
    ptr = malloc(bytes);
    assert_malloc(ptr);
  }
  return(SWARM_Bcast_cp(ptr, TH));
}

void   SWARM_free(void *ptr, THREADED) {
  on_one_thread {
#ifdef SUNMMAP
    ptr = (void *)NULL;
#else
    free(ptr);
#endif
  }
}

int    SWARM_Bcast_i(int    myval, THREADED) {

  SWARM_Barrier();

  on_one_thread {
    _SWARM_bcast_i = myval;
  }

  SWARM_Barrier();
  return (_SWARM_bcast_i);
}

long   SWARM_Bcast_l(long   myval, THREADED) {

  SWARM_Barrier();

  on_one_thread {
    _SWARM_bcast_l = myval;
  }

  SWARM_Barrier();
  return (_SWARM_bcast_l);
}

double SWARM_Bcast_d(double myval, THREADED) {

  SWARM_Barrier();

  on_one_thread {
    _SWARM_bcast_d = myval;
  }

  SWARM_Barrier();
  return (_SWARM_bcast_d);
}

char   SWARM_Bcast_c(char   myval, THREADED) {

  SWARM_Barrier();

  on_one_thread {
    _SWARM_bcast_c = myval;
  }

  SWARM_Barrier();
  return (_SWARM_bcast_c);
}

int    *SWARM_Bcast_ip(int    *myval, THREADED) {

  SWARM_Barrier();

  on_one_thread {
    _SWARM_bcast_ip = myval;
  }

  SWARM_Barrier();
  return (_SWARM_bcast_ip);
}

long   *SWARM_Bcast_lp(long   *myval, THREADED) {

  SWARM_Barrier();

  on_one_thread {
    _SWARM_bcast_lp = myval;
  }

  SWARM_Barrier();
  return (_SWARM_bcast_lp);
}

double *SWARM_Bcast_dp(double *myval, THREADED) {

  SWARM_Barrier();

  on_one_thread {
    _SWARM_bcast_dp = myval;
  }

  SWARM_Barrier();
  return (_SWARM_bcast_dp);
}

char   *SWARM_Bcast_cp(char   *myval, THREADED) {

  SWARM_Barrier();

  on_one_thread {
    _SWARM_bcast_cp = myval;
  }

  SWARM_Barrier();
  return (_SWARM_bcast_cp);
}

static _SWARM_MULTICORE_reduce_d_t red_d;

double SWARM_Reduce_d(double myval, reduce_t op, THREADED) {
  return (_SWARM_MULTICORE_reduce_d(red_d, myval, op));
}


static _SWARM_MULTICORE_reduce_i_t red_i;

int SWARM_Reduce_i(int myval, reduce_t op, THREADED) {
  return (_SWARM_MULTICORE_reduce_i(red_i,myval,op));
}

static _SWARM_MULTICORE_reduce_l_t red_l;

long SWARM_Reduce_l(long myval, reduce_t op, THREADED) {
  return (_SWARM_MULTICORE_reduce_l(red_l,myval,op));
}

static _SWARM_MULTICORE_scan_i_t scan_i;

int SWARM_Scan_i(int myval, reduce_t op, THREADED) {
  return(_SWARM_MULTICORE_scan_i(scan_i,myval,op,MYTHREAD));
}

static _SWARM_MULTICORE_scan_l_t scan_l;

long SWARM_Scan_l(long myval, reduce_t op, THREADED) {
  return(_SWARM_MULTICORE_scan_l(scan_l,myval,op,MYTHREAD));
}

static _SWARM_MULTICORE_scan_d_t scan_d;

double SWARM_Scan_d(double myval, reduce_t op, THREADED) {
  return(_SWARM_MULTICORE_scan_d(scan_d,myval,op,MYTHREAD));
}

static void SWARM_print_help(char **argv)
{
     printf ("SWARM usage: %s [-t #threads] [-o outfile]\n", argv[0]);
     printf ("\t-t #threads    overrides the default number of threads\n");
     printf ("\t-o outfile     redirects standard output to outfile\n");
}

#define ERRSTRINGSIZE 512

static void SWARM_error (int lineno, const char *func, const char *format, ...)
{
    char msg[ERRSTRINGSIZE];

    va_list args;
    va_start(args, format);
    vsprintf(msg, format, args);

    fprintf (stderr, "SWARM_%s (line %d): %s\n", func, lineno, msg);
    fflush (stderr);

    SWARM_Finalize();
    
    exit (-1);

}


FILE *SWARM_outfile;
static char *SWARM_outfilename;


/*
 *
 * Parses command line arguments passed to it in argc and argv, 
 * using getopt(3c). Sets the following flags when it sees the
 * corresponding command line arguments:
 *
 *   -t  (overrides the default number of threads)
 *   -o  (redirect stdout)
 *   -h  (help)
 *
 * Returns 0 if there are no more arguments, or the index of the next
 * non-option argument.
 */
static int SWARM_read_arguments (int argc, char **argv)
{
     extern char *optarg;
     extern int optind;
     char *tail;
     int c, i;

     if (argv[0] == NULL) 
	  SWARM_error (__LINE__, "SWARM_read_arguments", 
		       "invalid argument array"); 
     
     if (argc < 1) return 0;
     
     opterr = 0;
     while ((c = getopt (argc, argv, "ht:o:")) != -1)
     {
	  switch (c)
	  {
	       
	  case 't':
	       i = (int)strtol (optarg, &tail, 0);
	       if (optarg == tail)
		    SWARM_error (__LINE__, "read_arguments", 
				 "invalid argument %s to option -t", optarg); 
	       if (i <= 0)
		    SWARM_error (__LINE__, "read_arguments", 
				 "# of threads must be greater than zero");
	       else
		    THREADS = i;
	       break;

	  case 'o':
	       SWARM_outfilename = strdup(optarg);
	       if ((SWARM_outfile = fopen (SWARM_outfilename, "w")) == NULL)
		    SWARM_error (__LINE__, "read_arguments", 
				 "unable to open outfile %s", SWARM_outfilename);
	       break;
	       
	  case 'h':
	       SWARM_print_help(argv);
	       exit(0);
	       break;

	  default:
		SWARM_error (__LINE__, "read_arguments", 
			     "`%c': unrecognized argument", c);

                break;
	   }
      }
    

     if (argv[optind] != NULL) return optind;
     else return 0;
}



static void
SWARM_get_args(int *argc, char* **argv) {
  int numarg = *argc;
  int done = 0;
  char
    *s,**argvv = *argv;
  char
    *outfilename = NULL;

  SWARM_outfile = stdout;

  while ((--numarg > 0) && !done)
    if ((*++argvv)[0] == '-')
      for (s=argvv[0]+1; *s != '\0'; s++) {
	if (*s == '-')
	  done = 1;
	else {
	  switch (*s) {
	  case 'o':
	    if (numarg <= 1) 
	      perror("output filename expected after -o (e.g. -o filename)");
	    numarg--;
	    outfilename = (char *)malloc(MAXLEN*sizeof(char));
	    strcpy(outfilename, *++argvv); 
	    SWARM_outfile = fopen(outfilename,"a+");
	    break;
	  case 't':
	    if (numarg <= 1) 
	      perror("number of threads per node expected after -t");
	    numarg--;
	    THREADS = atoi(*++argvv);

	    break;
	  case 'h':
	    fprintf(SWARM_outfile,"SWARM Options:\n");
	    fprintf(SWARM_outfile," -t <number of threads per node>\n");
	    fprintf(SWARM_outfile,"\n\n");
	    fflush(SWARM_outfile);
	    break;
	    /*	default: perror("illegal option");  */
	  }
	}
      }
  if (done) {
    *argc = numarg;
    *argv = ++argvv;
  }
  else {
    *argc = 0;
    *argv = NULL;
  }

  return;
}

#ifdef WIN32

// Avoids including windows.h
// Used for getting number of cores information on Windows machine
typedef struct _SYSTEM_INFO 
{  
	union 
	{    
		unsigned long dwOemId;    
		struct 
		{      
			unsigned short wProcessorArchitecture;      
			unsigned short wReserved;    
		};  
	};  
	unsigned long dwPageSize;  
	unsigned long lpMinimumApplicationAddress;  
	unsigned long lpMaximumApplicationAddress;  
	unsigned long* dwActiveProcessorMask;  
	unsigned long dwNumberOfProcessors;  
	unsigned long dwProcessorType;  
	unsigned long dwAllocationGranularity;  
	unsigned short wProcessorLevel;  
	unsigned short wProcessorRevision;
}SYSTEM_INFO;

void __stdcall GetSystemInfo(SYSTEM_INFO*);

#endif

static int SWARM_get_num_cores(void)
{
	int num_cores = DEFAULT_THREADS;
	
	#ifdef WIN32
		SYSTEM_INFO siSysInfo;
	#endif

	#ifdef WIN32 
 		GetSystemInfo(&siSysInfo); 
 		num_cores = siSysInfo.dwNumberOfProcessors;
	#elif HAVE_SYSCONF
		#ifdef _SC_NPROCESSORS_ONLN
    		num_cores = (int)sysconf(_SC_NPROCESSORS_ONLN);
		#endif
	#endif

	return num_cores;
}
	

#ifdef HAVE_PTHREAD_SCHED_SUPPORTED
static pthread_attr_t pattr;
#endif

void SWARM_Init(int *argc, char* **argv)
{
	
	int i;
	#ifdef HAVE_PTHREAD_SCHED_SUPPORTED
    int rc;
	#endif
    uthread_info_t *ti;
    int moreargs;

	#ifdef HAVE_PTHREAD_SCHED_SUPPORTED
    struct sched_param psched;
	#endif

    THREADS = SWARM_get_num_cores();
    THREADS = 4;

    SWARM_outfile  = stdout;
    SWARM_outfilename = NULL;
    
    moreargs = SWARM_read_arguments (*argc, *argv);

#if INFO
    fprintf(SWARM_outfile,"THREADS: %d\n", THREADS);
    fflush(SWARM_outfile);
#endif /*INFO */

    /*********************************/
    /* ON ONE THREAD INITIALIZATION  */
    /*********************************/

    SWARM_Barrier_sync_init();
    SWARM_Barrier_tree_init();

    red_i = _SWARM_MULTICORE_reduce_init_i(THREADS);
    red_l = _SWARM_MULTICORE_reduce_init_l(THREADS);
    red_d = _SWARM_MULTICORE_reduce_init_d(THREADS);

    scan_i = _SWARM_MULTICORE_scan_init_i(THREADS);
    scan_l = _SWARM_MULTICORE_scan_init_l(THREADS);
    scan_d = _SWARM_MULTICORE_scan_init_d(THREADS);

#ifdef HAVE_PTHREAD_SCHED_SUPPORTED

    /*********************/
    /* THREAD  SCHEDULER */
    /*********************/
    rc = pthread_attr_init(&pattr);
    if (rc)
      perror("pthread_attr_init");

    rc = pthread_attr_setschedpolicy(&pattr, SCHED_FIFO);
    if (rc)
      perror("pthread_attr_setschedpolicy");
    
    psched.sched_priority = sched_get_priority_max(SCHED_FIFO);
    
    rc = pthread_attr_setschedparam(&pattr, &psched);
    if (rc)
      perror("pthread_attr_isetschedparam");
    
    rc = pthread_attr_setinheritsched(&pattr, PTHREAD_EXPLICIT_SCHED);
    if (rc)
      perror("pthread_attr_setinheritsched");
    
#endif /* HAVE_PTHREAD_SCHED_SUPPORTED */

#if (defined(SOLARIS))
    rc = pthread_setconcurrency(THREADS+1);
    if (rc)
      perror("pthread_setconcurrency");
#endif /* defined(SOLARIS) */
    
    spawn_thread = (pthread_t *)malloc(NOSHARE(THREADS)*
				       sizeof(pthread_t));
    assert_malloc(spawn_thread);
    uthread_info = (uthread_info_t *)malloc(NOSHARE(THREADS)*
					  sizeof(uthread_info_t));
    assert_malloc(uthread_info);

    ti = uthread_info;

    for (i=0 ; i<THREADS ; i++) {
      ti->mythread   = i;

      if (moreargs > 0) 
      {
	   ti->argc       = (*argc)-moreargs;
	   ti->argv       = (*argv)+moreargs;
      }
      else 
      {
	   ti->argc       = 0;
	   ti->argv       = (char **)NULL;
      }

#ifdef RRANDOM
      SWARM_rrandom_init(ti);
#endif /* RRANDOM */

#ifdef HAVE_LIBSPRNG
      THSPRNG        = (int *)NULL;
#endif
      ti += NOSHARE(1);
    }

    _swarm_init=1;
}

void SWARM_Run(void *start_routine) 
{
     int i, rc;
     int *parg;
     uthread_info_t *ti;
     void *(*f)(void *);
     
     f = (void *(*)(void *))start_routine;
     
     if (!_swarm_init) 
     {
	  fprintf(stderr,"ERROR: SWARM_Init() not called\n");
	  perror("SWARM_Run cannot call SWARM_Init()");
     }
     
     ti = uthread_info;
     
     for (i=0 ; i<THREADS ; i++) 
     {
	  
	  rc = pthread_create(spawn_thread + NOSHARE(i),
#ifdef HAVE_PTHREAD_SCHED_SUPPORTED
			      &pattr,
#else
			      NULL,
#endif
			      f,
			      ti);

	  if (rc != 0)
	  {
	       switch (rc)
	       {
	       case EAGAIN: 
		    SWARM_error (__LINE__, "Run:pthread_create", 
				 "not enough resources to create another thread");
		    break;

	       case EINVAL:
		    SWARM_error (__LINE__, "Run:pthread_create", 
				 "invalid thread attributes");
		    break;

	       case EPERM:
		    SWARM_error (__LINE__, "Run:pthread_create", 
				 "insufficient permissions for setting scheduling parameters or policy ");
		    break;

	       default:
		    SWARM_error (__LINE__, "Run:pthread_create", "error code %d", rc);
	       }
	  }
			 
	  ti += NOSHARE(1);
     }
     
     for (i=0 ; i<THREADS ; i++)
     {
	  rc = pthread_join(spawn_thread[NOSHARE(i)], (void *)&parg);
	  if (rc != 0)
	  {
	       switch (rc)
	       {
	       case EINVAL:
		    SWARM_error (__LINE__, "Run:pthread_join", "specified thread is not joinable");
		    break;

	       case ESRCH:
		    SWARM_error (__LINE__, "Run:pthread_join", "cannot find thread");
		    break;

	       default:
		    SWARM_error (__LINE__, "Run:pthread_join", "error code %d", rc);

	       }
	  }
     }
}

void SWARM_Finalize(void) 
{
     
     /*********************************/
     /* ONE ONE THREAD DESTRUCTION    */
     /*********************************/
     
     _SWARM_MULTICORE_reduce_destroy_i(red_i);
     _SWARM_MULTICORE_reduce_destroy_l(red_l);
     _SWARM_MULTICORE_reduce_destroy_d(red_d);
     _SWARM_MULTICORE_scan_destroy_i(scan_i);
     _SWARM_MULTICORE_scan_destroy_l(scan_l);
     _SWARM_MULTICORE_scan_destroy_d(scan_d);
     
     SWARM_Barrier_sync_destroy();
     SWARM_Barrier_tree_destroy();
     
     free(uthread_info);
     free(spawn_thread);
     
     if (SWARM_outfile != NULL)
	  fclose(SWARM_outfile);
     if (SWARM_outfilename != NULL)
	  free(SWARM_outfilename);
}

void SWARM_Cleanup(THREADED) 
{
#ifdef RRANDOM
     SWARM_rrandom_destroy(TH);
#endif /* RRANDOM */
     return;
}

void assert_malloc(void *ptr) 
{
	if (ptr==NULL)
    	perror("ERROR: assert_malloc");
}

double get_seconds(void)
{
	struct timeval t;
  	struct timezone z;
  	gettimeofday(&t,&z);
  	return (double)t.tv_sec+((double)t.tv_usec/(double)1000000.0);
}


#ifndef HAVE_GETTIMEOFDAY
	/*  Windows does not support the gettimeofday. This fix from Anders
 	*  Carlsson was lifted from:
 	*  http://lists.gnu.org/archive/html/bug-gnu-chess/2004-01/msg00020.html
	*/

 	/* These are winbase.h definitions, but to avoid including
    	tons of Windows related stuff, it is reprinted here */

	typedef struct _FILETIME
	{
    	unsigned long dwLowDateTime;
    	unsigned long dwHighDateTime;
	} FILETIME;

	void __stdcall GetSystemTimeAsFileTime(FILETIME*);

	void gettimeofday(struct timeval* p, struct timezone* tz /* IGNORED */)
	{
   		union 
		{
   			long long ns100; /*time since 1 Jan 1601 in 100ns units */
   			FILETIME ft;
		} _now;

		GetSystemTimeAsFileTime( &(_now.ft) );
   		p->tv_usec=(long)((_now.ns100 / 10LL) % 1000000LL );
   		p->tv_sec= (long)((_now.ns100-(116444736000000000LL))/10000000LL);
   		return;
	}
#endif





_SWARM_MULTICORE_barrier_t _SWARM_MULTICORE_barrier_init(int n_clients) {
  _SWARM_MULTICORE_barrier_t nbarrier = (_SWARM_MULTICORE_barrier_t)
    malloc(sizeof(struct _SWARM_MULTICORE_barrier));
  assert_malloc(nbarrier);
  
  if (nbarrier != NULL) {
    nbarrier->n_clients = n_clients;
    nbarrier->n_waiting = 0;
    nbarrier->phase     = 0;
    pthread_mutex_init(&nbarrier->lock, NULL);
    pthread_cond_init(&nbarrier->wait_cv, NULL);
  }
  return(nbarrier);
}

void _SWARM_MULTICORE_barrier_destroy(_SWARM_MULTICORE_barrier_t nbarrier) {
  pthread_mutex_destroy(&nbarrier->lock);
  pthread_cond_destroy(&nbarrier->wait_cv);
  free(nbarrier);
}

void _SWARM_MULTICORE_barrier_wait(_SWARM_MULTICORE_barrier_t nbarrier) {
  int my_phase;

  pthread_mutex_lock(&nbarrier->lock);
  my_phase = nbarrier->phase;
  nbarrier->n_waiting++;
  if (nbarrier->n_waiting == nbarrier->n_clients) {
    nbarrier->n_waiting = 0;
    nbarrier->phase     = 1 - my_phase;
    pthread_cond_broadcast(&nbarrier->wait_cv);
  }
  while (nbarrier->phase == my_phase) {
    pthread_cond_wait(&nbarrier->wait_cv, &nbarrier->lock);
  }
  pthread_mutex_unlock(&nbarrier->lock);
}

_SWARM_MULTICORE_reduce_i_t _SWARM_MULTICORE_reduce_init_i(int n_clients) {
  _SWARM_MULTICORE_reduce_i_t nbarrier = (_SWARM_MULTICORE_reduce_i_t)
    malloc(sizeof(struct _SWARM_MULTICORE_reduce_i_s));
  assert_malloc(nbarrier);
  
  if (nbarrier != NULL) {
    nbarrier->n_clients = n_clients;
    nbarrier->n_waiting = 0;
    nbarrier->phase     = 0;
    nbarrier->sum       = 0;
    pthread_mutex_init(&nbarrier->lock, NULL);
    pthread_cond_init(&nbarrier->wait_cv, NULL);
  }
  return(nbarrier);
}

void _SWARM_MULTICORE_reduce_destroy_i(_SWARM_MULTICORE_reduce_i_t nbarrier) {
  pthread_mutex_destroy(&nbarrier->lock);
  pthread_cond_destroy(&nbarrier->wait_cv);
  free(nbarrier);
}

int _SWARM_MULTICORE_reduce_i(_SWARM_MULTICORE_reduce_i_t nbarrier, int val, reduce_t op) {
  int my_phase;

  pthread_mutex_lock(&nbarrier->lock);
  my_phase = nbarrier->phase;
  if (nbarrier->n_waiting==0) {
    nbarrier->sum = val;
  }
  else {
    switch (op) {
    case MIN:  nbarrier->sum  = min(nbarrier->sum,val);  break;
    case MAX : nbarrier->sum  = max(nbarrier->sum,val);  break;
    case SUM : nbarrier->sum += val;  break;
    default  : perror("ERROR: _SWARM_MULTICORE_reduce_i() Bad reduction operator");
    }
  }
  nbarrier->n_waiting++;
  if (nbarrier->n_waiting == nbarrier->n_clients) {
    nbarrier->result    = nbarrier->sum;
    nbarrier->sum       = 0;
    nbarrier->n_waiting = 0;
    nbarrier->phase     = 1 - my_phase;
    pthread_cond_broadcast(&nbarrier->wait_cv);
  }
  while (nbarrier->phase == my_phase) {
    pthread_cond_wait(&nbarrier->wait_cv, &nbarrier->lock);
  }
  pthread_mutex_unlock(&nbarrier->lock);
  return(nbarrier->result);
}

_SWARM_MULTICORE_reduce_l_t _SWARM_MULTICORE_reduce_init_l(int n_clients) {
  _SWARM_MULTICORE_reduce_l_t nbarrier = (_SWARM_MULTICORE_reduce_l_t)
    malloc(sizeof(struct _SWARM_MULTICORE_reduce_l_s));
  assert_malloc(nbarrier);

  if (nbarrier != NULL) {
    nbarrier->n_clients = n_clients;
    nbarrier->n_waiting = 0;
    nbarrier->phase     = 0;
    nbarrier->sum       = 0;
    pthread_mutex_init(&nbarrier->lock, NULL);
    pthread_cond_init(&nbarrier->wait_cv, NULL);
  }
  return(nbarrier);
}

void _SWARM_MULTICORE_reduce_destroy_l(_SWARM_MULTICORE_reduce_l_t nbarrier) {
  pthread_mutex_destroy(&nbarrier->lock);
  pthread_cond_destroy(&nbarrier->wait_cv);
  free(nbarrier);
}

long _SWARM_MULTICORE_reduce_l(_SWARM_MULTICORE_reduce_l_t nbarrier, long val, reduce_t op) {
  int my_phase;

  pthread_mutex_lock(&nbarrier->lock);
  my_phase = nbarrier->phase;
  if (nbarrier->n_waiting==0) {
    nbarrier->sum = val;
  }
  else {
    switch (op) {
    case MIN:  nbarrier->sum  = min(nbarrier->sum,val);  break;
    case MAX : nbarrier->sum  = max(nbarrier->sum,val);  break;
    case SUM : nbarrier->sum += val;  break;
    default  : perror("ERROR: _SWARM_MULTICORE_reduce_l() Bad reduction operator");
    }
  }
  nbarrier->n_waiting++;
  if (nbarrier->n_waiting == nbarrier->n_clients) {
    nbarrier->result    = nbarrier->sum;
    nbarrier->sum       = 0;
    nbarrier->n_waiting = 0;
    nbarrier->phase     = 1 - my_phase;
    pthread_cond_broadcast(&nbarrier->wait_cv);
  }
  while (nbarrier->phase == my_phase) {
    pthread_cond_wait(&nbarrier->wait_cv, &nbarrier->lock);
  }
  pthread_mutex_unlock(&nbarrier->lock);
  return(nbarrier->result);
}

_SWARM_MULTICORE_reduce_d_t _SWARM_MULTICORE_reduce_init_d(int n_clients) {
  _SWARM_MULTICORE_reduce_d_t nbarrier = (_SWARM_MULTICORE_reduce_d_t)
    malloc(sizeof(struct _SWARM_MULTICORE_reduce_d_s));
  assert_malloc(nbarrier);

  if (nbarrier != NULL) {
    nbarrier->n_clients = n_clients;
    nbarrier->n_waiting = 0;
    nbarrier->phase     = 0;
    nbarrier->sum       = 0.000001;
    pthread_mutex_init(&nbarrier->lock, NULL);
    pthread_cond_init(&nbarrier->wait_cv, NULL);
  }
  return(nbarrier);
}

void _SWARM_MULTICORE_reduce_destroy_d(_SWARM_MULTICORE_reduce_d_t nbarrier) {
  pthread_mutex_destroy(&nbarrier->lock);
  pthread_cond_destroy(&nbarrier->wait_cv);
  free(nbarrier);
}

double _SWARM_MULTICORE_reduce_d(_SWARM_MULTICORE_reduce_d_t nbarrier, double val, reduce_t op) {
  int my_phase;

  pthread_mutex_lock(&nbarrier->lock);
  my_phase = nbarrier->phase;
  if (nbarrier->n_waiting==0) {
    nbarrier->sum = val;
  }
  else {
    switch (op) {
    case MIN:  nbarrier->sum  = min(nbarrier->sum,val);  break;
    case MAX : nbarrier->sum  = max(nbarrier->sum,val);  break;
    case SUM : nbarrier->sum += val;  break;
    default  : perror("ERROR: _SWARM_MULTICORE_reduce_i() Bad reduction operator");
    }
  }
  nbarrier->n_waiting++;
  if (nbarrier->n_waiting == nbarrier->n_clients) {
    nbarrier->result    = nbarrier->sum;
    nbarrier->sum       = 0.0;
    nbarrier->n_waiting = 0;
    nbarrier->phase     = 1 - my_phase;
    pthread_cond_broadcast(&nbarrier->wait_cv);
  }
  while (nbarrier->phase == my_phase) {
    pthread_cond_wait(&nbarrier->wait_cv, &nbarrier->lock);
  }
  pthread_mutex_unlock(&nbarrier->lock);
  return(nbarrier->result);
}

_SWARM_MULTICORE_scan_i_t _SWARM_MULTICORE_scan_init_i(int n_clients) {
  _SWARM_MULTICORE_scan_i_t nbarrier = (_SWARM_MULTICORE_scan_i_t)
    malloc(sizeof(struct _SWARM_MULTICORE_scan_i_s));
  assert_malloc(nbarrier);

  if (nbarrier != NULL) {
    nbarrier->n_clients = n_clients;
    nbarrier->n_waiting = 0;
    nbarrier->phase     = 0;
    nbarrier->result    = (int *)malloc(n_clients*sizeof(int));
    assert_malloc(nbarrier->result);
    pthread_mutex_init(&nbarrier->lock, NULL);
    pthread_cond_init(&nbarrier->wait_cv, NULL);
  }
  return(nbarrier);
}

void _SWARM_MULTICORE_scan_destroy_i(_SWARM_MULTICORE_scan_i_t nbarrier) {
  pthread_mutex_destroy(&nbarrier->lock);
  pthread_cond_destroy(&nbarrier->wait_cv);
  free(nbarrier->result);
  free(nbarrier);
}

int _SWARM_MULTICORE_scan_i(_SWARM_MULTICORE_scan_i_t nbarrier, int val, reduce_t op,int th_index) {
  int my_phase,i,temp;

  pthread_mutex_lock(&nbarrier->lock);
  my_phase = nbarrier->phase;
  nbarrier->result[th_index]  = val;

  nbarrier->n_waiting++;
  if (nbarrier->n_waiting == nbarrier->n_clients) { /* get the prefix result in result array*/
    switch (op) {
    case MIN : temp = nbarrier->result[0];
      for(i = 1; i < nbarrier->n_clients;i++) {
         temp  = min(nbarrier->result[i],temp);
         nbarrier->result[i] = temp;
      }  
      break;
    case MAX : temp = nbarrier->result[0];
      for(i = 1; i < nbarrier->n_clients;i++) {
         temp  = max(nbarrier->result[i],temp);
         nbarrier->result[i] = temp;
      }  
      break;
    case SUM : 
      for(i = 1; i < nbarrier->n_clients;i++) 
         nbarrier->result[i] += nbarrier->result[i-1];
      break;
    default  : perror("ERROR: _SWARM_MULTICORE_scan_i() Bad reduction operator");
    }
    nbarrier->n_waiting = 0;
    nbarrier->phase     = 1 - my_phase;
    pthread_cond_broadcast(&nbarrier->wait_cv);
  }
  while (nbarrier->phase == my_phase) {
    pthread_cond_wait(&nbarrier->wait_cv, &nbarrier->lock);
  }
  pthread_mutex_unlock(&nbarrier->lock);
  return(nbarrier->result[th_index]);
}

_SWARM_MULTICORE_scan_l_t _SWARM_MULTICORE_scan_init_l(int n_clients) {
  _SWARM_MULTICORE_scan_l_t nbarrier = (_SWARM_MULTICORE_scan_l_t)
    malloc(sizeof(struct _SWARM_MULTICORE_scan_l_s));
  assert_malloc(nbarrier);

  if (nbarrier != NULL) {
    nbarrier->n_clients = n_clients;
    nbarrier->n_waiting = 0;
    nbarrier->phase     = 0;
    nbarrier->result    = (long *)malloc(n_clients*sizeof(long));
    assert_malloc(nbarrier->result);
    pthread_mutex_init(&nbarrier->lock, NULL);
    pthread_cond_init(&nbarrier->wait_cv, NULL);
  }
  return(nbarrier);
}

void _SWARM_MULTICORE_scan_destroy_l(_SWARM_MULTICORE_scan_l_t nbarrier) {
  pthread_mutex_destroy(&nbarrier->lock);
  pthread_cond_destroy(&nbarrier->wait_cv);
  free(nbarrier->result);
  free(nbarrier);
}

long _SWARM_MULTICORE_scan_l(_SWARM_MULTICORE_scan_l_t nbarrier, long val, reduce_t op, int th_index) {
  int my_phase,i;
  long temp;

  pthread_mutex_lock(&nbarrier->lock);
  my_phase = nbarrier->phase;
  nbarrier->result[th_index] = val; 

  nbarrier->n_waiting++;
  if (nbarrier->n_waiting == nbarrier->n_clients) {/*get the prefix*/
    switch (op) {
    case MIN : temp = nbarrier->result[0];
      for(i = 1; i < nbarrier->n_clients;i++) {
         temp  = min(nbarrier->result[i],temp);
         nbarrier->result[i] = temp;
      }  
      break;
    case MAX : temp = nbarrier->result[0];
      for(i = 1; i < nbarrier->n_clients;i++) {
         temp  = max(nbarrier->result[i],temp);
         nbarrier->result[i] = temp;
      }  
      break;
    case SUM : 
      for(i = 1; i < nbarrier->n_clients;i++) 
         nbarrier->result[i] += nbarrier->result[i-1];
      break;
    default  : perror("ERROR: _SWARM_MULTICORE_scan_i() Bad reduction operator");
    }
    nbarrier->n_waiting = 0;
    nbarrier->phase     = 1 - my_phase;
    pthread_cond_broadcast(&nbarrier->wait_cv);
  }
  while (nbarrier->phase == my_phase) {
    pthread_cond_wait(&nbarrier->wait_cv, &nbarrier->lock);
  }
  pthread_mutex_unlock(&nbarrier->lock);
  return(nbarrier->result[th_index]);
}

_SWARM_MULTICORE_scan_d_t _SWARM_MULTICORE_scan_init_d(int n_clients) {
  _SWARM_MULTICORE_scan_d_t nbarrier = (_SWARM_MULTICORE_scan_d_t)
    malloc(sizeof(struct _SWARM_MULTICORE_scan_d_s));
  assert_malloc(nbarrier);

  if (nbarrier != NULL) {
    nbarrier->n_clients = n_clients;
    nbarrier->n_waiting = 0;
    nbarrier->phase     = 0;
    nbarrier->result    = (double *)malloc(n_clients*sizeof(double));
    assert_malloc(nbarrier->result);
    pthread_mutex_init(&nbarrier->lock, NULL);
    pthread_cond_init(&nbarrier->wait_cv, NULL);
  }
  return(nbarrier);
}

void _SWARM_MULTICORE_scan_destroy_d(_SWARM_MULTICORE_scan_d_t nbarrier) {

  pthread_mutex_destroy(&nbarrier->lock);
  pthread_cond_destroy(&nbarrier->wait_cv);
  free(nbarrier->result);
  free(nbarrier);
}

double _SWARM_MULTICORE_scan_d(_SWARM_MULTICORE_scan_d_t nbarrier, double val, reduce_t op,int th_index) {
  int my_phase,i;
  double temp;

  pthread_mutex_lock(&nbarrier->lock);
  my_phase = nbarrier->phase;
  nbarrier->result[th_index] = val;
  nbarrier->n_waiting++;
  if (nbarrier->n_waiting == nbarrier->n_clients) {
    switch (op) {
    case MIN : temp = nbarrier->result[0];
      for(i = 1; i < nbarrier->n_clients;i++) {
         temp  = min(nbarrier->result[i],temp);
         nbarrier->result[i] = temp;
      }  
      break;
    case MAX : temp = nbarrier->result[0];
      for(i = 1; i < nbarrier->n_clients;i++) {
         temp  = max(nbarrier->result[i],temp);
         nbarrier->result[i] = temp;
      }  
      break;
    case SUM : 
      for(i = 1; i < nbarrier->n_clients;i++) 
         nbarrier->result[i] += nbarrier->result[i-1];
      break;
    default  : perror("ERROR: _SWARM_MULTICORE_scan_i() Bad reduction operator");
    }
    nbarrier->n_waiting = 0;
    nbarrier->phase     = 1 - my_phase;
    pthread_cond_broadcast(&nbarrier->wait_cv);
  }
  while (nbarrier->phase == my_phase) {
    pthread_cond_wait(&nbarrier->wait_cv, &nbarrier->lock);
  }
  pthread_mutex_unlock(&nbarrier->lock);
  return(nbarrier->result[th_index]);
}



_SWARM_MULTICORE_spin_barrier_t _SWARM_MULTICORE_spin_barrier_init(int n_clients) {
  _SWARM_MULTICORE_spin_barrier_t sbarrier = (_SWARM_MULTICORE_spin_barrier_t)
    malloc(sizeof(struct _SWARM_MULTICORE_spin_barrier));
  assert_malloc(sbarrier);

  if (sbarrier != NULL) {
    sbarrier->n_clients = n_clients;
    sbarrier->n_waiting = 0;
    sbarrier->phase     = 0;
    pthread_mutex_init(&sbarrier->lock, NULL);
  }
  return(sbarrier);
}

void _SWARM_MULTICORE_spin_barrier_destroy(_SWARM_MULTICORE_spin_barrier_t sbarrier) {
  pthread_mutex_destroy(&sbarrier->lock);
  free(sbarrier);
}

void _SWARM_MULTICORE_spin_barrier_wait(_SWARM_MULTICORE_spin_barrier_t sbarrier) {
  int my_phase;

  while (pthread_mutex_trylock(&sbarrier->lock) == EBUSY) ;
  my_phase = sbarrier->phase;
  sbarrier->n_waiting++;
  if (sbarrier->n_waiting == sbarrier->n_clients) {
    sbarrier->n_waiting = 0;
    sbarrier->phase     = 1 - my_phase;
  }
  pthread_mutex_unlock(&sbarrier->lock);

  while (sbarrier->phase == my_phase) ;
}









#define DEBUG              0
#define TIMING             1

#define TEST_WIDE          0

#if TEST_WIDE
#define ARR_SIZE_SORT   ((long)pow(2.0,33))
#else
#define ARR_SIZE_SORT   (1<<14)
#endif
#define MIN_TIME       0.000001



static void test_radixsort_swarm(long N1, THREADED) 
{
  int *inArr, *outArr;

#if TIMING
  double secs, tsec;
#endif

  inArr  = (int *)SWARM_malloc_l(N1 * sizeof(int), TH);
  outArr = (int *)SWARM_malloc_l(N1 * sizeof(int), TH);

  create_input_nas_swarm(N1, inArr, TH);

#if TIMING
  SWARM_Barrier();
  secs = get_seconds();
#endif

  radixsort_swarm(N1, inArr, outArr, TH);

#if TIMING
  secs = get_seconds() - secs;
  secs = max(secs,MIN_TIME);
  tsec = SWARM_Reduce_d(secs,MAX, TH);
  on_one {
  }
#endif

  SWARM_Barrier();

  on_one radixsort_check(N1,  outArr);

  SWARM_Barrier();

  SWARM_free(outArr, TH);
  SWARM_free(inArr, TH);
}



static void *swarmtest(THREADED)
{
  long  i;

/* #if TEST_WIDE */
#define TEST_INC (i<<2)
/* #else */
/* #define TEST_INC (i+=8) */
/* #endif */
  

  SWARM_Barrier();
  
  //  for (i = ((long)1<<12) ; i<=ARR_SIZE_SORT ; i = TEST_INC)
  i = (long)1 << 4;
  test_radixsort_swarm(i, TH);

  SWARM_Barrier();

  /*******************************/
  /* End of program              */
  /*******************************/
  
  SWARM_done(TH);
}

int main(int argc, char **argv) 
{
  SWARM_Init(&argc,&argv);
  SWARM_Run((void *)swarmtest);
  SWARM_Finalize();
  return 0;
}



