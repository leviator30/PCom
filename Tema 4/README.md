# Tema 4 - Client web. Comunicatie cu REST API (Protocoale de comunicatii)

## Student: Amet Levent - 323CD

1.request.c
    - am folosit drept schelet al temei, laboratorul 9, impreuna cu biblioteca
        parson.h recomandata;
    - am implementat functiile de get_request, post_request si delete_request,
        conform laboratorului 9;
    - construiesc cererile de tip HTTP, in care adaug informatiile primite din
        apel.

2.client.c
    - am implementat functiile cerute, intr-o bucla while in care astept
        comenzile pe rand;
    - la inceput, am deschis la fiecare noua comanda un port nou, pe care il
        inchidem la final, alaturi de eliberarea memoriei pentru message si
        response;
    - prima oara am folosit scanf, dar am avut de verificat cazurile speciale
        cand un username se scrie cu spatii sau numarul de pagini nu e valid,
        deci am considera fgets.

3.parson.h
    - am ales sa folosesc biblioteca parson din simplul fapt ca aceasta ne-a
        fost recomandata sa o folosim la tema;
    - aceasta ofera functii de parsare simple si eficiente pentru manipularea
        JSON-ului, ceea ce rezulta in cod mai curat, frumos si usor de inteles.

4.Comenzi implementate:
    - REGISTER - construim o cerere HTTP de tip POST, cu datele de inregistrare,
                    pe care o trimitem la server;
                - daca se intoarce 201, utilizatorul a fost inregstrat cu succes;
                - se afiseaza mesajul serverului in caz de eroare;
                - am adaugat cazul in care username-ul nu e valid.

    - LOGIN - cerere HTTP de tip POST cu datele de logare, username si password
            - daca utilizatorul este conectat, nu va mai face operatia si va
                returna eroare;
            - din raspunsul serverului, se extrag cookie-urile care ne garanteaza 
                accesul la urmatoarele comenzi care necesita sa fii logat;
            - am adaugat cazul in care username-ul nu e valid.

    - ENTER_LIBRARY - cerere HTTP de tip GET, unde nu se cer date;
                    - pe baza tokenului de sesiune, realizam o conectare cu
                        biblioteca;
                    - din raspunsul serverului, scoatem un token de sesiune
                        activ pe care o sa-l includem in mesajele care necesita
                        acces la biblioteca;
                    - daca avem un token de sesiune activ, intoarcem un mesaj
                        care ne spune ca suntem deja conectati;
                    - daca variabila connectionStatus nu e activa, intoarcem un
                        mesaj care spune ca nu suntem logati.

    - GET_BOOKS - cerere HTTP de tip GET, unde nu se cer date;
                    - pe baza tokenului de sesiune verific conectarea la
                        biblioteca;
                    - se afiseaza cartile aflate in biblioteca fiecarui
                        utilizator, sub forma initiala de json.

    - GET_BOOK - cerere HTTP de tip GET, unde se cere id-ul cartii pe care
                    dorim sa o afisam, verificam ca id-ul sa fie valid;
                - verificam conectarea la biblioteca folosind tokenul de acces;
                - la finalul rutei de acces, adaugam si id-ul cartii, sub forma
                    de string;
                - pe baza noii rute de acces, extragem cartea prin mesaj si
                    raspuns de la server.

    - ADD_BOOK - cerere HTTP de tip POST, in care se cer datele despre carti,
                    avem cazul special de verificat cand numarul de pagini nu
                    este un numar si verificam ca fiecare caracter sa fie o
                    cifra valida (intoarcem eroare specifica daca gasim vreun
                    caracter din string care nu e cifra valida);
                - in rest, identic ca pana acum, cream cererea, o trimitem la
                    server si asteptam raspunsul de confirmare sau eroare.

    - DELETE_BOOK - cerere HTTP de tip DELETE, unde se cere id-ul cartii care
                    trebuie sterse, se verfica ca id-ul sa fie valid;
                    - in rest, identic ca pana acum, cream cererea, o trimitem la
                    server si asteptam raspunsul de confirmare sau eroare.

    - LOGOUT - cerere HTTP de tip GET, care nu are nevoie de parametrii;
                -se trimite cererea la server si se asteapta raspunsul de 
                    confirmare sau eroare.

    - EXIT - dam break la bucla while. 

    - bad writting - in caz ca se introduce o comanda neexistenta, se continua
                        bucla while cu o comanda noua.
