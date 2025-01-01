# README - Implementace soketoveho serveru a klienta

## Prehled projektu
Tento projekt zahrnuje vyvoj soketoveho serveru a klienta pro praci s obrazky ulozenymi v adresari `galerie`. Server zpracovava pozadavky klienta na seznam, zobrazeni, stazeni a nahrani obrazku prostrednictvim soketove komunikace. Projekt klade duraz na praci s vlakny, spravu procesu a pouziti semaforu.

## Funkcionalita
### Server
- Server posloucha prichozi pripojeni klientu a pro kazdeho klienta spusti nove vlakno.
- Server muze zpracovavat nasledujici prikazy klienta:
  - `ls` - Odesle klientovi seznam dostupnych obrazku z adresare `galerie`.
  - `show image [timeout]` - Zobrazi pozadovany obrazek pro klienta pomoci prikazu `display`. Pokud je zadan timeout, obrazek se zobrazi po urcitou dobu.
  - `get image` - Odesle klientovi pozadovany obrazek.
  - `put image` - Prijme obrazek od klienta a ulozi ho do adresare `galerie`.
- Server pouziva procesy (pomoci `fork`) pro zobrazeni obrazku a muze spravovat potomky prikazy jako `exec`, `sleep` a `kill`.
- Semafor je implementovan k zajisteni, ze v jednu chvili je zobrazen pouze jeden obrazek.
- Server potvrzuje uspesne operace `put image` odpovedi "OK".

### Klient
- Klient se pripoji k serveru a odesle prikazy.
- Prikazy jsou formatovany nasledovne:
  ```
  [prikaz] [parametry]
  ```
- Podporovane prikazy zahrnuji:
  - `ls` - Seznam obrazku ze serveru.
  - `show image [timeout]` - Pozadavek na zobrazeni obrazku.
  - `get image` - Stahne specifikovany obrazek ze serveru.
  - `put image` - Nahraje obrazek na server a ceka na odpoved "OK".
- Klient pred odeslanim prikazu overi potrebne parametry.

## Pozadavky na implementaci
- **Minimum**: Implementace prikazu `ls`, `show`, `get` a funkcni semafor.
- **Maximum**: Implementace timeoutu pro `show` a prikazu `put`.

## Nastaveni a spusteni
1. Zkompilujte a spustte server prikazem:
   ```
   ./server
   ```
2. Spustte klienta prikazem:
   ```
   ./client [adresa_serveru] [prikaz] [parametry]
   ```
3. Ujistete se, ze obrazky jsou ulozeny v adresari `galerie` na serveru.

## Poznamky
- Server obsluhuje kazdeho klienta v samostatnem vlakne pro zajisteni konkurence.
- Forkovane potomky spravuji zobrazeni obrazku, aby nedoslo k zablokovani hlavniho vlakna serveru.
- Spravna obsluha chyb je dulezita pro pripady, kdy obrazky nejsou nalezeny nebo jsou zadany neplatne prikazy.

## Budouci zlepseni
- Implementace logovani aktivity serveru.
- Zvyseni bezpecnosti validaci nahranych obrazku.
- Pridani podpory pro zmenu velikosti nebo konverzi obrazku pri nahrani.

