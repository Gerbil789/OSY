# Početní simultánka

Upravte si soketový server tak, aby klientům počítal jejich matematické výrazy a posílal zpět vypočtené.

Pro každého připojeného klienta si server vytvoří samostatné vlákno.

Server příjme maximálně spojení s `N` klienty, další zdvořile odmítne.

Pro střídání hry vytvořte na začátku programu pole N semaforů, kde se pro testování stačí `N=4`.

Střídání bude server provádět vždy po vypočtení 10 výrazů.

Semafor 0 musí být na začátku `1`, ostatní semafory startují od `0`.

Použijte pojmenované semafory. Jména semaforů musí obsahovat pořadové číslo a číslo portu.

Každé vlákno `[n]` bude opakovat nasledujicí sekvenci:

- `DOWN( sem [n])`
- oakuje `X(=10)x`
  - příjme výraz a odešle zpět vypočítaný výraz
- předá řízení dalšímu procesu `UP( sem[ ( n + 1 ) % M])`, kde M je počet aktuálně připojených klientů

---

Upravte soketového klienta, aby generoval náhodné matematické výrazy `"int[+-*/%]int"`.

Po odeslání výrazu vždy počká na odpověd.