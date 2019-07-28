.. index:: ! contract;abstract, ! abstract contract

.. _abstract-contract:

******************
Contratti astratti
******************

I contratti sono contrassegnati come astratti quando almeno una 
delle loro funzioni non ha un'implementazione, come nell'esempio 
seguente (si noti che l'intestazione della dichiarazione di funzione è terminata da ``;``): ::

    pragma solidity >=0.4.0 <0.7.0;

    contract Feline {
        function utterance() public returns (bytes32);
    }

Tali contratti non possono essere compilati (anche se contengono funzioni implementate insieme 
a funzioni non implementate), ma possono essere utilizzati come contratti di base: ::

    pragma solidity >=0.4.0 <0.7.0;

    contract Feline {
        function utterance() public returns (bytes32);
    }

    contract Cat is Feline {
        function utterance() public returns (bytes32) { return "miaow"; }
    }

Se un contratto eredita da un contratto astratto e non implementa tutte le funzioni 
non implementate mediante overriding, sarà esso stesso astratto.

Si noti che una funzione senza implementazione è diversa da un 
:ref:`Function Type <function_types>` anche se la loro sintassi risulta simile.

Esempio di funzione senza implementazione (dichiarazione di funzione)::

    function foo(address) external returns (address);

Esempio di Function Type (dichiarazione di variabile dove la variabile è di tipo ``function``)::

    function(address) external returns (address) foo;

I contratti astratti disaccoppiano la definizione di un contratto dalla sua implementazione
fornendo così una migliore estensibilità e documentazione, facilitando pattern come 
`Template method <https://en.wikipedia.org/wiki/Template_method_pattern>`_ 
e rimuovendo duplicati di codice.
I contratti astratti sono utili allo stesso modo in cui è utile definire i metodi in un'interfaccia. 
È un modo per il progettista del contratto astratto di dire "ogni mio figlio deve implementare questo metodo".