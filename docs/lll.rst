###
LLL
###

.. _lll:

LLL è un linguaggio di basso livello per EVM con una sintassi basata su s-expression.

Il repository Solidity contiene un compilatore LLL, che condivide il sottosistema 
assembler con Solidity.
Tuttavia, non vengono rilasciati più miglioramenti.

LLL non viene compilato se non specificamente richiesto con:

.. code-block:: bash

    $ cmake -DLLL=ON ..
    $ cmake --build .

.. warning::

    Il codice sorgente di LLL è deprecato e sarà rimosso dal repository Solidity in futuro.