.. index:: ! inheritance, ! base class, ! contract;base, ! deriving

************
Ereditarietà
************

Solidity supporta ereditarietà multipla, incluso il polimorfismo.

Tutte le chiamate di funzione sono virtuali, il che significa che 
viene chiamata la funzione più derivata, tranne quando viene 
specificato esplicitamente il nome del contratto o viene utilizzata 
la parola chiave ``super``.

Quando un contratto eredita da altri contratti, sulla blockchain viene 
creato un solo contratto e il codice di tutti i contratti di base viene 
compilato nel contratto creato. Ciò significa che anche tutte le chiamate 
interne a funzioni dei contratti di base utilizzano solo chiamate di funzioni interne
(`` super.f (..) `` utilizza JUMP e non una message call).

Il sistema di ereditarietà generale è molto simile a quello di
`Python <https://docs.python.org/3/tutorial/classes.html#inheritance>`_,
specialmente per quanto riguarda l'ereditarietà multipla, ma ci sono anche
:ref:`alcune differenze <multi-inheritance>`.

I dettagli sono riportati nel seguente esempio.

::

    pragma solidity >=0.5.0 <0.7.0;


    contract Owned {
        constructor() public { owner = msg.sender; }
        address payable owner;
    }


    // Viene utilizzato `is` per derivare da un altro contratto. I contratti 
    // derivati possono accedere a tutti gli elementi non privati comprese
    // le funzioni internal e le variabili di stato. Tuttavia queste non possono
    // essere accedute esternamente usando `this`.
    contract Mortal is Owned {
        function kill() public {
            if (msg.sender == owner) selfdestruct(owner);
        }
    }


    // Questi contratti astratti sono scritti solamente per rendere nota
    // l'interfaccia al compilatore. Da notare la funzione senza il body.
    // Se un contratto non implementa tutte le funzioni allora può essere
    // utilizzato solamente come interfaccia.
    contract Config {
        function lookup(uint id) public returns (address adr);
    }


    contract NameReg {
        function register(bytes32 name) public;
        function unregister() public;
    }


    // L'eredità multipla è consentita. Da notare che `owned` è anche
    // una classe base per `mortal`, ma c'è solamemte una singola
    // istanza di `owned` (come per la virtual inheritance in C++).
    contract Named is Owned, Mortal {
        constructor(bytes32 name) public {
            Config config = Config(0xD5f9D8D94886E70b06E474c3fB14Fd43E2f23970);
            NameReg(config.lookup(1)).register(name);
        }

        // Le funzioni possono essere sovrascritte da un'altra funzione con lo stesso nome
        // e lo stesso numero e tipo di input. Se la funzione che effettua overriding ha i tipi dei parametri
        // di output differenti, questo causa un errore.
        // Sia le chiamate a funzioni locali che message-based considerano questi override
        function kill() public {
            if (msg.sender == owner) {
                Config config = Config(0xD5f9D8D94886E70b06E474c3fB14Fd43E2f23970);
                NameReg(config.lookup(1)).unregister();
                // è ancora possibile chiamare una specifica funzione sovrascritta
                Mortal.kill();
            }
        }
    }


    // Se un costruttore riceve un argomento, questo deve essere fornito
    // nell'header (o attraverso modifier-invocation-style nel
    // costruttore del contratto derivato (vedere sotto)).
    contract PriceFeed is Owned, Mortal, Named("GoldFeed") {
        function updateInfo(uint newInfo) public {
            if (msg.sender == owner) info = newInfo;
        }

        function get() public view returns(uint r) { return info; }

        uint info;
    }

Da notare che sopra viene chiamata ``mortal.kill()`` per "inoltrare" la richiesta
di distruzione. Il modo in cui è fatto è problematico, come può essere visto in 
questo esempio::

    pragma solidity >=0.4.22 <0.7.0;

    contract owned {
        constructor() public { owner = msg.sender; }
        address payable owner;
    }

    contract mortal is owned {
        function kill() public {
            if (msg.sender == owner) selfdestruct(owner);
        }
    }

    contract Base1 is mortal {
        function kill() public { /* do cleanup 1 */ mortal.kill(); }
    }

    contract Base2 is mortal {
        function kill() public { /* do cleanup 2 */ mortal.kill(); }
    }

    contract Final is Base1, Base2 {
    }

