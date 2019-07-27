.. _natspec:

###############
Formato NatSpec
###############

I contratti Solidity possono utilizzare una speciale forma di commenti per 
fornire un'ampia documentazione per funzioni, valori di ritorno etc. 
Questo formato speciale è chiamato Ethereum Natural Language Specification 
Format (NatSpec).

Questa documentazione è suddivisa in messaggi riguardanti gli sviluppatori e 
messaggi riguardanti gli utenti. 
Questi messaggi possono essere mostrati all'utente finale (l'umano) quando 
interagiscono con il contratto (per esempio durante la firma di una transazione).

Si raccomanda che i contratti Solidity siano interamente annotati utilizzando NatSpec per
tutte le interfacce pubbliche (ogni dato nell'ABI).

NatSpec include specifiche per la formattazione dei commenti dello smart contract 
che sono interpretati dal compilatore Solidity. Sotto viene dettagliato
l'output del compilatore Solidity che estrae questi commenti e li trasforma in un
formato machine-readable.

.. _header-doc-example:

Esempio di Documentazione
=========================

La documentazione è inserita al di sopra di ogni ``class``, ``interface`` e
``function`` utilizzando il formato doxygen.

-  Per Solidity si può utilizzare ``///`` per un commento singola linea o multi-line
   oppure ``/**`` terminato da ``*/``.

-  Per Vyper, si utilizza ``"""`` indentato. Vedere la `documentazione Vyper
   <https://vyper.readthedocs.io/en/latest/structure-of-a-contract.html#natspec-metadata>`__.

L'esempio seguente mostra un contratto ed una funzione utilizzando tutti i tag disponibili.
I commenti non sono stati tradotti ma sono stati mantenuti in inglese.

.. note::

  NatSpec attualmente NON si applica a variabili di stato pubbliche (consultare
  `solidity#3418 <https://github.com/ethereum/solidity/issues/3418>`__),
  anche se sono dichiarate pubbliche e quindi interessano l'ABI.

  Il compilatore Solidity interpreta i tag solamente se sono external o
  public. I commenti per funzioni internal o private sono benvenuti ma non
  vengono tradotti.

.. code:: solidity

   pragma solidity ^0.5.6;

   /// @title A simulator for trees
   /// @author Larry A. Gardner
   /// @notice You can use this contract for only the most basic simulation
   /// @dev All function calls are currently implemented without side effects
   contract Tree {
       /// @author Mary A. Botanist
       /// @notice Calculate tree age in years, rounded up, for live trees
       /// @dev The Alexandr N. Tetearing algorithm could increase precision
       /// @param rings The number of rings from dendrochronological sample
       /// @return age in years, rounded up for partial years
       function age(uint256 rings) external pure returns (uint256) {
           return rings + 1;
       }
   }

.. _header-tags:

Tag
===

Tutti i tag sono opzionali. La seguente tabella illustra lo scopo di ciascun 
tag NatSpec e spiega dove può essere usato. Come caso speciale, se nessun tag
viene utilizzato, il compilatore Solidity interpreta i commenti `///` e `/**` come se 
fossero annotati con `@notice`.

=========== ===================================================================================== =============================
Tag                                                                                               Context
=========== ===================================================================================== =============================
``@title``  Titolo descrittivo del contratto/interfaccia                                          contract, interface
``@author`` Nome dell'autore                                                                      contract, interface, function
``@notice`` Spiegazione all'utente delle funzioni del contratto                                   contract, interface, function
``@dev``    Spiegazione allo sviluppatore, dettagli aggiuntivi                                    contract, interface, function
``@param``  Documenta un parametro, come in doxygen (deve essere seguito dal nome del parametro)  function
``@return`` Documenta il tipo di ritorno di una funzione del contratto                            function
=========== ===================================================================================== =============================

Se la funzione ha più parametri di ritorno, come per esempio ``(int quotient, int remainder)``
devono essere usati più ``@return`` statement con lo stesso formato dei ``@param`` statement.

.. _header-dynamic:

Espressioni Dinamiche
---------------------

Il compilatore Solidity analizza la documentazione NatSpec dal sorgente Solidity
all'ouput in JSON come descritto in questa guida. 
L'utilizzatore dell'ouput JSON, per sempio il software client dell'utente, 
può presentarlo direttamente all'utente finale o 
può applicare alcuni passi di pre-elaborazione.

Per esempio, alcuni client mostrano:

.. code:: solidity

   /// @notice This function will multiply `a` by 7

all'utente come:

.. code:: text

    This function will multiply 10 by 7

se una funzione viene chiamata e l'input ``a`` ha valore 10.

La specifica di queste espressioni dinamiche va oltre lo scopo di 
questa documentazione. Ulteriori informazioni possono essere trovate a
`the radspec project <https://github.com/aragon/radspec>`__.

.. _header-inheritance:

Note per l'Ereditarietà
-----------------------

Attualmente non è specificato se un contratto con una funzione che non ha
NatSpec erediterà il NatSpec di un contratto / interfaccia genitore per la
stessa funzione.

.. _header-output:

Output della Documentazione
===========================

Quando analizzata dal compilatore, la documentazione come quella dell'esempio
precedente produce due diversi file JSON. Uno è destinato ad essere
utilizzato dall'utente finale quando viene eseguita una funzione e il secondo 
dallo sviluppatore.

Se il contratto mostrato sopra è salvato come ``ex1.sol``, la documentazioe 
può essere generata utilizzando:

.. code::

   solc --userdoc --devdoc ex1.sol

Vedere sotto per l'output.

.. _header-user-doc:

Documentazione Utente
---------------------

La documentazione sopra produce il seguente file JSON come output:

.. code::

    {
      "methods" :
      {
        "age(uint256)" :
        {
          "notice" : "Calculate tree age in years, rounded up, for live trees"
        }
      },
      "notice" : "You can use this contract for only the most basic simulation"
    }

Notare che la chiave con la quale si recupera il metodo è la signature 
della funzione come definito in `Contract ABI <Ethereum-Contract-ABI#signature>`__ 
e non semplicemente il nome della funzione.

.. _header-developer-doc:

Documentazione per Sviluppatori
-------------------------------

Oltre al file di documentazione per gli utenti, viene prodotto anche un
file JSON come documentazione per gli sviluppatori simile a questo:

.. code::

    {
      "author" : "Larry A. Gardner",
      "details" : "All function calls are currently implemented without side effects",
      "methods" :
      {
        "age(uint256)" :
        {
          "author" : "Mary A. Botanist",
          "details" : "The Alexandr N. Tetearing algorithm could increase precision",
          "params" :
          {
            "rings" : "The number of rings from dendrochronological sample"
          },
          "return" : "age in years, rounded up for partial years"
        }
      },
      "title" : "A simulator for trees"
    }
