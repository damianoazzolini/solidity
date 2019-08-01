.. index:: abi, application binary interface

.. _ABI:

******************************
Specifiche ABI di un Contratto
******************************

Design di Base
==============

La Application Binary Interface (ABI) di un contratto è il modo standard per interagire con i contratti in Ethereum,
sia dall'esterno della blockchain che per una interazione tra contratti. I dati sono codificati secondo i tipi 
descritti in questa specifica. L'encoding non è autodescrittivo e necessita di uno schema per essere
decodificato.

Si assume ch ele interfacce delle funzioni di un contratto siano strongly typed, 
note a tempo di complazione e statiche. Si assume che tutti i contratti abbiano la definizione delle interfacce
di ogni contratto che chiameranno disponibili a tempo di compilazione.

Questa specifica non riguarda i contratti la cui interfaccia è dinamica o
nota solo in fase di esecuzione.

.. _abi_function_selector:

Selettori di Funzione
=====================

I primi quattro byte dei call data per una chiamata di funzione specificano la funzione da chiamare.
Sono i primi (sinistra, high-order in big-endian) 4 byte dell'hash Keccak-256 (SHA-3) 
della signature della fuznione. La signature è definita come il prototipo senza gli specificatori
delle posizioni dei dati, i.e. il nome della fuznione con l'elenco dei tipi dei parametri tra parentesi.
I tipi dei parametri vengono separati da virgole, non vengono utilizzati spazi.

.. note::
    Il tipi di ritorno di una fuznione non fa parte di questa signature. 
    Nell':ref:`overloading di funzioni in Solidity <overload-function>` i tipi di ritorno non
    sono considerati. Il motivo è quello di tenere la risoluzione della chiamata
    di funzione indipendente dal contesto.
    Tuttavia, la :ref:`descrizione JSON dell'ABI<abi_json>` contiene sia gli input che gli output.

Codifica degli Argomenti
========================

Partendo dal quinto byte, segue la codifica degli argomenti. 
Questa codifica viene utilizzata anche in altre posizioni, e.g. i valori di ritorno 
e anche gli argomenti degli eventi sono codificati nello stesso modo, 
senza i quattro byte che specificano la funzione.

Tipi
====

Esistono i seguenti tipi elementari:

- ``uint<M>``: intero senza segno di ``M`` bit, ``0 < M <= 256``, ``M % 8 == 0``. e.g. ``uint32``, ``uint8``, ``uint256``.

- ``int<M>``: intero con segno in complemento a due di ``M`` bit, ``0 < M <= 256``, ``M % 8 == 0``.

- ``address``: equivalente a ``uint160``, fatta eccezione per l'interpretazione. Per calcolare il selettore della funzione viene usato ``address``.

- ``uint``, ``int``: sinonimo per ``uint256`` e ``int256`` rispettivamente. Per calcolare il selettore della funzione devono essere usati ``uint256`` e ``int256``.

- ``bool``: equivalente a ``uint8`` ristretto ai valori 0 a 1. Per calcolare il selettore della funzione viene usato ``bool``.

- ``fixed<M>x<N>``: numero decimale con segno fixed-point di ``M`` bit, ``8 <= M <= 256``, ``M % 8 == 0``, e ``0 < N <= 80``, che rappresenta il numero ``v`` come ``v / (10 ** N)``.

- ``ufixed<M>x<N>``: variante senza segno di ``fixed<M>x<N>``.

- ``fixed``, ``ufixed``: sinonimo per ``fixed128x18`` e ``ufixed128x18`` rispettivamente. Per calcolare il selettore di funzione devono essere usati ``fixed128x18`` e ``ufixed128x18``.

- ``bytes<M>``: tipo binario di ``M`` byte, ``0 < M <= 32``.

- ``function``: un indirizzo (20 byte) seguito da un selettore di funzione (4 byte). Stessa codifica di ``bytes24``.

Esistono i seguenti tipi di array di dimensione fissata:

- ``<type>[M]``: array di lunghezza fissata di ``M`` elementi, ``M >= 0``, del tipo specificato.

Esistono i seguenti tipi di dato di dimensione non fissata:

- ``bytes``: sequenza dinamica di byte.

- ``string``: stringa unicode di codifica UTF-8 di dimensione dinamica.

- ``<type>[]``: array di elementi del tipo dato di lunghezza variabile.

I tipi possono essere racchiusi tra parentesi e combinati in una tupla separati da virgole:

- ``(T1,T2,...,Tn)``: tupla composta dai tipi ``T1``, ..., ``Tn``, ``n >= 0``

È	possibile formare tuple di tuple, array di tuple eccetera. È anche possibile formare tuple di dimensione 0 (dove ``n == 0``).

