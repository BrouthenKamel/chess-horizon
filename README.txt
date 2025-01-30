Programme de démonstration de MinMax avec élagage alpha/bêta pour le jeu d'échecs
A utiliser dans le module CRP/2CS-SID (Hidouci W.K. - ESI - 2024/2025)

Dans la version actuelle, le programme comporte une implémentation de "MinMax avec élagage alpha/bêta", les règles (presque complètes) du jeu d'échecs et quelques fonctions d'estimations utilisées pour l'évaluation des configurations situées à la "frontière de l'exploration".

"MinMax" est un algorithme de type 'parcours en profondeur' utilisé dans l'exploration des espaces de recherche.
Des coupes de branches sont effectuées afin de rendre l'exploration plus rapide (c'est "l'élagage alpha/bêta").

Pour diminuer "l'effet horizon", la stabilité des configurations est prise en considération pour décider s'il faut ou non, continuer l'exploration à des profondeurs plus grandes.
Dans cette version, une configuration est considérée comme étant "stable" si le nombre de pièces présentes est égal à celui de la configuration parante (c-a-d in n'y a pas eu de captures effectuées entre les 2 configurations).
La "frontière d'exploration" représente les configurations les plus éloignées qu'on a pu atteindre et évaluer.
La profondeur des configurations de la "frontière d'exploration" varie entre une hauteur minimale (hmin) et une hauteur maximale (hmax).

Afin d'augmenter les opportunités de coupes de branches, un tri des différents successeurs à évaluer est effectué:
Pour chaque coup généré (dans la fonction main ou dans la fonction minmax_ab), les successeurs sont d'abord évalués avec la fonction d'estimation choisie. Ces estimations sont ensuite utilisées pour ordonner les successeurs du plus prometteur au moins prometteur, avant d'être évaluées avec une profondeur plus grande dans [hmin,hmax]. Cet ordre d'évaluation des alternatives devrait augmenter le nombre de coupes de branches (si les fonctions d'estimations utilisées sont de bonnes qualités). 

Les principales fonctions sont:
- minmax_ab(...)  	// MinMax avec élagage Alpha-Bêta.
- generer_succ(...)  	// Génère les successeurs d'une configuration donnée.
- estim*(...)  		// Les fonctions d'estimations pouvant être utilisées dans minmax_ab(...)

------------------------------------------------------------------------

Explication de quelques variables globales dans le source :

int hmin;
hmin représente la hauteur minimale de la "frontière d'exploration"

int hmax;
hmax représente la hauteur maximale de la "frontière d'exploration"

Durant le parcours en profondeur par minmax_ab:
	- si la profondeur courante est < hmin, on continue à descendre dans l'arbre 
	- si la profondeur courante est >= hmin et < hmax, alors dès qu'on trouve une conf stable on s'arrête
	- si la profondeur courante = hmax, on arrête la descente (même si la configuration n'est pas stable)

struct config Partie[MAXPARTIE];
Un tableau stockant le déroulement d'une partie (les MAXPARTIE derniers coups).
Il est utilisé pour détecter si une configuration a déjà été générée, pour ne pas la considérer une 2e fois.

int num_coup = 0;
Le numéro du coup actuel.
Il est incrémenté à chaque tour.

int (*Est[10])(struct config *);
Un tableau de pointeurs de fonctions retournant un entier (int) et ayant comme paramètre une configuration.
Ce tableau est utilisé dans la fonction main() pour stocker les différentes fonctions d'estimations disponibles et permettre à l'utilisateur de choisir quelle fonction associer à chaque adversaire (Noir et Blanc)
Le tableau 'Est' est aussi utilisé dans la fonction minmax_ab() pour appeler la bonne fonction d'estimation quand la bonne profondeur est atteinte (c-a-d au niveau des configurations de la frontière d'exploration, quand la configuration est stable)

int nbEst;
C'est le nombre de fonctions d'estmations dans le tableau 'Est' (normalement < 10 sinon s'il y a plus de 10 fonctions d'estimations augmenter la taille du tableau 'Est' en conséquence)

------------------------------------------------------------------------

Toutes les fonctions du programme :

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

/* Teste si conf représente une fin de partie et retourne dans 'cout' la valeur associée */
int feuille( struct config *conf, int *cout )

/* Quelques exemples de fonctions d'estimation simples (estim1, estim2, ...) */
int estim1( struct config *conf )
int estim2( struct config *conf )
...

/* Génère, pour le joueur 'mode', les successeurs de la configuration 'conf' dans le tableau 'T', 
   retourne aussi dans 'n' le nombre de configurations filles générées */
void generer_succ( struct config *conf, int mode, struct config T[], int *n )

/* Génère dans T les configurations obtenues à partir de conf lorsqu'un pion (a,b) va atteindre 
   la limite de l'échiquier: pos (x,y)  */
void transformPion( struct config *conf, int a, int b, int x, int y, struct config T[], int *n )

/* Génere dans T tous les coups possibles de la pièce (de couleur N ou B) se trouvant à la pos x,y */
void deplacementsN(struct config *conf, int x, int y, struct config T[], int *n )
void deplacementsB(struct config *conf, int x, int y, struct config T[], int *n )

// Vérifie si la case (x,y) est menacée par une des pièces du joueur 'mode'
int caseMenaceePar( int mode, int x, int y, struct config *conf )

/* Fonctions de comparaison utilisées avec qsort     */
// 'a' et 'b' sont des configurations
int confcmp123(const void *a, const void *b)	// pour l'ordre croissant
int confcmp321(const void *a, const void *b)	// pour l'ordre decroissant

/* Le nombre de pièces de N et B dans l'echiquier 'conf' */
int npieces( struct config *conf )

/* Sauvegarder conf dans le fichier f (pour l'historique) */
void sauvConf( struct config *conf )

/* Intialise la disposition des pieces dans la configuration initiale conf */
void init_conf( struct config *conf )

/* génère un texte décrivant le dernier coup effectué (pour l'affichage) */
void formuler_coup( struct config *oldconf, struct config *newconf, char *coup )

/* Affiche la configuration conf */
void affich( struct config *conf, char *coup, int num )

/* Copie la configuration c1 dans c2  */
void copier( struct config *c1, struct config *c2 ) 

/* Teste si les échiquiers c1 et c2 sont égaux */
int egal(char c1[8][8], char c2[8][8] )

/* Teste si conf a déjà été jouée (dans le tableau partie) */
int dejaVisitee( struct config *conf )

/* Teste s'il n'y a aucun coup possible dans la configuration conf (non encore implémentée) */
int AucunCoupPossible( struct config *conf )

------------------------------------------------------------------------

La boucle principale dans la fonction main

// initialisations....

tour = MAX
while ( non fin de partie ) {
    Afficher la config courante : conf
    if ( tour == MAX )
	if ( joueur_humain )
	   Saisir le coup à jouer ==> newConf
	else
	   Generer les n successeurs de conf dans le tableau T
	   Estimer les n successeurs T[i]
	   Trier les n successeurs T[i] en ordre décroissant des estimations retournées
	   n' = min( n , Largeur d'exploration)
           /* si Largeur_exploration = +INFINI,  n' = n  */
	   Explorer les premiers n' successeurs T[i] (dans l'ordre décroissant) avec minmax_ab
	   ==> soit newConf le meilleur successeur (ayant la plus grande estimation)
    else // c-a-d tour == MIN
	if ( joueur_humain )
	   Saisir le coup à jouer ==> newConf
	else
	   Generer les n successeurs de conf dans le tableau T
	   Estimer les n successeurs T[i]
	   Trier les n successeurs T[i] en ordre décroissant des estimations retournées
	   n' = min( n , Largeur d'exploration)
           /* si Largeur_exploration = +INFINI,  n' = n  */
	   Explorer les premiers n' successeurs T[i] (dans l'ordre décroissant) avec minmax_ab
	   ==> soit newConf le meilleur successeur (ayant la plus petite estimation)
	
    conf = newConf
    tour = (tour == MAX ? MIN : MAX )
}


