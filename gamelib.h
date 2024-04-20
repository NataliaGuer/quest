// functions interfaces

void imposta_gioco(int *);
void gioca(int *);
void termina_gioco(int *);

// room data

enum Tipo_zona {
  corridio,
  scala,
  sala_banchetto,
  magazzino,
  giardino,
  posto_guardia,
  prigione,
  cucina,
  armeria,
  tempio
};

enum Tipo_tesoro { nessun_tesoro, veleno, guarigione, doppia_guarigione };

enum Tipo_porta { nessuna_porta, porta_normale, porta_da_scassinare };

struct Zona_segrete {
  enum Tipo_zona zona;
  enum Tipo_tesoro tesoro;
  enum Tipo_porta porta;
  struct Zona_segrete *zona_succesiva;
  struct Zona_segrete *zona_precedente;
};

// player data

enum classe_giocatore { barbaro, nano, elfo, mago };

struct Giocatore {
  char nome[20];
  enum classe_giocatore classe;
  struct Zona_segrete *posizione;
  unsigned char p_vita;
  unsigned char dadi_attacco;
  unsigned char dadi_difesa;
  unsigned char mente;
  unsigned char potere_speciale;
};