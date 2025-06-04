**#Analiza komponenti i plan integracije komponenti sa serverom**

Ovaj dokument definira status integracije unutar AKDB sustava i predlaže plan povezivanja preostalih komponenti sa serverskom aplikacijom (server.py)

**Analiza postojećih komponenti**

| Direktorij     | Modul           | Opis                                                  | Povezan sa `server.py` |
|----------------|----------------|--------------------------------------------------------|-------------------------|
| `auxi/`        | Auxiliary       | Pomoćne funkcije (npr. parseri, logika)               | Ne                      |
| `dm/`          | Data Manager    | Upravljanje pohranom podataka                         | Ne                      |
| `file/`        | File System     | Upravljanje stranicama i datotekama                   | Ne                      |
| `html_files/`  | Dokumentacija   | HTML dokumenti                                        | Ne                      |
| `mm/`          | Memory Mgmt     | Upravljanje memorijom                                 | Ne                      |
| `opti/`        | Optimizer       | Optimizacija SQL upita                                | Ne                      |
| `rec/`         | Recovery        | Modul za oporavak nakon pada                          | Ne                      |
| `rel/`         | Relational      | Relacijski operatori (join, filter, scan)             | Ne                      |
| `sql/`         | SQL Engine      | `sql_executor.py`, SQL parser                         | Da                      |   
| `srv/`         | Server Logic    | `server_functions.py`, mrežna komunikacija            | Da                      |
| `swig/`        | SWIG Bridge     | Python <-> C sučelje (`kalashnikovDB.py`)             | Da                      |
| `tools/`       | Alati/Test      | Debugging, test skripte                               | Ne

**Identifikacija ključnih komponenti za integraciju**

##Prioritetna lista integracije

Ovih pet komponenti bilo bi dobro integrirati sa serverom.

1. trans/ - Ključan za ACID
2. rec/ - Oproavak nakon pada servera
3. rel/ - Omogućava JOIN, SELECT, FILTER
4. opti/ - Optimizacija za bolje performanse
5. dm/ - Upravljanje fizičkom pohranom

## Razvoj integracijskog sloja (API sučelje)

Potrebno je napraviti funkcionalni most između poslužitelja i svih funkcionalnih slojeva.

### Koraci izrade:
1. Definirati API sučelje u server.py
Treba dodati API-endpointove za transakcije (BEGIN, COMMIT, ROLLBACK) i omogućiti SELECT/INSERT/DELETE koji uključuju rel, sql, trans

2. Dodati C-wrapper sloj (ukoliko je potreban)
Za komponente koje nisu direktno dostupne iz Pythona.

## Testiranje integracije
Potrebno je izvršiti testiranje da se provjeri radi li komunikacija između servera i novih integriranih komponenti. 

### Koraci testiranja

1. Pisanje integracijskih testova
2. Usporedba rezultat s lokalnim testovima
3. Testovi uključuju upite koji testiraju indeks, aktiviraju transakciju i rollback te oporavak nakon crasha

## Pisanje dokumentacije

Naprviti README file sa svim potrebnim informacijama.





