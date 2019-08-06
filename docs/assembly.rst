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
    Inline assembly è un metodi di basso livello per accedere alla Ethereum Virtual Machine.
    Questo bypassa molti controlli di sicurezza effettuati da Solidity quindi dovrebbe
    essere usato solamente se necessario e se si è confidenti nel suo utilizzo.

Sintassi
--------

Assembly analizza i commenti, letterali ed identificatori allo stesso modo di Solidity, quindi possono essere
utilizzati ``//`` e ``/* */`` per i commenti. C'è una eccezione: gli identificatori nell'inline assembly 
possono contenere ``.``. 
L'inline assembly è marcato con ``assembly { ... }`` e all'interno queste 
parentesi graffe, si possono usare i seguenti (vedere la sezione seguente per più dettagli):

 - letterali, i.e. ``0x123``, ``42`` o ``"abc"`` (stringhe fino a 32 caratteri)
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
  L'assembly standalone assembly è supportato per retrocompatibilità ma non è più documentato in questa guida.

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
        // Questa funzione è meno efficiente perché attualmente l'ottimizzatore non riesce a
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

Gli opcode ``pushi`` e ``jumpdest`` non possono essere utilizzati direttamente.

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
| jumpi(label, cond)      | `-` | F | jump alla label se la condizione non è zero                     |
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
---------

È possibile utilizzare costanti intere in notazione decimale o esadecimale e
le istruzioni appropriate `` PUSHi`` verranno generate automaticamente. Quanto segue crea il codice
per aggiungere 2 e 3 risultanti in 5 e quindi calcola `` AND`` bit a bit con la stringa "abc".
Il valore finale è assegnato ad una variabile locale chiamata `` x``.
Le stringhe sono memorizzate allineate a sinistra e non possono superare i 32 byte.

.. code::

    assembly { let x := and("abc", add(3, 2)) }


Stile Funzionale
----------------

Per una sequenza di opcode, è spesso difficile capire quali sono gli argomenti. 
Nell'esempio seguente, ``3`` viene aggiunto nel contesto in memoria alla posizione ``0x80``.

.. code::

    3 0x80 mload add 0x80 mstore

Solidity inline assembly offre una notazione in "stile funzionale" dove lo stesso codice
risulta scritto come:

.. code::

    mstore(0x80, add(mload(0x80), 3))

Se il codice viene letto da destra a sinistra, il tutto risulta esattamente nella stessa sequenza 
di costanti ed opcode ma è molto più chiara la disposizione dei valori.

Se si è interessati alla struttura esatta dello stack, notare che il primo argomento 
per una funzione o opcode sarà inserito in cima allo stack.

Accesso a Variabili Esterne, Funzioni e Librerie
------------------------------------------------

Si può accedere alle variabili Solidity e ad altri identificatori usando il loro nome.
Per le variabili archiviate nella posizione dei dati di memoria, 
questo spinge l'indirizzo e non il valore nello stack. 
Le variabili archiviate nella posizione storage sono diverse, in quanto potrebbero non 
occupare uno slot di archiviazione completo, quindi il loro "indirizzo" è 
composto da uno slot e da un offset di byte all'interno di quello slot. 
Per recuperare lo slot a cui punta la variabile `` x``, si usa `` x_slot``, e per 
recuperare l'offset di byte si usa `` x_offset``.

Le variabili locali Solidity sono disponibili per gli assegnamenti, per esempio:

.. code::

    pragma solidity >=0.4.11 <0.7.0;

    contract C {
        uint b;
        function f(uint x) public view returns (uint r) {
            assembly {
                r := mul(x, sload(b_slot)) // ignoriamo l'offset, è 0
            }
        }
    }

.. warning::
    Se vengono accedute variabili di un tipo che si estende per meno di 256 bit 
    (ad esempio `` uint64``, `` address``, `` bytes16`` o `` byte``), non si possono fare 
    ipotesi sui bit non facenti parte della codifica del tipo. 
    Soprattutto, non si deve dare per scontato che siano zero.
    Per sicurezza, cancellare sempre i dati correttamente prima di usarli in un 
    contesto in cui questo è importante: 
    `` uint32 x = f (); assembly {x: = and (x, 0xffffffff) / * ora usa x * /} ``
    Per pulire i tipi con segno, si può usare il codice operativo `` signextend``:
    ``assembly { signextend(<num_bytes_of_x_minus_one>, x) }``

