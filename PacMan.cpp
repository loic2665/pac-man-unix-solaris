#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <signal.h>
#include "GrilleSDL.h"
#include "Ressources.h"
#include "Ecran.h"

// Dimensions de la grille de jeu
#define NB_LIGNE 21
#define NB_COLONNE 17

// Macros utilisees dans le tableau tab
#define VIDE         0
#define MUR          1
#define PACMAN       -2
#define PACGOM       -3
#define SUPERPACGOM  -4
#define BONUS        -5

// Autres macros
#define LENTREE 15
#define CENTREE 8


int tab[NB_LIGNE][NB_COLONNE]
        = {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
           {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
           {1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1},
           {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
           {1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1},
           {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
           {1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1},
           {1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1},
           {1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1},
           {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0},
           {1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1},
           {1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1},
           {1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1},
           {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
           {1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1},
           {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
           {1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1},
           {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
           {1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1},
           {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
           {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};


pthread_mutex_t mutexTab;
pthread_mutex_t mutexAtt;

int L = LENTREE;
int C = CENTREE;
int dir = GAUCHE;


pthread_mutex_t mutexDelai;
int delaiFrameMs = 200;

void DessineGrilleBase();

void Attente(int milli);

void *threadPacMan(void *);

pthread_t pacMacThreadTid;

void SigPacManDirectionHandle(int);

void *threadSDLEvent(void *);

pthread_t threadThreadSDLEventTid;


void *threadPacGom(void *);

pthread_t threadPacGomTid;
int nbPacGom = 0;
pthread_cond_t pacGomCond;
pthread_mutex_t mutexPacGom;

int PacManLevel = 1;


bool MAJscore = true;
int score = 0;

void *threadScore(void *);

pthread_t threadScoreTid;
pthread_mutex_t mutexScore;
pthread_cond_t condScore;

char ok = 0;


pthread_t threadBonusTid;

void *threadBonus(void *);


int nbRouge = 0;
int nbVert = 0;
int nbMauve = 0;
int nbOrange = 0;

pthread_t threadCompteurFantomesTid;

void *threadCompteurFantomes(void *);

pthread_mutex_t mutexCompteurFantomes;
pthread_cond_t condCompteurFantomes;

void *threadFantome(void *);

pthread_key_t keyFantome;

typedef struct fantome {
    int L;
    int C;
    int couleur;
    int cache;
} S_FANTOME;

void *threadDebug(void *);

void *threadVie(void *);

pthread_t threadVieTid;

int nbVie = 3;

int mode = 1;
pthread_mutex_t mutexMode;
pthread_t threadTimeOutTid;

void *threadTimeOut(void *);

void handlerSigAlarm(int);
int nbSec = 0;

pthread_mutex_t mutexTransfertTimer;
pthread_cond_t condTransfertTimer;

void fantome_terminaison(void*);
void handlerSigChild(int);
///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {


    pthread_mutex_init(&mutexTab, NULL);

    pthread_cond_init(&pacGomCond, NULL);

    pthread_mutex_init(&mutexScore, NULL);
    pthread_mutex_init(&mutexAtt, NULL);
    pthread_mutex_init(&mutexDelai, NULL);

    pthread_cond_init(&condScore, NULL);

    pthread_mutex_init(&mutexCompteurFantomes, NULL);
    pthread_cond_init(&condCompteurFantomes, NULL);
    pthread_key_create(&keyFantome, NULL);
    pthread_mutex_init(&mutexMode, NULL);

    pthread_mutex_init(&mutexTransfertTimer, NULL);
    pthread_cond_init(&condTransfertTimer, NULL);

    srand((unsigned) time(NULL));

    fflush(stdout);
    if (OuvertureFenetreGraphique() < 0) {
        fflush(stdout);
        exit(1);
    }

    DessineGrilleBase();


    sigset_t maskSigAlarm;

    sigemptyset(&maskSigAlarm);
    // todo : à voir sigaddset(&maskSigAlarm, SIGALRM);
    sigprocmask(SIG_BLOCK, &maskSigAlarm, NULL);

    pthread_create(&threadThreadSDLEventTid, NULL, threadSDLEvent, NULL);
    pthread_create(&threadVieTid, NULL, threadVie, NULL);
    pthread_create(&threadPacGomTid, NULL, threadPacGom, NULL);
    pthread_create(&threadScoreTid, NULL, threadScore, NULL);
    pthread_create(&threadBonusTid, NULL, threadBonus, NULL);
    Attente(300);
    pthread_create(&threadCompteurFantomesTid, NULL, threadCompteurFantomes, NULL);

    pthread_create(NULL, NULL, threadDebug, NULL);
    pthread_join(threadThreadSDLEventTid, NULL);


}

//*********************************************************************************************
void Attente(int milli) {
    struct timespec del;
    del.tv_sec = milli / 1000;
    del.tv_nsec = (milli % 1000) * 1000000;
    nanosleep(&del, NULL);
}

//*********************************************************************************************
void DessineGrilleBase() {
    pthread_mutex_lock(&mutexTab);
    for (int l = 0; l < NB_LIGNE; l++)
        for (int c = 0; c < NB_COLONNE; c++) {
            if (tab[l][c] == VIDE) EffaceCarre(l, c);
            if (tab[l][c] == MUR) DessineMur(l, c);
        }

    pthread_mutex_unlock(&mutexTab);
}


/********************************************/
/*              DEBUT TRAVAIL               */
/********************************************/

void *threadDebug(void *) {

    while (!ok) {

        /*todo: DEBUG*/

        for (int i = 0; i < NB_LIGNE; i++) {
            for (int j = 0; j < NB_COLONNE; j++) {
                printf("%4d ", tab[i][j]);
            }
            printf("\n");
        }
        printf("\n");
        printf("NbOrange = %d\nNbMauve = %d\nNbRouge = %d\nNbVert = %d\n", nbOrange, nbMauve, nbRouge, nbVert);
        /*DEBUG*/
        printf("Score : %d\n", score);
        printf("Pacgom: %d\n", nbPacGom);
        printf("Vie   : %d\n", nbVie);
        printf("NbSec : %d\n\n", nbSec);
        printf("Mode  : %d\n", mode);
        Attente(300);
    }

}

void *threadPacMan(void *) {

    L = LENTREE;
    C = CENTREE;

    struct sigaction sa;
    sa.sa_handler = &SigPacManDirectionHandle;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL); // LEFT
    sigaction(SIGHUP, &sa, NULL); // RIGHT
    sigaction(SIGUSR1, &sa, NULL); // UP
    sigaction(SIGUSR2, &sa, NULL); // DOWN


    sigset_t maskSig;

    sigemptyset(&maskSig);
    sigaddset(&maskSig, SIGINT);
    sigaddset(&maskSig, SIGHUP);
    sigaddset(&maskSig, SIGUSR1);
    sigaddset(&maskSig, SIGUSR2);

    int lastC;
    int lastL;
    int move = 0;

    while (!ok) {

        lastC = C;
        lastL = L;


        move = 0;

        pthread_mutex_lock(&mutexTab);
        switch (dir) {
            case DROITE:
                if ((C + 1) <= NB_COLONNE && tab[L][C + 1] != MUR) {
                    C += 1;
                    move = 1;
                } else if (L == 9 && C == NB_COLONNE - 1) {
                    C = 0;
                    move = 1;
                }
                break;
            case GAUCHE:
                if ((C - 1) >= 0 && tab[L][C - 1] != MUR) {
                    C -= 1;
                    move = 1;
                } else if (L == 9 && C == 0) {
                    C = NB_COLONNE - 1;
                    move = 1;
                }
                break;
            case HAUT:
                if ((L - 1) > 0 && tab[L - 1][C] != MUR) {
                    L -= 1;
                    move = 1;
                }
                break;
            case BAS:
                if ((L + 1) < NB_LIGNE && tab[L + 1][C] != MUR) {
                    L += 1;
                    move = 1;
                }
                break;

        }

        if (move) {


            pthread_mutex_lock(&mutexScore);
            pthread_mutex_lock(&mutexMode);
            if (tab[L][C] == PACGOM) {
                MAJscore = true;
                score += 1;
            } else if (tab[L][C] == SUPERPACGOM) {
                MAJscore = true;

                mode = 2;

                int* tempsRestant = (int*) malloc(sizeof(int));
                *tempsRestant = alarm(0);

                if(*tempsRestant > 0){
                    pthread_kill(threadTimeOutTid, SIGQUIT);

                }

                pthread_mutex_lock(&mutexTransfertTimer);
                pthread_create(&threadTimeOutTid, NULL, threadTimeOut, tempsRestant);

                pthread_cond_wait(&condTransfertTimer, &mutexTransfertTimer);
                free(tempsRestant);
                pthread_mutex_unlock(&mutexTransfertTimer);

                pthread_mutex_unlock(&mutexMode);

                score += 5;


            } else if (tab[L][C] > 3 && mode == 2) {
                // mange fantôme

                pthread_t tidFantome = tab[L][C];
                pthread_kill(tidFantome, SIGCHLD);




            } else if (tab[L][C] > 3) {

                EffaceCarre(lastL, lastC);
                tab[lastL][lastC] = VIDE;
                dir = GAUCHE;
                pthread_mutex_unlock(&mutexMode);
                pthread_mutex_unlock(&mutexScore);
                pthread_mutex_unlock(&mutexTab);

                pthread_exit(NULL);

            } else if (tab[L][C] == BONUS) {
                MAJscore = true;
                score += 30;
            }

            if (tab[L][C] == PACGOM || tab[L][C] == SUPERPACGOM) {
                nbPacGom--;
                pthread_cond_signal(&pacGomCond);
            }

            tab[L][C] = PACMAN;
            tab[lastL][lastC] = 0;
            EffaceCarre(lastL, lastC);

            pthread_mutex_unlock(&mutexMode);
            pthread_mutex_unlock(&mutexScore);
        }

        if (MAJscore) {
            pthread_cond_signal(&condScore);
        }

        DessinePacMan(L, C, dir);
        pthread_mutex_unlock(&mutexTab);


        sigprocmask(SIG_BLOCK, &maskSig, NULL);
        Attente(delaiFrameMs);
        sigprocmask(SIG_UNBLOCK, &maskSig, NULL);

    }

    pthread_exit(NULL);

}


void SigPacManDirectionHandle(int signal) {

    if (signal == SIGINT) {
        dir = GAUCHE;
    } else if (signal == SIGHUP) {
        dir = DROITE;
    } else if (signal == SIGUSR1) {
        dir = HAUT;
    } else if (signal == SIGUSR2) {
        dir = BAS;
    }

}

void *threadSDLEvent(void *) {

    EVENT_GRILLE_SDL event;

    while (!ok) {
        event = ReadEvent();
        if (event.type == CLIC_GAUCHE) {
            printf("*******************************\n");
            printf("  CLICK ON LINE: %d / COL: %d  \n", event.ligne, event.colonne);
            printf("    Value is   : %d            \n", tab[event.ligne][event.colonne]);
            printf("*******************************\n");
        }
        if (event.type == CROIX) {
            ok = 1;
        }
        if (event.type == CLAVIER) {
            switch (event.touche) {
                case 'q' :
                    ok = 1;
                    break;

                case KEY_RIGHT :
                    pthread_kill(pacMacThreadTid, SIGHUP);
                    break;
                case KEY_LEFT :
                    pthread_kill(pacMacThreadTid, SIGINT);
                    break;
                case KEY_UP :
                    pthread_kill(pacMacThreadTid, SIGUSR1);
                    break;
                case KEY_DOWN :
                    pthread_kill(pacMacThreadTid, SIGUSR2);
                    break;
            }
        }
    }

    Attente(100);
    // -------------------------------------------------------------------------

    // Fermeture de la fenetre
    fflush(stdout);
    FermetureFenetreGraphique();
    fflush(stdout);

    pthread_exit(0);

}


void *threadPacGom(void *) {

    int centPacGom = 0;
    int dixPacGom = 0;
    int unPacGom = 0;

    while (!ok) {

        pthread_mutex_lock(&mutexPacGom); // todo à continuer

        while (nbPacGom > 0) {
            pthread_cond_wait(&pacGomCond, &mutexPacGom);

            centPacGom = nbPacGom / 100;
            dixPacGom = (nbPacGom - (centPacGom * 100)) / 10;
            unPacGom = (nbPacGom - (centPacGom * 100) - (dixPacGom * 10)) / 1;

            DessineChiffre(12, 22, centPacGom);
            DessineChiffre(12, 23, dixPacGom);
            DessineChiffre(12, 24, unPacGom);

            DessineChiffre(14, 22, PacManLevel);

            if (nbPacGom <= 0) {
                delaiFrameMs /= 2;
                PacManLevel++;
            }

        }
        pthread_mutex_unlock(&mutexPacGom);

        pthread_mutex_lock(&mutexTab);
        for (int i = 0; i < NB_LIGNE; i++) {
            for (int j = 0; j < NB_COLONNE; j++) {
                if (tab[i][j] == VIDE) {
                    tab[i][j] = PACGOM;
                    DessinePacGom(i, j);
                    nbPacGom++;
                }
            }
        }
        pthread_mutex_unlock(&mutexTab);

        pthread_mutex_lock(&mutexTab);
        tab[15][8] = VIDE;
        tab[8][8] = VIDE;
        tab[9][8] = VIDE;
        tab[2][1] = SUPERPACGOM;
        tab[2][15] = SUPERPACGOM;
        tab[15][1] = SUPERPACGOM;
        tab[15][15] = SUPERPACGOM;
        nbPacGom = nbPacGom - 3;
        pthread_mutex_unlock(&mutexTab);


        EffaceCarre(15, 8);
        EffaceCarre(8, 8);
        EffaceCarre(9, 8);
        DessineSuperPacGom(2, 1);
        DessineSuperPacGom(2, 15);
        DessineSuperPacGom(15, 1);
        DessineSuperPacGom(15, 15);

    }


}


void *threadScore(void *) {

    int milleScore = 0;
    int centScore = 0;
    int dixScore = 0;
    int unScore = 0;

    while (!ok) {

        pthread_mutex_lock(&mutexScore);
        while (MAJscore == false) {
            pthread_cond_wait(&condScore, &mutexScore);
        }
        MAJscore = false; // == ?????
        pthread_mutex_unlock(&mutexScore);

        milleScore = score / 1000;
        centScore = (score - milleScore) / 100;
        dixScore = (score - (milleScore * 1000) - (centScore * 100)) / 10;
        unScore = (score - (milleScore * 1000) - (centScore * 100) - (dixScore * 10)) / 1;


        DessineChiffre(16, 22, milleScore);
        DessineChiffre(16, 23, centScore);
        DessineChiffre(16, 24, dixScore);
        DessineChiffre(16, 25, unScore);

    }


}

void *threadBonus(void *) {

    int lBonus = 0;
    int cBonus = 0;
    int tempsAttMs;
    int caseVide = 0;



    srand(time(NULL));
    while (!ok) {
        tempsAttMs = ((rand() % 10) + 10) * 1000;
        Attente(tempsAttMs);
        while (caseVide == 0) {

            cBonus = rand() % NB_COLONNE;
            lBonus = rand() % NB_LIGNE;

            pthread_mutex_lock(&mutexTab);
            if (tab[lBonus][cBonus] == VIDE) {
                caseVide = 1;
            }
            pthread_mutex_unlock(&mutexTab);
        }
        caseVide = 0;

        pthread_mutex_lock(&mutexTab);
        tab[lBonus][cBonus] = BONUS;
        pthread_mutex_unlock(&mutexTab);
        DessineBonus(lBonus, cBonus);

        Attente(10000); // 10 secs

        pthread_mutex_lock(&mutexTab);
        if (tab[lBonus][cBonus] == BONUS) {
            tab[lBonus][cBonus] = VIDE;
            EffaceCarre(lBonus, cBonus);
        }
        pthread_mutex_unlock(&mutexTab);


    }

}

void *threadCompteurFantomes(void *) {

    int color = 0;
    int result = 0;
    int modeLocale;

    while (!ok) {

        while (nbMauve == 2 && nbOrange == 2 && nbRouge == 2 && nbVert == 2) {
            pthread_cond_wait(&condCompteurFantomes, &mutexCompteurFantomes);
        }
        while (nbMauve < 2 || nbOrange < 2 || nbRouge < 2 || nbVert < 2) {
            if (nbMauve < 2) {
                color = MAUVE;
            } else if (nbOrange < 2) {
                color = ORANGE;
            } else if (nbRouge < 2) {
                color = ROUGE;
            } else if (nbVert < 2) {
                color = VERT;
            }

            S_FANTOME *addrParam = (S_FANTOME *) (malloc(sizeof(S_FANTOME)));
            addrParam->couleur = color;
            addrParam->cache = VIDE; // ce qu'il cache !
            addrParam->L = 9;
            addrParam->C = 8;


            pthread_mutex_lock(&mutexTab);

            pthread_mutex_lock(&mutexMode);
                modeLocale = mode;
            pthread_mutex_unlock(&mutexMode);

            if (tab[9][8] == VIDE && modeLocale == 1) {
                result = pthread_create(NULL, NULL, threadFantome, addrParam);
                if (result == 0) {
                    if (color == ROUGE) {
                        nbRouge++;
                    } else if (color == VERT) {
                        nbVert++;
                    } else if (color == ORANGE) {
                        nbOrange++;
                    } else if (color == MAUVE) {
                        nbMauve++;
                    }
                }


            }
            pthread_mutex_unlock(&mutexTab);


            Attente(delaiFrameMs);
        }
    }

}

void *threadFantome(void *param) {


    sigset_t maskSigChild;
    sigemptyset(&maskSigChild);
    sigaddset(&maskSigChild, SIGCHLD);
    sigprocmask(SIG_UNBLOCK, &maskSigChild, NULL);


    struct sigaction sa;
    sa.sa_handler = &handlerSigChild;
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    pthread_setspecific(keyFantome, param);

    S_FANTOME *fantome;
    fantome = (S_FANTOME *) pthread_getspecific(keyFantome);

    pthread_cleanup_push(fantome_terminaison, fantome);

    // todo: faire une verif pour ne pas spawn sur une case pas vide

    int directionFantome = HAUT;
    bool canMove = true;
    int dirTab[4];
    int cptTab = 0;
    int vitesseLocale;
    int modeLocale;

    while (!ok) {
        pthread_mutex_lock(&mutexMode);
            modeLocale = mode;
        pthread_mutex_unlock(&mutexMode);

        pthread_mutex_lock(&mutexTab);

        tab[fantome->L][fantome->C] = fantome->cache;
        switch (fantome->cache) {
            case PACGOM:
                DessinePacGom(fantome->L, fantome->C);
                break;
            case SUPERPACGOM:
                DessineSuperPacGom(fantome->L, fantome->C);
                break;
            case BONUS:
                DessineBonus(fantome->L, fantome->C);
                break;
            default: // normalement vide
                EffaceCarre(fantome->L, fantome->C);

        }


        if(modeLocale == 1){
            switch (directionFantome) {
                case HAUT:
                    if (tab[fantome->L - 1][fantome->C] < 1) {
                        fantome->L -= 1;
                    } else {
                        canMove = false;
                    }
                    break;
                case BAS:
                    if (tab[fantome->L + 1][fantome->C] < 1) {

                        fantome->L += 1;
                    } else {
                        canMove = false;
                    }
                    break;
                case GAUCHE:
                    if (tab[fantome->L][fantome->C - 1] < 1) {

                        fantome->C -= 1;
                    } else {
                        canMove = false;
                    }
                    break;
                case DROITE:
                    if (tab[fantome->L][fantome->C + 1] < 1) {

                        fantome->C += 1;
                    } else {
                        canMove = false;
                    }
                    break;

            }
        }else{

            switch (directionFantome) {
                case HAUT:
                    if (tab[fantome->L - 1][fantome->C] < 1 && tab[fantome->L - 1][fantome->C] != PACMAN) {
                        fantome->L -= 1;
                    } else {
                        canMove = false;
                    }
                    break;
                case BAS:
                    if (tab[fantome->L + 1][fantome->C] < 1 && tab[fantome->L + 1][fantome->C] != PACMAN) {

                        fantome->L += 1;
                    } else {
                        canMove = false;
                    }
                    break;
                case GAUCHE:
                    if (tab[fantome->L][fantome->C - 1] < 1 && tab[fantome->L][fantome->C - 1] != PACMAN) {

                        fantome->C -= 1;
                    } else {
                        canMove = false;
                    }
                    break;
                case DROITE:
                    if (tab[fantome->L][fantome->C + 1] < 1 && tab[fantome->L][fantome->C + 1] != PACMAN) {

                        fantome->C += 1;
                    } else {
                        canMove = false;
                    }
                    break;

            }

        }



        // todo : si il y a des trucs chelou, alors mettre le mecanisme plus bas
        // todo : dans le "if"


        if (!canMove) {
            cptTab = 0;

            if (tab[fantome->L - 1][fantome->C] < 1) {
                dirTab[cptTab] = HAUT;
                cptTab++;
            }
            if (tab[fantome->L + 1][fantome->C] < 1) {
                dirTab[cptTab] = BAS;
                cptTab++;
            }
            if (tab[fantome->L][fantome->C - 1] < 1) {
                dirTab[cptTab] = GAUCHE;
                cptTab++;
            }
            if (tab[fantome->L][fantome->C + 1] < 1) {
                dirTab[cptTab] = DROITE;
                cptTab++;
            }
            if (cptTab != 0) {
                directionFantome = dirTab[rand() % cptTab];
                canMove = true;
            } else {
                directionFantome = HAUT;
                canMove = true;
            }


        }


        if (tab[fantome->L][fantome->C] == PACMAN && modeLocale != 2) {
            pthread_cancel(pacMacThreadTid);
            fantome->cache = VIDE;
        } else if(tab[fantome->L][fantome->C] != PACMAN){
            fantome->cache = tab[fantome->L][fantome->C];
        }

        tab[fantome->L][fantome->C] = pthread_self();
        if(modeLocale == 1){
            DessineFantome(fantome->L, fantome->C, fantome->couleur, directionFantome);
        }else{
            DessineFantomeComestible(fantome->L, fantome->C);

        }

        pthread_mutex_unlock(&mutexTab);

        pthread_mutex_lock(&mutexAtt);
        vitesseLocale = (delaiFrameMs / 3) * 5;
        pthread_mutex_unlock(&mutexAtt);
        // pour éviter une lecture incorrecte lors du changement
        Attente(vitesseLocale);


    }

    pthread_cleanup_pop(1);

}

// push du fantôme

void fantome_terminaison(void* fantome){


    pthread_mutex_lock(&mutexScore);
    pthread_mutex_lock(&mutexPacGom);

    S_FANTOME* descFantome = (S_FANTOME*) malloc(sizeof(S_FANTOME));

    *descFantome = *(S_FANTOME*) fantome;

    int cache = 0;

    if(descFantome->cache == PACGOM){
        cache = 1;
        nbPacGom--;
    } else if(descFantome->cache == BONUS){
        cache = 30;
    } else if(descFantome->cache == SUPERPACGOM ){
        cache = 5;
        nbPacGom--;
    }


    MAJscore = true;
    score += 50 + cache;

    if(descFantome->couleur == ROUGE){
        nbRouge--;
    }else if(descFantome->couleur == VERT){
        nbVert--;
    }else if(descFantome->couleur == MAUVE){
        nbMauve--;
    }else if(descFantome->couleur == ORANGE){
        nbOrange--;
    }


    pthread_mutex_unlock(&mutexPacGom);
    pthread_mutex_unlock(&mutexScore);


}

void *threadVie(void *) {

    while (nbVie > 0) {

        DessineChiffre(18, 22, nbVie);


        pthread_create(&pacMacThreadTid, NULL, threadPacMan, NULL);
        pthread_join(pacMacThreadTid, NULL);
        nbVie--;
    }

    if (nbVie < 1) {
        pthread_mutex_lock(&mutexTab);
        DessineGameOver(9, 4);
    }

}

void *threadTimeOut(void *nbSecRemain) {



    sigset_t maskSigAlarm;
    sigemptyset(&maskSigAlarm);
    sigaddset(&maskSigAlarm, SIGALRM);
    sigprocmask(SIG_UNBLOCK, &maskSigAlarm, NULL);

    struct sigaction sa;
    sa.sa_handler = &handlerSigAlarm;
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);


    pthread_mutex_lock(&mutexTransfertTimer);
    int temps = *((int*) nbSecRemain);
    pthread_mutex_unlock(&mutexTransfertTimer);

    pthread_cond_signal(&condTransfertTimer);

    nbSec = temps + 8 + (rand() % (15 - 8));
    alarm(nbSec);
    pause();



    pthread_exit(0);

    // todo: quand  on le mange, ajouter ce qu'il cache aussi au score

}

void handlerSigAlarm(int sig){
    // todo : changer le mode la dedans
    // et voir qui receptionne le signal
    TraceLine("****************************************\n****************************************\n****************************************\n****************************************\n****************************************\n****************************************\n****************************************\n****************************************\n");
    pthread_mutex_lock(&mutexMode);
    mode = 1;
    pthread_mutex_unlock(&mutexMode);
    pthread_cond_signal(&condCompteurFantomes);
    nbSec = 0;
    // todo: enlever le nbsec = 0, car on sait qu'il est a zero
}

void handlerSigChild(int Sig){

    pthread_exit(0);
}