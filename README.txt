Tema 1 SPRC
Saceleanu Andrei-Iulian, 343C1

*** Stub-ul server svc.cpp a fost modificat.
Toate fisierele necesare sunt incluse in arhiva.

Client side
Programul client proceseaza in ordine FIFO comenzile din operations_file.

Pentru actiuni de tip request:
1. cerere token de autorizare
- OK -> se trece la 2.
- status code != OK -> USER_NOT_FOUND

2. se aproba token-ul de autorizare
- OK -> se trece la 3.
- status code != OK -> "user-ul" nu a aprobat cererea, se returneaza token-ul
                        neschimbat si se trece la 3. ,
                        insa va esua cu REQUEST_DENIED

3. cerere token de acces
- OK -> toate structurile de gestionare din aplicatie sunt setate
        iar user-ul are un token de acces valid(si eventual un refresh_token)
- status code != OK -> REQUEST_DENIED

Pentru actiuni pe resurse(i.e. RIMDX)
1. daca actiunea de realizat va determina un TOKEN_EXPIRED si exista 
   refresh activ pentru user-ul care realizeaza actiunea, se cere un token
   de acces nou inainte de a trece la efectuarea propriu-zisa 2.

2. validate delegated action
- returneaza status code conform situatiei curente


Server side

1. Initializare folosind fisierele date ca parametru
- baza de date de utilizatori (un unordered_set)
- baza de date de resurse (un unordered_set)
- baza de date de permisiuni aprobate pentru fiecare request
  (un vector de unordered_map)

2. Ascultare port RPC pentru cereri de la clienti si procesare
actiuni si token requests


Modificari checker si teste

- Ending newline pentru fiecare fisier din directoarele expected_output
- valabilitatea jetoanelor pasata ca parametru server-ului pentru fiecare test
  folosind $(cat readme|grep number)
- sleep 1 - necesar pentru a avea un interval de timp intre initializare RPC
  la server si conectarea clientului

Observatii Makefile

- oprite register si unused-variable warnings datorita codului generat de 
  rpcgen 