Etichette
---------

Il supporto per le etichette è stato rimosso in Solidity versione 0.5.0.
Usare funzioni, loop, if e swich.

Dichiarazione di Variabili Locali Assembly
------------------------------------------

Può essere utilizzata la keyword ``let`` per dichiarare variabili che sono solamente
visibili nell'inline assembly, di fatto solameente nel blocco ``{...}`` corrente. 
Le istruzioni ``let`` creano un nuovo slot dello stack riservato alle variabili che
verrà rimosso automaticamente al raggiungimento della fine del blocco.
Devono essere specificati i valori iniziali delle variabili che possono essere sia 
``0`` ma anche complesse espressioni in stile funzionale.

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
                } // y viene "deallocata" qui
                b := add(b, v)
            } // v viene "deallocata" qui
        }
    }

Assegnamenti
------------

Gli assegamenti sono possibli per le variabli locali assembly e alle variabili
delle funzioni locali. Notare che, quando viene assegnata una variabile che punta
alla mamoria o allo storage, verrà cambiato solamente il puntatore e non i dati.

Alle variabili possono essere assegnate solo espressioni che generano esattamente un valore.
Se si desidera assegnare i valori restituiti da una funzione con più parametri di ritorno, 
è necessario fornire più variabili.

.. code::

    {
        let v := 0
        let g := add(v, 2)
        function f() -> a, b { }
        let c, d := f()
    }

If
--

L'istruzione if può essere utilizzata per l'esecuzione condizionale del codice.
Non esiste una parte "else". Considerare l'utilizzo di "switch" 
(vedi sotto) se si ha bisogno di più alternative.

.. code::

    {
        if eq(value, 0) { revert(0, 0) }
    }

Le parentesi graffe per il body sono necessarie.

Switch
------

È possibile utilizzare un'istruzione switch come versione di base di "if / else".
In questo caso, il valore di un'espressione viene confrontato con diverse costanti.
Viene preso il ramo corrispondente alla costante che fa match. 
Contrariamente al comportamento soggetto ad errori di alcuni linguaggi di programmazione, 
il flusso di controllo non continua da un caso all'altro. 
Può esserci un caso di fallback o predefinito chiamato `` default``.

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

L'elenco dei casi non richiede parentesi graffe, 
ma il corpo di un caso le richiede.

Cicli
-----

Assembly supporta cicli in stile for. I cicli for sono composti da
un header contenente una parte di inizializzazione, una condizione 
ed una parte di post iterazione.
La condizione deve essere un'espressione in stile funzionale mentre le 
altre due componenti sono blocchi. Se la parte di inizializzazione dichiara 
delle variabili, lo scope di queste variabili è esteso anche al body (compresa
la condizione e la parte di post iterazione).

L'esempio seguente calcola la somma di un'area in memoria.

.. code::

    {
        let x := 0
        for { let i := 0 } lt(i, 0x100) { i := add(i, 0x20) } {
            x := add(x, mload(i))
        }
    }

I cicli for possono essere scritti in modo tale che si comportino come cicli while,
semplicemente lasciando le parti di inizializzazione e post iterazione vuote.

.. code::

    {
        let x := 0
        let i := 0
        for { } lt(i, 0x100) { } {     // while(i < 0x100)
            x := add(x, mload(i))
            i := add(i, 0x20)
        }
    }

Funzioni
--------

Assembly consente la definizione di funzioni di basso livello. 
Queste prendono gli argomenti (e un PC di ritorno) dallo stack ed inseriscono i risultati nello stack. 
La chiamata ad una funzione ha lo stesso aspetto dell'esecuzione di un opcode di tipo funzionale. 

Le funzioni possono essere definite ovunque e sono visibili nel blocco in cui sono dichiarate. 
All'interno di una funzione, non è possibile accedere alle variabili locali 
definite al di fuori di quella funzione. 
Non esiste un'istruzione esplicita di `` ritorno ''.

Se viene chiamata una funzione che restituisce più valori, bisogna assegnarli ad una tupla 
utilizzando ``a, b := f(x)`` o ``let a, b := f(x)``.

L'esempio corrente implementa la funzione potenza attraverso il metodo square-and-multiply.

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

