/********************************************************************************
*********************************************************************************

Programme de démonstration de MinMax avec élagage alpha/bêta pour le jeu d'échecs
Hidouci / CRP - 2CS-SID / ESI 2024/2025

Header du module : jeu.h

*********************************************************************************
*********************************************************************************/

#ifndef JEU_H
#define JEU_H

#include <limits.h>     // pour INT_MAX

#define MAX +1          // Joueur Maximisant
#define MIN -1          // Joueur Minimisant

#define INFINI INT_MAX
#define MAXPARTIE 50    // Taille max du tableau 'Partie[]'
                        // qui sert à vérifier si une conf a déjà été générée
                        // pour ne pas le re-considérer une 2e fois.
                        // On se limitera donc aux MAXPARTIE derniers coups

// Le type d'une configuration
struct config {
   char mat[8][8];            // Echiquier
   int val;                   // Estimation de la config
   char xrN, yrN, xrB, yrB;   // Positions des rois Noir et Blanc
   char roqueN, roqueB;       // Indicateurs de roque pour N et B : 
                                   // 'g' grand roque non réalisable
                                   // 'p' petit roque non réalisable
                                   // 'n' non réalisable des 2 cotés
                                   // 'r' réalisable (valeur initiale)
                                   // 'e' effectué
};

/**************************/
/* Entête des fonctions : */
/**************************/

/* MinMax avec élagage alpha-beta :
 Evalue la configuration 'conf' du joueur 'mode' en descendant de 'hmin' niveaux au minimum.
 Le paramètre 'h' est incrémenté à chaque niveau (appel récursif).
 Le maximum de niveaux atteints est déterminé par l'atteinte de configurations stables.
 Il peut atteindre au plus 'hmax' niveaux
 'alpha' et 'beta' représentent les bornes initiales de l'intervalle d'intérêt 
 (pour pouvoir effectuer les coupes alpha et bêta).
 'largeur' représente le nombre max d'alternatives à explorer en profondeur à chaque niveau.
 Si 'largeur == +INFINI' toutes les alternatives seront prises en compte 
 (c'est le comportement par défaut).
 'numFctEst' est le numéro de la fonction d'estimation à utiliser lorsqu'on arrive à la
 frontière d'exploration (c-a-d lorsque la conf est stable et la profondeur est dans [hmin,hmax]).
 npp : le nombre de pieces dans la conf parente (pour verifier si conf est stable ou non) 
*/
int minmax_ab( struct config *conf, int mode, int h, int alpha, int beta, int largeur, \
               int numFctEst, int npp, int *profMax );

/* ---------------------------------- 
  La fonction d'estimation à utiliser, retourne une valeur dans ]-100, +100[ 
  quelques fonctions d'estimation disponibles (comme exemples).
  Le paramètre 'conf' représente la configuration à estimer
*/
int estim1( struct config *conf );
int estim2( struct config *conf );
int estim3( struct config *conf );
int estim4( struct config *conf );
int estim5( struct config *conf );
int estim6( struct config *conf );
/*
   Pour tester votre propre fonction d'estimation: changer le code de estim7 
   et utilisez la pour un des 2 joueurs N ou B (voir la fonction main)
*/
int estim7( struct config *conf );

/* ---------------------------------- 
   Génère, pour le joueur 'mode', les successeurs de la configuration 'conf' dans le tableau 'T', 
   retourne aussi dans 'n' le nombre de configurations filles générées
*/ 
void generer_succ( struct config *conf, int mode, struct config T[], int *n );

/* ----------------------------------
  Génère dans 'T' les configurations obtenues à partir de 'conf' lorsqu'un pion à la pos (a,b) 
  va atteindre la limite de l'échiquier (x,y) et se transformer en une autre pièce.
  Le nombre de possibilités sera retourné dans 'n'
*/ 
void transformPion( struct config *conf, int a, int b, int x, int y, struct config T[], int *n );

/* ---------------------------------- 
  Génere dans 'T' tous les coups possibles de la pièce (de couleur N ou B) 
  se trouvant à la pos 'x','y'. Le nombre de possibilités sera retourné dans 'n'
*/
void deplacementsN(struct config *conf, int x, int y, struct config T[], int *n );
void deplacementsB(struct config *conf, int x, int y, struct config T[], int *n );

/* ---------------------------------- 
  Vérifie si la case (x,y) est menacée par une des pièces du joueur 'mode'
*/
int caseMenaceePar( int mode, int x, int y, struct config *conf );

/* ----------------------------------
  Intialise la disposition des pieces dans la configuration initiale 'conf'
*/
void init_conf( struct config *conf );

/* ----------------------------------
  Compte le nombre pieces (blancs et noirs) dans l'echiquier
*/
int npieces( struct config *conf );

/* ----------------------------------
  Emoji de la pièce d'échecs
*/
char* emoji(char piece);

/* ---------------------------------- 
  Affiche la configuration 'conf'
*/

void affich( struct config *conf, char *coup, int num, int affich_emoji );

/* ----------------------------------
  Teste si 'conf' représente une configuration déjà jouée (dans le tableau Partie)
  auquel cas elle retourne le num du coup + 1
*/
int dejaVisitee( struct config *conf );

/* ---------------------------------- 
  Savegarde la config 'conf' dans le fichier f (global)
*/
void sauvConf( struct config *conf );

/* ----------------------------------
  Copie la configuration 'c1' dans 'c2'
*/
void copier( struct config *c1, struct config *c2 );

/* ---------------------------------- 
  Teste si les 2 grilles (échiquiers) 'c1' et 'c2' sont égales
*/ 
int egal(char c1[8][8], char c2[8][8] );

/* ----------------------------------
  Teste s'il n'y a aucun coup possible à partir de la configuration 'conf'
*/ 
int AucunCoupPossible( struct config *conf );

/* ---------------------------------- 
  Teste si 'conf' représente une fin de partie 
  et retourne dans 'cout' son score -100, 0 ou +100
*/ 
int feuille( struct config *conf, int *cout );

/* ----------------------------------
  Comparaisons des configurations 'a' et 'b' pour le compte de qsort 
  pour l'ordre croissant (confcmp123) et décroissant (confcmp321)
*/
int confcmp123(const void *a, const void *b);        
int confcmp321(const void *a, const void *b);

/* ---------------------------------- 
  Génère dans 'coup' un texte décrivant le dernier coup effectué (pour l'affichage)
  en comparant deux configurations: l'ancienne 'oldconf' et la nouvelle 'newconf'
*/
void formuler_coup( struct config *oldconf, struct config *newconf, char *coup );

#endif
