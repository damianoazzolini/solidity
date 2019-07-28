.. index:: ! constant

***************************
Variabili di Stato Costanti
***************************

Le variabili di stato possono essere dichiarate come `` costanti ''. 
In questo caso, devono essere definite tali da un'espressione che 
è costante al momento della compilazione. 
Qualsiasi espressione
che accede alla memoria o ai dati della blockchain (ad es. `` now``, 
`` address (this) .balance`` o `` block.number``) o
ai dati di esecuzione (`` msg.value`` o `` gasleft () ``) 
o effettua chiamate a contratti esterni non è consentita. 
Espressioni che potrebbero avere un effetto collaterale sull'allocazione 
della memoria sono consentite, ma quelle che potrebbero avere un effetto 
collaterale sulla memoria di altri oggetti non lo sono. 
Le funzioni built-in ``keccak256``, ``sha256``, ``ripemd160``, ``ecrecover``, ``addmod`` 
e ``mulmod`` sono consentite (anche se, ad eccezione di ``keccak256``, chiamano contratti esterni).

Il motivo dietro la concessione di effetti collaterali sull'allocazione di memoria è che
dovrebbe essere possibile costruire oggetti complessi come ad esempio lookup-tables.
Questa funzione non è ancora completamente utilizzabile.

Il compilatore non riserva uno slot di memoria per queste variabili e ogni occorrenza è
sostituita dalla rispettiva espressione costante (che potrebbe essere calcolata su un 
singolo valore dall'ottimizzatore).

Al momento non tutti i tipi di costanti sono implementati. Gli unici tipi supportati sono
tipi di valore e stringhe.

::

    pragma solidity >=0.4.0 <0.7.0;

    contract C {
        uint constant x = 32**22 + 8;
        string constant text = "abc";
        bytes32 constant myHash = keccak256("abc");
    }
