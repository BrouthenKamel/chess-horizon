/********************************************************************************
*********************************************************************************

Programme de démonstration de MinMax avec élagage alpha/bêta pour le jeu d'échecs
Hidouci / CRP 2CS-SID / ESI 2024/2025

Programme princial : main.c

Le programme comporte une interface textuelle permettant 2 types de jeu:
  - utilisateur contre ordinateur
  - ordinateur contre lui même 

Cette version inclus :
  - Traitement de "l'effet horizon" en adaptant la profondeur d'exploration en  
    fonction de la stabilité des configuration
  - Maximiser les coupes alpha/beta à l'aide du "Move Ordering" 
  - Influence des fonctions d'estimation                        
                                                                   
*********************************************************************************
********************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h>

#include "jeu.h"


/************************/
/* Variables Globales : */
/************************/

// la hauteur minimale et la hauteur maximale dans les explorations minmax
int hmin = 4;   // la borne inférieure de la "frontière d'exploration"
int hmax = 30;  // la borne supérieure de la "frontière d'exploration"


// Tableau de config pour garder la trace des conf déjà visitées
struct config Partie[MAXPARTIE];

// Fichier pour sauvegarder l'historique des parties
FILE *f;

// compteur de coups effectués
int num_coup = 0;          

// profondeur initiale d'exploration préliminaire avant le tri des alternatives
//int h0 = 0;

// tableau de pointeurs de fonctions (les fonctions d'estimations disponibles)
int (*Est[10])(struct config *);

// nb de fonctions d'estimation dans le tableau précédent
int nbEst;



// pour statistques sur le nombre de coupes effectuées
int nbAlpha = 0;
int nbBeta = 0;

// pour compter la profondeur maximale atteinte lors de l'évaluation d'une conf
int prof = 0;
int profMax = 0;


/*******************************************/
/*********** Programme principal  **********/
/*******************************************/