Cose da Evitare
---------------

Inline assembly potrebbe apparire di alto livello, ma in realtà è di livello estremamente basso. 
Chiamate a funzione, loop, if e switch vengono convertiti da semplici regole di riscrittura e, 
successivamente, l'unica cosa che l'assemblatore fa è riorganizzare gli opcode in ​​stile funzionale, 
contare l'altezza dello stack per l'accesso alle variabili e rimuovere gli slot dello stack per 
le variabili locali di assembly quando viene raggiunta la fine del loro blocco. 

Convenzioni in Solidity
-----------------------

In contrasto con EVM assembly, Solidity offre tipi di dato che sono più piccoli di 256 bit,
e.g. ``uint24``. Per questioni di efficienza, gran parte delle operazioni aritmetiche ignora 
il fatto che i tipi possano essere più piccoli di 256 bit e i bit di ordine superiore sono 
puliti se necessario, per esempio prima di una scrittura in memoria o prima di effettuare 
confronti. Questo significa che, se una variabile viene acceduta tramite inline assembly, 
i bit di ordine superiore devono essere puliti manualmente.

Solidity gestisce la memoria nel seguente modo. 
C'è un "puntatore di memoria libera" nella posizione ``0x40``` in memoria. 
Se si desidera allocare la memoria, utilizzare la memoria a partire da dove questo puntatore punta e aggiornarlo.
Non vi è alcuna garanzia che la memoria non sia stata utilizzata in precedenza e quindi non si può 
presumere che il suo contenuto sia pari a zero byte.
Non c'è un meccanismo integrato per rilasciare o liberare la memoria allocata.
Ecco un frammento di codice assembly che si può usare per allocare la 
memoria che segue il processo descritto sopra::

    function allocate(length) -> pos {
      pos := mload(0x40)
      mstore(0x40, add(pos, length))
    }

I primi 64 byte di memoria possono essere utilizzati come "spazio scratch" per l'allocazione a breve termine. 
I 32 byte dopo il puntatore di memoria libera (cioè, a partire da ``0x60``) sono destinati ad 
essere zero permanentemente e sono utilizzati come valore iniziale per array a memoria dinamica vuoti.
Ciò significa che la memoria allocabile inizia a ``0x80``, che è il valore iniziale del puntatore di memoria libera.

Gli elementi in memory array in Solidity occupano sempre multipli di 32 byte 
(questo vale anche per ``byte[]``, ma non per ``byte`` e ``string``). 
I memory array multidimensionali sono puntatori a memory array. 
La lunghezza di un array dinamico è memorizzata nel primo slot dell'array e seguita dagli elementi dell'array.

.. warning::
    Gli array di memoria (memory array) di dimensioni statiche non hanno un campo di lunghezza, 
    ma potrebbe essere aggiunto in seguito per consentire una migliore 
    convertibilità tra array di dimensioni statiche e dinamiche, quindi non fare affidamento su questo.

Standalone Assembly
===================

Il linguaggio assembly sopra descritto come inline assembly può essere utilizzato anche 
come linguaggio autonomo e, infatti, si prevede di utilizzarlo come linguaggio intermedio 
per il compilatore Solidity. 
In questa forma, gli obiettivi che si vogliono raggiungere sono:

1. I programmi scritti in esso dovrebbero essere leggibili, anche se il codice è generato da un compilatore Solidity.
2. La traduzione da assembly a bytecode dovrebbe contenere il minor numero possibile di "sorprese".
3. Il flusso di controllo dovrebbe essere facile da rilevare per aiutare nella verifica formale e nell'ottimizzazione.

