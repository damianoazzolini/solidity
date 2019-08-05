#################
Solidity Assembly
#################

.. index:: ! assembly, ! asm, ! evmasm

Solidity definisce un linguaggio assembly che può essere utilizzato senza Solidity e come
"inline assembly" all'interno del codice Solidity. Questa guida inizia descrivendo come utilizzare
l'inline assembly, quali sono le differenze con assembly standalone, e specifica l'assembly stesso.

.. _inline-assembly:

Inline Assembly
===============

Si può inserire del codice assemby all'interno di Solidity in un linguaggio simile
a quello della virtual machine. Questo consente di avere un controllo più preciso
specialmente durante lo sviluppo di librerie.

Poiché la EVM è una stack machine, solitamente è difficile indirizzare il corretto slot nello stack
e fornire gli argomenti agli opcode nel corretto punto dello stack. L'inline assembly di Solidity
facilita questa operazione e risolve alcuni problemi che si possono incontrare nella scrittura 
manuale di assembly.

L'inline assembly ha le seguenti caratteristiche:

* opcode in stile funzionale: ``mul(1, add(2, 3))``
* variabili locali assembly: ``let x := add(2, 3)  let y := mload(0x40)  x := add(x, y)``
* accesso a variabili esterne: ``function f(uint x) public { assembly { x := sub(x, 1) } }``
* cicli: ``for { let i := 0 } lt(i, x) { i := add(i, 1) } { y := mul(2, y) }``
* if statement: ``if slt(x, 0) { x := sub(0, x) }``
* switch statement: ``switch x case 0 { y := mul(x, 2) } default { y := 0 }``
* chiamate a funzione: ``function f(x) -> y { switch x case 0 { y := 1 } default { y := mul(x, f(sub(x, 1))) }   }``

.. warning::
    Inline assembly è un metododi basso livello per accedere alla Ethereum Virtual Machine.
    Questo bypassa molti controlli di sicurezza effettuati da Solidity quindi dovrebbe
    essere usato solamente se necessario e se si è confidenti nel suo utilizzo.

Sintassi
--------

Assembly analizza i commenti, letterali ed indentificatori allo stesso modo di Solidity, quindi possono essere
utilizzati ``//`` e ``/* */`` per i commenti. C'è una eccezione: gli identificatori nell'inline assembly 
possono contenere ``.``. 
L'inline assembly è marcato con ``assembly { ... }`` e all'interno queste 
parentesi graffe, si possono usare i seguenti (vedere la sezione seguente per più dettagli):

 - letteralo, i.e. ``0x123``, ``42`` o ``"abc"`` (stringhe fino a 32 caratteri)
 - opcode in stile funzionale, e.g. ``add(1, mlod(0))``
 - dichiarazioni di variabili, e.g. ``let x := 7``, ``let x := add(y, 3)`` o ``let x`` (viene assegnato come valore iniziale 0)
 - identificatori (variabili locali ad assembly ed esterne se vengono utilizzate come inline assembly), e.g. ``add(3, x)``, ``sstore(x_slot, 2)``
 - assegamenti, e.g. ``x := add(y, 3)``
 - blocchi nei qualli hanno scope le variabili locali, e.g. ``{ let x := 3 { let y := add(x, 1) } }``

Le seguenti caratteristiche sono disponibili per standalone assembly:

 - controllo dello stack diretto con ``dup1``, ``swap1``, ...
 - assegnamenti diretti nello stack ("instruction style"), e.g. ``3 =: x``
 - label, e.g. ``name:``
 - opcode jump

.. note::
  Standalone assembly è supportato per backwards compatibility ma non è più documentato in questa guida.

Alla fine del blocco ``assembly { ... }``, lo stack deve essere bilanciato, almeno che non sia
richiesto esplicitamente. Se non bilanciato, il compilatore genera un warning.

Esempio
-------

Il seguente esempio fornisce il codice libreria per accedere il codice di un altro contratto e
caricato in una variabile ``bytes``. Questo non è possibile con "plain Solidity" e l'idea è che le
librerie assembly sono usate per migliorare Solidity.

.. code::

    pragma solidity >=0.4.0 <0.7.0;

    library GetCode {
        function at(address _addr) public view returns (bytes memory o_code) {
            assembly {
                // recupera la dimensione del codice
                let size := extcodesize(_addr)
                // alloca l'array di byte di output - questo può essere fatto anche senza assembly
                // utilizzando o_code = new bytes(size)
                o_code := mload(0x40)
                // nuovo "memory end" incluso il padding
                mstore(0x40, add(o_code, and(add(add(size, 0x20), 0x1f), not(0x1f))))
                // salva la lunghezza in memoria
                mstore(o_code, size)
                // recupera il codice, serve assembly
                extcodecopy(_addr, add(o_code, 0x20), 0, size)
            }
        }
    }

