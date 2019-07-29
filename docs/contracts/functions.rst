.. index:: ! functions

.. _functions:

********
Funzioni
********

.. _function-parameters-return-variables:

Parametri di Funzioni e Valori di Ritorno
=========================================

Come in JavaScript, le funzioni possono ricevere parametri in input. 
A differenza di JavaScript e C, le funzioni possono restituire un
numero arbitrario di valori.

Parametri delle Funzioni
------------------------

I parametri delle funzioni sono dichiarati allo stesso modo delle variabili, 
ed il nome dei parametri non utilizzati può essere omesso.

Per esempio, se il contratto accetta chiamate esterne con due interi, si può utilizzare
qualcosa del tipo::

    pragma solidity >=0.4.16 <0.7.0;

    contract Simple {
        uint sum;
        function taker(uint _a, uint _b) public {
            sum = _a + _b;
        }
    }

I parametri delle funzioni possono essere usati come una qualsiasi altra variabile
locale e possono essere anche assegnati ad un valore.

.. note::

  Una :ref:`external function<external-function-calls>` non può accettare array 
  multi dimensionali come parametro di input. 
  Questa funzionalità è disponibile se abilitata ``ABIEncoderV2`` attraverso l'inserimento 
  della riga ``pragma experimental ABIEncoderV2;`` nel codice sorgente.

  Una :ref:`internal function<external-function-calls>` può accettare una array multidimensionale
  senza attivare la funzionalità precedente..

.. index:: return array, return string, array, string, array of strings, dynamic array, variably sized array, return struct, struct

Variabili di Ritorno
--------------------

Le variabili di ritorno di una funzione sono dichiarate con la stessa sintassi dopo
la keyword ``returns``.

Per esempio, si supponga che vogliano essere restituiti due valori: la somma e il 
prodotto di due interi passati come parametri alla funzione. Si utilizza::

    pragma solidity >=0.4.16 <0.7.0;

    contract Simple {
        function arithmetic(uint _a, uint _b)
            public
            pure
            returns (uint o_sum, uint o_product)
        {
            o_sum = _a + _b;
            o_product = _a * _b;
        }
    }

I nomi delle variabili di ritorno possono essere omessi.
Le variabili di ritorno possono essere usate come qualsiasi altra variabile locale
e sono inizializzate con il loro :ref:`default value <default-value>` ed 
hanno quel valore se non impostato esplicitamente.

Si possono anche assegnare esplicitamente i valori di ritorno e successivamente
abbandonare la funzione utilizzando ``return;``,
o si possono direttamente fornire i valori di ritorno 
(sia uno solo che :ref:`più di uno<multi-return>`) direttamente con il ``return``
statement::

    pragma solidity >=0.4.16 <0.7.0;

    contract Simple {
        function arithmetic(uint _a, uint _b)
            public
            pure
            returns (uint o_sum, uint o_product)
        {
            return (_a + _b, _a * _b);
        }
    }

Questo formato è equivalente ad assegnare prima i valori alle variabili
di ritorno e poi utilizzare ``return;`` per abbandonare la funzione.

.. note::
    Non si possono restituire alcuni tipi da funzioni non internal,
    in particolare, array dinamici multidimensionali e strutture.
    Se la feature ``ABIEncoderV2`` è abilitata con ``pragma experimental
    ABIEncoderV2;`` nel codice sorgente, sono disponibili altri tipi di dati, 
    ma i tipi ``mapping`` sono ancora limitati ad un contratto singolo e non
    possono essere trasferiti.

.. _multi-return:

Restituire Molteplici Valori
----------------------------

Quando una funzione ha più valori di ritorno, lo statement 
``return (v0, v1, ..., vn)`` può essere utilizzato per restituire 
più valori. Il numero di componenti deve essere lo stesso del numero di
valori di ritorno.

.. index:: ! view function, function;view

.. _view-functions:

Funzioni View
=============

Le funzioni possono essere dichiarate ``view``, in questo caso 
assicurano di non modificare lo stato.

.. note::
  Se il target della compilazione è la EVM Byzantium o nuova (default) l'opcode
  ``STATICCALL`` viene utilizzato per le funzioni ``view`` che forzano lo stato
  a rimanere immutato durante l'esecuzione EVM. Per le funzioni ``view`` di libreria,
  viene utilizzato ``DELEGATECALL`` perché non esiste una combinazione di ``DELEGATECALL`` e ``STATICCALL``.
  Questo significa che le funzioni di libreria ``view`` non hanno controllo 
  run-time che evitano la modifica dello stato. 
  Questo fatto non dovrebbe impattare negativamente sulla sicurezza perché il codice 
  delle librerie solitamente è noto a compile-time e il controllo statico 
  effettua controllo a compile-time.

I seguenti statement sono considerati modificatori dello stato:

#. Scrittura su variabili di stato.
#. :ref:`Emissione di eventi <events>`.
#. :ref:`Creazione di altri contratti <creating-contracts>`.
#. Utilizzo di ``selfdestruct``.
#. Invio di Ether attraverso calls.
#. Chiamata ad una funzione non definita ``view`` o ``pure``.
#. Utilizzo di chiamate a basso livello.
#. Utilizzo di inline assembly con alcuni opcode.

