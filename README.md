<!---ENGLISH TRANSLATION SOON-->

# Kullman Alexandru, 323CA


# Tema 2 - Aplicatie client-server TCP si UDP pentru gestionarea mesajelor


# Info

- Aplicatia are in componenta fisiere sursa pentru server, client TCP si client
UDP. Serverul are rol de intermediar intre clienti, avand rolul de a gestiona
si interpreta informatia, a o trimite mai departe la destinatar sau de
a afisa anumite mesaje.
- Clientul TCP poate fi abonat la anumite topic-uri si va primi mesaje in
functie de acestea. Chiar si in cazul deconectarii, se pot pastra mesajele
restante si trimise cand clientul reface legatura cu serverul.

# Server

* In faza initiala
	* pentru server:
		* sunt setate informatiile despre server si portul acestuia, preluat din
		  linia de comanda
		* se goleste multimea de citire a file descriptorilor si cea temporara
		* se adauga file descriptorii pentru TCP, UDP si citirea de la STDIN in
		  multimea folosita de select
		* sunt declarati vectorul de clienti si map-ul pentru mesajele ce vor fi
		  stocate in timpul in care un client e deconectat

	* pentru UDP:
		* se creeaza file descriptor-ul
		* se creeaza legatura cu serverul prin intermediul comenzii **bind**
	* pentru TCP
		* se creeaza file descriptor-ul
		* se creeaza legatura cu serverul prin intermediul comenzii **bind**
		* este setat portul TCP pentru acceptarea unor viitoare conexiuni
		  prin comanda *listen*
* In timpul in care serverul este activ
	* Se verifica daca au fost primite date pe unul din socketi si programul
	  alege din urmatoarele cazuri:
		1. Socket TCP 
		   * se accepta conexiunea, se aloca un nou file descriptor
		   pentru socketul noului client si se adauga in multimea desscriptorilor
		   prin metoda de **accept**
		   * se goleste bufferul si se citesc datele primite pe noul socket
		   * se verifica daca id-ul clientului este valid, insemnand ca nu exista
		     deja un client conectat cu acelasi id. In cazul in care exista deja 
		     un client cu acelasi id, serverul inchide conexiunea cu cel care tocmai
		     a incercat conectarea
		   * se verifica daca este vorba despre o reconectare a unui client. In caz
		     afirmativ, acestuia i se trimit toate mesajele (cu sf = 1) primite in
		     timpul in care era deconectat
		   * daca este vorba despre un nou client, cu id valid, acesta este adaugat
		     in vectorul de clienti si se dezactiveaza algoritmul Nagle pe socket-ul
		     acestuia
		2. Socket UDP
			* dupa primirea datelor de pe socket-ul de UDP, urmeaza ca serverul sa 
			  parseze informatia tinand cont de tipul de date al mesajului
			* se apeleaza functia **parse_input** care completeaza campurile structurii
			  new_msg cu campurile corespunzatoare
			* se verifica pentru fiecare client in parte daca este abonat la topic-ul
			  mesajului primit, in caz afirmativ, trebuie sa ii fie trimis acest mesaj
			* pentru a trimite mesaje cu lungimi variabile la clientul TCP, ii este
			  transmisa prima oara dimensiunea mesajului, apoi mesajul in sine, de
			  dimensiune variabila, in functie de tipul de date
			* se verifica lista de clienti inactivi si se adauga mesajele pentru cei
			  care sunt abonati la respectivul topic, avand setat sf = 1, in cazul unei
			  eventuale reconectari
		3. Date primite de la STDIN
			* se verifica daca este primita comanda de exit. In caz afirmativ, se inchid
			  toate conexiunile clientilor TCP cu serverul si viceversa. Serverul
			  trimite un mesaj de close pe care clientii il vor interpreta si vor
			  inchide la randul lor conexiunea cu serverul.
			* altfel, comanda este invalida
		4. Date de la un client deja conectat
			* daca nu este primit niciun byte, inseamna ca respectivul client s-a
			  deconectat, caz in care se afiseaza un mesaj corespunzator si este
			  setat campul clientului de "active" la false
			* mesajul primit pentru subscribe este de tipul |<topic><S/U><0/1>|
			  cu semnificatia de subscribe/unsubscribe - topic - sf=0/sf=1
			* pentru subscribe, se adauga topic-ul cu sf-ul corespondent in map-ul
		        topics pentru acel client
			* pentru unsubscribe, se elimina din map perechea corespunzatoare topic-ului
* Finalul loop-ului
	* se elibereaza socket-ul de TCP si cel de UDP


# Client TCP (Subscriber)

* In faza initiala:
	* e setat socket-ul
	* sunt setate campurile serverului
	* se conecteaza clientul la server prin comanda de **connect**
	* se trimite un mesaj catre server reprezentand id-ul clientului
	* este setata dimensiunea de citire la 10 bytes

* In timpul in care clientul este activ:
	* sunt resetate campurile file descriptorilor in multimea de citire
	* programul alege din urmatoarele cazuri, in functie de socket-ul pe care
	  s-au primit date:
		1. Date de la STDIN
			* daca input-ul este de "exit", se iese din loop
			* se goleste buffer-ul si se citesc datele primite
			* se construieste string-ul ce va fi trimis la server in functie de comanda
			  de subscribe/unsubscribe care a fost data de la tastatura
			* e dezactivat Nagle-ul pentru server si se trimit datele corespunzatoare
		2. Date de la server
			* daca dimensiunea de citire este default (=10), atunci vom primi de la
			  server dimensiunea mesajului urmator si modificam variabila **read_size**
			* altfel, afisam buffer-ul si resetam dimensiunea de citire la cea default
			  (= 10)
		3. 
* La finalul loop-ului
	* se inchide conexiunea cu serverul


# helpers: protocols.cpp, protocols.h utils.h
	* functii de printare + definitia structurilor folosite
	* macro-ul DIE


Surse si materiale utilizate in rezolvarea temei:

https://ocw.cs.pub.ro/courses/pc/laboratoare/08
https://www.geeksforgeeks.org/udp-server-client-implementation-c/
https://stackoverflow.com/questions/31997648/where-do-i-set-tcp-nodelay-in-this-c-tcp-client


### Usage
```
usage: udp_client.py [-h] [--input_file FILE] [--source-address SOURCE_ADDRESS] [--source-port 1024-65535] [--mode {all_once,manual,random}] [--count COUNT] [--delay DELAY] server_ip server_port

UDP Client for Communication Protocols (2021-2022) Homework #2

optional arguments:
  -h, --help            show this help message and exit

Input:
  --input_file FILE     JSON file to read payloads from (default: sample_payloads.json)

Server Address & Port (required):
  server_ip             Server IP
  server_port           Server Port

Source Address:
  --source-address SOURCE_ADDRESS
                        IP Address to be bind by UDP client (default: unspecified)
  --source-port (1024-65535)
                        UDP port to be used as source for this client (default: random port)

Workload changing parameters:
  --mode {all_once,manual,random}
                        Specifies the mode used for the load generator as following:
                        * all_once - send each payload in the list once
                        * manual - let you choose which message to send next
                        * random - continuously send random payloads from the list

Load characteristics:
  --count COUNT         Number of packets to be send (only used for when mode is random, default: infinity)
  --delay DELAY         Wait time (in ms) between two messages (default: 0)
```