Inline assembly è anche utile nel caso in cui l'ottimizzatore non produca codice efficiente, per esempio:

.. code::

    pragma solidity >=0.4.16 <0.7.0;


    library VectorSum {
        // Questa funzione è meno efficente perché attualmente l'ottimizzatore non riesce a
        // rimuovere i controlli sulla dimensione dell'array durante l'accesso.
        function sumSolidity(uint[] memory _data) public pure returns (uint sum) {
            for (uint i = 0; i < _data.length; ++i)
                sum += _data[i];
        }

        // L'array può essere acceduto solamente all'interno dei suoi limiti quindi i controlli possono essere evitati.
        // 0x20 deve essere aggiunto all'array perché la prima locazione contiene la lunghezza dell'array.
        function sumAsm(uint[] memory _data) public pure returns (uint sum) {
            for (uint i = 0; i < _data.length; ++i) {
                assembly {
                    sum := add(sum, mload(add(add(_data, 0x20), mul(i, 0x20))))
                }
            }
        }

        // Come sopra ma senza inline assembly.
        function sumPureAsm(uint[] memory _data) public pure returns (uint sum) {
            assembly {
                // Load the length (first 32 bytes)
                let len := mload(_data)

                // Salto del campo della lunghezza.
                // Viene mantenuta una variabile locale così può essere incrementata in place.
                // NOTA: incrementare _data fa sì che la variabile _data sia inutilizzabile dopo
                // questo blocco assembly.
                let data := add(_data, 0x20)

                // Iterazione finchè non raggiungo la fine.
                for
                    { let end := add(data, mul(len, 0x20)) }
                    lt(data, end)
                    { data := add(data, 0x20) }
                {
                    sum := add(sum, mload(data))
                }
            }
        }
    }


.. _opcodes:

Opcode
------

Questo documento non vuole essere una descrizione completa della Ethereum virtual machine,
ma la seguente lista può essere utilizzata come riferimento per i suoi opcode.

Se un opcode richiede degli argomenti (sempre dalla cima dello stack), questi vengono passati tra parentesi.
Si noti che l'ordine degli argomenti può essere visto come invertito in uno stile non funzionale (spiegato di seguito).
Gli opcode contrassegnati con `` -`` non inseriscono un oggetto nello stack (non restituiscono un risultato),
quelli contrassegnati con `` * `` sono speciali e tutti gli altri inseriscono esattamente un oggetto nello stack (il loro "valore di ritorno").
Gli opcode contrassegnati con `` F``, `` H``, `` B`` o `` C`` sono presenti rispettivamente da Frontier, Homestead, Byzantium o Constantinople.

Nella seguente lista, ``mem[a...b)`` indica i byte di memoria che partono dalla posizione ``a`` fino a (ma non inclusa)
``b`` e ``storage[p]`` indica il contenuto dello storage in posizione ``p``.

Gli opcode ``pushi`` e ``jumpdest`` non possono essere utilizzati diretamente.

Nella grammatica, gli opcode sono rappresentati come identificatori predefiniti.