Mappatura tra Solidity a Tipi ABI 
---------------------------------

Solidity supporta tutti i tipi presentati sopra con gli stessi nomi ad eccezione delle tuple. 
Tuttavia, alcuni tipi di Solidity non sono supportati dall'ABI. 
La tabella seguente mostra nella colonna di sinistra i tipi di Solidity che non fanno parte 
dell'ABI e nella colonna di destra i tipi di ABI che li rappresentano.

+-------------------------------+-----------------------------------------------------------------------------+
|      Solidity                 |                                           ABI                               |
+===============================+=============================================================================+
|:ref:`address payable<address>`|``address``                                                                  |
+-------------------------------+-----------------------------------------------------------------------------+
|:ref:`contract<contracts>`     |``address``                                                                  |
+-------------------------------+-----------------------------------------------------------------------------+
|:ref:`enum<enums>`             |Il più piccolo tipo ``uint`` grande abbastanza per contenere tutti i valori. |
|                               |                                                                             |
|                               |Per esempio, un ``enum`` di 255 valori o meno è mappato in un ``uint8`` e    |
|                               |un ``enum`` di 256 valori è mappato in un ``uint16``.                        |
+-------------------------------+-----------------------------------------------------------------------------+
|:ref:`struct<structs>`         |``tuple``                                                                    |
+-------------------------------+-----------------------------------------------------------------------------+

Criteri di Progettazione per la Codifica
========================================

L'encoding è progettato per avere le seguenti proprietà che sono particolarmete utili se 
alcuni argomenti sono array innestati.

  1. Il numero di letture necessarie per accedere a un valore è al massimo la profondità del valore all'interno della struttura dell'array di argomenti, i.e. sono necessarie quattro letture per recuperare ``a_i[k][l][r]``. In una versione precedente dell'ABI, il numero di letture scalava, nel caso peggiore, in maniera lineare con il numero totale di parametri dinamici.
  2. I dati di una variabile o di un elemento array non sono intercalati con altri dati e sono trasferibile, ovvero utilizzano solo "indirizzi" relativi.


Specifiche Formali della Codifica
=================================

Vengono distinti tipi static e dinamici. I tipi statici sono codificati in-place mentre i tipi dinamici sono codificati in una locazione separata dopo il blocco corrente.

**Definizione:** i seguenti tipi sono definiti "dinamici (dynamic)":

* ``bytes``
* ``string``
* ``T[]`` per ogni ``T``
* ``T[k]`` per ogni dynamic ``T`` ed ogni ``k >= 0``
* ``(T1,...,Tk)`` se ``Ti`` è dinamico per un qualche ``1 <= i <= k``

Tutti gli altri tipi sono considerati "statici (static)".

**Definizione:** ``len(a)`` è il numero di byte in una stringa binaria ``a``.
Il tipo di ``len(a)`` si suppone che sia ``uint256``.

Viene definito ``enc``, la codifica reale, come un mapping di valori dai tipi dell'ABI a stringhe binarie tale che
``len(enc(X))`` dipende dal valore di ``X`` se e solo se il tipo di ``X`` è dinamico.

**Definizione:** Per ogni valore ``X`` dell'ABI, viene definito ricorsivamente ``enc(X)``, dipendentemente 
dal tipo di ``X`` come:

- ``(T1,...,Tk)`` per ``k >= 0`` ed ogni tipo ``T1``, ..., ``Tk``

  ``enc(X) = head(X(1)) ... head(X(k)) tail(X(1)) ... tail(X(k))``

  dove ``X = (X(1), ..., X(k))`` e
  ``head`` e ``tail`` sono definiti per ``Ti`` come tipi statici come

    ``head(X(i)) = enc(X(i))`` e ``tail(X(i)) = ""`` (stringa vuota)

  e come

    ``head(X(i)) = enc(len(head(X(1)) ... head(X(k)) tail(X(1)) ... tail(X(i-1)) ))``
    ``tail(X(i)) = enc(X(i))``

  altrimenti, i.e. se ``Ti`` è un tipo dinamico.

  Notare che nel caso di tipo dinamico, ``head(X(i))`` è well-defined dato che 
  la lunghezza della parte di heads dipende solamente dal tipo e non dal valore.
  Il suo valore è l'offset all'inizio di ``tail(X(i))`` relativo all'inizio di ``enc(X)``.

- ``T[k]`` per ogni ``T`` e ``k``:

  ``enc(X) = enc((X[0], ..., X[k-1]))``

  i.e. è codificato come se fosse una tupla con ``k`` elementi dello stesso tipo.

