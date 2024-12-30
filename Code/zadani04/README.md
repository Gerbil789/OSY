# Simultánka s klienty

Upravte si soketového klienta tak, aby neustále posílal měnící se řádky textu na server a přijatá data zobrazoval.

Upravte soketový server tak, aby pomocí více procesů obsluhoval více klientů a odesílal zpět klientovi všechna data, která obdrží.

Kód pro potomka oddělte do samostatné funkce, ať je zřejmé, kdo provádí kterou část kódu.

Pro střídání hry vytvořte pole `N` semaforů (pro testování stačí `N=2`, pro předvedení musí být `N` minimálně `3`).

Semafor 1 musí být na začátku `1`, ostatní semafory startují od `0`.

Použijte pojmenované semafory. Jména semaforů musí obsahovat pořadové číslo a číslo portu.

Každý potomek `[n]` bude opakovat následujicí sekvenci:
- `DOWN( sem[ n ] )`
- příjme a odešle zpět `(M=)10` očíslovaných řádků
- předá řízení dalšímu procesu `UP( sem[ ( n + 1) % N])`