.. index:: abi, application binary interface

.. _ABI:

******************************
Specifiche ABI di un Contratto
******************************

Design di Base
==============

La Application Binary Interface (ABI) di un contratto è il modo standard per interagire con i contratti in Ethereum,
sia dall'esterno della blockchain che per una interazione tra contratti. I dati sono codificati secondo i tipi 
descritti in questa specifica. L'codifica non è autodescrittivo e necessita di uno schema per essere
decodificato.

Si assume che le interfacce delle funzioni di un contratto siano strongly typed, 
note a tempo di compilazionee statiche. Si assume che tutti i contratti abbiano la definizione delle interfacce
di ogni contratto che chiameranno disponibili a tempo di compilazione.

Questa specifica non riguarda i contratti la cui interfaccia è dinamica o
nota solo in fase di esecuzione.

.. _abi_function_selector:

Selettori di Funzione
=====================

I primi quattro byte dei call data per una chiamata di funzione specificano la funzione da chiamare.
Sono i primi (sinistra, high-order in big-endian) 4 byte dell'hash Keccak-256 (SHA-3) 
della signature della funzione. La signature è definita come il prototipo senza gli specificatori
delle posizioni dei dati, i.e. il nome della funzione con l'elenco dei tipi dei parametri tra parentesi.
I tipi dei parametri vengono separati da virgole, non vengono utilizzati spazi.

.. note::
    Il tipi di ritorno di una funzione non fa parte di questa signature. 
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

Solidity supportatutti i tipi presentati sopra con gli stessi nomi ad eccezione delle tuple. 
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

La codifica è progettato per avere le seguenti proprietà che sono particolarmenteutili se 
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

  ``enc(X) = enc(enc_utf8(X))``, i.e. ``X`` ha codifica utf-8 e questo valore viene interpretato come se fosse di tipo ``bytes`` e codificato nuovamente. Si noti che la lunghezza utilizzata in questa codifica successiva è il numero di byte della stringa codificata utf-8, non il suo numero di caratteri.

- ``uint<M>``: ``enc(X)`` è la codifica big-endian di ``X``, con padding a sinistra con zero-byte tale che la lunghezza sia 32 byte
- ``address``: come nel caso di ``uint160``
- ``int<M>``: ``enc(X)`` big-endian complemento a due della codifica di ``X``, con padding a sinistra con ``0xff`` per valori negativi di ``X`` e con zero per valori positivi ``X`` tale che la lunghezza sia 32 byte
- ``bool``: come nel caso di ``uint8``, dove ``1`` è usato per ``true`` e ``0`` per ``false``
- ``fixed<M>x<N>``: ``enc(X)`` è ``enc(X * 10**N)`` dove ``X * 10**N`` è interpretato come ``int256``
- ``fixed``: come nel caso di ``fixed128x18``
- ``ufixed<M>x<N>``: ``enc(X)`` è ``enc(X * 10**N)`` dove ``X * 10**N`` è interpretato come un ``uint256``
- ``ufixed``: come nel caso di ``ufixed128x18``
- ``bytes<M>``: ``enc(X)`` sequenza di byte in ``X`` con padding di trailing zero-bytes per arrivare ad una lunghezza di 32 byte.

Notare che per ogni ``X``, ``len(enc(X))`` è un multiplo di 32

