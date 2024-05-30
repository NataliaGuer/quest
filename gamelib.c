#include "gamelib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//--------------PLAYER RELATED CONST--------------

static const int NUM_PLAYERS = 4;
static const char *const PLAYER_CLASS_STR[] = {[barbaro] = "barbaro", [elfo] = "elfo", [nano] = "nano", [mago] = "mago"};

//--------------MAP RELATED CONST--------------

static const int NUM_MAP_ZONES = 15;
static const char *const ZONES_STR[] = {[corridio] = "corridoio",  [scala] = "scala",       [sala_banchetto] = "sala banchetto",
                                        [magazzino] = "magazzino", [giardino] = "giardino", [posto_guardia] = "posto guardia",
                                        [prigione] = "prigione",   [cucina] = "cucina",     [armeria] = "armeria",
                                        [tempio] = "tempio"};
static const char *const DOORS_STR[] = {[nessuna_porta] = "nessuna porta", [porta_normale] = "porta normale", [porta_da_scassinare] = "porta da scassinare"};
static const char *const TREASURES_STR[] = {
    [nessun_tesoro] = "nessun tesoro", [veleno] = "veleno", [guarigione] = "guarigione", [doppia_guarigione] = "doppia guarigione"};

//--------------GAME RELATED CONST--------------
static const char *OPPONENTS_STR[] = {"Goblin", "Fimir", "Gargoyle", "Orco", "Guerriero del caos"};

typedef struct Giocatore Giocatore;
typedef struct Zona_segrete Zona_segrete;

static Giocatore **players = NULL;
static Zona_segrete *pFirst = NULL;
static Zona_segrete *pLast = NULL;
static int mapSize = 0;
static int mapSetupComplete = 0;

struct Avversario {
  char nome[25];
  unsigned char dadi_attacco;
  unsigned char dadi_difesa;
  unsigned char p_vita;
};
typedef struct Avversario Avversario;

struct Turno {
  Giocatore *player;
  int can_advance;
  int remaining_moves;
};
typedef struct Turno Turno;

static int gameStatus = 1;
static Giocatore *winner = NULL;

//--------------PLAYER FUNCTIONS DECLARATION--------------

static int setupPlayers();
static int setupPlayer(Giocatore *);
static void setupBarbaro(Giocatore *);
static void setupNano(Giocatore *);
static void setupElfo(Giocatore *);
static void setupMago(Giocatore *);
static void printPlayer(Giocatore *);
static void freePlayer(Giocatore *);
static void freePlayers();

//--------------MAP FUNCTIONS DECLARATION--------------

static int setupMap();
static void initZone(Zona_segrete *);
static void generateMap();
static int handleInsertZoneMenuChoice();
static void insertZone(int);
static int handleDeleteZoneMenuChoice();
static void deleteZone(int);
static void freeMap();
static void printMap();
static void printZone(Zona_segrete *, int);
static void closeMap();

//--------------GAME FUNCTIONS DECLARATION--------------

static unsigned char *getTurnOrder();
static void initTurn(Turno *, Giocatore *);
static int interactWithDoor(Zona_segrete *, Giocatore *, Turno *);
static void moveToNextZone(Giocatore *, Turno *);
static void turnBack(Giocatore *, Turno *);
static Avversario *generateOpponent(Zona_segrete *);
static void useSpecialPower(Giocatore *, Avversario *);
static void printOpponent(Avversario *);
static int figthOpponent(Giocatore *, Avversario *);
static void playerAttacksOpponent(Giocatore *, Avversario *);
static void opponentAttacksPlayer(Avversario *, Giocatore *, int);
static void runAwayFromOpponent(Giocatore *, Avversario *, Turno *);
static int handleFightEnd(Avversario *, Giocatore *);
static void movePlayerPosition(Giocatore *, Zona_segrete *, Turno *);
static void freeOpponent(Avversario *);
static void takeTreasure(Giocatore *);
static void takePoison(Giocatore *);
static void takeHealing(Giocatore *);
static void takeDoubleHealing(Giocatore *);
static int printFightMenu(Giocatore *);
static int printZoneMenu(Giocatore *, Turno *);
static void handleFight(Giocatore *, Avversario *, Zona_segrete *, Turno *);
//--------------GENERAL PURPOSE FUNCTIONS DECLARATION--------------