::

    pragma solidity >=0.5.0 <0.7.0;

    contract C {
        function f(uint a, uint b) public view returns (uint) {
            return a * (b + 42) + now;
        }
    }

.. note::
  ``constant`` nelle funzioni è utilizzato come alias per ``view``, ma nella versione 0.5.0 è stato abbandonato.

.. note::
  I metodi getter sono marcati automaticamente ``view``.

.. note::
  Prima della versione 0.5.0, il compilatore non usa l'opcode ``STATICCALL``
  per le funzioni ``view``.
  Questo abilita la modifica dello stato nelle funzioni ``view`` attraverso l'uso 
  di conversioni di tipo non valide.
  Utilizzando ``STATICCALL`` per funzioni ``view``, le modifiche allo stato 
  sono bloccate a livello EVM.

.. index:: ! pure function, function;pure

.. _pure-functions:

Pure Function
=============

Le funzioni possono essere dichiarate ``pure``. In questo caso, queste funzioni
promettono di non leggere dallo stato o modificarlo.

.. note::
  Se la EVM target della compilazione è Byzantium o nuova (default) l'opcode ``STATICCALL`` 
  viene usato, il quale però non garantisce che lo stato non sia letto, me almeno non è
  modificato.

In aggiunta alla lista di operazioni modificatori di stato elencate sopra, le seguenti sono
considerate operazioni che leggono lo stato:

#. Lettura di variabili si stato.
#. Accesso a ``address(this).balance`` o ``<address>.balance``.
#. Accesso ad uno qualsiasi dei membri di ``block``, ``tx``, ``msg`` (ad eccezione di ``msg.sig`` e ``msg.data``).
#. Chiamata ad una funzione non contrassegnata con ``pure``.
#. Utilizzo di inline assembly che contiene alcuni opcode particolari.

::

    pragma solidity >=0.5.0 <0.7.0;

    contract C {
        function f(uint a, uint b) public pure returns (uint) {
            return a * (b + 42);
        }
    }

Le funzioni pure possono utilizzare le funzioni `revert()` e `require()` per ripristinare
potenziali cambi di stato quando :ref:`accade un errore <assert-and-require>`.
Il ripristino di una modifica di stato non è considerato una "modifica di stato".
Questo comportamento è in linea con l'opcode ``STATICCALL``.

.. warning::
  Non è possibile impedire ad una funzione la lettura dello stato a livello
  EVM, è solo possibile impedire la scrittura
  (solo ``view`` può essere applicato a livello EVM level, ``pure`` no).

.. note::
  Precedentemente alla versione 0.5.0, il compilatore non usa l'opcode ``STATICCALL``
  per le funzioni ``pure``.
  Questo permette la modifica di stato nelle funzioni
  ``pure`` attraverso l'utilizzo di conversioni tra tipi espliciti non valide.
  Utilizzando ``STATICCALL`` per funzioni ``pure``, le modifiche allo stato sono bloccate
  a livello EVM.

.. note::
  Prima della versione 0.4.17 il compilatore non imponeva che ``pure`` non leggesse lo stato.
  È un controllo a compile-time, che può essere evitato attraverso conversioni esplicite
  tra tipi, perché il compilatore può solamente verificare che i tipi non effettuino operazioni di cambi
  di stato, ma non può controllare che il contratto che verrà chiamato a runtime 
  sia di fatto di quel tipo.

.. index:: ! fallback function, function;fallback

.. _fallback-function:

Funzione Fallback
=================

Un contratto può avere esattamente una funzione senza nome. 
Questa funzione non può avere argomenti, non può restituire nulla e deve avere 
una visibilità `` external``.
Viene eseguito a seguito di una chiamata al contratto se nessuna delle altre 
funzioni corrisponde all'identificatore di funzione fornito (o se non è stato fornito 
alcun dato).

Inoltre, questa funzione viene eseguita ogni volta che il contratto riceve 
Ether (senza dati). Per ricevere Ether e aggiungerlo al saldo totale del contratto, 
la funzione di fallback deve essere contrassegnata come ``payable``. 
Se tale funzione non esiste, il contratto non può ricevere Ether attraverso transazioni 
regolari e genera un'eccezione.

Nel peggiore dei casi, la funzione di fallback può fare affidamento solo sulla 
disponibilità di 2300 gas (ad esempio quando si utilizza `send` o` transfer`), l
asciando poco spazio per eseguire altre operazioni tranne logging di base. 
Le seguenti operazioni consumano più gas rispetto al valore di 2300:

- Scrittura nello storage
- Creazione di un contratto
- Chiamata di una funzione external che consuma una grande quantità di gas
- Invio di Ether

Come qualsiasi funzione, la funzione di fallback può eseguire operazioni complesse purché vi sia abbastanza gas.