Al fine di raggiungere il primo e l'ultimo obiettivo, assembly fornisce costrutti 
di alto livello come cicli ``for``, ``if``, ``switch`` e chiamate di funzione. 
Dovrebbe essere possibile scrivere programmi assembly che non fanno uso esplicito di  
``SWAP``, ``DUP``, ``JUMP`` e ``JUMPI``, perché i primi due offuscano il flusso di dati 
e gli ultimi due offuscano il flusso di controllo. 
Inoltre, le dichiarazioni funzionali della forma ``mul(add(x, y), 7)``` sono preferite 
alle dichiarazioni di puro opcode come ``7 y x add mul`` perché nella prima forma 
è molto più facile vedere quale operando è usato per quale opcode.

Il secondo obiettivo è raggiunto compilando i costrutti di livello superiore a bytecode 
in modo molto regolare.
L'unica operazione non locale eseguita dall'assembler è la ricerca di 
identificatori definiti dall'utente (funzioni, variabili, ....), che seguono 
regole di scoping molto semplici e regolari e la pulizia delle variabili locali dallo stack.

Scoping: un identificatore che viene dichiarato (etichetta, variabile, funzione, gruppo) 
è visibile solo nel blocco in cui è stato dichiarato (compresi i blocchi annidati 
all'interno del blocco corrente). 
Non è legale accedere alle variabili locali tra i confini delle funzioni anche se nello 
stesso scope. Lo shadowing non è consentito.
Le variabili locali non sono accessibili prima che siano state dichiarate, 
ma funzioni e gli assemblies possono. Gli assemblies sono blocchi speciali che vengono 
utilizzati per esempio per la restituzione del codice di runtime o la creazione di contratti.
Nessun identificatore di un assembky esterno è visibile in un suo sub assembly.

Se il flusso di controllo passa oltre la fine di un blocco, vengono inserite istruzioni 
pop che corrispondono al numero di variabili locali dichiarate in quel blocco.
Ogni volta che una variabile locale viene referenziata,
il generatore di codice deve conoscere la sua posizione relativa attuale nello stack 
e quindi deve tenere traccia della cosiddetta altezza attuale dello stack.
Poiché tutte le variabili locali sono rimosse alla fine di un blocco, l'altezza dello stack
prima e dopo il blocco dovrebbe essere la stessa.
Se questo non si verifica, la compilazione fallisce.

Usando ``switch``, ``for`` e funzioni, dovrebbe essere possibile scrivere 
codice complesso senza utilizzare ``jump`` o ``jumpi`` manualmente. 
Questo rende molto più facile analizzare il flusso di controllo, 
il che permette una migliore verifica formale e ottimizzazione.

Inoltre, se sono consentiti i jump manuali, il calcolo dell'altezza dello stack 
è piuttosto complicato.
La posizione di tutte le variabili locali sullo stack deve essere nota, 
altrimenti né i riferimenti alle variabili locali né la rimozione automatica 
delle variabili locali dallo stack alla fine di un blocco funzioneranno correttamente.

Esempio:

Di sqguito un esempio di compilazione da Solidity a assembly.
Viene considerato il bytecode runtime del seguente programma Solidity::

    pragma solidity >=0.4.16 <0.7.0;

    contract C {
        function f(uint x) public pure returns (uint y) {
            y = 1;
            for (uint i = 0; i < x; i++)
                y = 2 * y;
        }
    }

Viene generato il seguente assembly ::

    {
      mstore(0x40, 0x80) // store per il "puntatore alla memoria libera"
      // function dispatcher
      switch div(calldataload(0), exp(2, 226))
      case 0xb3de648b {
        let r := f(calldataload(4))
        let ret := $allocate(0x20)
        mstore(ret, r)
        return(ret, 0x20)
      }
      default { revert(0, 0) }
      // allocatore di memoria
      function $allocate(size) -> pos {
        pos := mload(0x40)
        mstore(0x40, add(pos, size))
      }
      // la funzione del contratto
      function f(x) -> y {
        y := 1
        for { let i := 0 } lt(i, x) { i := add(i, 1) } {
          y := mul(2, y)
        }
      }
    }


Grammatica Assembly
-------------------

I compiti del parser sono i seguenti:

- Trasformare il flusso di byte in un flusso di token, scartando i commenti in stile C++ 
  (esiste un commento speciale per i riferimenti ai sorgenti, ma non verrà spiegato qui)
- Trasformare lo stream di token un AST secondo la grammatica sottostante
- Registrare gli identificatori assieme al blocco nel quale sono definiti
  (nota al nodo AST) e segnare da quale punto in poi le variabili possono 
  essere accedute

Il lexer assembly segue quello definito da Solidity stesso.

Lo spazio bianco è usato per delimitare i token e consiste nei caratteri Spazio, 
Tab e Linefeed. I commenti sono normali commenti JavaScript/C++ e sono interpretati 
allo stesso modo degli spazi bianchi.

Grammatica::

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