Selettore di Funzione e Codifica degli Argomenti
================================================

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
 - ``0x0000000000000000000000000000000000000000000000000000000000000080`` (offset dall'inizio della parte di dati del secondo parametro, 4*32 byte, esattamente la dimensione della parte di head)
 - ``0x3132333435363738393000000000000000000000000000000000000000000000`` (``"1234567890"`` con padding a 32 byte a destra)
 - ``0x00000000000000000000000000000000000000000000000000000000000000e0`` (offset dall'inizio della parte di dati del quarto parametro = offset offset dall'inizio del primo parametro dynamic + dimensione della parte di dati del primo parametro dynamic = 4\*32 + 3\*32 (vedere sotto))

Dopo questa sezione segue la parte di dati del primo argomento dinamico ``[0x456, 0x789]``:

 - ``0x0000000000000000000000000000000000000000000000000000000000000002`` (numero di elementi nell'array, 2)
 - ``0x0000000000000000000000000000000000000000000000000000000000000456`` (primo elemento)
 - ``0x0000000000000000000000000000000000000000000000000000000000000789`` (secondo elemento)

Infine viene codificata la parte di dati del secondo argomento dinamico, ``"Hello, world!"``:

 - ``0x000000000000000000000000000000000000000000000000000000000000000d`` (numero di elementi (byte in questo caso): 13)
 - ``0x48656c6c6f2c20776f726c642100000000000000000000000000000000000000`` (``"Hello, world!"`` con padding a destra di 32 byte)

Complessivamente, la codifica è (nuova linea dopo ogni slettore di funzione ed ogni 32 byte per chiarezza):

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

Applicando lo stesso principio per codificare i dati per la funzione con signature ``g(uint[][],string[])`` 
con valori ``([[1, 2], [3]], ["one", "two", "three"])`` si ottiene:

Prima si codifica la lunghezza e i dati del primo array dinamico ``[1, 2]`` del primo array ``[[1, 2], [3]]``:

 - ``0x0000000000000000000000000000000000000000000000000000000000000002`` (numero di elementi nel primo array, 2; gli elementi sono ``1`` e ``2``)
 - ``0x0000000000000000000000000000000000000000000000000000000000000001`` (primo elemento)
 - ``0x0000000000000000000000000000000000000000000000000000000000000002`` (secondo elemento)

Successivamente si codifica la lunghezza e i dati del secondo array dinamico ``[3]`` del primo array ``[[1, 2], [3]]``:

 - ``0x0000000000000000000000000000000000000000000000000000000000000001`` (numero di elementi nel secondo array, 1; l'elemento è ``3``)
 - ``0x0000000000000000000000000000000000000000000000000000000000000003`` (primo elemento)

Poi bisogna ricavare l'offset ``a`` e ``b`` per i rispettivi array dinamici  ``[1, 2]`` e ``[3]``. 
Per calcolare gli offset, si può analizzare la parte codificata del del primo array ``[[1, 2], [3]]`` 
enumerando ogni linea nella codifica:

.. code-block:: none

    0 - a                                                                - offset di [1, 2]
    1 - b                                                                - offset di [3]
    2 - 0000000000000000000000000000000000000000000000000000000000000002 - contatore per [1, 2]
    3 - 0000000000000000000000000000000000000000000000000000000000000001 - codifica di 1
    4 - 0000000000000000000000000000000000000000000000000000000000000002 - codifica di 2
    5 - 0000000000000000000000000000000000000000000000000000000000000001 - contatore per [3]
    6 - 0000000000000000000000000000000000000000000000000000000000000003 - codifica di 3

L'offset ``a`` punta all'inizio del contenuto dell'array ``[1, 2]`` che è linea 2 (64 byte); quindi ``a = 0x0000000000000000000000000000000000000000000000000000000000000040``.

L'offset ``b`` punta all'inizio del contenuto dell'array ``[3]`` che è linea 5 (160 byte); quindi ``b = 0x00000000000000000000000000000000000000000000000000000000000000a0``.

Poi si codificano le stringhe del secondo array:

 - ``0x0000000000000000000000000000000000000000000000000000000000000003`` (numero di caratteri nella parola ``"one"``)
 - ``0x6f6e650000000000000000000000000000000000000000000000000000000000`` (rappresentazione utf8 della parola ``"one"``)
 - ``0x0000000000000000000000000000000000000000000000000000000000000003`` (numero di caratteri nella parola ``"two"``)
 - ``0x74776f0000000000000000000000000000000000000000000000000000000000`` (rappresentazione utf8 della parola ``"two"``)
 - ``0x0000000000000000000000000000000000000000000000000000000000000005`` (numero di caratteri nella parola ``"three"``)
 - ``0x7468726565000000000000000000000000000000000000000000000000000000`` (rappresentazione utf8 della parola ``"three"``)

Parallelamente al primo array, poché le stringhe sono elementi dinamici bisgona calcolare il loro offset ``c``, ``d`` e ``e``:

.. code-block:: none

    0 - c                                                                - offset per "one"
    1 - d                                                                - offset per "two"
    2 - e                                                                - offset per "three"
    3 - 0000000000000000000000000000000000000000000000000000000000000003 - contatore per "one"
    4 - 6f6e650000000000000000000000000000000000000000000000000000000000 - codifica di "one"
    5 - 0000000000000000000000000000000000000000000000000000000000000003 - contatore per "two"
    6 - 74776f0000000000000000000000000000000000000000000000000000000000 - codifica di "two"
    7 - 0000000000000000000000000000000000000000000000000000000000000005 - contatore per "three"
    8 - 7468726565000000000000000000000000000000000000000000000000000000 - codifica di "three"

L'offset ``c`` punta all'inizio del contenuto della stringa ``"one"`` che è linea 3 (96 bytes); quindi ``c = 0x0000000000000000000000000000000000000000000000000000000000000060``.

L'offset ``d`` punta all'inizio del contenuto della stringa ``"two"`` che è linea 5 (160 bytes); quindi ``d = 0x00000000000000000000000000000000000000000000000000000000000000a0``.

L'offset ``e`` punta all'inizio del contenuto della stringa ``"three"`` che è linea 7 (224 bytes); quindi ``e = 0x00000000000000000000000000000000000000000000000000000000000000e0``.

Notare che le codifiche degli elementi degli array di base sono indipendenti e hanno la stessa codifica di una funzione con signature ``g(string[],uint[][])``.

Successivamente si codifica la lunghezza del primo array di base:

 - ``0x0000000000000000000000000000000000000000000000000000000000000002`` (numero di elementi nel primo array di base, 2; gli elementi sono ``[1, 2]`` sono ``[3]``)

Quindi si codifica la lunghezza del secondo array di base:

 - ``0x0000000000000000000000000000000000000000000000000000000000000003`` (numero di stringhe nel secondo array di base, 3; le stringhe sono ``"one"``, ``"two"`` e ``"three"``)

Infine si calcolano gli offset ``f`` e ``g`` per i rispettivi array dinamici di base ``[[1, 2], [3]]`` e ``["one", "two", "three"]``, 
e si compongono le parti nell'ordine corretto:

.. code-block:: none

    0x2289b18c                                                            - signature della funzione
     0 - f                                                                - offset di [[1, 2], [3]]
     1 - g                                                                - offset di ["one", "two", "three"]
     2 - 0000000000000000000000000000000000000000000000000000000000000002 - contatore per [[1, 2], [3]]
     3 - 0000000000000000000000000000000000000000000000000000000000000040 - offset di [1, 2]
     4 - 00000000000000000000000000000000000000000000000000000000000000a0 - offset di [3]
     5 - 0000000000000000000000000000000000000000000000000000000000000002 - contatore per [1, 2]
     6 - 0000000000000000000000000000000000000000000000000000000000000001 - codifica di 1
     7 - 0000000000000000000000000000000000000000000000000000000000000002 - codifica di 2
     8 - 0000000000000000000000000000000000000000000000000000000000000001 - contatore per [3]
     9 - 0000000000000000000000000000000000000000000000000000000000000003 - codifica di 3
    10 - 0000000000000000000000000000000000000000000000000000000000000003 - contatore per ["one", "two", "three"]
    11 - 0000000000000000000000000000000000000000000000000000000000000060 - offset per "one"
    12 - 00000000000000000000000000000000000000000000000000000000000000a0 - offset per "two"
    13 - 00000000000000000000000000000000000000000000000000000000000000e0 - offset per "three"
    14 - 0000000000000000000000000000000000000000000000000000000000000003 - contatore per "one"
    15 - 6f6e650000000000000000000000000000000000000000000000000000000000 - codifica di "one"
    16 - 0000000000000000000000000000000000000000000000000000000000000003 - contatore per "two"
    17 - 74776f0000000000000000000000000000000000000000000000000000000000 - codifica di "two"
    18 - 0000000000000000000000000000000000000000000000000000000000000005 - contatore per "three"
    19 - 7468726565000000000000000000000000000000000000000000000000000000 - codifica di "three"

L'offset ``f`` punta all'inizio del contenuto dell'array ``[[1, 2], [3]]`` che è linea 2 (64 bytes); quindi ``f = 0x0000000000000000000000000000000000000000000000000000000000000040``.

L'Offset ``g`` punta all'inizio del contenuto dell'array ``["one", "two", "three"]`` che è linea 10 (320 bytes); quindi ``g = 0x0000000000000000000000000000000000000000000000000000000000000140``.

.. _abi_events:

Eventi
======

Gli eventi sono una astrazione del meccanismo di logging/event-watching di Ethereum. 
Le entry nel log forniscono l'indirizzo del contratto, una serie di massimo quattro topic ed alcuni dati binari di lunghezza arbitraria. 
Gli eventi su basano sugli ABI delle funzioni per interpretare gli eventi (assieme ad una specifica di interfaccia) come una struttura propriamente definita.

Dato un nome di un evento ed una serie di parametri, questi vegnono suddivisi in sotto-serie: 
quelli che sono indicizzati e quelli che non lo sono. Quelli indicizzati, che possono essere al massimo 3, ù
sono utilizzati parallelamente all'hash Keccak della signature dell'evento per formare il topic della entry nel log. 
Quelli non indicizzati formano l'array di byte dell'evento.

Una entry nel log entry che utilizza questo ABI è descritta come:

- ``address``: l'indirizzo del contratto (fornito intrinsecamente da Ethereum);
- ``topics[0]``: ``keccak(EVENT_NAME+"("+EVENT_ARGS.map(canonical_type_of).join(",")+")")`` (``canonical_type_of`` è una funzione che restituisce il tipo canonico di un dato argomento, e.g. per ``uint indexed foo``, restituisce ``uint256``). Se l'evento è dichiarato come ``anonymous`` il ``topics[0]`` non viene generato;
- ``topics[n]``: ``abi_encode(EVENT_INDEXED_ARGS[n - 1])`` (``EVENT_INDEXED_ARGS`` è la serie di ``EVENT_ARGS`` indicizzati);
- ``data``: ABI codifica di ``EVENT_NON_INDEXED_ARGS`` (``EVENT_NON_INDEXED_ARGS`` è la serie di ``EVENT_ARGS`` non indicizzati, ``abi_encode`` è la codifica ABI della funzione usata per restituire una serie di valori con tipo da una funzione, come descritto sopra).

Per tutti i tipi di lunghezza al più 32 byte, l'array ``EVENT_INDEXED_ARGS`` contiene direttamente i valori
con padding o con segno esteso (per gli interi con segno) a 32 byte, come per la codifica ABI.
Comunque per tutti i tipi "complessi" o di lunghezza dinamica, includendo tutti gli array, ``string``, ``bytes`` e strutture,
``EVENT_INDEXED_ARGS`` contiene *l'hash Keccak* di un valore speciale (see :ref:`indexed_event_encoding`), 
invece del valore codificato direttamente.
Questo permette alle applicazioni di richiedere efficientemente i valori degli array di lunghezza dinamica
(impostando l'hash del valore codificato e per il topic), ma rende le applicazioni impossibilitate a
decodificare i valori indicizzati che non sono stati richiesti. Per tipi di lunghezza dinamica,
gli sviluppatori di applicazioni devono trovare un compromesso tra ricerca veloce per valori predeterminati
(se gli argomenti erano indicizzati) e and leggibilità di valori arbitrari (che richiedono gli argomenti 
non siano indicizzati). Gli sviluppatori possono superare questo limite ed ottenere sia una
ricerca efficiente e leggibilità arbitraria definendo eventi con due argomenti — uno indicizzato
e uno no — entrambi contenenti lo stesso valore.

.. _abi_json:

JSON
====

Il formato JSON per l'interfaccia di un contratto è composta da un array di funzioni e/o descrizioni di eventi.
La descrizione di una funzione è un oggetto JSON con i seguenti campi:

- ``type``: ``"function"``, ``"constructor"``, o ``"fallback"`` (la :ref:`unnamed "default" function <fallback-function>`);
- ``name``: nome della funzione;
- ``inputs``: un array di oggetti, ognuno dei quali contenenti:

  * ``name``: il nome del parametro
  * ``type``: il tipo del parametro (più informazioni sotto)
  * ``components``: usato per i tipi tuple (più informazioni sotto)

- ``outputs``: un array di oggetti simile a ``inputs``, può essere omesso se la funzione non ha valori di ritorno
- ``stateMutability``: una stringa con uno dei seguenti valori: ``pure`` (:ref:`specificato per non leggere lo stato della blockchain <pure-functions>`), ``view`` (:ref:`specificato per non modificare lo stato della blockchain <view-functions>`), ``nonpayable`` (funzione che non accetta Ether) e ``payable`` (funzione che accetta Ether)
- ``payable``: ``true`` se la funzione accetta Ether, ``false`` altrimenti
- ``constant``: ``true`` se la funzione è ``pure`` o ``view``, ``false`` altrimenti.

``type`` può essere omesso, default a ``"function"``, similmente ``payable`` e ``constant`` possono essere omessi, entrambi ``false`` di default.

Il costruttore e la funzione di fallback non hanno ``name`` o ``outputs``. La funzione di fallback non ha nemmeno ``inputs``.

.. warning::
  I campi ``constant`` e ``payable`` sono deprecati e saranno rimossi in futuro. Invece, il campo ``stateMutability`` può essere usato per determinare le stesse proprietà.

.. note::
  L'invio di un valore di Ether ad una funzione non payable annulla la transazione.

La descrizione di un evento è un oggetto JSON con campi simili:

- ``type``: sempre ``"event"``
- ``name``: nome dell'evento
- ``inputs``: array di oggetti, ognuno dei quali contiene:

  * ``name``: nome del parametro
  * ``type``: tipo del parametro (più informazioni sotto).
  * ``components``: usato per i tipi tupla (più informazioni sotto).
  * ``indexed``: ``true`` se il campo fa parte del topic del log, ``false`` se fa parte di uno dei segmenti di dati del log

- ``anonymous``: ``true`` se l'evento è stato dichiarato ``anonymous``.

per esempio,

::

    pragma solidity >=0.5.0 <0.7.0;


    contract Test {
        constructor() public { b = hex"12345678901234567890123456789012"; }
        event Event(uint indexed a, bytes32 b);
        event Event2(uint indexed a, bytes32 b);
        function foo(uint a) public { emit Event(a, b); }
        bytes32 b;
    }

risulta nel JSON:

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

Gestire i Tipi di Dato Tupla
----------------------------

Nonostante i nomi non facciano intenzionalmente parte della codifica ABI, questi vengono inclusi nel JSON 
per consentire la visualizzazione all'utente finale. La struttura è nidificata nel modo seguente:

Un oggetto con campi ``name``, ``type`` e potenzialmente ``components`` descrivere una variabile con tipo.
I componenti di una tupla sono salvati nel campo ``components``,
che è di tipo arrat ed ha la stessa struttura come l'oggetto del top-level tranne per il fatto che
``indexed`` non è permesso.

Per esempio

::

    pragma solidity >=0.4.19 <0.7.0;
    pragma experimental ABIEncoderV2;


    contract Test {
        struct S { uint a; uint[] b; T[] c; }
        struct T { uint x; uint y; }
        function f(S memory s, T memory t, uint a) public;
        function g() public returns (S memory s, T memory t, uint a);
    }

risulta nel JSON:

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

Strict encoding mode è il modo che permette esattamente la stessa codifica come definito nella specifica fomale sopra.
Questo significa che l'offset deve essere il più piccolo possibile ma non creare sovrapposizioni nell'area di dati.

Solitamente, i decoder ABI sono scritti in maniera diretta seguendo il puntatore dell'offset, ma alcuni decoder
ma alcuni decoder possono imporre la strict mode. Il decoder ABI Solidity attualmente non impone la strict mode, ma l'encoder
crea sempre i dati in strict mode.

Non-standard Packed Mode
========================

Attraverso ``abi.encodePacked()``, Solidity supporta una non-standard packed mode dove:

- types shorter than 32 bytes are neither zero padded nor sign extended and
- dynamic types are encoded in-place and without the length.
- array elements are padded, but still encoded in-place

Inoltre, sia le strutture che gli array innestati non sono supportati.

Per esempio, la codifica di ``int16(-1), bytes1(0x42), uint16(0x03), string("Hello, world!")`` risulta in:

.. code-block:: none

    0xffff42000348656c6c6f2c20776f726c6421
      ^^^^                                 int16(-1)
          ^^                               bytes1(0x42)
            ^^^^                           uint16(0x03)
                ^^^^^^^^^^^^^^^^^^^^^^^^^^ string("Hello, world!") without a length field

Più in dettaglio:
 - Durante la codifica, tutto viene codificato in-place. Questo significa che non c'è distinzione tra 
   head e tail, come avviene nella codifica ABI, e la lunghezza dell'array non è codificata.
 - Gli argomenti di ``abi.encodePacked`` sono codificati senza padding se non sono array (o ``string`` o ``bytes``).
 - La codifica di un array è la concatenazione della codifica dei suoi elementi **con** padding.
 - Tipi di dimensione dinamica come ``string``, ``bytes`` o ``uint[]`` sono codificati senza il campo lunghezza.
 - La codifica di ``string`` o ``bytes`` non applica il padding alla fine a meno che non faccia parte di un array o 
   una struttura (in quel caso viene effettuato il padding a un multiplo di 32 byte).

In generale, la codifica è ambigua se ci sono due elementi di dimensione dinamica, a causa della mancanza del campo lunghezza.

Se il padding è necessario, si può utilizzare una conversione esplicita: ``abi.encodePacked(uint16(0x12)) == hex"0012"``.

Poiché la codifica packed non viene utilizzata durante al chiamata a funzione, non c'è un supporto per
inserire davanti un selettore di funzione. Poiché la codifica è ambigua, non esiste una funzione di decodifica.

.. warning::

  Se viene utilizzato ``keccak256(abi.encodePacked(a, b))`` e sia ``a`` e ``b`` sono tipi dinamici,
  è facile generare collisioni nei valori dell'hash value spostando parte di ``a`` in ``b`` e viceversa.
  Più in dettaglio, ``abi.encodePacked("a", "bc") == abi.encodePacked("ab", "c")``.
  Se viene utilizzato ``abi.encodePacked`` per le signature, autenticazione o integrità dei dati, assicurarsi di
  utilizzare sempre gli stessi tipi e controllare che al più uno di questi sia dinamico.
  A meno che non ci sia un motivo specifico, ``abi.encode`` dovrebbe essere utilizzato.


.. _indexed_event_encoding:

Codifica di Parametri di Eventi Indicizzati
===========================================

I parametri di eventi indicizzati che non sono tipi, i.e. array e strutture, non sono salvati 
direttamente ma viene salvato un hash keccak256-hash della codifica. Questa codifica è definita come segue:

 - la codifica di un valore ``bytes`` e ``string`` è semplicemente il contenuto della stringa
   senza padding o un prefisso con la lunghezza
 - la codifica di una struttura è la concatenazione della codifica dei suoi elementi,
   sempre con padding ad un multiplo di 32 byte (anche ``bytes`` e ``string``)
 - la codifica di un array (sia di dimensione statica che dinamica) è la concatenazione della
   codifica dei suoi elementi, sempre con padding ad un multiplo di 32 byte (anche ``bytes`` e ``string``) 
   senza prefisso di lunghezza

Per un numero negativo si effettua il padding estendendo il segno e non con zeri.
Ai tipi ``bytesNN`` viene applicato il padding a destra mentre a ``uintNN`` / ``intNN`` padding a sinistra.

.. warning::

    La codifica di una struttura è ambigua se contiene più di un array di dimensione dinamica. 
    A causa di questo, controllare sempre gli eventi dei dati e non affidarsi solamente sui risultati
    della ricerca sui parametri indicizzati.
