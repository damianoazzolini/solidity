.. index:: ! visibility, external, public, private, internal

.. _visibility-and-getters:

*******************
Visibilità e Getter
*******************


Poiché in Solidity eistono due tipi di chiamate di funzione 
(internal che non creano una vera chiamata EVM (chiamata anche "messgae call") 
ed external quelli che lo fanno), esistono quattro tipi di visibilità per 
funzioni e variabili di stato.

Le funzioni devono essere specificate con ``external``,
``public``, ``internal`` o ``private``.
Per le variabili di stato, ``external`` non è consentito.

``external``:
    Le funzioni external fanno parte dell'interfaccia del contratto, 
    il che significa che possono essere chiamate da altri contratti e 
    tramite transazioni. Una funzione esterna `` f`` non può essere 
    richiamata internamente (cioè `` f () `` non funziona, ma `` this.f () `` funziona).
    Le funzioni esterne sono talvolta più efficienti quando ricevono 
    grandi array di dati.
    
``public``:
    Le funzioni pubbliche fanno parte dell'interfaccia del contratto 
    e possono essere chiamate internamente o tramite messaggi. 
    Per le variabili di stato pubbliche, viene generata una funzione 
    getter automatica (vedi sotto).

``internal``:
    Tali funzioni e variabili di stato sono accessibili solo internamente 
    (ovvero dall'interno del contratto o dei contratti che ne derivano), 
    senza usare ``this``.

``private``:
    Le funzioni private e le variabili di stato sono visibili solo nel 
    contratto nel quale sono definite e non nei contratti derivati.

.. note::
    Tutto ciò che è all'interno di un contratto è visibile a tutti gli 
    osservatori esterni alla blockchain. Dichiarare qulcosa come ``privato`` 
    impedisce solo ad altri contratti di leggere o modificare le informazioni, 
    ma sarà comunque visibile a tutto il mondo al di fuori della blockchain.

L'identificatore di visibilità viene fornito dopo il tipo per le variabili 
di stato e tra l'elenco dei parametri e l'elenco dei parametri di ritorno per le funzioni.

::

    pragma solidity >=0.4.16 <0.7.0;

    contract C {
        function f(uint a) private pure returns (uint b) { return a + 1; }
        function setData(uint a) internal { data = a; }
        uint public data;
    }

Nell'esempio seguente, ``D`` può chiamare ``c.getData()`` per recuperare il valore di
``data`` nello storage dello stato, ma non può chiamare ``f``. 
Il contratto ``E`` è derivato da ``C`` e quindi può chiamare ``compute``.

::

    pragma solidity >=0.4.0 <0.7.0;

    contract C {
        uint private data;

        function f(uint a) private pure returns(uint b) { return a + 1; }
        function setData(uint a) public { data = a; }
        function getData() public view returns(uint) { return data; }
        function compute(uint a, uint b) internal pure returns (uint) { return a + b; }
    }

    // Questo non compila
    contract D {
        function readData() public {
            C c = new C();
            uint local = c.f(7); // errore: `f` non è visibile
            c.setData(3);
            local = c.getData();
            local = c.compute(3, 5); // erorre: `compute` non è visibile
        }
    }

    contract E is C {
        function g() public {
            C c = new C();
            uint val = compute(3, 5); // accesso ad un elemento internal (dal contratto derivato a quello genitore)
        }
    }

.. index:: ! getter;function, ! function;getter
.. _getter-functions:

Funzioni Getter
===============


Il compilatore crea automaticamente funzioni getter per tutte le variabili di stato **public**. 
Per il contratto indicato di seguito, il compilatore genera una funzione chiamata 
``data`` che non accetta alcun argomento e restituisce un ``uint``, il valore della 
variabile di stato ``data``. 
Le variabili di stato possono essere inizializzate quando vengono dichiarate.

::

    pragma solidity >=0.4.0 <0.7.0;

    contract C {
        uint public data = 42;
    }

    contract Caller {
        C c = new C();
        function f() public view returns (uint) {
            return c.data();
        }
    }

Le funzioni getter hanno visibilità external. Se il dato viene acceduto internamente
(i.e. senza ``this.``), allora si comporta come una variabile di stato. 
Se acceduto esternamente, i.e. con ``this.``, si comporta come una funzione.

::

    pragma solidity >=0.4.0 <0.7.0;

    contract C {
        uint public data;
        function x() public returns (uint) {
            data = 3; // accesso interno
            return this.data(); // accesso esterno
        }
    }

Se si dispone di una variabile di stato ``public`` di tipo array, 
è possibile recuperare solo singoli elementi dell'array tramite la funzione 
getter generata. Questo meccanismo esiste per evitare elevati costi di gas 
quando si restituisce un intero array. È possibile utilizzare gli argomenti 
per specificare quale singolo elemento restituire, ad esempio ``data(0)``. 
Se si desidera restituire un intero array in una chiamata, 
è necessario scrivere una funzione, ad esempio:

::

  pragma solidity >=0.4.0 <0.7.0;

  contract arrayExample {
    // variabile di stato pubblica
    uint[] public myArray;

    // Funzione getter generata dal compilatore
    /*
    function myArray(uint i) returns (uint) {
        return myArray[i];
    }
    */

    // Funzione che restituisce un intero array
    function getArray() returns (uint[] memory) {
        return myArray;
    }
  }

Ora si può utilizzare ``getArray()`` per recuperare un intero array, invece di
``myArray(i)``, che restituisce un singolo elementoper ogni chiamata.

L'esempio seguente è più complicato:

::

    pragma solidity >=0.4.0 <0.7.0;

    contract Complex {
        struct Data {
            uint a;
            bytes3 b;
            mapping (uint => uint) map;
        }
        mapping (uint => mapping(bool => Data[])) public data;
    }

L'esempio sopra genera una funzione col la seguente struttura. 
Il mapping nella struct è omesso perché non esiste un metodo
efficace per fornire la chiave del mapping:

::

    function data(uint arg1, bool arg2, uint arg3) public returns (uint a, bytes3 b) {
        a = data[arg1][arg2][arg3].a;
        b = data[arg1][arg2][arg3].b;
    }
