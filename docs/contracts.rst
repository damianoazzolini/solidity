.. index:: ! contract

.. _contracts:

##########
Contratti
##########

I contratti in Solidity sono simili a classi in linguaggi orientati agli oggetti. 
Essi contengono dati persistenti in variabili di stato e funzioni che possono 
modificare le variabili. La chiamata ad una funzione su un altro contratto (istanza)
effettua una chiamata di funzione EVM e quindi effettua un cambio di contesto.
In questo modo le variabili di stato diventano inaccessibili. 
Un contratto e le sue funzioni devono essere chiamati affinché succeda qualcosa.
In Ethereum non esiste il concetto di "cron" per chiamare automaticamente una 
funzione a seguito di un particolare evento.

.. include:: contracts/creating-contracts.rst

.. include:: contracts/visibility-and-getters.rst

.. include:: contracts/function-modifiers.rst

.. include:: contracts/constant-state-variables.rst
.. include:: contracts/functions.rst

.. include:: contracts/events.rst

.. include:: contracts/inheritance.rst

.. include:: contracts/abstract-contracts.rst
.. include:: contracts/interfaces.rst

.. include:: contracts/libraries.rst

.. include:: contracts/using-for.rst