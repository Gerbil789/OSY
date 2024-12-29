# Piškvorky

## Úkol

1. Upravte soketový server tak, aby přijímal spojení od více klientů a pro každého klienta si vytvořil samostatný proces. Poté, co přijme první dva klienty, bude další spojení odmítat.

2. Kód pro klienta pište do samostatné funkce.

## Pravidla hry

- Dva připojení klienti spolu budou hrát piškvorky.
- Server si pro hru vytvoří hrací pole minimálně 5x5 ve sdílené paměti a vyplní ho tečkami.
  - Pro data je vhodné vytvořit strukturu, lépe se pak pracuje s typy a přetypováním.
- Indexování hracího pole bude: řádky číslem a sloupce písmenem.
- Klienti budou střídavě zasílat své požadavky na provedení tahu ve tvaru "znak-číslo\n", tedy např. "C-3\n".
- Při zadání špatného tahu bude hráč vyzván k novému zadání.
- Po správném zadání tahu se klientovi odešle aktuální stav hracího pole a provede se přepnutí na druhého hráče.

## Činnost serveru

1. Server:
   - Vytvoří sdílenou paměť (a semafory).
   - Přijme spojení s klientem, přidělí mu index 0/1, vytvoří mu proces a dále opakuje.
   - Zašle hlášku: "Prosím o strpení...".

## Poznámky
- Každý proces klienta:
  - Zajišťuje komunikaci se svým příslušným klientem.
  - Aktualizuje sdílené hrací pole na základě tahů hráče.
- Server po připojení dvou klientů odmítá další připojení.