- ``T[]`` dove ``X`` ha ``k`` elementi (``k`` si suppone sia di tipo ``uint256``):

  ``enc(X) = enc(k) enc([X[0], ..., X[k-1]])``

  i.e. è codificato come se fosse un array di dimensione statica ``k``, con un prefisso 
  del numero di elementi.

- ``bytes``, di lunghezza ``k`` (si suppone sia di tipo ``uint256``):

  ``enc(X) = enc(k) pad_right(X)``, i.e. il numero di byte è codificato come
  ``uint256`` seguito dal reale valore di ``X`` come sequenza di byte, seguito dal minimo
  numero di byte a zero tale che ``len(enc(X))`` sia multiplo di 32.

- ``string``:

  ``enc(X) = enc(enc_utf8(X))``, i.e. ``X`` ha encoding utf-8 e questo valore viene interpretato come se fosse di tipo ``bytes`` e codificato nuovamente. Si noti che la lunghezza utilizzata in questa codifica successiva è il numero di byte della stringa codificata utf-8, non il suo numero di caratteri.

- ``uint<M>``: ``enc(X)`` è l'encoding big-endian di ``X``, con padding a sinistra con zero-byte tale che la lunghezza sia 32 byte
- ``address``: come nel caso di ``uint160``
- ``int<M>``: ``enc(X)`` big-endian complemento a due della codifica di ``X``, con padding a sinistra con ``0xff`` per valori negativi di ``X`` e con zero per valori positivi ``X`` tale che la lunghezza sia 32 byte
- ``bool``: come nel caso di ``uint8``, dove ``1`` è usato per ``true`` e ``0`` per ``false``
- ``fixed<M>x<N>``: ``enc(X)`` è ``enc(X * 10**N)`` dove ``X * 10**N`` è interpretato come ``int256``
- ``fixed``: come nel caso di ``fixed128x18``
- ``ufixed<M>x<N>``: ``enc(X)`` è ``enc(X * 10**N)`` dove ``X * 10**N`` è interpretato come un ``uint256``
- ``ufixed``: come nel caso di ``ufixed128x18``
- ``bytes<M>``: ``enc(X)`` sequenza di byte in ``X`` con padding di trailing zero-bytes per arrivare ad una lunghezza di 32 byte.

Notare che per ogni ``X``, ``len(enc(X))`` è un multiplo di 32

Selettore di Funzione e Argument Encoding
=========================================

Una chiamata alla funzione ``f`` con parametri ``a_1, ..., a_n`` è codificata come

  ``function_selector(f) enc((a_1, ..., a_n))``

è i valori di ritorno ``v_1, ..., v_k`` of ``f`` sono codificati come

  ``enc((v_1, ..., v_k))``

i.e. i valori sono combinati in una tupla e codificati.

Esempi
======

Dato il contratto:

::

    pragma solidity >=0.4.16 <0.7.0;


    contract Foo {
        function bar(bytes3[2] memory) public pure {}
        function baz(uint32 x, bool y) public pure returns (bool r) { r = x > 32 || y; }
        function sam(bytes memory, bool, uint[] memory) public pure {}
    }


Per l'esempio ``Foo`` se si vuole chiamare ``baz`` con i parametri ``69`` e ``true``, verranno passati 68 bytes in totale, che possono essere suddivisi in:

- ``0xcdcd77c0``: Method ID. Derivato dai primi 4 byte dell'hash Keccak del formato ASCII della signature ``baz(uint32,bool)``
- ``0x0000000000000000000000000000000000000000000000000000000000000045``: primo parametro, valore uint32 ``69`` con padding a 32 byte
- ``0x0000000000000000000000000000000000000000000000000000000000000001``: secondo parametro - boolean ``true``, con padding a 32 byte

Complessivamente:

.. code-block:: none

    0xcdcd77c000000000000000000000000000000000000000000000000000000000000000450000000000000000000000000000000000000000000000000000000000000001

Restituisce un singolo ``bool``. Se, per esempio, il valore di ritorno è ``false``, l'output è il singolo array di byte ``0x0000000000000000000000000000000000000000000000000000000000000000``, un singolo bool.

Se si vuole chiamare ``bar`` con gli argomenti ``["abc", "def"]``, si devono passare 68 byte in totale, suddivisi in:

- ``0xfce353f6``: Method ID. Derivato dalla signature ``bar(bytes3[2])``.
- ``0x6162630000000000000000000000000000000000000000000000000000000000``: prima parte del primo parametro, valore ``bytes3`` di ``"abc"`` (left-aligned).
- ``0x6465660000000000000000000000000000000000000000000000000000000000``: the second part of the first parameter, valore ``bytes3`` di ``"def"`` (left-aligned).

Complessivamente:

.. code-block:: none

    0xfce353f661626300000000000000000000000000000000000000000000000000000000006465660000000000000000000000000000000000000000000000000000000000

