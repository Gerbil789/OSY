# Piškvorky

Upravte soketový server tak, aby příjmal spojení od více klientů a pro první dva klienty si vytvoří samostatná **vlákna**.

Poté co příjme první dva klienty, bude další klienty evidovat jen jako "*diváky*"

První dva připojení klienti spolu budou hrát piškvorky.

Server si pro hru vytvoří hrací pole minimálně 6x6 a vyplní ho tečkami.

Indexování hracího pole bude: řádky číslem a sloupce písmenem.
Klienti budou střídavě zasílat své požadavky na provedení tahu ve tvaru `"znak:číslo\n"`, tedy např. `"C:3\n"`.

Při zadání špatného tahu bude hráč vyzván k novému zadání.

Po správném zadání tahu se oběma hráčum i divákům odešle aktuální stav hracího pole a provede se přepnutí na druhého hráče pomocí **semaforu**.

Činnost serveru:
- inicializuje potřebné proměné (? a **pojmenované semafory**, jména musí obsahovat číslo portu)
- příjme spojení se dvěma klienty, přiřadí jim index 0/1 a vytvoří jim **vlákno**.
- další klienty zařadí mezi "*diváky*" (bez vlákna) a dále pro hráče opakuje:
  - zašle hráči hlášku: prosím o strpení
  - ? `DOWN(sem[index])`
  - zašle hrací pole
  - zašle hlášku: zadej tah
  - počká na tah od klienta
  - zpracuje tah
  - pokud je tah špatný, požádá o nový
  - zašle hrací pole sobě a divákům
  - ? `UP( sem[!index])`

Střídání hráčů zajistí dvojice **semaforů**.

---

Upravte kód tak, aby server ignoroval data, která jsou serveru zaslána dříve než je hráč vyzván k dalšímu tahu. Použijte **poll**

---

### moje poznámky
- nevím k čemu je v zadání ten pojmenovaný semafor :d
- nepoužil jsem **poll** pro čtení dat od hráče protože se mi nechce. Někdo to může spravit xd
- Piškvorky reálně nefungujou, není žádná kontrola jestli je nějaký počet znaků v řadě a není žádný vítěz
- Když se odpojí hráč v průběhu hry tak se to asi rozbije xd

---

build: `$ make`

run server: `$ ./socket_srv -d 3333`

run client: `$ ./socket_cl -d localhost 3333`