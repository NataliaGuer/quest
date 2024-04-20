#include "gamelib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//--------------PLAYER RELATED CONST--------------

static const int NUM_PLAYERS = 4;
static const char *const PLAYER_CLASS_STR[] = {
  [barbaro] = "barbaro",
  [elfo] = "elfo",
  [nano] = "nano",
  [mago] = "mago"
};

//--------------MAP RELATED CONST--------------

static const int NUM_MAP_ZONES = 2;
static const char *const ZONES_STR[] = {
  [corridio] = "corridoio", 
  [scala] = "scala",
  [sala_banchetto] = "sala banchetto",
  [magazzino] = "magazzino", 
  [giardino] = "giardino", 
  [posto_guardia] = "posto guardia",
  [prigione] = "prigione",
  [cucina] = "cucina",
  [armeria] = "armeria",
  [tempio] = "tempio"
};
static const char *const PORTS_STR[] = {
  [nessuna_porta] = "nessuna porta", 
  [porta_normale] = "porta normale", 
  [porta_da_scassinare] = "porta da scassinare"
};
static const char *const TREASURES_STR[] = {
  [nessun_tesoro] = "nessun tesoro", 
  [veleno] = "veleno", 
  [guarigione] = "guarigione", 
  [doppia_guarigione] = "doppia guarigione"
};

typedef struct Giocatore Giocatore;
typedef struct Zona_segrete Zona_segrete;

static Giocatore **players = NULL;
static Zona_segrete *pFirst = NULL;
static Zona_segrete *pLast = NULL;
static int mapSize = 0;
static int mapSetupComplete = 0;

//--------------PLAYER FUNCTIONS DECLARATION--------------

/**
 * the function is responsible for the managment of the steps
 * required to create a new player
 */
static void setupPlayer(Giocatore *, int *);
/**
 * the function contains the setup values to create a barbarian class player
 */
static void setupBarbaro(Giocatore *);
/**
 * the function contains the setup values to create a dwarf class player
 */
static void setupNano(Giocatore *);
/**
 * the function contains the setup values to create a elf class player
 */
static void setupElfo(Giocatore *);
/**
 * the function contains the setup values to create a wizard class player
 */
static void setupMago(Giocatore *);
static void printPlayer(Giocatore *);
static void freePlayers();

//--------------MAP FUNCTIONS DECLARATION--------------

/**
 * the function manage the interaction with the game master for the creation of the game map
 */
static void setupMap(int *);
/**
 * the function contains the logic to generate a map zone
 */
static void initZone(Zona_segrete *);
/**
 * the function is part of the user menu during the map generation process,
 */
static void generateMap();
/**
 * asks the game master the info required to create a new zone for the maps
 * and handle its creation
*/
static void handleInsertZoneMenuChoice(int *);
/**
 * inserts a zone in the map at the given position
 */
static void insertZone(int);
static void handleDeleteZoneMenuChoice(int *playing);
/**
 * removes the zone in the map at the given position
 */
static void deleteZone(int);
/**
 * removes all the zones in the map
 */
static void freeMap();
static void printMap();
static void printZone(Zona_segrete *);
/**
 * checks the validity of the generated map and saves its state
 */
static void closeMap();

//--------------GENERAL PURPOSE FUNCTIONS DECLARATION--------------

static int printMenu(int , int , char *);
static void endGame(int *);
static void cleanStdin();

//--------------LIB FUNCTIONS--------------

void imposta_gioco(int *playing) {
  // chiede di inserire il numero di giocatori

  players = malloc(sizeof(Giocatore *) * NUM_PLAYERS);
  // check if the memory allocation was successful
  if (players == NULL) {
    printf("Errore durante l'impostazione del gioco, riprova.\n");
    return endGame(playing);
  }

  printf("\nInizio impostazione del gioco,\nogni volta che ti verrà mostrato un menu potrai scegliere di interrompere la procedura e tornare al menu principale selezionando 0.\n\n");

  int playersNum = printMenu(1, 4, "Inserisci il numero di giocatori, da 1 a 4\n");
  if (playersNum == 0) {
    return endGame(playing);
  }

  int i = 0;
  while (i < playersNum && *playing == 1)
  {
    Giocatore *player = malloc(sizeof(Giocatore));
    players[i] = player;
    setupPlayer(player, playing);
    i++;
  }
  
  //if there was an error during the initialization of the player we exit the game
  if (*playing == 0) {
    return endGame(playing);
  }

  setupMap(playing);

  endGame(playing);
}

void gioca(int *playing) {}

void termina_gioco(int *playing) {}

//--------------PLAYER FUNCTIONS--------------

