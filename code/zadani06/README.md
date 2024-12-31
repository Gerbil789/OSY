# Síťový semafor

Upravte soketový server tak, aby pomocí více procesů obsluhoval více klientů a odpovídal jim na požadavky:

- `"UP\n"`, na který odpoví klientovi `"UP-OK\n"`
- `"DOWN\n"`, na který odpoví klientovi `"DOWN-OK\n"`
- na jakýkoliv jiný požadavek odpoví `"ERR\n"`

Na serveru se pak musí vytvořit jeden skutečný semafor a pro každého klienta se pak v procesech potomků budou koordinovat požadavky od klienta s tímto semaforem:

- server obdrží `"DOWN\n"`
- server zavolá `DOWN()`
- server odpoví `"DOWN-OK\n"`

Podobně bude proveden i požadavek `UP`.

Použijte **pojmenovaný semafor**. Jméno semaforu musí obsahovat i číslo portu.

Kód pro potomka oddělte do samostatné funkce, ať je zřejmé, kdo provádí kterou část kódu.

Dále upravte soketového klienta tak, aby vykonával *"nějakou"* činnost a po připojení min. 3 klientů k serveru bude zřejmé, že je jejich činnost koordinována semaforem ze serveru. 