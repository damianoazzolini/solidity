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

Functions can be declared ``pure`` in which case they promise not to read from or modify the state.

.. note::
  If the compiler's EVM target is Byzantium or newer (default) the opcode ``STATICCALL`` is used,
  which does not guarantee that the state is not read, but at least that it is not modified.

In addition to the list of state modifying statements explained above, the following are considered reading from the state:

#. Reading from state variables.
#. Accessing ``address(this).balance`` or ``<address>.balance``.
#. Accessing any of the members of ``block``, ``tx``, ``msg`` (with the exception of ``msg.sig`` and ``msg.data``).
#. Calling any function not marked ``pure``.
#. Using inline assembly that contains certain opcodes.

::

    pragma solidity >=0.5.0 <0.7.0;

    contract C {
        function f(uint a, uint b) public pure returns (uint) {
            return a * (b + 42);
        }
    }

Pure functions are able to use the `revert()` and `require()` functions to revert
potential state changes when an :ref:`error occurs <assert-and-require>`.

Reverting a state change is not considered a "state modification", as only changes to the
state made previously in code that did not have the ``view`` or ``pure`` restriction
are reverted and that code has the option to catch the ``revert`` and not pass it on.

This behaviour is also in line with the ``STATICCALL`` opcode.

.. warning::
  It is not possible to prevent functions from reading the state at the level
  of the EVM, it is only possible to prevent them from writing to the state
  (i.e. only ``view`` can be enforced at the EVM level, ``pure`` can not).

.. note::
  Prior to version 0.5.0, the compiler did not use the ``STATICCALL`` opcode
  for ``pure`` functions.
  This enabled state modifications in ``pure`` functions through the use of
  invalid explicit type conversions.
  By using  ``STATICCALL`` for ``pure`` functions, modifications to the
  state are prevented on the level of the EVM.

.. note::
  Prior to version 0.4.17 the compiler did not enforce that ``pure`` is not reading the state.
  It is a compile-time type check, which can be circumvented doing invalid explicit conversions
  between contract types, because the compiler can verify that the type of the contract does
  not do state-changing operations, but it cannot check that the contract that will be called
  at runtime is actually of that type.

.. index:: ! fallback function, function;fallback

.. _fallback-function:

Fallback Function
=================

A contract can have exactly one unnamed function. This function cannot have
arguments, cannot return anything and has to have ``external`` visibility.
It is executed on a call to the contract if none of the other
functions match the given function identifier (or if no data was supplied at
all).

Furthermore, this function is executed whenever the contract receives plain
Ether (without data). To receive Ether and add it to the total balance of the contract, the fallback function
must be marked ``payable``. If no such function exists, the contract cannot receive
Ether through regular transactions and throws an exception.

In the worst case, the fallback function can only rely on 2300 gas being
available (for example when `send` or `transfer` is used), leaving little
room to perform other operations except basic logging. The following operations
will consume more gas than the 2300 gas stipend:

- Writing to storage
- Creating a contract
- Calling an external function which consumes a large amount of gas
- Sending Ether

Like any function, the fallback function can execute complex operations as long as there is enough gas passed on to it.

.. warning::
    The fallback function is also executed if the caller meant to call
    a function that is not available. If you want to implement the fallback
    function only to receive ether, you should add a check
    like ``require(msg.data.length == 0)`` to prevent invalid calls.

.. warning::
    Contracts that receive Ether directly (without a function call, i.e. using ``send`` or ``transfer``)
    but do not define a fallback function
    throw an exception, sending back the Ether (this was different
    before Solidity v0.4.0). So if you want your contract to receive Ether,
    you have to implement a payable fallback function.

.. warning::
    A contract without a payable fallback function can receive Ether as a recipient of a `coinbase transaction` (aka `miner block reward`)
    or as a destination of a ``selfdestruct``.

.. note::
    Even though the fallback function cannot have arguments, one can still use ``msg.data`` to retrieve
    any payload supplied with the call.

    A contract cannot react to such Ether transfers and thus also cannot reject them. This is a design choice of the EVM and Solidity cannot work around it.

    It also means that ``address(this).balance`` can be higher than the sum of some manual accounting implemented in a contract (i.e. having a counter updated in the fallback function).

::

    pragma solidity >=0.5.0 <0.7.0;

    contract Test {
        // This function is called for all messages sent to
        // this contract (there is no other function).
        // Sending Ether to this contract will cause an exception,
        // because the fallback function does not have the `payable`
        // modifier.
        function() external { x = 1; }
        uint x;
    }


    // This contract keeps all Ether sent to it with no way
    // to get it back.
    contract Sink {
        function() external payable { }
    }

    contract Caller {
        function callTest(Test test) public returns (bool) {
            (bool success,) = address(test).call(abi.encodeWithSignature("nonExistingFunction()"));
            require(success);
            // results in test.x becoming == 1.

            // address(test) will not allow to call ``send`` directly, since ``test`` has no payable
            // fallback function. It has to be converted to the ``address payable`` type via an
            // intermediate conversion to ``uint160`` to even allow calling ``send`` on it.
            address payable testPayable = address(uint160(address(test)));

            // If someone sends ether to that contract,
            // the transfer will fail, i.e. this returns false here.
            return testPayable.send(2 ether);
        }
    }

.. index:: ! overload

.. _overload-function:

Function Overloading
====================

A contract can have multiple functions of the same name but with different parameter
types.
This process is called "overloading" and also applies to inherited functions.
The following example shows overloading of the function
``f`` in the scope of contract ``A``.

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

Overloaded functions are also present in the external interface. It is an error if two
externally visible functions differ by their Solidity types but not by their external types.

::

    pragma solidity >=0.4.16 <0.7.0;

    // This will not compile
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


Both ``f`` function overloads above end up accepting the address type for the ABI although
they are considered different inside Solidity.

Overload resolution and Argument matching
-----------------------------------------

Overloaded functions are selected by matching the function declarations in the current scope
to the arguments supplied in the function call. Functions are selected as overload candidates
if all arguments can be implicitly converted to the expected types. If there is not exactly one
candidate, resolution fails.

.. note::
    Return parameters are not taken into account for overload resolution.

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

Calling ``f(50)`` would create a type error since ``50`` can be implicitly converted both to ``uint8``
and ``uint256`` types. On another hand ``f(256)`` would resolve to ``f(uint256)`` overload as ``256`` cannot be implicitly
converted to ``uint8``.