static int printMenu(int, int, char *);
static void cleanAll();
static void endGame(int *);
static void cleanStdin();

//--------------LIB FUNCTIONS--------------

void imposta_gioco(int *playing) {

  if (players != NULL) {
    freePlayers();
  }

  int setupRes = setupPlayers();
  if (!setupRes) {
    return;
  }

  if (pFirst != NULL) {
    freeMap();
  }

  setupMap();
}

void gioca(int *playing) {
  gameStatus = 1;

  if (mapSetupComplete == 0 || players == NULL) {
    printf("Prima di poter iniziare a giocare devi impostare il gioco\n");
    return;
  }
  // all the players are positioned in the first zone of the map
  for (int i = 0; i < NUM_PLAYERS; i++) {
    if (players[i] != NULL) {
      players[i]->posizione = pFirst;
    }
  }

  int zoneChoice = 0;
  Turno *turn = malloc(sizeof(Turno));
  while (gameStatus == 1) {
    unsigned char *order = getTurnOrder();
    int i = 0;
    while (i < NUM_PLAYERS && gameStatus == 1) {

      unsigned char ordinal = order[i];

      if (players[ordinal] != NULL && players[ordinal]->p_vita > 0) {

        initTurn(turn, players[ordinal]);
        printf("\nÈ il turno di %s\n", players[ordinal]->nome);
        printf("Ti trovi nella zona %d\n", players[ordinal]->posizione->ordinal);

        while (gameStatus == 1 && players[ordinal] != NULL && turn->remaining_moves > 0) {
          zoneChoice = printZoneMenu(players[ordinal], turn);
          switch (zoneChoice) {
          case 0:
            gameStatus = 0;
            break;
          case 1:
            printPlayer(players[ordinal]);
            break;
          case 2:
            printZone(players[ordinal]->posizione, 0);
            break;
          case 3:
            turn->remaining_moves = 0;
            break;
          case 4:
            takeTreasure(players[ordinal]);
            break;
          case 5:
            turnBack(players[ordinal], turn);
            break;
          case 6:
            moveToNextZone(players[ordinal], turn);
            break;
          case 7:
            interactWithDoor(players[ordinal]->posizione, players[ordinal], turn);
            break;
          default:
            break;
          }
        }
      }

      i++;
    }
  }

  // gameStatus is 0 when a player reach the last zone or when all the players are dead
  if (winner != NULL) {
    printf("Complimenti %s, hai vinto!\n", winner->nome);
  } else {
    printf("Il gioco termina senza un vincitore.\n");
  }

  free(turn);
  cleanAll();
}

void termina_gioco(int *playing) {
  printf("\nGrazie per aver giocato.\n");
  endGame(playing);
}

//--------------PLAYER FUNCTIONS--------------

static int setupPlayers() {

  printf("\n*** Impostazione del gioco ***\nogni volta che ti verrà mostrato un menu potrai scegliere di interrompere la procedura e tornare al menu "
         "principale selezionando 0.\n\n");

  int playersNum = printMenu(1, 4, "Inserisci il numero di giocatori, da 1 a 4\n");
  if (!playersNum) {
    return 0;
  }

  players = malloc(sizeof(Giocatore *) * NUM_PLAYERS);
  // check if the memory allocation was successful
  if (players == NULL) {
    printf("Errore durante l'impostazione del gioco, riprova.\n");
    return 0;
  }

  int i = 0;
  int setup = 1;
  while (i < playersNum && setup) {
    Giocatore *player = malloc(sizeof(Giocatore));
    players[i] = player;
    setup = setupPlayer(player);
    i++;
  }

  if (!setup) {
    freePlayers();
    return 0;
  }

  for (int i = playersNum; i < NUM_PLAYERS; i++) {
    players[i] = NULL;
  }

  return 1;
}

