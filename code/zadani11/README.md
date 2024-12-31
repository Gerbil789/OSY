# Remote shell

Upravte soketový server tak, aby po soketu přijaté řádky interpretoval jako příkazy s parametry, provedl je a výstup přesměroval zpět do soketu.

Server pro každého nového klienta provede následující kroky:

- pro nového klienta vytvoří nový proces, kód procesu bude v samostatné funkci
- rozloží si přijatý řádek na příkaz a argumenty
- vytvoří proces pro příkaz
  - provede přesměrování a příkaz
- počká na dokončení příkazu
- končí

ošetřete pomocí semaforu, aby se nikdy neprováděly dva příkazy současně.

na příkaz `"exit"` bude reagovat ukončením spojení.