Se si vuole chiamare ``sam`` con gli argomenti ``"dave"``, ``true`` e ``[1,2,3]``, si devono passare 292 byte in totale, suddivisi in:

- ``0xa5643bf2``: Method ID. Derivato dalla signature ``sam(bytes,bool,uint256[])``. Si noti che ``uint`` è sostituito dalla rappresentazione ``uint256``.
- ``0x0000000000000000000000000000000000000000000000000000000000000060``: posizione della parte di dati del primo parametro (tipo dynamic), misurata in byte dell'inizio del blocco con gli argomenti. In questo caso, ``0x60``
- ``0x0000000000000000000000000000000000000000000000000000000000000001``: secondo parametro: boolean true.
- ``0x00000000000000000000000000000000000000000000000000000000000000a0``: posizione della parte di dati del terzo parametro (tipo dynamic), misurata in byte. In questo caso, ``0xa0``
- ``0x0000000000000000000000000000000000000000000000000000000000000004``: parte di dati del primo argomento, inizia con la lunghezza dell'array di byte, in questo caso 4
- ``0x6461766500000000000000000000000000000000000000000000000000000000``: contenuto del primo argomento: codifica UTF-8 (uguale ad ASCII in questo caso) di ``"dave"``, con padding a destra a 32 byte
- ``0x0000000000000000000000000000000000000000000000000000000000000003``: parte di dati del terzo argomento, inizia con la lunghezza dell'array, in questo caso 3
- ``0x0000000000000000000000000000000000000000000000000000000000000001``: prima entry del terzo parametro.
- ``0x0000000000000000000000000000000000000000000000000000000000000002``: seconda entry del terzo parametro.
- ``0x0000000000000000000000000000000000000000000000000000000000000003``: terza entry del terzo parametro.

Complessivamente:

.. code-block:: none

    0xa5643bf20000000000000000000000000000000000000000000000000000000000000060000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000a0000000000000000000000000000000000000000000000000000000000000000464617665000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000003000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000003

Utilizzo dei Tipi Dinamici
==========================

Una chiamata ad una funzione con signature ``f(uint,uint32[],bytes10,bytes)`` con valori ``(0x123, [0x456, 0x789], "1234567890", "Hello, world!")`` viene codificata nel seguente modo:

Vengono selezionati i primi 4 byte ``sha3("f(uint256,uint32[],bytes10,bytes)")``, i.e. ``0x8be65246``.
Successivamente viene codificata la parte di head di tutti i 4 argomenti. 
I tipi statici ``uint256`` e ``bytes10`` sono direttamente i valori che si vogliono passare, mentre per i tipi dinamici ``uint32[]`` e ``bytes`` viene utilizzato l'offset in bytes dall'inizio dell'area di dati, 
misurato dall'inizio della codifica del valore (i.e. non considerando i primi 4 byte contenenti l'hash della signature della funzione). Questi sono:

 - ``0x0000000000000000000000000000000000000000000000000000000000000123`` (``0x123`` con padding a 32 byte)
 - ``0x0000000000000000000000000000000000000000000000000000000000000080`` (offset to start of data part of second parameter, 4*32 bytes, exactly the size of the head part)
 - ``0x3132333435363738393000000000000000000000000000000000000000000000`` (``"1234567890"`` padded to 32 bytes on the right)
 - ``0x00000000000000000000000000000000000000000000000000000000000000e0`` (offset to start of data part of fourth parameter = offset to start of data part of first dynamic parameter + size of data part of first dynamic parameter = 4\*32 + 3\*32 (see below))

After this, the data part of the first dynamic argument, ``[0x456, 0x789]`` follows:

 - ``0x0000000000000000000000000000000000000000000000000000000000000002`` (number of elements of the array, 2)
 - ``0x0000000000000000000000000000000000000000000000000000000000000456`` (first element)
 - ``0x0000000000000000000000000000000000000000000000000000000000000789`` (second element)

Finally, we encode the data part of the second dynamic argument, ``"Hello, world!"``:

 - ``0x000000000000000000000000000000000000000000000000000000000000000d`` (number of elements (bytes in this case): 13)
 - ``0x48656c6c6f2c20776f726c642100000000000000000000000000000000000000`` (``"Hello, world!"`` padded to 32 bytes on the right)

All together, the encoding is (newline after function selector and each 32-bytes for clarity):

.. code-block:: none

    0x8be65246
      0000000000000000000000000000000000000000000000000000000000000123
      0000000000000000000000000000000000000000000000000000000000000080
      3132333435363738393000000000000000000000000000000000000000000000
      00000000000000000000000000000000000000000000000000000000000000e0
      0000000000000000000000000000000000000000000000000000000000000002
      0000000000000000000000000000000000000000000000000000000000000456
      0000000000000000000000000000000000000000000000000000000000000789
      000000000000000000000000000000000000000000000000000000000000000d
      48656c6c6f2c20776f726c642100000000000000000000000000000000000000

