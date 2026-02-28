Student: Amet Levent
Grupa: 323CDa
Tema 1: Dataplane Router
--------------------

1. Procesul de dirijare:
-----------------------
Am considerat schema de realizare a laboratorului 4, in care urmam urmatorii
    pasi pentru a trimite pacehete IPv4;
        - verificam ca headerul sa fie de tip Ethernet(0x0800);
        - in cazurile in care, checksum-ul nu mai este bun, campul time to live
            este 1 sau un eventul drum pana la destinatie nu exista, aruncam
            pachetul;
        - decrementam ttl-ul si recalculam checksum-ul pentru noile atribute;
        - aflam interfata viitoarei destinatii (de pe ruta cea mai buna), si
            punem adresele corespunzatoare sursei si destinatiei in header;
        - trimitem pachetul;

2. Longest Prefix Match eficient:
--------------------------------
Pentru a afla cel mai bun drum pe care putem trimite pachetele, trebuie sa
    aplicam algoritmul LPM pe tabela de rutare. Nu mai descriu ce face acesta, 
    pentru ca ar insemna sa dau copy-paste la enunt. In schimb, as scoate in
    evidenta faptul ca trebuie sa alegem cea mai specifica cale, de aceea in
    loc sa o gasim printr-o parcurgere liniara care se poate dovedi ineficienta,
    eu am ales sa fac un binary search pe lista de intrari (sortata descrescator
    cu qsort dupa prefix, apoi dupa masca).Atunci cand gasim o valoare, care se
    potriveste ca prefix, nu e suficient sa o alegem, ci tb sa le vedem si pe
    cele dinainte, care pot sa fie mai specifice decat ce am gasit.

3. Protocolul ARP:
-----------------
Nu am implementat, chiar imi pare rau ca nu am facut-o pentru ca nu parea greu,
    dar a durat ceva timp pana sa ma lamuresc de celelalte aspecte ale temei.

4. Protocolul ICMP:
------------------
Am implementat o functie care da un reply de tip ICMP, atunci cand este apelata.
    Aceasta functie este apelata cu un specificator de tip pentru a sti ce fel
    de pachet ICMP intoarcem ; 0 - Echo Reply, 11 - Time Exceeded, 
    3 - Destination Unreachable.

Pareri personale:
 - Mi se pare foarte interesanta tema si am invatat foarte multe din ea.
 - Consider totusi ca e destul de greu ca un student sa se prinda foarte rapid
    de anumite aspecte ale temei (trebuie mult timp de studiu individual)
 - Per total, o tema care mi-a placut.

Probleme intampinate:
 - Cand dau reply de tip ICMP pentru Time Exceeded, conditia trebuie sa fie ca
    ttl-ul sa fie mai mic sau egal cu 1, ceea ce mi se pare foarte dubios si
    acum (rezolvarea problemei aflata de la colegi);
 - LPM nu-mi functioneaza in acest moment perfect, pica ultimul test, poate
    reusesc sa-l rezolv, am scris aici ca sa se consemneze.(PS; Pana la urma s-a
    rezolvat, incurcam termenii si semnele intre ei intre ei)