.. warning::
    La funzione di fallback viene eseguita anche se il chiamante intendeva chiamare una 
    funzione che non è disponibile. Se si desidera implementare la funzione di fallback 
    solo per ricevere Ether, è necessario aggiungere un controllo come 
    ``require(msg.data.length == 0)`` per evitare chiamate non valide.

.. warning::
    I contratti che ricevono Ether direttamente (senza una function call, usando per esempio ``send`` o ``transfer``)
    ma non definiscono una funzione di fallback 
    lanciano un'eccezione, mandando indietro Ether (comportamento diverso prima di
    Solidity v0.4.0). Se un contratto deve ricevere Ether, bisogna implementare una 
    funzione di fallback payable.

.. warning::
    Un contratto senza una funzione di fallback payable può ricevere Ether come a destinatario 
    di una `coinbase transaction` (`miner block reward`)
    o come destinazione di ``selfdestruct``.

.. note::
    Anche se la funzione di fallback non può avere argomenti, si può comunque usare ``msg.data`` per recuperare qualsiasi payload fornito con la chiamata.

    Un contratto non può reagire a tali trasferimenti di Ether e quindi non può rifiutarli. 
    Questa è una scelta progettuale dell'EVM e Solidity non può essere aggirata.

    Significa anche che `` address (this) .balance`` può essere superiore alla somma 
    di un qualche tipo di contatore manuale implementato in un contratto (ovvero avere 
    un contatore nella funzione di fallback).

::

    pragma solidity >=0.5.0 <0.7.0;

    contract Test {
        // Questa funzione è chiamata per tutti i messaggi inviati a
        // questo contratto (non c'è nessuna altra funzione).
        // L'invio di Ether a questo contratto causa una eccezione,
        // perché la funzione di fallback non ha il modificatore `payable`.
        function() external { x = 1; }
        uint x;
    }


    // Questo contratto mantiene tutto l'Ether inviatogli, senza possibilità
    // di recuperarlo.
    contract Sink {
        function() external payable { }
    }

    contract Caller {
        function callTest(Test test) public returns (bool) {
            (bool success,) = address(test).call(abi.encodeWithSignature("nonExistingFunction()"));
            require(success);
            // risultato di test.x == 1.

            // address(test) non permette la chiamata diretta a ``send``, dato che ``test`` non ha una 
            // funzione di fallback payable. Deve essere convertita al tipo ``address payable`` attraverso una
            // conversione intermedia a ``uint160`` per autorizzare anche la chiamata ``send``.
            address payable testPayable = address(uint160(address(test)));

            // Se qualcuno invia Ether al contratto,
            // il trasferimento fallisce, questo return restituisce false.
            return testPayable.send(2 ether);
        }
    }

.. index:: ! overload

.. _overload-function:

Overloading di Funzioni
=======================

Un contratto può avere diverse funzioni con lo stesso nome ma con
parametri di tipi differenti.
Questo processo è chiamato "overloading" e si applica anche alle funzioni ereditate.
L'esempio seguente mostra l'verloading della funzione ``f`` nello scope del contratto ``A``.

::

    pragma solidity >=0.4.16 <0.7.0;

    contract A {
        function f(uint _in) public pure returns (uint out) {
            out = _in;
        }

        function f(uint _in, bool _really) public pure returns (uint out) {
            if (_really)
                out = _in;
        }
    }

Le funzioni overloaded sono presenti anche nell'interfaccia esterna. 
È un errore se due funzioni esternamente visibili differiscono 
solamente nei tipi Solidity ma non negli external type.

::

    pragma solidity >=0.4.16 <0.7.0;

    // Questo esempio non compila
    contract A {
        function f(B _in) public pure returns (B out) {
            out = _in;
        }

        function f(address _in) public pure returns (address out) {
            out = _in;
        }
    }

    contract B {
    }


Entrambe le funzioni ``f`` dell'esempio sopra accettano il tipo di indirizzo per ABI 
anche se sono differenti all'interno di Solidity.

Risoluzione dell'Overload e Matching degli Argomenti
----------------------------------------------------

Le funzioni overloaded sono selezionate facendo il matching con la dichiarazione 
di funzione nello scope corrente con gli argomenti forniti alla chiamata di funzione. 
Le funzioni sono selezionate come candidati per l'overload se tutti 
gli argomenti possono essere convertiti implicitamente ai tipi richiesti. 
Se non esiste un candidato, la risoluzione fallisce.

.. note::
    I parametri di ritorno non sono considerati per la risoluzione dell'overload.

::

    pragma solidity >=0.4.16 <0.7.0;

    contract A {
        function f(uint8 _in) public pure returns (uint8 out) {
            out = _in;
        }

        function f(uint256 _in) public pure returns (uint256 out) {
            out = _in;
        }
    }

La chiamata a ``f(50)`` crea un errore di tipo perché ``50`` può essere implicitamente convertito sia a ``uint8``
che ``uint256``. D'altra parte, ``f(256)`` risolve a ``f(uint256)`` perché ``256`` non può
essere implicitamente convertito a ``uint8``.