Let us apply the same principle to encode the data for a function with a signature ``g(uint[][],string[])`` with values ``([[1, 2], [3]], ["one", "two", "three"])`` but start from the most atomic parts of the encoding:

First we encode the length and data of the first embedded dynamic array ``[1, 2]`` of the first root array ``[[1, 2], [3]]``:

 - ``0x0000000000000000000000000000000000000000000000000000000000000002`` (number of elements in the first array, 2; the elements themselves are ``1``  and ``2``)
 - ``0x0000000000000000000000000000000000000000000000000000000000000001`` (first element)
 - ``0x0000000000000000000000000000000000000000000000000000000000000002`` (second element)

Then we encode the length and data of the second embedded dynamic array ``[3]`` of the first root array ``[[1, 2], [3]]``:

 - ``0x0000000000000000000000000000000000000000000000000000000000000001`` (number of elements in the second array, 1; the element is ``3``)
 - ``0x0000000000000000000000000000000000000000000000000000000000000003`` (first element)

Then we need to find the offsets ``a`` and ``b`` for their respective dynamic arrays  ``[1, 2]`` and ``[3]``. To calculate the offsets we can take a look at the encoded data of the first root array ``[[1, 2], [3]]`` enumerating each line in the encoding:

.. code-block:: none

    0 - a                                                                - offset of [1, 2]
    1 - b                                                                - offset of [3]
    2 - 0000000000000000000000000000000000000000000000000000000000000002 - count for [1, 2]
    3 - 0000000000000000000000000000000000000000000000000000000000000001 - encoding of 1
    4 - 0000000000000000000000000000000000000000000000000000000000000002 - encoding of 2
    5 - 0000000000000000000000000000000000000000000000000000000000000001 - count for [3]
    6 - 0000000000000000000000000000000000000000000000000000000000000003 - encoding of 3

Offset ``a`` points to the start of the content of the array ``[1, 2]`` which is line 2 (64 bytes); thus ``a = 0x0000000000000000000000000000000000000000000000000000000000000040``.

Offset ``b`` points to the start of the content of the array ``[3]`` which is line 5 (160 bytes); thus ``b = 0x00000000000000000000000000000000000000000000000000000000000000a0``.


Then we encode the embedded strings of the second root array:

 - ``0x0000000000000000000000000000000000000000000000000000000000000003`` (number of characters in word ``"one"``)
 - ``0x6f6e650000000000000000000000000000000000000000000000000000000000`` (utf8 representation of word ``"one"``)
 - ``0x0000000000000000000000000000000000000000000000000000000000000003`` (number of characters in word ``"two"``)
 - ``0x74776f0000000000000000000000000000000000000000000000000000000000`` (utf8 representation of word ``"two"``)
 - ``0x0000000000000000000000000000000000000000000000000000000000000005`` (number of characters in word ``"three"``)
 - ``0x7468726565000000000000000000000000000000000000000000000000000000`` (utf8 representation of word ``"three"``)

In parallel to the first root array, since strings are dynamic elements we need to find their offsets ``c``, ``d`` and ``e``:

.. code-block:: none

    0 - c                                                                - offset for "one"
    1 - d                                                                - offset for "two"
    2 - e                                                                - offset for "three"
    3 - 0000000000000000000000000000000000000000000000000000000000000003 - count for "one"
    4 - 6f6e650000000000000000000000000000000000000000000000000000000000 - encoding of "one"
    5 - 0000000000000000000000000000000000000000000000000000000000000003 - count for "two"
    6 - 74776f0000000000000000000000000000000000000000000000000000000000 - encoding of "two"
    7 - 0000000000000000000000000000000000000000000000000000000000000005 - count for "three"
    8 - 7468726565000000000000000000000000000000000000000000000000000000 - encoding of "three"

Offset ``c`` points to the start of the content of the string ``"one"`` which is line 3 (96 bytes); thus ``c = 0x0000000000000000000000000000000000000000000000000000000000000060``.

Offset ``d`` points to the start of the content of the string ``"two"`` which is line 5 (160 bytes); thus ``d = 0x00000000000000000000000000000000000000000000000000000000000000a0``.

Offset ``e`` points to the start of the content of the string ``"three"`` which is line 7 (224 bytes); thus ``e = 0x00000000000000000000000000000000000000000000000000000000000000e0``.


Note that the encodings of the embedded elements of the root arrays are not dependent on each other and have the same encodings for a function with a signature ``g(string[],uint[][])``.