+-------------------------+-----+---+-----------------------------------------------------------------+
| Istruzione              |     |   | Descrizione                                                     |
+=========================+=====+===+=================================================================+
| stop                    + `-` | F | ferma l'esecuzione, identico a return(0,0)                      |
+-------------------------+-----+---+-----------------------------------------------------------------+
| add(x, y)               |     | F | x + y                                                           |
+-------------------------+-----+---+-----------------------------------------------------------------+
| sub(x, y)               |     | F | x - y                                                           |
+-------------------------+-----+---+-----------------------------------------------------------------+
| mul(x, y)               |     | F | x * y                                                           |
+-------------------------+-----+---+-----------------------------------------------------------------+
| div(x, y)               |     | F | x / y                                                           |
+-------------------------+-----+---+-----------------------------------------------------------------+
| sdiv(x, y)              |     | F | x / y, per numeri con segno in complemento a due                |
+-------------------------+-----+---+-----------------------------------------------------------------+
| mod(x, y)               |     | F | x % y                                                           |
+-------------------------+-----+---+-----------------------------------------------------------------+
| smod(x, y)              |     | F | x % y, per numeri con segno in complemento a due                |
+-------------------------+-----+---+-----------------------------------------------------------------+
| exp(x, y)               |     | F | x alla potenza di y                                             |
+-------------------------+-----+---+-----------------------------------------------------------------+
| not(x)                  |     | F | ~x, ogni bit di x viene negato                                  |
+-------------------------+-----+---+-----------------------------------------------------------------+
| lt(x, y)                |     | F | 1 se x < y, 0 altrimenti                                        |
+-------------------------+-----+---+-----------------------------------------------------------------+
| gt(x, y)                |     | F | 1 se x > y, 0 altrimenti                                        |
+-------------------------+-----+---+-----------------------------------------------------------------+
| slt(x, y)               |     | F | 1 se x < y, 0 altrimenti, per numeri con segno in complemento   | 
|                         |     |   | a due                                                           |
+-------------------------+-----+---+-----------------------------------------------------------------+
| sgt(x, y)               |     | F | 1 se x > y, 0 altrimenti, per numeri con segno in complemento   |
|                         |     |   | a due                                                           |
+-------------------------+-----+---+-----------------------------------------------------------------+
| eq(x, y)                |     | F | 1 se x == y, 0 altrimenti                                       |
+-------------------------+-----+---+-----------------------------------------------------------------+
| iszero(x)               |     | F | 1 se x == 0, 0 altrimenti                                       |
+-------------------------+-----+---+-----------------------------------------------------------------+
| and(x, y)               |     | F | bitwise and di x e y                                            |
+-------------------------+-----+---+-----------------------------------------------------------------+
| or(x, y)                |     | F | bitwise or di x e y                                             |
+-------------------------+-----+---+-----------------------------------------------------------------+
| xor(x, y)               |     | F | bitwise xor di x e y                                            |
+-------------------------+-----+---+-----------------------------------------------------------------+
| byte(n, x)              |     | F | n-esimo byte di x, dove il byte più significativo è in          |
|                         |     |   | posizione 0                                                     |
+-------------------------+-----+---+-----------------------------------------------------------------+
| shl(x, y)               |     | C | shift logico a sinistra di y di x bit                           |
+-------------------------+-----+---+-----------------------------------------------------------------+
| shr(x, y)               |     | C | shift logico destra di y di x bit                               |
+-------------------------+-----+---+-----------------------------------------------------------------+
| sar(x, y)               |     | C | shift aritmetico a destra di y per x bit                        |
+-------------------------+-----+---+-----------------------------------------------------------------+
| addmod(x, y, m)         |     | F | (x + y) % m con precisione aritmetica arbitraria                |
+-------------------------+-----+---+-----------------------------------------------------------------+
| mulmod(x, y, m)         |     | F | (x * y) % m con precisione aritmetica arbitraria                |
+-------------------------+-----+---+-----------------------------------------------------------------+
| signextend(i, x)        |     | F | estensione di segno dal (i*8+7)-esimo bit partendo dal meno     |
|                         |     |   | significativo                                                   |
+-------------------------+-----+---+-----------------------------------------------------------------+
| keccak256(p, n)         |     | F | keccak(mem[p...(p+n)))                                          |
+-------------------------+-----+---+-----------------------------------------------------------------+
| jump(label)             | `-` | F | salto alla label / posizione del codice                         |
+-------------------------+-----+---+-----------------------------------------------------------------+
| jumpi(label, cond)      | `-` | F | jump alla label se la condizione no è zero                      |
+-------------------------+-----+---+-----------------------------------------------------------------+
| pc                      |     | F | posizione corrente nel codice                                   |
+-------------------------+-----+---+-----------------------------------------------------------------+
| pop(x)                  | `-` | F | rimuove l'elemento inserito da x                                |
+-------------------------+-----+---+-----------------------------------------------------------------+
| dup1 ... dup16          |     | F | copia l'n-esimo slot dello stack in cima (contando dalla cima)  |
+-------------------------+-----+---+-----------------------------------------------------------------+
| swap1 ... swap16        | `*` | F | scambia il primo elemento con l'n-esimo al di sotto di esso     |
+-------------------------+-----+---+-----------------------------------------------------------------+
| mload(p)                |     | F | mem[p...(p+32))                                                 |
+-------------------------+-----+---+-----------------------------------------------------------------+
| mstore(p, v)            | `-` | F | mem[p...(p+32)) := v                                            |
+-------------------------+-----+---+-----------------------------------------------------------------+
| mstore8(p, v)           | `-` | F | mem[p] := v & 0xff (modifica solo un singolo byte)              |
+-------------------------+-----+---+-----------------------------------------------------------------+
| sload(p)                |     | F | storage[p]                                                      |
+-------------------------+-----+---+-----------------------------------------------------------------+
| sstore(p, v)            | `-` | F | storage[p] := v                                                 |
+-------------------------+-----+---+-----------------------------------------------------------------+
| msize                   |     | F | dimensione della memoria, i.e. l'indice più grande accessibile  |
+-------------------------+-----+---+-----------------------------------------------------------------+
| gas                     |     | F | gas ancora disponibile per l'esecuzione                         |
+-------------------------+-----+---+-----------------------------------------------------------------+
| address                 |     | F | indirizzo del contratto corrente / contesto d'esecuzione        |
+-------------------------+-----+---+-----------------------------------------------------------------+
| balance(a)              |     | F | wei balance all'indirizzo a                                     |
+-------------------------+-----+---+-----------------------------------------------------------------+
| caller                  |     | F | call sender (tranne ``delegatecall``)                           |
+-------------------------+-----+---+-----------------------------------------------------------------+
| callvalue               |     | F | wei inviato assieme alla chiamata corrente                      |
+-------------------------+-----+---+-----------------------------------------------------------------+
| calldataload(p)         |     | F | dati di chiamata partendo dalla posizione p (32 byte)           |
+-------------------------+-----+---+-----------------------------------------------------------------+
| calldatasize            |     | F | dimensione dei dati di chiamata in byte                         |
+-------------------------+-----+---+-----------------------------------------------------------------+
| calldatacopy(t, f, s)   | `-` | F | copia s byte dai dati di chiamata alla posizione f dalla        |
|                         |     |   | memoria in posizione t                                          |
+-------------------------+-----+---+-----------------------------------------------------------------+
| codesize                |     | F | dimensione del codice del contratto corrente / contesto         |
|                         |     |   | d'esecuzione                                                    | 
+-------------------------+-----+---+-----------------------------------------------------------------+
| codecopy(t, f, s)       | `-` | F | copia s byte dal codice in posizione f alla memoria in          |
|                         |     |   | posizione t                                                     |
+-------------------------+-----+---+-----------------------------------------------------------------+
| extcodesize(a)          |     | F | dimensione del codice all'indirizzo a                           |
+-------------------------+-----+---+-----------------------------------------------------------------+
| extcodecopy(a, t, f, s) | `-` | F | come codecopy(t, f, s) ma prende il codice dall'indirizzo a     |
+-------------------------+-----+---+-----------------------------------------------------------------+
| returndatasize          |     | B | dimensione degli ultimi dati di ritorno                         |
+-------------------------+-----+---+-----------------------------------------------------------------+
| returndatacopy(t, f, s) | `-` | B | copia s byte dai dati di ritorno in posizione f nella memoria   |
|                         |     |   | in posizione t                                                  |
+-------------------------+-----+---+-----------------------------------------------------------------+
| extcodehash(a)          |     | C | hash del codice all'indirizzo a                                 |
+-------------------------+-----+---+-----------------------------------------------------------------+
| create(v, p, n)         |     | F | crea un nuovo contratto con codice mem[p...(p+n)), invia v wei  |
|                         |     |   | e restituisce il nuovo indirizzo                                |
+-------------------------+-----+---+-----------------------------------------------------------------+
| create2(v, p, n, s)     |     | C | crea un nuovo contratto con codice mem[p...(p+n)) all'indirizzo |
|                         |     |   | keccak256(0xff . this . s . keccak256(mem[p...(p+n)))           |
|                         |     |   | invia v wei e restituisce il nuovo indirizzo, dove ``0xff`` è   |
|                         |     |   | un valore a 8 byte, ``this`` è l'indirizzo del contratto        |
|                         |     |   | corrente come valore a 20 byte e ``s`` è un valore big-endian   |  
|                         |     |   | a 256-bit                                                       |
+-------------------------+-----+---+-----------------------------------------------------------------+
| call(g, a, v, in,       |     | F | chiama il contratto all'indirizzo a con input                   |
| insize, out, outsize)   |     |   | mem[in...(in+insize)) fornendo g gas e v wei e area di output   |
|                         |     |   | mem[out...(out+outsize)) restituendo 0 in caso di errore        |
|                         |     |   | (eg. out of gas) e 1 in caso di successo                        |
+-------------------------+-----+---+-----------------------------------------------------------------+
| callcode(g, a, v, in,   |     | F | uguale a ``call`` ma usa solo il codice da a e rimane nel       |
| insize, out, outsize)   |     |   | contesto del codice corrente                                    |
+-------------------------+-----+---+-----------------------------------------------------------------+
| delegatecall(g, a, in,  |     | H | uguale a ``callcode`` ma mantiene anche  ``caller``             |
| insize, out, outsize)   |     |   | e ``callvalue``                                                 |
+-------------------------+-----+---+-----------------------------------------------------------------+
| staticcall(g, a, in,    |     | B | uguale a ``call(g, a, 0, in, insize, out, outsize)`` ma non     |
| insize, out, outsize)   |     |   | permette modifiche di stato                                     |
+-------------------------+-----+---+-----------------------------------------------------------------+
| return(p, s)            | `-` | F | termina l'esecuzione, restituisce i dati mem[p...(p+s))         |
+-------------------------+-----+---+-----------------------------------------------------------------+
| revert(p, s)            | `-` | B | termina l'esecuzione, annulla i cambiamenti di stato,           |
|                         |     |   | restituise i dati mem[p...(p+s))                                |
+-------------------------+-----+---+-----------------------------------------------------------------+
| selfdestruct(a)         | `-` | F | termina l'esecuzione, distrugge il contratto corrente ed invia  | 
|                         |     |   | i fondi ad a                                                    |
+-------------------------+-----+---+-----------------------------------------------------------------+
| invalid                 | `-` | F | termine dell'eseecuzione con una istruzione non valida          |
+-------------------------+-----+---+-----------------------------------------------------------------+
| log0(p, s)              | `-` | F | log senza topic e dati mem[p...(p+s))                           |
+-------------------------+-----+---+-----------------------------------------------------------------+
| log1(p, s, t1)          | `-` | F | log con topic t1 e dati mem[p...(p+s))                          |
+-------------------------+-----+---+-----------------------------------------------------------------+
| log2(p, s, t1, t2)      | `-` | F | log con topic t1, t2 e dati mem[p...(p+s))                      |
+-------------------------+-----+---+-----------------------------------------------------------------+
| log3(p, s, t1, t2, t3)  | `-` | F | log con topic t1, t2, t3 e dati mem[p...(p+s))                  |
+-------------------------+-----+---+-----------------------------------------------------------------+
| log4(p, s, t1, t2, t3,  | `-` | F | log con topic t1, t2, t3, t4 e dati mem[p...(p+s))              |
| t4)                     |     |   |                                                                 |
+-------------------------+-----+---+-----------------------------------------------------------------+
| origin                  |     | F | mittente della transazione                                      |
+-------------------------+-----+---+-----------------------------------------------------------------+
| gasprice                |     | F | prezzo in gas per transazione                                   |
+-------------------------+-----+---+-----------------------------------------------------------------+
| blockhash(b)            |     | F | hash del blocco numero b - solo per gli ultimi 256 blocchi      |
|                         |     |   | tranne quello corrente                                          |
+-------------------------+-----+---+-----------------------------------------------------------------+
| coinbase                |     | F | beneficiario del mining corrente                                |
+-------------------------+-----+---+-----------------------------------------------------------------+
| timestamp               |     | F | timestamp del blocco corrente in secondi da l'epoch             |
+-------------------------+-----+---+-----------------------------------------------------------------+
| number                  |     | F | numero del blocco corrente                                      |
+-------------------------+-----+---+-----------------------------------------------------------------+
| difficulty              |     | F | difficoltà del blocco corrente                                  |
+-------------------------+-----+---+-----------------------------------------------------------------+
| gaslimit                |     | F | block gas limit del blocco corrente                             |
+-------------------------+-----+---+-----------------------------------------------------------------+

Letterali
--------_

You can use integer constants by typing them in decimal or hexadecimal notation and an
appropriate ``PUSHi`` instruction will automatically be generated. The following creates code
to add 2 and 3 resulting in 5 and then computes the bitwise ``AND`` with the string "abc".
The final value is assigned to a local variable called ``x``.
Strings are stored left-aligned and cannot be longer than 32 bytes.

.. code::

    assembly { let x := and("abc", add(3, 2)) }


Functional Style
-----------------

For a sequence of opcodes, it is often hard to see what the actual
arguments for certain opcodes are. In the following example,
``3`` is added to the contents in memory at position ``0x80``.

.. code::

    3 0x80 mload add 0x80 mstore

Solidity inline assembly has a "functional style" notation where the same code
would be written as follows:

.. code::

    mstore(0x80, add(mload(0x80), 3))

If you read the code from right to left, you end up with exactly the same
sequence of constants and opcodes, but it is much clearer where the
values end up.

If you care about the exact stack layout, just note that the
syntactically first argument for a function or opcode will be put at the
top of the stack.

Access to External Variables, Functions and Libraries
-----------------------------------------------------

You can access Solidity variables and other identifiers by using their name.
For variables stored in the memory data location, this pushes the address, and not the value
onto the stack. Variables stored in the storage data location are different, as they might not
occupy a full storage slot, so their "address" is composed of a slot and a byte-offset
inside that slot. To retrieve the slot pointed to by the variable ``x``, you
use ``x_slot``, and to retrieve the byte-offset you use ``x_offset``.

Local Solidity variables are available for assignments, for example:

.. code::

    pragma solidity >=0.4.11 <0.7.0;

    contract C {
        uint b;
        function f(uint x) public view returns (uint r) {
            assembly {
                r := mul(x, sload(b_slot)) // ignore the offset, we know it is zero
            }
        }
    }

.. warning::
    If you access variables of a type that spans less than 256 bits
    (for example ``uint64``, ``address``, ``bytes16`` or ``byte``),
    you cannot make any assumptions about bits not part of the
    encoding of the type. Especially, do not assume them to be zero.
    To be safe, always clear the data properly before you use it
    in a context where this is important:
    ``uint32 x = f(); assembly { x := and(x, 0xffffffff) /* now use x */ }``
    To clean signed types, you can use the ``signextend`` opcode:
    ``assembly { signextend(<num_bytes_of_x_minus_one>, x) }``

Labels
------

Support for labels has been removed in version 0.5.0 of Solidity.
Please use functions, loops, if or switch statements instead.

Declaring Assembly-Local Variables
----------------------------------

You can use the ``let`` keyword to declare variables that are only visible in
inline assembly and actually only in the current ``{...}``-block. What happens
is that the ``let`` instruction will create a new stack slot that is reserved
for the variable and automatically removed again when the end of the block
is reached. You need to provide an initial value for the variable which can
be just ``0``, but it can also be a complex functional-style expression.

.. code::

    pragma solidity >=0.4.16 <0.7.0;

    contract C {
        function f(uint x) public view returns (uint b) {
            assembly {
                let v := add(x, 1)
                mstore(0x80, v)
                {
                    let y := add(sload(v), 1)
                    b := y
                } // y is "deallocated" here
                b := add(b, v)
            } // v is "deallocated" here
        }
    }


Assignments
-----------

Assignments are possible to assembly-local variables and to function-local
variables. Take care that when you assign to variables that point to
memory or storage, you will only change the pointer and not the data.

Variables can only be assigned expressions that result in exactly one value.
If you want to assign the values returned from a function that has
multiple return parameters, you have to provide multiple variables.

.. code::

    {
        let v := 0
        let g := add(v, 2)
        function f() -> a, b { }
        let c, d := f()
    }

If
--

The if statement can be used for conditionally executing code.
There is no "else" part, consider using "switch" (see below) if
you need multiple alternatives.

.. code::

    {
        if eq(value, 0) { revert(0, 0) }
    }

The curly braces for the body are required.

Switch
------

You can use a switch statement as a very basic version of "if/else".
It takes the value of an expression and compares it to several constants.
The branch corresponding to the matching constant is taken. Contrary to the
error-prone behaviour of some programming languages, control flow does
not continue from one case to the next. There can be a fallback or default
case called ``default``.

.. code::

    {
        let x := 0
        switch calldataload(4)
        case 0 {
            x := calldataload(0x24)
        }
        default {
            x := calldataload(0x44)
        }
        sstore(0, div(x, 2))
    }

The list of cases does not require curly braces, but the body of a
case does require them.

Loops
-----

Assembly supports a simple for-style loop. For-style loops have
a header containing an initializing part, a condition and a post-iteration
part. The condition has to be a functional-style expression, while
the other two are blocks. If the initializing part
declares any variables, the scope of these variables is extended into the
body (including the condition and the post-iteration part).

The following example computes the sum of an area in memory.

.. code::

    {
        let x := 0
        for { let i := 0 } lt(i, 0x100) { i := add(i, 0x20) } {
            x := add(x, mload(i))
        }
    }

For loops can also be written so that they behave like while loops:
Simply leave the initialization and post-iteration parts empty.

.. code::

    {
        let x := 0
        let i := 0
        for { } lt(i, 0x100) { } {     // while(i < 0x100)
            x := add(x, mload(i))
            i := add(i, 0x20)
        }
    }

Functions
---------

Assembly allows the definition of low-level functions. These take their
arguments (and a return PC) from the stack and also put the results onto the
stack. Calling a function looks the same way as executing a functional-style
opcode.

Functions can be defined anywhere and are visible in the block they are
declared in. Inside a function, you cannot access local variables
defined outside of that function. There is no explicit ``return``
statement.

If you call a function that returns multiple values, you have to assign
them to a tuple using ``a, b := f(x)`` or ``let a, b := f(x)``.

The following example implements the power function by square-and-multiply.

.. code::

    {
        function power(base, exponent) -> result {
            switch exponent
            case 0 { result := 1 }
            case 1 { result := base }
            default {
                result := power(mul(base, base), div(exponent, 2))
                switch mod(exponent, 2)
                    case 1 { result := mul(base, result) }
            }
        }
    }

Things to Avoid
---------------

Inline assembly might have a quite high-level look, but it actually is extremely
low-level. Function calls, loops, ifs and switches are converted by simple
rewriting rules and after that, the only thing the assembler does for you is re-arranging
functional-style opcodes, counting stack height for
variable access and removing stack slots for assembly-local variables when the end
of their block is reached.

Conventions in Solidity
-----------------------

In contrast to EVM assembly, Solidity has types which are narrower than 256 bits,
e.g. ``uint24``. For efficiency, most arithmetic operations ignore the fact that types can be shorter than 256
bits, and the higher-order bits are cleaned when necessary,
i.e., shortly before they are written to memory or before comparisons are performed.
This means that if you access such a variable
from within inline assembly, you might have to manually clean the higher-order bits
first.

Solidity manages memory in the following way. There is a "free memory pointer"
at position ``0x40`` in memory. If you want to allocate memory, use the memory
starting from where this pointer points at and update it.
There is no guarantee that the memory has not been used before and thus
you cannot assume that its contents are zero bytes.
There is no built-in mechanism to release or free allocated memory.
Here is an assembly snippet you can use for allocating memory that follows the process outlined above::

    function allocate(length) -> pos {
      pos := mload(0x40)
      mstore(0x40, add(pos, length))
    }

The first 64 bytes of memory can be used as "scratch space" for short-term
allocation. The 32 bytes after the free memory pointer (i.e., starting at ``0x60``)
are meant to be zero permanently and is used as the initial value for
empty dynamic memory arrays.
This means that the allocatable memory starts at ``0x80``, which is the initial value
of the free memory pointer.

Elements in memory arrays in Solidity always occupy multiples of 32 bytes (this is
even true for ``byte[]``, but not for ``bytes`` and ``string``). Multi-dimensional memory
arrays are pointers to memory arrays. The length of a dynamic array is stored at the
first slot of the array and followed by the array elements.

.. warning::
    Statically-sized memory arrays do not have a length field, but it might be added later
    to allow better convertibility between statically- and dynamically-sized arrays, so
    do not rely on this.


Standalone Assembly
===================

The assembly language described as inline assembly above can also be used
standalone and in fact, the plan is to use it as an intermediate language
for the Solidity compiler. In this form, it tries to achieve several goals:

1. Programs written in it should be readable, even if the code is generated by a compiler from Solidity.
2. The translation from assembly to bytecode should contain as few "surprises" as possible.
3. Control flow should be easy to detect to help in formal verification and optimization.

In order to achieve the first and last goal, assembly provides high-level constructs
like ``for`` loops, ``if`` and ``switch`` statements and function calls. It should be possible
to write assembly programs that do not make use of explicit ``SWAP``, ``DUP``,
``JUMP`` and ``JUMPI`` statements, because the first two obfuscate the data flow
and the last two obfuscate control flow. Furthermore, functional statements of
the form ``mul(add(x, y), 7)`` are preferred over pure opcode statements like
``7 y x add mul`` because in the first form, it is much easier to see which
operand is used for which opcode.

The second goal is achieved by compiling the
higher level constructs to bytecode in a very regular way.
The only non-local operation performed
by the assembler is name lookup of user-defined identifiers (functions, variables, ...),
which follow very simple and regular scoping rules and cleanup of local variables from the stack.

Scoping: An identifier that is declared (label, variable, function, assembly)
is only visible in the block where it was declared (including nested blocks
inside the current block). It is not legal to access local variables across
function borders, even if they would be in scope. Shadowing is not allowed.
Local variables cannot be accessed before they were declared, but
functions and assemblies can. Assemblies are special blocks that are used
for e.g. returning runtime code or creating contracts. No identifier from an
outer assembly is visible in a sub-assembly.

If control flow passes over the end of a block, pop instructions are inserted
that match the number of local variables declared in that block.
Whenever a local variable is referenced, the code generator needs
to know its current relative position in the stack and thus it needs to
keep track of the current so-called stack height. Since all local variables
are removed at the end of a block, the stack height before and after the block
should be the same. If this is not the case, compilation fails.

Using ``switch``, ``for`` and functions, it should be possible to write
complex code without using ``jump`` or ``jumpi`` manually. This makes it much
easier to analyze the control flow, which allows for improved formal
verification and optimization.

Furthermore, if manual jumps are allowed, computing the stack height is rather complicated.
The position of all local variables on the stack needs to be known, altrimenti
neither references to local variables nor removing local variables automatically
from the stack at the end of a block will work properly.

Example:

We will follow an example compilation from Solidity to assembly.
We consider the runtime bytecode of the following Solidity program::

    pragma solidity >=0.4.16 <0.7.0;


    contract C {
        function f(uint x) public pure returns (uint y) {
            y = 1;
            for (uint i = 0; i < x; i++)
                y = 2 * y;
        }
    }

The following assembly will be generated::

    {
      mstore(0x40, 0x80) // store the "free memory pointer"
      // function dispatcher
      switch div(calldataload(0), exp(2, 226))
      case 0xb3de648b {
        let r := f(calldataload(4))
        let ret := $allocate(0x20)
        mstore(ret, r)
        return(ret, 0x20)
      }
      default { revert(0, 0) }
      // memory allocator
      function $allocate(size) -> pos {
        pos := mload(0x40)
        mstore(0x40, add(pos, size))
      }
      // the contract function
      function f(x) -> y {
        y := 1
        for { let i := 0 } lt(i, x) { i := add(i, 1) } {
          y := mul(2, y)
        }
      }
    }


Assembly Grammar
----------------

The tasks of the parser are the following:

- Turn the byte stream into a token stream, discarding C++-style comments
  (a special comment exists for source references, but we will not explain it here).
- Turn the token stream into an AST according to the grammar below
- Register identifiers with the block they are defined in (annotation to the
  AST node) and note from which point on, variables can be accessed.

The assembly lexer follows the one defined by Solidity itself.

Whitespace is used to delimit tokens and it consists of the characters
Space, Tab and Linefeed. Comments are regular JavaScript/C++ comments and
are interpreted in the same way as Whitespace.

Grammar::

    AssemblyBlock = '{' AssemblyItem* '}'
    AssemblyItem =
        Identifier |
        AssemblyBlock |
        AssemblyExpression |
        AssemblyLocalDefinition |
        AssemblyAssignment |
        AssemblyStackAssignment |
        LabelDefinition |
        AssemblyIf |
        AssemblySwitch |
        AssemblyFunctionDefinition |
        AssemblyFor |
        'break' |
        'continue' |
        SubAssembly
    AssemblyExpression = AssemblyCall | Identifier | AssemblyLiteral
    AssemblyLiteral = NumberLiteral | StringLiteral | HexLiteral
    Identifier = [a-zA-Z_$] [a-zA-Z_0-9.]*
    AssemblyCall = Identifier '(' ( AssemblyExpression ( ',' AssemblyExpression )* )? ')'
    AssemblyLocalDefinition = 'let' IdentifierOrList ( ':=' AssemblyExpression )?
    AssemblyAssignment = IdentifierOrList ':=' AssemblyExpression
    IdentifierOrList = Identifier | '(' IdentifierList ')'
    IdentifierList = Identifier ( ',' Identifier)*
    AssemblyStackAssignment = '=:' Identifier
    LabelDefinition = Identifier ':'
    AssemblyIf = 'if' AssemblyExpression AssemblyBlock
    AssemblySwitch = 'switch' AssemblyExpression AssemblyCase*
        ( 'default' AssemblyBlock )?
    AssemblyCase = 'case' AssemblyExpression AssemblyBlock
    AssemblyFunctionDefinition = 'function' Identifier '(' IdentifierList? ')'
        ( '->' '(' IdentifierList ')' )? AssemblyBlock
    AssemblyFor = 'for' ( AssemblyBlock | AssemblyExpression )
        AssemblyExpression ( AssemblyBlock | AssemblyExpression ) AssemblyBlock
    SubAssembly = 'assembly' Identifier AssemblyBlock
    NumberLiteral = HexNumber | DecimalNumber
    HexLiteral = 'hex' ('"' ([0-9a-fA-F]{2})* '"' | '\'' ([0-9a-fA-F]{2})* '\'')
    StringLiteral = '"' ([^"\r\n\\] | '\\' .)* '"'
    HexNumber = '0x' [0-9a-fA-F]+
    DecimalNumber = [0-9]+
