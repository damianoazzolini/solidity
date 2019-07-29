.. index:: ! contract;interface, ! interface contract

.. _interfaces:

**********
Interfacce
**********

Le interfacce sono simili a contratti astratti ma non possono avere nessuna
funzione implementata. Ci sono ulteriori restrizioni:

- Non possono ereditare altri contratti o interfacce.
- Tutte le funzioni dichiarate devono essere external.
- Non possono dichiarare un costruttore.
- Non possono dichiarare variabili di stato.

Alcune di queste restrizioni potrebbero essere rimosse in futuro.

Le interfacce sono sostanzialmente limitate a ciò che l'ABI 
del contratto può rappresentare e la conversione tra l'ABI e un'interfaccia 
dovrebbe essere possibile senza alcuna perdita di informazioni.

Le interfacce sono indicate dalla loro stessa parola chiave:

::

    pragma solidity >=0.5.0 <0.7.0;

    interface Token {
        enum TokenType { Fungible, NonFungible }
        struct Coin { string obverse; string reverse; }
        function transfer(address recipient, uint amount) external;
    }

I contratti possono ereditare interfacce come erediterebbero altri contratti.

I tipi definiti all'interno di interfacce ed altre strutture simili a contratti
possono essere accedute da altri contratti: ``Token.TokenType`` o ``Token.Coin``.

.. warning:

    Le interfacce supportano i tipi ``enum`` dalla :doc:`versione di Solidity 0.5.0 <050-breaking-changes>`. 
    Assicurarsi che la pragma version specifichi almeno questa versione.