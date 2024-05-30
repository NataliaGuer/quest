# Progetto-finale-2023-Scalogna-Quest
Progetto finale Programmazione Procedurale UniPG Informatica

## Nome: 
Natalia

## Cognome: 
Guerrini

## Matricola:
320165

## Commenti/modifiche al progetto:

### Generazione della mappa
La funzione genera_mappa sovrascrive le eventuali 15 zone della mappa precedentemente generate. 

### Logiche di generazione avversario
I valori assunti dalle caratteristiche dadi_attacco, dadi_difesa, p_vita degli abitanti delle segrete che possono comparire durante il gioco vengono decisi in base al tipo di tesoro contenuto nella stanza in cui compare l'abitante; ogni caratteristica può assumere uno di due valori con uguali probabilità.

Nel dettaglio, se la stanza contiene:
- un tesoro di tipo guarigione:
  - dadi_attacco in [2,3]
  - dadi_difesa in [2,3]
  - p_vita in [3,4]
- un tesoro di tipo doppia guarigione
  - dadi_attacco in [1,2]
  - dadi_difesa in [2,3]
  - p_vita in [2,3]
- veleno o nessun tesoro
  - dadi_attacco in [1,2]
  - dadi_difesa in [1,2]
  - p_vita in [2,3]

Se ci troviamo nell'ultima stanza tutti i valori vengono incrementati di 1.