static int setupPlayer(Giocatore *player) {
  char name[21];
  printf("\n*** Inizializzione nuovo giocatore ***\nInserisci il tuo nome (max 20 caratteri)\n");
  scanf("%20s", name);
  strcpy(player->nome, name);

  // class choise

  void (*setupF[4])(Giocatore *) = {&setupBarbaro, &setupNano, &setupElfo, &setupMago};

  int classChoice = printMenu(1, 4, "\nScegli la tua classe:\n1) barbaro\n2) nano\n3) elfo\n4) mago\n");
  if (!classChoice) {
    return 0;
  }

  // if we are here the current player selected a valid option

  classChoice--; // we work in the range [0,3]
  (*setupF[classChoice])(player);

  printPlayer(player);

  cleanStdin();
  char switchChoice;
  printf("Vuoi sacrificare un punto mente per un punto vita? (s=sì)\n");
  scanf("%1c", &switchChoice);
  if (switchChoice == 's') {
    player->mente--;
    player->p_vita++;
    printPlayer(player);
  }

  cleanStdin();
  switchChoice = '-';
  printf("Vuoi sacrificare un punto vita per un punto mente? (s=sì)\n");
  scanf("%c", &switchChoice);
  if (switchChoice == 's') {
    player->p_vita--;
    player->mente++;
    printPlayer(player);
  }

  return 1;
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
  if (player == NULL) {
    return;
  }

  printf("\n%s ecco le tue caratteristiche:\n"
         "Classe: %s\n"
         "Dadi attacco: %d\n"
         "Dadi difesa: %d\n"
         "Punti vita: %d\n"
         "Mente: %d\n"
         "Potere speciale: %d\n\n",
         player->nome, PLAYER_CLASS_STR[player->classe], player->dadi_attacco, player->dadi_difesa, player->p_vita, player->mente, player->potere_speciale);
}

static void freePlayer(Giocatore *player) {

  int i = 0;
  while (players[i] != player && i < NUM_PLAYERS) {
    i++;
  }

  free(players[i]);
  players[i] = player = NULL;

  // check if all the players are dead
  int allDead = 1;
  i = 0;
  while (allDead == 1 && i < NUM_PLAYERS) {
    if (players[i] != NULL) {
      allDead = 0;
    }
    i++;
  }

  if (allDead == 1) {
    gameStatus = 0;
  }
}

static void freePlayers() {
  if (players == NULL) {
    return;
  }

  for (int i = 0; i < NUM_PLAYERS; i++) {
    if (players[i]) {
      free(players[i]);
    }
  }
  free(players);
  players = NULL;
}

//--------------MAP FUNCTIONS--------------

