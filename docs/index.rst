Solidity
========

.. image:: logo.svg
    :width: 120px
    :alt: Solidity logo
    :align: center

Solidity è un linguaggio di programmazione object oriented ad alto livello per implementare smart contract.
Uno smart contract è un programma che gestisce il comportamento degli account in Ethereum.

Solidity è stato influenzato da diversi linguaggi di programmazione tra cui C++, Python e
JavaScript ed è stato sviluppato per essere compilato per la Ethereum Virtual Machine (EVM).

Solidity è un linguaggio con tipizzazione statica, supporta l'ereditarietà, librerie ed 
anche la definizione di tipi di dato user-defined.

Utilizzando Solidity, si possono sviluppare contratti per la gestione di diverse
situazioni come votazioni, crowdfunding, aste e wallet multi-signature.

Nello sviluppo di uno smart contract si dovrebbe utilizzare sempre l'ultima versione di 
Solidity. Questo perché cambiamenti non retrocompatibili, nuove funzionalità e 
correzione di bachi sono sviluppati ed aggiunti regolarmente.

.. warning::

  Recentemente è stata rilasciata la versione 0.5.X che introduce diversi cambiamenti non
  retrocompatibili. Assicurarsi di leggere la :doc:`lista completa <050-breaking-changes>`.

Documentazione
--------------

Se il lettore è nuovo allo sviluppo di smart contract, si raccomanda di iniziare con
un :ref:`esempio di smart contract <simple-smart-contract>` scritto in Solidity. 
Successivamente è consigliata la lettura delle sezioni 
:doc:`"Solidity by Example" <solidity-by-example>` e di 
:doc:`"Solidity in Dettaglio" <solidity-in-depth>` per prendere familiarità
con i concetti fondamentali del linguaggio.

Per ulteriori letture, consultare :ref:`i fondamenti delle blockchain <blockchain-basics>`
e i dettagli della :ref:`Ethereum Virtual Machine <the-ethereum-virtual-machine>`.

.. hint::
  È possibile provare gli esempi di codice nel browser utilizzando 
  `Remix IDE <https://remix.ethereum.org>`_. Remix è un IDE online
  che permette la scrittura di smart contract in Solidity e la loro 
  esecuzione. Il caricamento della pagina web può richiedere tempo.
  
.. warning::
  Poiché il software è scritto da umani, questo può presentare bachi.
  È consigliato seguire le guide linea per lo sviluppo di software durante
  la scrittura di uno smart contract. Queste includono testing, auditing e 
  dimostrazioni di correttezza. Gli utenti di uno smart contract sono solitamente
  più fiduciosi del codice rispetto ai loro scrittori e blockchain e smart contract
  possono presentare diversi problemi. Si consiglia quindi la lettura della sezione
  :ref:`considerazioni di sicurezza <security_considerations>`.

Per eventuali domande, si può cercare una risposta o fare una domanda sul sito
`Ethereum Stackexchange <https://ethereum.stackexchange.com/>`_, o sul 
`canale gitter <https://gitter.im/ethereum/solidity/>`_.

Idee per migiorare Solidity o questa documentazione sono sempre benvenute. 
Per maggiori dettagli leggere la :doc:`contributors guide <contributing>`.

.. _translations:

Traduzioni
----------

I volontari della community aiutano a tradurre la seguente documentazione.
Le varie traduzioni presentano diversi livelli di completezza ed aggiornamento.
La documentazione in inglese rimane il principale riferimento.

* `Inglese <https://solidity.readthedocs.io>`_
* `Cinese semplificato <http://solidity-cn.readthedocs.io>`_ (in progress)
* `Spagnolo <https://solidity-es.readthedocs.io>`_
* `Turco <https://github.com/denizozzgur/Solidity_TR/blob/master/README.md>`_ (parziale)
* `Russo <https://github.com/ethereum/wiki/wiki/%5BRussian%5D-%D0%A0%D1%83%D0%BA%D0%BE%D0%B2%D0%BE%D0%B4%D1%81%D1%82%D0%B2%D0%BE-%D0%BF%D0%BE-Solidity>`_ (rather outdated)
* `Coreano <http://solidity-kr.readthedocs.io>`_ (in progress)
* `Francese <http://solidity-fr.readthedocs.io>`_ (in progress)

Contenuto
=========

:ref:`Indice delle parole chiave <genindex>`, :ref:`Pagina di Ricerca <search>`

.. toctree::
  :maxdepth: 2

  introduction-to-smart-contracts.rst
  installing-solidity.rst
  solidity-by-example.rst
  solidity-in-depth.rst
  natspec-format.rst
  security-considerations.rst
  resources.rst
  using-the-compiler.rst
  metadata.rst
  abi-spec.rst
  yul.rst
  style-guide.rst
  common-patterns.rst
  bugs.rst
  contributing.rst
  lll.rst
