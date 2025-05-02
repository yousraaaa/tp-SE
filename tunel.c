#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>

#define NB_X 5
#define NB_Y 4
#define ALLER_RETOURS 10

typedef enum { DIRECTION_XY, DIRECTION_YX, AUCUNE } Direction;

pthread_mutex_t verrous = PTHREAD_MUTEX_INITIALIZER;
Direction sens_actuel = AUCUNE;
int bus_dans_tunnel = 0;
int attente_XY = 0, attente_YX = 0;

void pause_aleatoire() {
    usleep((rand() % 500 + 1000) * 1000); // 1 à 1.5 secondes
}

const char* direction_to_string(Direction d) {
    return d == DIRECTION_XY ? "X->Y" : "Y->X";
}

void entrer(Direction d) {
    int en_attente = 1;
    while (en_attente) {
        pthread_mutex_lock(&verrous);
        if (bus_dans_tunnel == 0)
            sens_actuel = d;

        if (sens_actuel == d) {
            bus_dans_tunnel++;
            en_attente = 0;
        } else {
            if (d == DIRECTION_XY)
                attente_XY++;
            else
                attente_YX++;
        }
        pthread_mutex_unlock(&verrous);
        if (en_attente)
            usleep(100000); // Pause courte avant de réessayer
    }
}

void sortir(Direction d) {
    pthread_mutex_lock(&verrous);
    bus_dans_tunnel--;
    if (bus_dans_tunnel == 0) {
        if (d == DIRECTION_XY && attente_YX > 0) {
            sens_actuel = DIRECTION_YX;
            attente_YX = 0;
        } else if (d == DIRECTION_YX && attente_XY > 0) {
            sens_actuel = DIRECTION_XY;
            attente_XY = 0;
        } else {
            sens_actuel = AUCUNE;
        }
    }
    pthread_mutex_unlock(&verrous);
}

void* routine_bus(void* arg) {
    int id = *(int*)arg;
    free(arg);

    const char* ville_origine = id <= NB_X ? "X" : "Y";
    const char* ville_dest = id <= NB_X ? "Y" : "X";
    Direction aller = id <= NB_X ? DIRECTION_XY : DIRECTION_YX;
    Direction retour = id <= NB_X ? DIRECTION_YX : DIRECTION_XY;

    for (int i = 1; i <= ALLER_RETOURS; i++) {
        entrer(aller);
        printf("Bus %d de %s : %s -> %s (Trajet %d)\n", id, ville_origine, ville_origine, ville_dest, i);
        pause_aleatoire();
        sortir(aller);

        entrer(retour);
        printf("Bus %d de %s : %s -> %s (Trajet %d)\n", id, ville_origine, ville_dest, ville_origine, i);
        pause_aleatoire();
        sortir(retour);
    }

    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));
    pthread_t bus_threads[NB_X + NB_Y];

    for (int i = 0; i < NB_X + NB_Y; i++) {
        int* id_ptr = malloc(sizeof(int));
        *id_ptr = i + 1;
        pthread_create(&bus_threads[i], NULL, routine_bus, id_ptr);
    }

    for (int i = 0; i < NB_X + NB_Y; i++) {
        pthread_join(bus_threads[i], NULL);
    }

    printf("\nTous les trajets de bus sont terminés.\n");
    return 0;
}
