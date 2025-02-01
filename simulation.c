#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h>
#include <omp.h>

#include "jeu.h"

int hmin = 4;
int hmax = 30;
int largeur = 20; // +INFINI; // 10; // +INFINI;

struct config Partie[MAXPARTIE];

int num_coup = 0;

int (*Est[10])(struct config *);
int nbEst;

int nbAlpha = 0;
int nbBeta = 0;

int prof = 0;
int profMax = 0;

FILE *f;

int main( int argc, char *argv[] )
{
   int n, i, j, score, stop, cout, tour, estMin, estMax, nbp;
   double max_s, max_e, min_s, min_e, max_t, min_t;

   struct config T[100], conf;

   Est[0] = estim1; // nbr de pieces
   Est[1] = estim2; // nbr de pieces, occupation, defense roi, roques
   Est[2] = estim3; // nbr de pieces, bruit
   Est[3] = estim4; // nbr de pieces, menace
   Est[4] = estim5; // nbr de pieces, occupation
   Est[5] = estim6; // (<25) 1, (>25) 4, (>35) 3
   Est[6] = estim7;
   nbEst = 7;

   estMax=1;
   estMin=1;

   max_t = 0;
   min_t = 0;

   printf("MAX(B) = %d, MIN(N) = %d\n\n", estMax, estMin);

   init_conf( &conf );
   for (i=0; i<MAXPARTIE; i++)
              copier( &conf, &Partie[i] );
  
   srandom( time(NULL) );


   stop = 0;
   tour = MAX;
   nbAlpha = nbBeta = 0;
   
   while ( !stop ) {
      copier( &conf, &Partie[num_coup % MAXPARTIE] );

      if ( tour == MAX ) {   // au tour de MAX ...
        max_s = omp_get_wtime();

        printf("MAX\n"); fflush(stdout);
        
        generer_succ(  &conf, MAX, T, &n );
        nbp = npieces( &conf );
        printf("ALTERNATIVES = %d\n", n); fflush(stdout);

        for (i=0; i<n; i++) 
            T[i].val = Est[estMax]( &T[i] );

        qsort(T, n, sizeof(struct config), confcmp321);
        if ( largeur < n ) n = largeur;

        score = -INFINI;
        j = -1;

        profMax = prof = 0;
        printf("DEPTH = ");
        for (i=0; i<n; i++) {
            cout = minmax_ab( &T[i], MIN, 0, score, +INFINI, largeur, estMax, nbp, &prof );
            if ( prof > profMax ) profMax = prof;
            printf("%d ", prof); fflush(stdout);
            
            if ( cout > score ) {
                score = cout;
                j = i;
            }
            if ( cout == 100 ) {
                // printf("WIN(B)"); fflush(stdout);
                break;
            }
        }
        if ( j != -1 ) {
            printf("\nCHOICE = %d, SCORE = %d, MAX_DEPTH = %d\n", j+1, score, profMax);
            printf("\n");
            copier( &T[j], &conf );
            conf.val = score;
        }
        else
            stop = 1;

        if (stop) {
        // printf("\nLOSE(B)\n");
        printf("\n\nWIN = MIN\n");
        }
        max_e = omp_get_wtime();
        max_t += (max_e - max_s);

      }  // if ( tour == MAX )

      else {  // donc tour == MIN 
        min_s = omp_get_wtime();

        printf("MIN\n"); fflush(stdout);
        
        generer_succ(  &conf, MIN, T, &n );
        nbp = npieces( &conf);
        printf("ALTERNATIVES = %d\n", n); fflush(stdout);

        for (i=0; i<n; i++) 
            T[i].val = Est[estMin]( &T[i] );

        qsort(T, n, sizeof(struct config), confcmp123);   // en ordre croissant des évaluations
        if ( largeur < n ) n = largeur;

        score = +INFINI;
        j = -1;
        profMax = prof = 0;
        printf("DEPTH = ");             
        for (i=0; i<n; i++) {
            cout = minmax_ab( &T[i], MAX, 0, -INFINI, score, largeur, estMin, nbp, &prof );
            if ( prof > profMax ) profMax = prof;

            printf("%d ", prof); fflush(stdout);
            if ( cout < score ) {
                score = cout;
                j = i;
            }
            if ( cout == -100 ) {
                // printf("WIN(N)"); fflush(stdout);
                break;
            }
        }
        if ( j != -1 ) {
            printf("\nCHOICE = %d, SCORE = %d, MAX_DEPTH = %d\n", j+1, score, profMax);
            printf("\n");
            copier( &T[j], &conf );
            conf.val = score;
        }
        else
            stop = 1;
                
        if (stop) {
        // printf("\nLOSE(N)\n");
        printf("\n\nWIN = MAX\n");
        }

        min_e = omp_get_wtime();
        min_t += (min_e - min_s);
      } // fin else // tour == MIN

    num_coup++;
    tour = ( tour == MIN ? MAX : MIN );

   } // fin while ( !stop )

   printf("\nREDUCTION = %d, ALPHA = %d, BETA = %d", nbAlpha+nbBeta, nbAlpha, nbBeta);
   printf("\nTIME = %.1f, MAX_T = %.1f, MIN_T = %.1f", (max_t+min_t), max_t, min_t);

   printf("\n");

   return 0;

} // fin de main