Una chiamata a ``Final.kill()`` chiama ``Base2.kill`` ma bypassa
``Base1.kill``. Per evitare questo comportamento basta utilizzare ``super``::

    pragma solidity >=0.4.22 <0.7.0;

    contract owned {
        constructor() public { owner = msg.sender; }
        address payable owner;
    }

    contract mortal is owned {
        function kill() public {
            if (msg.sender == owner) selfdestruct(owner);
        }
    }

    contract Base1 is mortal {
        function kill() public { /* do cleanup 1 */ super.kill(); }
    }


    contract Base2 is mortal {
        function kill() public { /* do cleanup 2 */ super.kill(); }
    }

    contract Final is Base1, Base2 {
    }

Se ``Base2`` chiama una funzione di ``super``, non chiama solamente questa funzione
in uno dei contratti base ma chiama questa funzione sul prossimo contratto
base nel grafo di ereditarietà, quindi ``Base1.kill()`` (notare che la sequenza
finale di ereditarietà è -- partendo dal contratto più derivato: Final, Base2, 
Base1, mortal, owned).
La funzione che viene chiamata utilizzando super non è nota nel contesto della classe 
nella quale è usata anche se il tipo è noto. 

.. index:: ! constructor

.. _constructor:

Costruttori
===========

Un costruttore è una funzione opzionale dichiarata con la keyword ``constructor`` 
che viene eseguita alla creazione del contratto che serve per inizializzare il 
contratto stesso.

Prima dell'esecuzione del costruttore, le variabili di stato sono inizializzate 
al loro valore specifico se inizializzate inline, altrimenti a 0.

Dopo l'esecuzione del costruttore, il codice del contratto viene caricato in 
blockchain. Il caricamento costa gas in proporzione lineare alla lunghezza del 
contratto. Questo codice include tutte le funzioni che sono parte dell'interfaccia 
pubblica e tutte le funzioni che sono raggiungibili attraverso chiamate a funzioni.
Non vengono inclusi il costruttore e le funzioni internal che sono chiamate solamente
dal costruttore.

Le funzioni costruttore possono essere sia ``public`` che ``internal``. 
Se non è presente un costruttore, il contratto eredita il costruttore di
default che è equivalente a ``constructor() public {}``. Per esempio:

::

    pragma solidity >=0.5.0 <0.7.0;

    contract A {
        uint public a;

        constructor(uint _a) internal {
            a = _a;
        }
    }

    contract B is A(1) {
        constructor() public {}
    }

Un costruttore marcato con ``internal`` fa sì che il contratto sia :ref:`astratto <abstract-contract>`.

.. warning ::
    Prima della versione 0.4.22, i costruttori venivano definiti come funzioni 
    con lo stesso nome del contratto. Questa sintassi è stata deprecata e non 
    è più consentita nella versione 0.5.0.


.. index:: ! base;constructor

Argomenti per i Costruttori Base
================================

I costruttori di tutti i contratti di base verranno chiamati seguendo le regole di 
linearizzazione spiegate di seguito. Se i costruttori di base hanno argomenti, i 
contratti derivati ​​devono specificarli tutti. Questo può essere fatto in due modi::

    pragma solidity >=0.4.22 <0.7.0;

    contract Base {
        uint x;
        constructor(uint _x) public { x = _x; }
    }

    // Sia specificandolo direttamente nella lista di ereditarietà...
    contract Derived1 is Base(7) {
        constructor() public {}
    }

    // o attraverso un "modificatore" del costruttore derivato.
    contract Derived2 is Base {
        constructor(uint _y) Base(_y * _y) public {}
    }

Un modo è direttamente nella lista di ereditarietà (``is Base(7)``).  