Then we encode the length of the first root array:

 - ``0x0000000000000000000000000000000000000000000000000000000000000002`` (number of elements in the first root array, 2; the elements themselves are ``[1, 2]``  and ``[3]``)

Then we encode the length of the second root array:

 - ``0x0000000000000000000000000000000000000000000000000000000000000003`` (number of strings in the second root array, 3; the strings themselves are ``"one"``, ``"two"`` and ``"three"``)

Finally we find the offsets ``f`` and ``g`` for their respective root dynamic arrays ``[[1, 2], [3]]`` and ``["one", "two", "three"]``, and assemble parts in the correct order:

.. code-block:: none

    0x2289b18c                                                            - function signature
     0 - f                                                                - offset of [[1, 2], [3]]
     1 - g                                                                - offset of ["one", "two", "three"]
     2 - 0000000000000000000000000000000000000000000000000000000000000002 - count for [[1, 2], [3]]
     3 - 0000000000000000000000000000000000000000000000000000000000000040 - offset of [1, 2]
     4 - 00000000000000000000000000000000000000000000000000000000000000a0 - offset of [3]
     5 - 0000000000000000000000000000000000000000000000000000000000000002 - count for [1, 2]
     6 - 0000000000000000000000000000000000000000000000000000000000000001 - encoding of 1
     7 - 0000000000000000000000000000000000000000000000000000000000000002 - encoding of 2
     8 - 0000000000000000000000000000000000000000000000000000000000000001 - count for [3]
     9 - 0000000000000000000000000000000000000000000000000000000000000003 - encoding of 3
    10 - 0000000000000000000000000000000000000000000000000000000000000003 - count for ["one", "two", "three"]
    11 - 0000000000000000000000000000000000000000000000000000000000000060 - offset for "one"
    12 - 00000000000000000000000000000000000000000000000000000000000000a0 - offset for "two"
    13 - 00000000000000000000000000000000000000000000000000000000000000e0 - offset for "three"
    14 - 0000000000000000000000000000000000000000000000000000000000000003 - count for "one"
    15 - 6f6e650000000000000000000000000000000000000000000000000000000000 - encoding of "one"
    16 - 0000000000000000000000000000000000000000000000000000000000000003 - count for "two"
    17 - 74776f0000000000000000000000000000000000000000000000000000000000 - encoding of "two"
    18 - 0000000000000000000000000000000000000000000000000000000000000005 - count for "three"
    19 - 7468726565000000000000000000000000000000000000000000000000000000 - encoding of "three"

Offset ``f`` points to the start of the content of the array ``[[1, 2], [3]]`` which is line 2 (64 bytes); thus ``f = 0x0000000000000000000000000000000000000000000000000000000000000040``.

Offset ``g`` points to the start of the content of the array ``["one", "two", "three"]`` which is line 10 (320 bytes); thus ``g = 0x0000000000000000000000000000000000000000000000000000000000000140``.

.. _abi_events:

Events
======

Events are an abstraction of the Ethereum logging/event-watching protocol. Log entries provide the contract's address, a series of up to four topics and some arbitrary length binary data. Events leverage the existing function ABI in order to interpret this (together with an interface spec) as a properly typed structure.

Given an event name and series of event parameters, we split them into two sub-series: those which are indexed and those which are not. Those which are indexed, which may number up to 3, are used alongside the Keccak hash of the event signature to form the topics of the log entry. Those which are not indexed form the byte array of the event.

In effect, a log entry using this ABI is described as:

- ``address``: the address of the contract (intrinsically provided by Ethereum);
- ``topics[0]``: ``keccak(EVENT_NAME+"("+EVENT_ARGS.map(canonical_type_of).join(",")+")")`` (``canonical_type_of`` is a function that simply returns the canonical type of a given argument, e.g. for ``uint indexed foo``, it would return ``uint256``). If the event is declared as ``anonymous`` the ``topics[0]`` is not generated;
- ``topics[n]``: ``abi_encode(EVENT_INDEXED_ARGS[n - 1])`` (``EVENT_INDEXED_ARGS`` is the series of ``EVENT_ARGS`` that are indexed);
- ``data``: ABI encoding of ``EVENT_NON_INDEXED_ARGS`` (``EVENT_NON_INDEXED_ARGS`` is the series of ``EVENT_ARGS`` that are not indexed, ``abi_encode`` is the ABI encoding function used for returning a series of typed values from a function, as described above).