static void setupPlayer(Giocatore *player, int *playing) {
  char name[20];
  printf("\n*** Inizializzione nuovo giocatore ***\nInserisci il tuo nome (max 20 caratteri)\n");
  scanf("%s", name);
  strcpy(player->nome, name);

  // class choise

  void (*setupF[4])(Giocatore *) = {&setupBarbaro, &setupNano, &setupElfo, &setupMago};

  int classChoice = printMenu(1, 4, "Scegli la tua classe:\n1) barbaro\n2) nano\n3) elfo\n4) mago\n");
  if(classChoice == 0) {
    *playing = 0;
    return;
  }
  // if we are here the current player selected a valid option

  classChoice--; // we work in the range [0,3]
  (*setupF[classChoice])(player);

  printPlayer(player);

  cleanStdin();
  char switchChoice;
  printf("Vuoi sacrificare un punto mente per un punto vita? (s=sì,n=no)\n");
  scanf("%c", &switchChoice);
  if (switchChoice == 's') {
    player->mente--;
    player->p_vita++;
    printPlayer(player);
  }

  cleanStdin();
  switchChoice = '-';
  printf("Vuoi sacrificare un punto vita per un punto mente? (s=sì,n=no)\n");
  scanf("%c", &switchChoice);
  if (switchChoice == 's') {
    player->p_vita--;
    player->mente++;
    printPlayer(player);
  }
}

static void setupBarbaro(Giocatore *player) {
  player->classe = barbaro;
  player->dadi_attacco = 3;
  player->dadi_difesa = 2;
  player->p_vita = 8;
  player->mente = (rand() % 2) + 1;
  player->potere_speciale = 0;
}

static void setupNano(Giocatore *player) {
  player->classe = nano;
  player->dadi_attacco = 2;
  player->dadi_difesa = 2;
  player->p_vita = 7;
  player->mente = (rand() % 2) + 2;
  player->potere_speciale = 1;
}

static void setupElfo(Giocatore *player) {
  player->classe = elfo;
  player->dadi_attacco = 2;
  player->dadi_difesa = 2;
  player->p_vita = 6;
  player->mente = (rand() % 2) + 3;
  player->potere_speciale = 1;
}

static void setupMago(Giocatore *player) {
  player->classe = mago;
  player->dadi_attacco = 1;
  player->dadi_difesa = 2;
  player->p_vita = 4;
  player->mente = (rand() % 2) + 4;
  player->potere_speciale = 3;
}

static void printPlayer(Giocatore *player) {
  printf("%s ecco le tue caratteristiche:\n"
         "Classe: %s\n"
         "Dadi attacco: %d\n"
         "Dadi difesa: %d\n"
         "Punti vita: %d\n"
         "Mente: %d\n"
         "Potere speciale:%d\n\n",
         player->nome, PLAYER_CLASS_STR[player->classe], player->dadi_attacco, player->dadi_difesa, player->p_vita, player->mente, player->potere_speciale);
}

static void freePlayers() {
  for (int i = 0; i < NUM_PLAYERS; i++) {
    free(players[i]);
  }
  free(players);
}

//--------------MAP FUNCTIONS--------------

static void setupMap(int *playing) {

  printf("pFirst: %p\t pLast: %p\n", pFirst, pLast);
  printf("\n*** Generazione della mappa ***\n");
  // the menu options are: generate map, insert zone, delete zone, print map, close map
  while (mapSetupComplete == 0 && *playing == 1) {
    char *menuPrompt = "\nMenu generazione mappa:\n"
      "1) genera mappa: genera le prime 15 zone della mappa\n"
      "2) inserisci zona: aggiunge una nuova zona nella posizione indicata\n"
      "3) cancella zona: cancella la zona nella posizione indicata\n"
      "4) stampa mappa\n"
      "5) chiudi mappa\n";

    int choice = printMenu(1, 5, menuPrompt);

    switch (choice) {
      case 0:
        *playing = 0;
        break;
      case 1:
        generateMap();
        break;
      case 2:
        printf("pFirst: %p\n", pFirst);
        handleInsertZoneMenuChoice(playing);
        break;
      case 3:
        handleDeleteZoneMenuChoice(playing);
        break;
      case 4:
        printMap();
        break;
      case 5:
        closeMap();
        break;
      default:
        break;
    }
  }
}

static void initZone(Zona_segrete *zone) {
  zone->zona = rand() % 10;
  zone->porta = rand() % 3;
  zone->tesoro = rand() % 4;
  zone->zona_precedente = NULL;
  zone->zona_succesiva = NULL;
}

static void generateMap() {
  if (pFirst != pLast) {
    freeMap(pLast);
  }

  Zona_segrete *currentZone = NULL;
  for (int i = 0; i < NUM_MAP_ZONES; i++) {
    Zona_segrete *newZone = malloc(sizeof(Zona_segrete));
    initZone(newZone);
    newZone->zona_precedente = currentZone;

    if (currentZone) {
      currentZone->zona_succesiva = newZone;
    } else {
      // we are creating the first zone
      pFirst = newZone;
    }

    currentZone = newZone;
  }

  // when the for loop ends currentZone points to the last zone of the map
  pLast = currentZone;
  pLast->zona_succesiva = NULL;

  mapSize = NUM_MAP_ZONES;
  printMap();
}

static void handleInsertZoneMenuChoice(int *playing) {
  char menuPrompt[90];
  sprintf(menuPrompt, "\nInserisci la posizione in cui aggiungere la nuova zona, da %d a %d\n", 1, mapSize+1);
  
  int position = printMenu(1, mapSize+1, menuPrompt);

  if (position == 0) {
    *playing = 0;
    return;
  }
  
  insertZone(position);
  printMap();
}