static int setupMap() {

  printf("\n*** Generazione della mappa ***\n");
  // the menu options are: generate map, insert zone, delete zone, print map, close map
  int setup = 1;
  while (mapSetupComplete == 0 && setup == 1) {
    char *menuPrompt = "\nMenu generazione mappa:\n"
                       "1) genera mappa: genera una nuova mappa con 15 zone\n"
                       "2) inserisci zona: aggiunge una nuova zona nella posizione indicata\n"
                       "3) cancella zona: cancella la zona nella posizione indicata\n"
                       "4) stampa mappa\n"
                       "5) chiudi mappa: salva la mappa generata\n";

    int choice = printMenu(1, 5, menuPrompt);
    switch (choice) {
    case 0:
      setup = 0;
      break;
    case 1:
      generateMap();
      break;
    case 2:
      setup = handleInsertZoneMenuChoice();
      break;
    case 3:
      setup = handleDeleteZoneMenuChoice();
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

  return setup;
}

static void initZone(Zona_segrete *zone) {
  zone->zona = rand() % 10;
  zone->porta = rand() % 3;
  zone->porta_aperta = 0;
  zone->tesoro = rand() % 4;
  zone->contiene_tesoro = 1;
  zone->zona_precedente = NULL;
  zone->zona_succesiva = NULL;
}

static void generateMap() {
  if (pFirst != pLast) {
    freeMap();
  }

  Zona_segrete *currentZone = NULL;
  for (int i = 0; i < NUM_MAP_ZONES; i++) {
    Zona_segrete *newZone = malloc(sizeof(Zona_segrete));
    newZone->ordinal = i + 1;
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

static int handleInsertZoneMenuChoice() {
  char menuPrompt[90];
  sprintf(menuPrompt, "\nInserisci la posizione in cui aggiungere la nuova zona, da %d a %d\n", 1, mapSize + 1);

  int position = printMenu(1, mapSize + 1, menuPrompt);
  if (position == 0) {
    return 0;
  }

  insertZone(position);
  printMap();

  return 1;
}

static void insertZone(int position) {

  Zona_segrete *newZone = malloc(sizeof(Zona_segrete));
  initZone(newZone);
  newZone->ordinal = position;

  if (pFirst == NULL && pFirst == pLast) {
    // the map is empty
    pFirst = pLast = newZone;
    mapSize++;
    return;
  }

  // position is 1-based
  position--;
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

static int handleDeleteZoneMenuChoice() {
  if (mapSize == 0) {
    printf("La mappa è vuota!\n");
    return 0;
  }

  char menuPrompt[80];
  sprintf(menuPrompt, "\nInserisci la posizione della zona da eliminare, da %d a %d\n", 1, mapSize);

  int position = printMenu(1, mapSize, menuPrompt);

  if (!position) {
    return 0;
  }

  deleteZone(position);
  printMap();

  return 1;
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
    // we are deleting the only zone in the map
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
  if (pFirst == NULL && pLast == NULL) {
    return;
  }

  Zona_segrete *current = pLast;
  while (current != NULL) {
    Zona_segrete *prev = current->zona_precedente;
    free(current);
    current = prev;
  }

  mapSize = 0;
  mapSetupComplete = 0;
  pFirst = pLast = NULL;
}

static void printMap() {
  if (mapSize == 0) {
    printf("La mappa è vuota.\n\n");
    return;
  }

  int i = 1;
  printf("\n*** Mappa ***\n");
  Zona_segrete *zone = pFirst;
  while (zone != NULL) {
    printf("Zona %d:\n", i);
    printZone(zone, 1);

    zone = zone->zona_succesiva;
    i++;
  }

  printf("\n");
}

static void printZone(Zona_segrete *zone, int full) {
  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  if (full == 1) {
    printf("  zona numero: %d\n"
           "  tipo zona: %s\n"
           "  tipo tesoro: %s\n"
           "  tipo porta: %s\n",
           zone->ordinal, ZONES_STR[zone->zona], TREASURES_STR[zone->tesoro], DOORS_STR[zone->porta]);
  } else {
    char *porta = zone->porta == nessuna_porta ? "nessuna porta" : "zona con porta";
    char *tesoro = zone->tesoro == nessun_tesoro ? "nessun tesoro" : "zona con tesoro";
    printf("  zona numero: %d\n"
           "  tipo zona: %s\n"
           "  %s\n"
           "  %s\n",
           zone->ordinal, ZONES_STR[zone->zona], porta, tesoro);
  }
  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

static void closeMap() {
  if (mapSize < NUM_MAP_ZONES) {
    printf("La mappa contiene meno di %d zone, non è possibile chiuderla.\n", NUM_MAP_ZONES);
    return;
  }

  mapSetupComplete = 1;
}

//--------------GAME FUNCTIONS--------------
static unsigned char *getTurnOrder() {
  unsigned char *order = malloc(NUM_PLAYERS);
  // order population
  for (int i = 0; i < NUM_PLAYERS; i++) {
    order[i] = i;
  }

  for (int i = NUM_PLAYERS - 1; i >= 0; i--) {
    int j = rand() % (i + 1);

    unsigned char temp = order[i];
    order[i] = order[j];
    order[j] = temp;
  }

  return order;
}

static void initTurn(Turno *turn, Giocatore *player) {
  turn->player = player;
  turn->can_advance = 1;
  turn->remaining_moves = 3;
}

static int interactWithDoor(Zona_segrete *zone, Giocatore *player, Turno *turn) {

  if (zone->porta == porta_normale) {
    printf("Hai aperto la porta, ora puoi avanzare nella prossima zona.\n");
    zone->porta_aperta = 1;
    return 1;
  }

  printf("\nStai cercando di aprire una %s\n", DOORS_STR[zone->porta]);
  int diceRoll = (rand() % 6) + 1;
  printf("Lanci un D6 e ottieni %d, la tua caratteristica mente vale %d\n", diceRoll, player->mente);
  if (player->mente >= diceRoll) {
    printf("Complimenti hai aperto la porta, puoi avanzare nella prossima zona!\n");
    zone->porta_aperta = 1;
    return 1;
  }

  diceRoll = rand() % 10;
  printf("Non sei riuscito ad aprire la porta\n");
  // 5/10 chanches to lose a life point
  if (diceRoll <= 4) {
    printf("Perdi un punto vita\n");
    player->p_vita--;

    if (player->p_vita == 0) {
      printf("Sei morto!\n");
      freePlayer(player);
    }

    return 0;
  }

  // 4/10 chanches that an oppent will appear
  if (diceRoll <= 8) {
    Avversario *opponent = generateOpponent(player->posizione);
    printf("\nAppare un abitante delle segrete\n");
    printOpponent(opponent);
    handleFight(player, opponent, player->posizione, turn);
    return 0;
  }

  // 1/10 chanche that the player will start over
  printf("Torni indietro alla prima zona.\n");
  player->posizione = pFirst;
  return 0;
}

static void moveToNextZone(Giocatore *player, Turno *turn) {
  if (turn->can_advance == 0) {
    printf("Puoi avanzare solo una volta per turno\n");
    return;
  }

  if (player->posizione->porta != nessuna_porta && player->posizione->porta_aperta == 0) {
    printf("\nDevi prima aprire la porta della zona!\n");
    return;
  }

  movePlayerPosition(player, player->posizione->zona_succesiva, turn);

  int diceRoll = 0;
  if (player->posizione->zona_succesiva == NULL) {
    printf("\nSei arrivato nella zona finale!\n");
  } else {
    diceRoll = rand() % 3;
  }

  if (diceRoll == 0) {
    printf("\nAppare un abitante delle segrete\n");
    Avversario *opponent = generateOpponent(player->posizione);
    printOpponent(opponent);
    handleFight(player, opponent, player->posizione, turn);
  }
}

static void turnBack(Giocatore *player, Turno *turn) {
  // when the player decides to move to the previous zone there is one chance in three that
  // an opponent is in the zone
  if (player->posizione->zona_precedente == NULL) {
    printf("\nTi trovi nella prima zona, non puoi indietreggiare\n");
    return;
  }

  movePlayerPosition(player, player->posizione->zona_precedente, turn);
  int diceRoll = rand() % 3;
  if (diceRoll == 0) {
    printf("\nAppare un abitante delle segrete\n");
    Avversario *opponent = generateOpponent(player->posizione);
    handleFight(player, opponent, player->posizione, turn);
  }
}

static void handleFight(Giocatore *player, Avversario *opponent, Zona_segrete *zone, Turno *turn) {
  int fightChoice = 0;

  while (fightChoice == 0) {
    fightChoice = printFightMenu(player);
  }

  switch (fightChoice) {
  case 1:
    figthOpponent(player, opponent);
    break;
  case 2:
    runAwayFromOpponent(player, opponent, turn);
    break;
  case 3:
    useSpecialPower(player, opponent);
    break;
  default:
    break;
  }
}

static Avversario *generateOpponent(Zona_segrete *zone) {
  int base = rand() % 2;

  Avversario *opponent = malloc(sizeof(Avversario));
  switch (zone->tesoro) {
  case doppia_guarigione:
    // tra 2 e 3
    opponent->dadi_attacco = base + 2;
    // tra 2 e 3
    opponent->dadi_difesa = base + 2;
    // tra 3 e 4
    opponent->p_vita = base + 3;
    break;
  case guarigione:
    opponent->dadi_attacco = base + 1;
    opponent->dadi_difesa = base + 2;
    opponent->p_vita = base + 2;
    break;
  case veleno:
  case nessun_tesoro:
    opponent->dadi_attacco = base + 1;
    opponent->dadi_difesa = base + 1;
    opponent->p_vita = base + 2;
    break;
  default:
    break;
  }

  if (zone->zona_succesiva == NULL) {
    opponent->dadi_attacco++;
    opponent->dadi_difesa++;
    opponent->p_vita++;
  }

  strcpy(opponent->nome, OPPONENTS_STR[rand() % 5]);

  return opponent;
}

static void useSpecialPower(Giocatore *player, Avversario *opponent) {
  if (player->potere_speciale <= 0) {
    return;
  }

  player->potere_speciale--;
  printf("\nUccidi l'avversario utilizzando il tuo potere speciale\n");
  freeOpponent(opponent);
}

static void printOpponent(Avversario *opponent) {
  printf(">>> Il tuo avversario:\n"
         ">>> tipo: %s\n"
         ">>> dadi attacco: %d\n"
         ">>> dadi difesa: %d\n"
         ">>> punti vita: %d\n",
         opponent->nome, opponent->dadi_attacco, opponent->dadi_difesa, opponent->p_vita);
}

static int figthOpponent(Giocatore *player, Avversario *opponent) {
  while (player->p_vita > 0 && opponent->p_vita > 0) {
    printf("\n--- inizia un nuovo turno ---\n");
    int playerDiceRoll = rand() % 6;
    int opponentDiceRoll = rand() % 6;

    if (playerDiceRoll >= opponentDiceRoll) {
      playerAttacksOpponent(player, opponent);
      if (opponent->p_vita > 0 && player->p_vita > 0) {
        // after defending himself, the opponent attacks
        opponentAttacksPlayer(opponent, player, 0);
      }
    } else {
      opponentAttacksPlayer(opponent, player, 0);
      if (player->p_vita > 0 && opponent->p_vita > 0) {
        // after defending himself, the player attacks
        playerAttacksOpponent(player, opponent);
      }
    }

  }

  return handleFightEnd(opponent, player);
}

static void playerAttacksOpponent(Giocatore *player, Avversario *opponent) {
  unsigned char i = 0;
  int playerHits = 0;
  while (i < player->dadi_attacco) {
    int diceRoll = rand() % 2;
    // the player scores an hit when the dice roll results in a skull
    // there a 3 skulls on a dice, so the player has 1 possibility over 2 to score an hit
    if (diceRoll == 0) {
      playerHits++;
    }
    i++;
  }

  printf("\n--- %s attacca %s ---\n", player->nome, opponent->nome);
  printf("Lanci i tuoi dadi attacco e ottieni %d teschi\n", playerHits);

  if (playerHits == 0) {
    printf("Non infliggi nessun dannno all'avversario.\n");
    return;
  }

  int opponentDefenses = 0;
  i = 0;
  while (i < opponent->dadi_difesa && i < playerHits) {
    // the opponent has 1 possibility over 6 to defend
    int diceRoll = rand() % 6;
    if (diceRoll == 0) {
      opponentDefenses++;
    }
    i++;
  }

  printf("L'avversario riesce a parare %d colpi\n", opponentDefenses);
  if (playerHits > opponentDefenses) {
    int opponentDemage = playerHits - opponentDefenses;
    if (opponent->p_vita <= opponentDemage) {
      opponent->p_vita = 0;
    } else {
      opponent->p_vita -= opponentDemage;
    }
    printf("L'avversario perde %d punti vita\n", opponentDemage);
  }
}

static void opponentAttacksPlayer(Avversario *opponent, Giocatore *player, int duringEscape) {
  unsigned char i = 0;
  int opponentHits = 0;
  while (i < opponent->dadi_attacco) {
    int diceRoll = rand() % 2;
    if (diceRoll == 0) {
      opponentHits++;
    }
    i++;
  }

  printf("\n--- %s attacca %s ---\n", opponent->nome, player->nome);
  printf("Il mostro lancia tutti i suoi dadi attacco e ottiene %d teschi\n", opponentHits);

  if (opponentHits == 0) {
    printf("Il mostro non ti infligge danni.\n");
    return;
  }

  int playerDefenses = 0;
  i = 0;
  int playerDefenseDice = duringEscape == 0 ? player->dadi_difesa : player->dadi_difesa / 2;
  while (i < playerDefenseDice && i < opponentHits) {
    // there are 2 white shields on a defense dice
    // the player has 1 possibility over 3 to defend
    int diceRoll = rand() % 3;
    if (diceRoll == 0) {
      playerDefenses++;
    }
    i++;
  }

  printf("Riesci a parare %d colpi\n", playerDefenses);
  if (opponentHits > playerDefenses) {
    int playerDemage = opponentHits - playerDefenses;
    if (player->p_vita <= playerDemage) {
      player->p_vita = 0;
    } else {
      player->p_vita -= playerDemage;
    }
    printf("Perdi %d punti vita\n", playerDemage);
  }
}

static void runAwayFromOpponent(Giocatore *player, Avversario *opponent, Turno *turn) {

  if (player->posizione->zona_precedente == NULL) {
    printf("\nSei nella prima zona, non puoi scappare!\n");
    opponentAttacksPlayer(opponent, player, 1);

    handleFightEnd(opponent, player);
    return;
  }

  // the player can decide whether fo fight or run away
  int diceRoll = (rand() % 6) + 1;
  printf("\nLanci un D6, ottieni: %d\nLa tua caratteristica mente vale: %d\n", diceRoll, player->mente);
  if (diceRoll <= player->mente) {
    printf("Riesci ad indietreggiare\n");
    freeOpponent(opponent);
    // in the case that the player ran away from an oppent his position is updated and there is no generation
    // of a new opponent
    movePlayerPosition(player, player->posizione->zona_precedente, turn);
    return;
  }

  printf("Non sei riuscito a indietreggiare, l'avversario ti attacca\n");
  // the opponent attaks and the player defens with half dice
  opponentAttacksPlayer(opponent, player, 1);
  handleFightEnd(opponent, player);
}

static int handleFightEnd(Avversario *opponent, Giocatore *player) {
  if (opponent->p_vita == 0) {
    printf("\nHai ucciso l'avversario\n");
    freeOpponent(opponent);
    if (player->posizione->zona_succesiva == NULL) {
      printf("\nAbbiamo un vincitore!\n");
      gameStatus = 0;
      winner = player;
    }
    return 1;
  }

  printf("\nSei stato ucciso dall'avversario\n");
  freePlayer(player);
  return 0;
}

static void movePlayerPosition(Giocatore *player, Zona_segrete *to, Turno *turn) {
  player->posizione = to;
  // when one player enters a zone the treasure is regenerated
  to->contiene_tesoro = 1;
  turn->remaining_moves--;
  turn->can_advance = 0;

  printf("\nLa tua nuova posizione:\n");
  printZone(player->posizione, 0);
}

static void freeOpponent(Avversario *opponent) {
  free(opponent);
  opponent = NULL;
}

static void takeTreasure(Giocatore *player) {
  if (player->posizione->tesoro == nessun_tesoro) {
    printf("\nNon c'è nessun tesoro in questa zona\n");
    return;
  } else if (player->posizione->contiene_tesoro == 0) {
    printf("\nIl tesoro di questa zona è già stato raccolto\n");
    return;
  }

  void (*effect[3])(Giocatore *) = {&takePoison, &takeHealing, &takeDoubleHealing};
  (*effect[player->posizione->tesoro - 1])(player);

  // the treasure is removed once collected
  player->posizione->contiene_tesoro = 0;
}

static void takePoison(Giocatore *player) {
  printf("\nIl tesoro è un veleno, perdi 2 punti vita\n");
  if (player->p_vita <= 2) {
    printf("Il veleno si rivela mortale!\n");
    freePlayer(player);
  } else {
    player->p_vita -= 2;
  }
}

static void takeHealing(Giocatore *player) {
  printf("\nIl tesoro è una guarigione, guadagni un punto vita\n");
  player->p_vita++;
}

static void takeDoubleHealing(Giocatore *player) {
  printf("\nIl tesoro è una guarigione doppia, guadagni due punti vita\n");
  player->p_vita += 2;
}

static int printFightMenu(Giocatore *player) {
  if (player->potere_speciale == 0) {
    return printMenu(1, 2, "\nMenu combattimento:\n1) combatti\n2) scappa\n");
  }
  return printMenu(1, 3, "\nMenu combattimento:\n1) combatti\n2) scappa\n3) usa potere speciale\n");
}

static int printZoneMenu(Giocatore *player, Turno *turn) {

  if (turn->remaining_moves == 0) {
    return printMenu(0, 4,
                     "\n*** Menu movimento ***\n"
                     "1) stampa le tue info\n"
                     "2) stampa info zona\n"
                     "3) passa il turno\n"
                     "4) raccogli tesoro\n");
  }

  if (turn->can_advance == 0) {
    return printMenu(0, 5,
                     "\n*** Menu movimento ***\n"
                     "1) stampa le tue info\n"
                     "2) stampa info zona\n"
                     "3) passa il turno\n"
                     "4) raccogli tesoro\n"
                     "5) indietreggia\n");
  }

  if (player->posizione->zona_succesiva != NULL) {
    if (player->posizione->porta == nessuna_porta || player->posizione->porta_aperta == 1) {
      return printMenu(0, 6,
                       "\n*** Menu movimento ***\n"
                       "1) stampa le tue info\n"
                       "2) stampa info zona\n"
                       "3) passa il turno\n"
                       "4) raccogli tesoro\n"
                       "5) indietreggia\n"
                       "6) avanza\n");
    } else {
      return printMenu(0, 7,
                       "\n*** Menu movimento ***\n"
                       "1) stampa le tue info\n"
                       "2) stampa info zona\n"
                       "3) passa il turno\n"
                       "4) raccogli tesoro\n"
                       "5) indietreggia\n"
                       "6) avanza\n"
                       "7) apri porta\n");
    }
  }

  return 0;
}
//--------------GENERAL PURPOSE FUNCTIONS--------------

static int printMenu(int optionsFrom, int optionsTo, char *menuPrompt) {
  int choice = -1;

  do {
    printf("%s", menuPrompt);
    if (scanf("%d", &choice) != 1) {
      cleanStdin();
      choice = -1;
    }

    if (choice != 0 && choice < optionsFrom || choice > optionsTo) {
      printf("Seleziona una delle opzioni disponibili, 0 per tornare al menu principale.\n");
      choice = -1;
    }
  } while (choice == -1);

  return choice;
}

static void cleanAll() {
  freeMap();
  freePlayers();
}

static void endGame(int *playing) {
  cleanAll();
  *playing = 0;
}

static void cleanStdin() {
  while (getchar() != '\n') {
  };
}