int main( int argc, char *argv[] )
{

   int n, i, j, score, stop, cout, largeur, tour, estMin, estMax, nbp;

   int sx, dx, cout2, legal;
   int cmin, cmax;
   int typeExec, refaire;

   char coup[20] = "";
   char nomf[20];  // nom du fichier de sauvegarde
   char ch[100];
   char sy, dy;

   struct config T[100], conf, conf1;

   // initialiser le tableau des fonctions d'estimation
   Est[0] = estim1;
   Est[1] = estim2;
   Est[2] = estim3;
   Est[3] = estim4;
   Est[4] = estim5;
   Est[5] = estim6;
   Est[6] = estim7;
   // Nombre de fonctions d'estimation disponibles
   nbEst = 7;     

   // mode affichage
   int affich_emoji = 1;   

   // Choix du type d'exécution (pc-contre-pc ou user-contre-pc) ...
   printf("Type de parties (B:Blancs  N:Noirs) :\n");
   printf("1- PC(B)   contre PC(N)\n");
   printf("2- USER(N) contre PC(B)\n");
   printf("3- USER(B) contre PC(N)\n");
   printf("\tChoix : ");
   scanf(" %d", &typeExec);
   if (typeExec != 2 && typeExec != 3) typeExec = 1;

   // Choix des fonctions d'estimation à utiliser ...
   do {
      printf("\nLes fonctions d'estimations disponibles sont:\n");
      printf("1- basée uniquement sur le nombre de pièces\n");
      printf("2- basée sur le nb de pieces, l'occupation, la défense du roi et les roques\n");
      printf("3- basée sur le nb de pièces et une perturbation aléatoire\n");
      printf("4- basée sur le nb de pieces et les menaces\n");
      printf("5- basée sur le nb de pieces et l'occupation\n");
      printf("6- basée sur une combinaisant de 3 estimations: (2 -> 5 -> 4)\n");
      printf("7- une fonction d'estimation aléatoire (ou à définir) \n\n");
      if (typeExec != 3) {
         printf("Donnez la fonction d'estimation utilisée par le PC pour le joueur B : ");
         scanf(" %d", &estMax);
      }
      else
         estMax = 7;
      if (typeExec != 2) {
         printf("Donnez la fonction d'estimation utilisée par le PC pour le joueur N : ");
         scanf(" %d", &estMin);
      }
      else
         estMin = 7;
   }
   while ( estMax < 1 || estMax > nbEst || estMin < 1 || estMin > nbEst );

   estMax--; 
   estMin--;

   printf("\n--- Estimation_pour_Blancs = %d \t Estimation_pour_Noirs = %d ---\n", \
           (typeExec != 3 ? estMax+1 : 0), (typeExec != 2 ? estMin+1 : 0) );
     
   printf("\n'B' : joueur maximisant et 'N' : joueur minimisant\n");
   if (typeExec == 1)
      printf("\nPC 'B' contre PC 'N'\n\n");
   if (typeExec == 2)
      printf("\nUSER 'N' contre PC 'B'\n\n");
   if (typeExec == 3)
      printf("\nUSER 'B' contre PC 'N'\n\n");

   printf("Paramètres de MinMax\n");

   printf("Profondeur minimale d'exploration (par exemple %d) : ", hmin);
   scanf(" %d", &hmin);

   printf("Profondeur miximale d'exploration (par exemple %d) : ", hmax);
   scanf(" %d", &hmax);

   printf("Nombre d'alternatives maximum à considérer dans chaque coup (0 pour +infini) : ");
   scanf(" %d", &largeur);
   if ( largeur == 0 ) largeur = +INFINI;

   // Initialise la configuration de départ
   init_conf( &conf );
   for (i=0; i<MAXPARTIE; i++)
              copier( &conf, &Partie[i] );

   num_coup = 0;
  
   // initialise le générateur de nombre aléatoire pour la fonction estim3(...) si elle est utilisée
   srandom( time(NULL) ); 

   // sauter la fin de ligne des lectures précédentes
   while ((i = getchar()) != '\n' && i != EOF) { } 

   printf("\nNom du fichier texte où sera sauvegarder la partie : ");
   fgets(ch, 20, stdin);
   sscanf(ch," %s", nomf);
   f = fopen(nomf, "w");
   
   fprintf(f, "--- Estimation_pour_Blancs = %d \t Estimation_pour_Noirs = %d ---\n", \
           (typeExec != 3 ? estMax+1 : 0), (typeExec != 2 ? estMin+1 : 0) );

   printf("\n---------------------------------------------\n\n");

   // Boucle principale du déroulement d'une partie ...
   stop = 0;
   tour = MAX;            // le joueur MAX commence en premier
   nbAlpha = nbBeta = 0;  // les compteurs de coupes alpha et beta
   
   // boucle principale d'une partie de jeu ...
   while ( !stop ) {

      affich( &conf, coup, num_coup, affich_emoji );
      copier( &conf, &Partie[num_coup % MAXPARTIE] );        // rajouter conf au tableau 'Partie'
      sauvConf( &conf );
      refaire = 0;  // indicateur de coup illegal, pour refaire le mouvement

      if ( tour == MAX ) {   // au tour de MAX ...

         if (typeExec == 3) {   // c-a-d MAX ===> USER
            printf("Au tour du joueur maximisant USER 'B'\n"); 
            // récupérer le coup du joueur ...
            do {
               printf("Donnez srcY srcX destY destX (par exemple d2d3) : ");
               fgets(ch, 100, stdin);
               i = sscanf(ch, " %c %d %c %d", &sy, &sx, &dy, &dx );
               if (i != 4) 
                  printf("Lecture incorrecte, recommencer ...\n");
            }
            while ( i != 4 );

            copier(&conf, &conf1);

            // Traitement du coup du joueur ...

            if (sx == conf.xrB+1 && sy-'a' == conf.yrB && dy == sy + 2) { // petit roque ...
               conf1.mat[0][4] = 0;
               conf1.mat[0][7] = 0;
               conf1.mat[0][6] = 'r'; 
               conf1.xrB = 0; conf1.yrB = 6;
               conf1.mat[0][5] = 't';
               conf1.roqueB = 'e';
            }
            else
               if (sx == conf.xrB+1 && sy-'a' == conf.yrB && dy == sy - 2) { // grand roque ...
                  conf1.mat[0][4] = 0;
                  conf1.mat[0][0] = 0;
                  conf1.mat[0][2] = 'r'; 
                  conf1.xrB = 0; conf1.yrB = 2;
                  conf1.mat[0][3] = 't';
                  conf1.roqueB = 'e';
               } 
               else  {  // deplacement normal (les autres coups) ...
                  conf1.mat[dx-1][dy-'a'] = conf1.mat[sx-1][sy-'a'];
                  conf1.mat[sx-1][sy-'a'] = 0;
                  // vérifier possibilité de transformation d'un pion arrivé en fin d'échiquier ...
                  if (dx == 8 && conf1.mat[dx-1][dy-'a'] == 'p') {
                     printf("Pion arrivé en ligne 8, transformer en (p/c/f/t/n) : ");
                     scanf(" %s", ch);
                     switch (ch[0]) {
                        case 'c' : conf1.mat[dx-1][dy-'a'] = 'c'; break;
                        case 'f' : conf1.mat[dx-1][dy-'a'] = 'f'; break;
                        case 't' : conf1.mat[dx-1][dy-'a'] = 't'; break;
                        case 'p' : conf1.mat[dx-1][dy-'a'] = 'p'; break;
                        default  : conf1.mat[dx-1][dy-'a'] = 'n';
                     }
                  }
               }

            // vérifier si victoire (le roi N n'existe plus) ...
            if ( conf1.xrN == dx-1 && conf1.yrN == dy-'a' ) {
               conf1.xrN = -1;
               conf1.yrN = -1;
            }

            // vérification de la légalité du coup effectué par le joueur ...
            generer_succ(  &conf, MAX, T, &n );

            legal = 0;
            for (i=0; i<n && !legal; i++)
                if ( egal(T[i].mat, conf1.mat) )  legal = 1;

            if ( legal && !feuille(&conf1,&cout) ) {
               printf("OK\n\n");
               i--;
               formuler_coup( &conf, &T[i], coup );
               copier( &T[i], &conf );
            }
            else
               if ( !legal && n > 0 ) { 
                  printf("Coup illégal (%c%d%c%d) -- réessayer\n", sy,sx,dy,dx);
                  refaire = 1; // pour forcer la prochaine itération à rester MAX
               }
               else
                  stop = 1;
         } // if (typeExec == 3)

         else { // MAX ===> PC

            printf("Au tour du joueur maximisant PC 'B' "); fflush(stdout);
            
            generer_succ(  &conf, MAX, T, &n );
            nbp = npieces( &conf );
            printf("  hauteur = [%d,%d]   ,   nb alternatives = %d : \n", hmin, hmax, n); fflush(stdout);
        
            // On effectue un tri sur les alternatives selon l'estimation de leur qualité 
            // Le but est d'explorer les alternatives les plus prometteuses d'abord 
            // pour maximiser les coupes lors des évaluation minmax avec alpha-bêta

            // 1- on commence par évaluer chaque successeur avec la fonction d'estimation 
            for (i=0; i<n; i++) 
                T[i].val = Est[estMax]( &T[i] );

            // 2- on réalise le tri des alternatives T suivant les estimations récupérées:                
            qsort(T, n, sizeof(struct config), confcmp321);   // en ordre décroissant des évaluations
            if ( largeur < n ) n = largeur;
               
            // 3- on lance l'exploration des alternatives triées avec minmax_ab:
            score = -INFINI;
            j = -1;
            //niv = 0;

            profMax = prof = 0;
            printf("[profB]: ");
            for (i=0; i<n; i++) {
                cout = minmax_ab( &T[i], MIN, 0, score, +INFINI, largeur, estMax, nbp, &prof );
                if ( prof > profMax ) profMax = prof;
                printf("%d ", prof); fflush(stdout);
                
                if ( cout > score ) {  // Choisir le meilleur coup (c-a-d le plus grand score)
                   score = cout;
                   j = i;
                }
                if ( cout == 100 ) {
                   printf("v"); fflush(stdout);
                   break;
                }
            }
            if ( j != -1 ) { // jouer le coup et aller à la prochaine itération ...
               printf("\nchoix_B=%d (scoreB = %d   ,   profondeur Max atteinte = %d niveaux)\n", j+1, score, profMax);
               printf("\n");
               formuler_coup( &conf, &T[j], coup );
               copier( &T[j], &conf );
               conf.val = score;
            }
            else   // S'il n'y a pas de successeur possible, le joueur MAX à perdu
               stop = 1;
         } // fin else // MAX ===> PC
 
         if (stop) {
            printf("\n *** le joueur maximisant 'B' a perdu ***\n");
            fprintf(f, "Victoire de 'N'\n");
         }

      }  // if ( tour == MAX )

      else {  // donc tour == MIN 

         if (typeExec == 2) {   // c-a-d MIN ===> USER
            printf("Au tour du joueur minimisant USER 'N'\n"); 
            // récupérer le coup du joueur ...
            do {
               printf("Donnez srcY srcX destY destX (par exemple d7d5) : ");
               fgets(ch, 100, stdin);
               i = sscanf(ch, " %c %d %c %d", &sy, &sx, &dy, &dx );
               if (i != 4) 
                  printf("Lecture incorrecte, recommencer ...\n");
            }
            while ( i != 4 );

            copier(&conf, &conf1);

            // Traitement du coup du joueur ...

            if (sx == conf.xrN+1 && sy-'a' == conf.yrN && dy == sy + 2) { // petit roque ...
               conf1.mat[7][4] = 0;
               conf1.mat[7][7] = 0;
               conf1.mat[7][6] = -'r'; 
               conf1.xrN = 7; conf1.yrN = 6;
               conf1.mat[7][5] = -'t';
               conf1.roqueN = 'e';
            }
            else
               if (sx == conf.xrN+1 && sy-'a' == conf.yrN && dy == sy - 2) { // grand roque ...
                  conf1.mat[7][4] = 0;
                  conf1.mat[7][0] = 0;
                  conf1.mat[7][2] = -'r'; 
                  conf1.xrN = 7; conf1.yrN = 2;
                  conf1.mat[7][3] = -'t';
                  conf1.roqueN = 'e';
               } 
               else  {  // deplacement normal (les autres coups) ...
                  conf1.mat[dx-1][dy-'a'] = conf1.mat[sx-1][sy-'a'];
                  conf1.mat[sx-1][sy-'a'] = 0;
                  // vérifier possibilité de transformation d'un pion arrivé en fin d'échiquier ...
                  if (dx == 1 && conf1.mat[dx-1][dy-'a'] == -'p') {
                     printf("Pion arrivé en ligne 8, transformer en (p/c/f/t/n) : ");
                     scanf(" %s", ch);
                     switch (ch[0]) {
                       case 'c' : conf1.mat[dx-1][dy-'a'] = -'c'; break;
                       case 'f' : conf1.mat[dx-1][dy-'a'] = -'f'; break;
                       case 't' : conf1.mat[dx-1][dy-'a'] = -'t'; break;
                       case 'p' : conf1.mat[dx-1][dy-'a'] = -'p'; break;
                       default  : conf1.mat[dx-1][dy-'a'] = -'n';
                     }
                  }
               }

            // vérifier si victoire (le roi B n'existe plus) ...
            if ( conf1.xrB == dx-1 && conf1.yrB == dy-'a' ) {
               conf1.xrB = -1;
               conf1.yrB = -1;
            }

            // vérification de la légalité du coup effectué par le joueur ...
            generer_succ(  &conf, MIN, T, &n );

            legal = 0;
            for (i=0; i<n && !legal; i++)
                if ( egal(T[i].mat, conf1.mat) )  legal = 1;

            if ( legal && !feuille(&conf1,&cout) ) {
               printf("OK\n\n");
               i--;
               formuler_coup( &conf, &T[i], coup );
               copier( &T[i], &conf );
            }
            else
               if ( !legal && n > 0 ) {
                  printf("Coup illégal (%c%d%c%d) -- réessayer\n", sy,sx,dy,dx);
                  refaire = 1; // pour forcer la prochaine itération à rester MIN
               }
               else
                  stop = 1;
         } // if (typeExec == 2)

         else { // MIN ===> PC

            printf("Au tour du joueur minimisant PC 'N' "); fflush(stdout);
            
            // Générer tous les coups possibles pour le joueur N dans le tableau T
            generer_succ(  &conf, MIN, T, &n );
            nbp = npieces( &conf);
            printf("  hauteur = [%d,%d]   ,   nb alternatives = %d : \n", hmin, hmax, n); fflush(stdout);
        
            // On effectue un tri sur les alternatives selon l'estimation de leur qualité 
            // Le but est d'explorer les alternatives les plus prometteuses d'abord 
            // pour maximiser les coupes lors des évaluation minmax avec alpha-bêta

            // 1- on commence par évaluer chaque successeur avec une fonction d'estimation 
            for (i=0; i<n; i++) 
                T[i].val = Est[estMin]( &T[i] );

            // 2- on réalise le tri des alternatives T suivant les estimations récupérées:
            qsort(T, n, sizeof(struct config), confcmp123);   // en ordre croissant des évaluations
            if ( largeur < n ) n = largeur;

            // 3- on lance l'exploration des alternatives triées avec minmax_ab:
            score = +INFINI;
            j = -1;
            //niv = 0;
            profMax = prof = 0;
            printf("[profN]: ");                
            for (i=0; i<n; i++) {
                cout = minmax_ab( &T[i], MAX, 0, -INFINI, score, largeur, estMin, nbp, &prof );
                if ( prof > profMax ) profMax = prof;

                printf("%d ", prof); fflush(stdout);
                if ( cout < score ) {  // Choisir le meilleur coup (c-a-d le plus petit score)
                   score = cout;
                   j = i;
                }
                if ( cout == -100 ) {
                   printf("v"); fflush(stdout);
                   break;
                }
            }
            if ( j != -1 ) { // jouer le coup et aller à la prochaine itération ...
               printf("\nchoix_N=%d (scoreN = %d   ,   profondeur Max atteinte = %d niveaux)\n", j+1, score, profMax);
               printf("\n");
               formuler_coup( &conf, &T[j], coup );
               copier( &T[j], &conf );
               conf.val = score;
            }
            else  // S'il n'y a pas de successeur possible, le joueur MIN à perdu
               stop = 1;
                    
         } // fin else // MIN ===> PC

         if (stop) {
            printf("\n *** le joueur minimisant 'N' a perdu ***\n");
            fprintf(f, "Victoire de 'B'\n");
         }

      } // fin else // tour == MIN

      if ( !refaire ) {
         num_coup++;
         tour = ( tour == MIN ? MAX : MIN );
      }

   } // fin while ( !stop )

   printf("\nNb de coupes (alpha:%d + beta:%d)  = %d", nbAlpha, nbBeta,nbAlpha+nbBeta); 
   printf("\nFin de partie\n");

   return 0;

} // fin de main