L'altro è nel modo in cui un modificatore viene invocato come parte 
del costruttore derivato (``Base(_y * _y)``). Il primo modo per farlo è più 
conveniente se l'argomento del costruttore è una costante e definisce il 
comportamento del contratto o lo descrive.
Il secondo modo deve essere usato se gli argomenti del costruttore di base 
dipendono da quelli del contratto derivato. 
Gli argomenti devono essere indicati nell'elenco di ereditarietà o 
in stile modificatore nel costruttore derivato.
La specifica degli argomenti in entrambe le posizioni è un errore.


Se un contratto derivato non specifica gli argomenti di tutti 
i costruttori dei suoi contratti base, sarà astratto.

.. index:: ! inheritance;multiple, ! linearization, ! C3 linearization

.. _multi-inheritance:

Ereditarietà Multipla e Linearizzazione
=======================================

I linguaggi che consentono l'ereditarietà multipla devono affrontare diversi problemi.
Uno di questi è il `Diamond Problem <https://en.wikipedia.org/wiki/Multiple_inheritance#The_diamond_problem>`_.
Solidity è simile a Python perché usa la "`C3 Linearization <https://en.wikipedia.org/wiki/C3_linearization>`_"
per forzare uno specifico ordine nel grafo diretto aciclico (DAG) delle classi base. 

Ciò si traduce nella proprietà desiderabile della monotonicità ma non consente alcuni grafici di ereditarietà.
In particolare, l'ordine in cui le classi di base sono indicate nella direttiva `` is`` 
è importante: devono essere elencati i contratti di base diretti nell'ordine 
da "più simile a quello base" a "più derivato". Da notare che questo ordine è il contrario 
di quello usato in Python.
Un altro modo più semplice per spiegarlo è che, quando viene chiamata una funzione 
definita più volte in contratti diversi, i contratti base dati vengono scanditi 
da destra a sinistra (da sinistra a destra in Python) in maniera depth-first, 
fermandosi alla prima corrispondenza. 
Se un contratto di base è già stato analizzato, viene ignorato.

Nel codice seguente, Solidity darà l'errore "Linearizzazione del 
grafico dell'ereditarietà impossibile".

::

    pragma solidity >=0.4.0 <0.7.0;

    contract X {}
    contract A is X {}
    // Questo codice non compila
    contract C is A, X {}

Il motivo è che ``C`` richiede ``X`` per sovrascrivere ``A``
(specificando ``A, X`` in questo ordine), ma ``A`` stesso
richiede di sovrascrivere ``X``, il che causa una contraddizione che non 
puù essere risolta.


Un'area in cui la linearizzazione dell'ereditarietà è particolarmente importante e 
forse non altrettanto chiara è quando vi sono più costruttori nella gerarchia 
dell'ereditarietà. I costruttori saranno sempre eseguiti nell'ordine linearizzato,
indipendentemente dall'ordine in cui i loro argomenti sono forniti nel 
costruttore del contratto ereditario. Per esempio:

::

    pragma solidity >=0.4.0 <0.7.0;

    contract Base1 {
        constructor() public {}
    }

    contract Base2 {
        constructor() public {}
    }

    // I costruttori sono eseguiti nel seguente ordine:
    //  1 - Base1
    //  2 - Base2
    //  3 - Derived1
    contract Derived1 is Base1, Base2 {
        constructor() public Base1() Base2() {}
    }

    // I costruttori sono eseguiti nel seguente ordine:
    //  1 - Base2
    //  2 - Base1
    //  3 - Derived2
    contract Derived2 is Base2, Base1 {
        constructor() public Base2() Base1() {}
    }

    // I costruttori sono eseguiti nel seguente ordine:
    //  1 - Base2
    //  2 - Base1
    //  3 - Derived3
    contract Derived3 is Base2, Base1 {
        constructor() public Base1() Base2() {}
    }


Ereditarietà di Diversi Tipi di Membri dello Stesso Nome
========================================================

Quando l'eredità si traduce in un contratto con una funzione e un modificatore 
con lo stesso nome, questo viene considerato come un errore.
Questo errore è prodotto anche da un evento e un modificatore con lo stesso nome e da
una funzione e un evento con lo stesso nome.
Come eccezione, un getter di variabili di stato può sovrascrivere una funzione pubblica.