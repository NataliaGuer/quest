#include <stdio.h>

#include "gamelib.h"

int main(int argc, char const *argv[]) {
  void (*p[3])(int *) = {&imposta_gioco, &gioca, &termina_gioco};

  /**
   * the playing variable is used to keep trace of the internal status of the
   * game; its editable from the game's functions so when an exeption occurs or
   * the players want to, the game can be stopped
   */
  int playing = 1;
  do {
    // prints menu
    int menuChoice;
    int temp;
    printf("\n*** Menu principale ***:\n1) Imposta gioco\n2) Gioca\n3) Termina Gioco\n");
    if (scanf("%d", &menuChoice) != 1) {
      // buffer cleaning
      while (getchar() != '\n') {
      }
      menuChoice = 0;
    }

    if (!(menuChoice >= 1 && menuChoice <= 3)) {
      printf("Seleziona una delle opzioni disponibili!\n");
    } else {
      // we use the range [0,2] to invoke the proper function
      menuChoice--;
      (*p[menuChoice])(&playing);
    }

  } while (playing);

  return 0;
}