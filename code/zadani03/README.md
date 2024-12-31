# Přenos .txt souboru

Soketový server příjme soubor a odešle zpět s očíslovanými řádky.

Klient odešle textový soubor a následně uloží do souboru přijatá data.


Upravte soketový server tak, aby s pomocí vláken byl schopen komunikovat s více klienty.

Od těchto klientů vždy nejprve příjme řádek `"LENGTH\n"` (např. `"1485\n"`) s délkou následujicích dat a odpoví `"OK\n"`.

Činnost serveru:
- po připojení nového klienta pro něj vytvoří vlákno
  - příjme `"LENGHT\n"`
  - ? `DOWN()`
  - odešle `"OK\n"`
  - vytvoří si buffer požadované délky
  - příjme zadaný počet bajtů
  - odešle zpět data s očíslovanými řádky
  - ? `UP()`
  - zavírá soket, zruší buffer

Semaforem zajistěte, aby číslování řádků mohlo provádět vždy jen jedno vlákno.

---

Upravte si soketového klienta tak, aby měl na vstupu další dva argumenty: `"$./socket_cl <ip> <port> <InFile> <OutFile>"`

Program si otevře zadaný soubor, zjistí si jeho velikost a informuje o ni server.

Počká na odpověd `"OK\n"` a odešle zadaný soubor.
Po odeslání dat příjme data ze serveru a uloží do souboru.

Činnost klienta:
- navázání spojení
- otevření obou souboru a odeslání velikosti vstupního souboru na server
- čekání na `"OK\n"`
- odeslání celého souboru na server
- čtení dat ze serveru a ukladání do souboru

---

build: `$ make`

run server: `$ ./socket_srv -d 3333`

run client: `$ ./socket_cl -d localhost 3333 input.txt output.txt`

### moje poznámky
- toto řešení nefunguje pro dlouhe soubory (`input_2.txt`)