static void insertZone(int position) {
  
  Zona_segrete *newZone = malloc(sizeof(Zona_segrete));
  initZone(newZone);

  if (pFirst == NULL && pFirst == pLast) {
    //the map is empty
    pFirst = pLast = newZone;
    mapSize++;
    return;
  }

  //position is 1-based
  position --;
  Zona_segrete *prevZone = NULL;
  Zona_segrete *nextZone = pFirst;
  int i = 0;
  // we want to point to the zone that will be the next for the new zone
  while (i < position && nextZone != NULL) {
    prevZone = nextZone;
    nextZone = nextZone->zona_succesiva;
    i++;
  }

  newZone->zona_precedente = prevZone;
  newZone->zona_succesiva = nextZone;
  if (prevZone != NULL) {
    prevZone->zona_succesiva = newZone;
  } else {
    // if the new zone hasn't a prevzone it's the new first zone
    pFirst = newZone;
  }
  if (nextZone != NULL) {
    nextZone->zona_precedente = newZone;
  } else {
    pLast = newZone;
  }

  mapSize++;
}

static void handleDeleteZoneMenuChoice(int *playing) {
  if (mapSize == 0) {
    printf("La mappa è vuota!\n");
    return;
  }

  char menuPrompt[80];
  sprintf(menuPrompt, "\nInserisci la posizione della zona da eliminare, da %d a %d\n", 1, mapSize);
  
  int position = printMenu(1, mapSize+1, menuPrompt);

  if (position == 0) {
    *playing = 0;
    return;
  }

  deleteZone(position);
  printMap();
}

static void deleteZone(int position) {
  if (pFirst == NULL && pFirst == pLast) {
    return;
  }

  position--;
  Zona_segrete *currentZone = pFirst;

  int i = 0;
  while (i < position && currentZone != NULL) {
    currentZone = currentZone->zona_succesiva;
    i++;
  }

  if (currentZone == NULL) {
    printf("Errore durante la cancellazione della zona.\n");
    return;
  }

  if (currentZone->zona_precedente == NULL && currentZone->zona_succesiva == NULL) {
    //we are deleting the only zone in the map
    pFirst = pLast = NULL;
    free(currentZone);
    mapSize--;
    return;
  }

  if (currentZone->zona_precedente != NULL) {
    currentZone->zona_precedente->zona_succesiva = currentZone->zona_succesiva;
  } else {
    // the zone to delete is the first
    pFirst = currentZone->zona_succesiva;
  }

  if (currentZone->zona_succesiva != NULL) {
    currentZone->zona_succesiva->zona_precedente = currentZone->zona_precedente;
  } else {
    // the zone to delete is the last
    pLast = currentZone->zona_precedente;
  }
  free(currentZone);

  mapSize--;
}

static void freeMap() {
  Zona_segrete *current = pLast;
  while (current != NULL)
  {
    Zona_segrete *prev = current->zona_precedente;
    free(current);
    current = prev;
  }
  
  mapSize = 0;
  pFirst = pLast = NULL;
}

static void printMap(){
  if (mapSize == 0) {
    printf("La mappa è vuota.\n\n");
    return;
  }

  int i = 1;
  printf("***Mappa***\n");
  Zona_segrete *zone = pFirst;
  while (zone != NULL)
  {
    printf("Zona %d:\n", i);
    printZone(zone);

    zone = zone->zona_succesiva;
    i++;
  }

  printf("\n");
}

static void printZone(Zona_segrete *zone) {
  //TODO remove pointer info
  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  printf("  zona: %p\n"
         "  tipo zona: %s\n"
         "  tipo tesoro: %s\n"
         "  tipo porta: %s\n"
         "  zona prec: %p\n"
         "  zona succ: %p\n",
         zone, ZONES_STR[zone->zona], TREASURES_STR[zone->tesoro], PORTS_STR[zone->porta], zone->zona_precedente, zone->zona_succesiva);
  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

static void closeMap() {
  if (mapSize < NUM_MAP_ZONES) {
    printf("La mappa contiene meno di %d zone, non è possibile chiuderla.\n", NUM_MAP_ZONES);
    return;
  }

  mapSetupComplete = 1;
}

//--------------GENERAL PURPOSE FUNCTIONS--------------

static int printMenu(int optionsFrom, int optionsTo, char *menuPrompt){
    int choice = -1;
    do
    {
      printf("%s", menuPrompt);
      if (scanf("%d", &choice) != 1) {
        cleanStdin();
        choice = -1;
      }
      
      if (choice != 0 && choice < optionsFrom || choice > optionsTo){
        printf("Seleziona una delle opzioni disponibili, 0 per tornare al menu principale.\n");
        choice = -1;
      }
    } while (choice == -1);

    return choice;
}

static void endGame(int *playing) {
  printf("endgame: pFirst: %p pLast:%p\n", pFirst, pLast);
  freeMap(pLast);
  freePlayers();
  *playing = 0;
}

static void cleanStdin() {
  while (getchar() != '\n') {
  };
}