For all types of length at most 32 bytes, the ``EVENT_INDEXED_ARGS`` array contains
the value directly, padded or sign-extended (for signed integers) to 32 bytes, just as for regular ABI encoding.
However, for all "complex" types or types of dynamic length, including all arrays, ``string``, ``bytes`` and structs,
``EVENT_INDEXED_ARGS`` will contain the *Keccak hash* of a special in-place encoded value
(see :ref:`indexed_event_encoding`), rather than the encoded value directly.
This allows applications to efficiently query for values of dynamic-length types
(by setting the hash of the encoded value as the topic), but leaves applications unable
to decode indexed values they have not queried for. For dynamic-length types,
application developers face a trade-off between fast search for predetermined values
(if the argument is indexed) and legibility of arbitrary values (which requires that
the arguments not be indexed). Developers may overcome this tradeoff and achieve both
efficient search and arbitrary legibility by defining events with two arguments — one
indexed, one not — intended to hold the same value.

.. _abi_json:

JSON
====

The JSON format for a contract's interface is given by an array of function and/or event descriptions.
A function description is a JSON object with the fields:

- ``type``: ``"function"``, ``"constructor"``, or ``"fallback"`` (the :ref:`unnamed "default" function <fallback-function>`);
- ``name``: the name of the function;
- ``inputs``: an array of objects, each of which contains:

  * ``name``: the name of the parameter;
  * ``type``: the canonical type of the parameter (more below).
  * ``components``: used for tuple types (more below).

- ``outputs``: an array of objects similar to ``inputs``, can be omitted if function doesn't return anything;
- ``stateMutability``: a string with one of the following values: ``pure`` (:ref:`specified to not read blockchain state <pure-functions>`), ``view`` (:ref:`specified to not modify the blockchain state <view-functions>`), ``nonpayable`` (function does not accept Ether) and ``payable`` (function accepts Ether);
- ``payable``: ``true`` if function accepts Ether, ``false`` otherwise;
- ``constant``: ``true`` if function is either ``pure`` or ``view``, ``false`` otherwise.

``type`` can be omitted, defaulting to ``"function"``, likewise ``payable`` and ``constant`` can be omitted, both defaulting to ``false``.

Constructor and fallback function never have ``name`` or ``outputs``. Fallback function doesn't have ``inputs`` either.

.. warning::
    The fields ``constant`` and ``payable`` are deprecated and will be removed in the future. Instead, the ``stateMutability`` field can be used to determine the same properties.

.. note::
    Sending non-zero Ether to non-payable function will revert the transaction.

An event description is a JSON object with fairly similar fields:

- ``type``: always ``"event"``
- ``name``: the name of the event;
- ``inputs``: an array of objects, each of which contains:

  * ``name``: the name of the parameter;
  * ``type``: the canonical type of the parameter (more below).
  * ``components``: used for tuple types (more below).
  * ``indexed``: ``true`` if the field is part of the log's topics, ``false`` if it one of the log's data segment.

- ``anonymous``: ``true`` if the event was declared as ``anonymous``.

For example,

::

    pragma solidity >=0.5.0 <0.7.0;


    contract Test {
        constructor() public { b = hex"12345678901234567890123456789012"; }
        event Event(uint indexed a, bytes32 b);
        event Event2(uint indexed a, bytes32 b);
        function foo(uint a) public { emit Event(a, b); }
        bytes32 b;
    }

would result in the JSON:

.. code-block:: json

  [{
  "type":"event",
  "inputs": [{"name":"a","type":"uint256","indexed":true},{"name":"b","type":"bytes32","indexed":false}],
  "name":"Event"
  }, {
  "type":"event",
  "inputs": [{"name":"a","type":"uint256","indexed":true},{"name":"b","type":"bytes32","indexed":false}],
  "name":"Event2"
  }, {
  "type":"function",
  "inputs": [{"name":"a","type":"uint256"}],
  "name":"foo",
  "outputs": []
  }]

Handling tuple types
--------------------

Despite that names are intentionally not part of the ABI encoding they do make a lot of sense to be included
in the JSON to enable displaying it to the end user. The structure is nested in the following way:

An object with members ``name``, ``type`` and potentially ``components`` describes a typed variable.
The canonical type is determined until a tuple type is reached and the string description up
to that point is stored in ``type`` prefix with the word ``tuple``, i.e. it will be ``tuple`` followed by
a sequence of ``[]`` and ``[k]`` with
integers ``k``. The components of the tuple are then stored in the member ``components``,
which is of array type and has the same structure as the top-level object except that
``indexed`` is not allowed there.

As an example, the code

::

    pragma solidity >=0.4.19 <0.7.0;
    pragma experimental ABIEncoderV2;


    contract Test {
        struct S { uint a; uint[] b; T[] c; }
        struct T { uint x; uint y; }
        function f(S memory s, T memory t, uint a) public;
        function g() public returns (S memory s, T memory t, uint a);
    }

would result in the JSON:

.. code-block:: json

  [
    {
      "name": "f",
      "type": "function",
      "inputs": [
        {
          "name": "s",
          "type": "tuple",
          "components": [
            {
              "name": "a",
              "type": "uint256"
            },
            {
              "name": "b",
              "type": "uint256[]"
            },
            {
              "name": "c",
              "type": "tuple[]",
              "components": [
                {
                  "name": "x",
                  "type": "uint256"
                },
                {
                  "name": "y",
                  "type": "uint256"
                }
              ]
            }
          ]
        },
        {
          "name": "t",
          "type": "tuple",
          "components": [
            {
              "name": "x",
              "type": "uint256"
            },
            {
              "name": "y",
              "type": "uint256"
            }
          ]
        },
        {
          "name": "a",
          "type": "uint256"
        }
      ],
      "outputs": []
    }
  ]

.. _abi_packed_mode:

Strict Encoding Mode
====================

Strict encoding mode is the mode that leads to exactly the same encoding as defined in the formal specification above.
This means offsets have to be as small as possible while still not creating overlaps in the data areas and thus no gaps are
allowed.

Usually, ABI decoders are written in a straightforward way just following offset pointers, but some decoders
might enforce strict mode. The Solidity ABI decoder currently does not enforce strict mode, but the encoder
always creates data in strict mode.

Non-standard Packed Mode
========================

Through ``abi.encodePacked()``, Solidity supports a non-standard packed mode where:

- types shorter than 32 bytes are neither zero padded nor sign extended and
- dynamic types are encoded in-place and without the length.
- array elements are padded, but still encoded in-place

Furthermore, structs as well as nested arrays are not supported.

As an example, the encoding of ``int16(-1), bytes1(0x42), uint16(0x03), string("Hello, world!")`` results in:

.. code-block:: none

    0xffff42000348656c6c6f2c20776f726c6421
      ^^^^                                 int16(-1)
          ^^                               bytes1(0x42)
            ^^^^                           uint16(0x03)
                ^^^^^^^^^^^^^^^^^^^^^^^^^^ string("Hello, world!") without a length field

More specifically:
 - During the encoding, everything is encoded in-place. This means that there is
   no distinction between head and tail, as in the ABI encoding, and the length
   of an array is not encoded.
 - The direct arguments of ``abi.encodePacked`` are encoded without padding,
   as long as they are not arrays (or ``string`` or ``bytes``).
 - The encoding of an array is the concatenation of the
   encoding of its elements **with** padding.
 - Dynamically-sized types like ``string``, ``bytes`` or ``uint[]`` are encoded
   without their length field.
 - The encoding of ``string`` or ``bytes`` does not apply padding at the end
   unless it is part of an array or struct (then it is padded to a multiple of
   32 bytes).

In general, the encoding is ambiguous as soon as there are two dynamically-sized elements,
because of the missing length field.

If padding is needed, explicit type conversions can be used: ``abi.encodePacked(uint16(0x12)) == hex"0012"``.

Since packed encoding is not used when calling functions, there is no special support
for prepending a function selector. Since the encoding is ambiguous, there is no decoding function.

.. warning::

  If you use ``keccak256(abi.encodePacked(a, b))`` and both ``a`` and ``b`` are dynamic types,
  it is easy to craft collisions in the hash value by moving parts of ``a`` into ``b`` and
  vice-versa. More specifically, ``abi.encodePacked("a", "bc") == abi.encodePacked("ab", "c")``.
  If you use ``abi.encodePacked`` for signatures, authentication or data integrity, make
  sure to always use the same types and check that at most one of them is dynamic.
  Unless there is a compelling reason, ``abi.encode`` should be preferred.


.. _indexed_event_encoding:

Encoding of Indexed Event Parameters
====================================

Indexed event parameters that are not value types, i.e. arrays and structs are not
stored directly but instead a keccak256-hash of an encoding is stored. This encoding
is defined as follows:

 - the encoding of a ``bytes`` and ``string`` value is just the string contents
   without any padding or length prefix.
 - the encoding of a struct is the concatenation of the encoding of its members,
   always padded to a multiple of 32 bytes (even ``bytes`` and ``string``).
 - the encoding of an array (both dynamically- and statically-sized) is
   the concatenation of the encoding of its elements, always padded to a multiple
   of 32 bytes (even ``bytes`` and ``string``) and without any length prefix

In the above, as usual, a negative number is padded by sign extension and not zero padded.
``bytesNN`` types are padded on the right while ``uintNN`` / ``intNN`` are padded on the left.

.. warning::

    The encoding of a struct is ambiguous if it contains more than one dynamically-sized
    array. Because of that, always re-check the event data and do not rely on the search result
    based on the indexed parameters alone.
