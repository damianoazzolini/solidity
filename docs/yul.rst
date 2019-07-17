#########################################
Yul [da controllare - da terminare lista]
#########################################

.. _yul:

.. index:: ! assembly, ! asm, ! evmasm, ! yul, julia, iulia

Yul (predendentemente chiamato JULIA o IULIA) è un linguaggio intermedio che può essere compilato in bytecode.

Yul è sviluppato per essere un comune denominatore per le piattaforme EVM 1.0, EVM 1.5 ed eWASM poiché è stato pianificato
il supporto per tutte e tre.
Attualmente può essere utilizzato come "inline assembly" in solidity e le versioni future del compilatore
Solidity utilizzeranno Yul come linguaggio intermedio. Yul è un buon target per ottimizzazioni di alto livello
che possono migliorare le performance su tutte le piattaforme.

Utilizzato come "inline assembly", Yul può essere usato come linguaggio per la 
:ref:`standard-json interface <compiler-api>`:

::

    {
        "language": "Yul",
        "sources": { "input.yul": { "content": "{ sstore(0, 1) }" } },
        "settings": {
            "outputSelection": { "*": { "*": ["*"], "": [ "*" ] } },
            "optimizer": { "enabled": true, "details": { "yul": true } }
        }
    }

E in linea di comando con il parametro ``--strict-assembly``.

.. warning::

    Yul è attualmente in sviluppo e la generazione di bytecode è completamente implemeentata
    solamente per untyped Yul (tutti i tipi di dato sono ``u256``), con target la EVM 1.0
    e, gli :ref:`EVM opcodes <opcodes>` sono uitlizzati come funzioni built-in.

I componenti principali di Yul sono funzioni, blocchi, variabili, letterali, cicli for,
istruzioni condizionali if e swich, espressioni ed assegnamenti a variabili.

In Yul, sia le variabili che i letterali devono specificare il tipo di dato
con la notazione postfissa. 
I tipi supportati sono ``bool``, ``u8``, ``s8``, ``u32``, ``s32``,
``u64``, ``s64``, ``u128``, ``s128``, ``u256`` e ``s256``.

Yul nativamente non fornisce operatori. Nel caso in cui sia compilato per
la EVM, gli operatori sono disponibili come funzioni built-in functions,
ma possono anche essere reimplemetati. 
Per una lista di funzioni built-in obbligatorie vedere la sezione successiva.

Il codice di esempio seguente suppone che gli EVM opcode ``mul``, ``div``
e ``mod`` siano disponibili nativamente o come funzioni.
Il seguente codice non ha tipi di dato e può essere compilato usando 
``solc --strict-assembly``.

.. code::

    {
        function power(base, exponent) -> result
        {
            switch exponent
            case 0 { result := 1 }
            case 1 { result := base }
            default
            {
                result := power(mul(base, base), div(exponent, 2))
                switch mod(exponent, 2)
                    case 1 { result := mul(base, result) }
            }
        }
    }

È inoltre possibile implemeentare la stessa funzione utilizzando un for loop invece
della ricorsione. In questo caso gli opcode ``lt`` (less-than, minore di)
e ``add`` devono essere disponibili.

.. code::

    {
        function power(base, exponent) -> result
        {
            result := 1
            for { let i := 0 } lt(i, exponent) { i := add(i, 1) }
            {
                result := mul(result, base)
            }
        }
    }

Specifiche Yul
====================

Questo capitolo descrive il codice Yul. Solitamente è inserito all'interno di un 
oggetto Yul (descritto nel capitolo seguente).

Grammatica::

    Block = '{' Statement* '}'
    Statement =
        Block |
        FunctionDefinition |
        VariableDeclaration |
        Assignment |
        If |
        Expression |
        Switch |
        ForLoop |
        BreakContinue
    FunctionDefinition =
        'function' Identifier '(' TypedIdentifierList? ')'
        ( '->' TypedIdentifierList )? Block
    VariableDeclaration =
        'let' TypedIdentifierList ( ':=' Expression )?
    Assignment =
        IdentifierList ':=' Expression
    Expression =
        FunctionCall | Identifier | Literal
    If =
        'if' Expression Block
    Switch =
        'switch' Expression ( Case+ Default? | Default )
    Case =
        'case' Literal Block
    Default =
        'default' Block
    ForLoop =
        'for' Block Expression Block Block
    BreakContinue =
        'break' | 'continue'
    FunctionCall =
        Identifier '(' ( Expression ( ',' Expression )* )? ')'
    Identifier = [a-zA-Z_$] [a-zA-Z_$0-9.]*
    IdentifierList = Identifier ( ',' Identifier)*
    TypeName = Identifier | BuiltinTypeName
    BuiltinTypeName = 'bool' | [us] ( '8' | '32' | '64' | '128' | '256' )
    TypedIdentifierList = Identifier ':' TypeName ( ',' Identifier ':' TypeName )*
    Literal =
        (NumberLiteral | StringLiteral | HexLiteral | TrueLiteral | FalseLiteral) ':' TypeName
    NumberLiteral = HexNumber | DecimalNumber
    HexLiteral = 'hex' ('"' ([0-9a-fA-F]{2})* '"' | '\'' ([0-9a-fA-F]{2})* '\'')
    StringLiteral = '"' ([^"\r\n\\] | '\\' .)* '"'
    TrueLiteral = 'true'
    FalseLiteral = 'false'
    HexNumber = '0x' [0-9a-fA-F]+
    DecimalNumber = [0-9]+

Restrizioni della Grammatica
----------------------------

Gli switch devono avere almeno un caso (compreso il caso di default).
Se tutti i possbili valori di un'espressione sono coperti, il caso di default
non dovrebbe essere inserito (per esempio, uno switch con una 
espressione ``bool`` che ha entrambi i casi false e true non dovrebbe
avere il caso di default). Tutti i valori dei case devono
essere dello stesso tipo.

Ogni espressione calcola zero o più valori. Identificatori e letterali calcolano 
esattamente un valore e chiamate a funzioni calcolano un numero di valori uguali
al numero di valori di ritorno della funzione chiamata.

Nella dichiarazione e nell'assegnamento di variabili, la parte di destra (right-hand side),
se presente, deve risultare in un numero di valori uguale al numero di variabili
nella parte di sinistra (left-hand side). Questo è l'unico caso in cui un'espressione
risulta in più valori.

Espressioni che sono anche statement (a livello di blocco) 
devono risultare in zero valori.

In tutte le altre situazioni, le espressioni devono calcolare esattamente un valore.

Le istruzioni ``continue`` e ``break`` possono essere utilizzate solamente
all'interno di loop e devono essere nella stessa funzione in cui si trova il loop
(o devono essere entrambe al top level). Parte condizionale del loop deve risultare
esattamente in solo un valore. Le funzioni non possono essere definite all'interno di
un for loop.

I letterali non possono contenere un valore più grande del loro tipo.
Il tipo di dato più grande è 256 bit.

Regole di visibilità
--------------------

La visibilità è legata ai Block (eccezione fatta per le funzioni e i for loop
come spiegato sotto) e tutte le dichiarazioni (``FunctionDefinition``, 
``VariableDeclaration``) introducono nuovi identificatori all'interno dello scope.

Gli identificatori sono visibili nel blocco in cui sono definiti (inclusi tutti i 
sotto blocchi). Fanno eccezione gli identificatori definiti nella parte "init" del 
ciclo for (il primo block) che sono visibili in tutte le altre parti del ciclo 
for (ma non al di fuori del loop).
Gli identificatori dichiarati in altre parti del loop rispettano le tradizionali
regole di visibilità.
I parametri e i parametri di ritorno delle funzioni sono visibili nel corpo della
fuznione e i loro nomi devono essere univoci.

Le variabili possono essere referenziate solamente dopo la dichiarazione.
In particolare, le variabili non possono essere referenziate nella parte 
destra (right hand side) della loro stessa dichiarazione.
Le funzioni, se visibili, possono essere referenziate anche prima della 
dichiarazione.

Lo Shadowing non è permesso: non si può dichiarare un identificatore in un 
punto del codice dove un'altro identificatore con lo stesso nome è già visibile, 
anche se non acessibile.

All'interno di funzioni non è possibile accedere a variabili che sono state
dichiarate al di fuori della funzione.

Specifiche Formali
------------------
In questa sezione specifichiamo formalemente Yul fornendo la valutazione di 
una funzione E overloaded sui vari nodi dell'AST.
Ogni fuznione può avere side effect, quindi E prende in ingresso due
state object e il nodo dell'AST e restituisce due nuovi state object e un 
numero variabile di altri valori.
I due state object sono il global state object (che nel contesto della 
EVM è la memoria, lo storage e lo stato della blockchain) e il local state object  
(lo stato delle variabili locali, una parte dello stack della EVM).
Se il nodo dell'AST è uno statement, E restituisce due state object ed un "mode" che 
è usato per gli statement ``break`` e ``continue``.
Se il nodo AST è una espressione, E restituisce i due state object e tanti valori
quanti l'espressione ne calcola.

L'esatta natura dello stato globale rimane non specificata per questa descrizione
di alto livello. Lo stao locale ``L`` è una mappatura tra identificatori ``i`` e
valori ``v``, rappresentato con la scrittura ``L[i] = v``.

Sia ``$v`` il nome dell'identificatore ``v``.

Utilizzeremo una notazione di destrutturazione per i nodi dell'AST.

.. code::

    E(G, L, <{St1, ..., Stn}>: Block) =
        let G1, L1, mode = E(G, L, St1, ..., Stn)
        let L2 be a restriction of L1 to the identifiers of L
        G1, L2, mode
    E(G, L, St1, ..., Stn: Statement) =
        if n is zero:
            G, L, regular
        else:
            let G1, L1, mode = E(G, L, St1)
            if mode is regular then
                E(G1, L1, St2, ..., Stn)
            otherwise
                G1, L1, mode
    E(G, L, FunctionDefinition) =
        G, L, regular
    E(G, L, <let var1, ..., varn := rhs>: VariableDeclaration) =
        E(G, L, <var1, ..., varn := rhs>: Assignment)
    E(G, L, <let var1, ..., varn>: VariableDeclaration) =
        let L1 be a copy of L where L1[$vari] = 0 for i = 1, ..., n
        G, L1, regular
    E(G, L, <var1, ..., varn := rhs>: Assignment) =
        let G1, L1, v1, ..., vn = E(G, L, rhs)
        let L2 be a copy of L1 where L2[$vari] = vi for i = 1, ..., n
        G, L2, regular
    E(G, L, <for { i1, ..., in } condition post body>: ForLoop) =
        if n >= 1:
            let G1, L1, mode = E(G, L, i1, ..., in)
            // mode has to be regular due to the syntactic restrictions
            let G2, L2, mode = E(G1, L1, for {} condition post body)
            // mode has to be regular due to the syntactic restrictions
            let L3 be the restriction of L2 to only variables of L
            G2, L3, regular
        else:
            let G1, L1, v = E(G, L, condition)
            if v is false:
                G1, L1, regular
            else:
                let G2, L2, mode = E(G1, L, body)
                if mode is break:
                    G2, L2, regular
                else:
                    G3, L3, mode = E(G2, L2, post)
                    E(G3, L3, for {} condition post body)
    E(G, L, break: BreakContinue) =
        G, L, break
    E(G, L, continue: BreakContinue) =
        G, L, continue
    E(G, L, <if condition body>: If) =
        let G0, L0, v = E(G, L, condition)
        if v is true:
            E(G0, L0, body)
        else:
            G0, L0, regular
    E(G, L, <switch condition case l1:t1 st1 ... case ln:tn stn>: Switch) =
        E(G, L, switch condition case l1:t1 st1 ... case ln:tn stn default {})
    E(G, L, <switch condition case l1:t1 st1 ... case ln:tn stn default st'>: Switch) =
        let G0, L0, v = E(G, L, condition)
        // i = 1 .. n
        // Evaluate literals, context doesn't matter
        let _, _, v1 = E(G0, L0, l1)
        ...
        let _, _, vn = E(G0, L0, ln)
        if there exists smallest i such that vi = v:
            E(G0, L0, sti)
        else:
            E(G0, L0, st')

    E(G, L, <name>: Identifier) =
        G, L, L[$name]
    E(G, L, <fname(arg1, ..., argn)>: FunctionCall) =
        G1, L1, vn = E(G, L, argn)
        ...
        G(n-1), L(n-1), v2 = E(G(n-2), L(n-2), arg2)
        Gn, Ln, v1 = E(G(n-1), L(n-1), arg1)
        Let <function fname (param1, ..., paramn) -> ret1, ..., retm block>
        be the function of name $fname visible at the point of the call.
        Let L' be a new local state such that
        L'[$parami] = vi and L'[$reti] = 0 for all i.
        Let G'', L'', mode = E(Gn, L', block)
        G'', Ln, L''[$ret1], ..., L''[$retm]
    E(G, L, l: HexLiteral) = G, L, hexString(l),
        where hexString decodes l from hex and left-aligns it into 32 bytes
    E(G, L, l: StringLiteral) = G, L, utf8EncodeLeftAligned(l),
        where utf8EncodeLeftAligned performs a utf8 encoding of l
        and aligns it left into 32 bytes
    E(G, L, n: HexNumber) = G, L, hex(n)
        where hex is the hexadecimal decoding function
    E(G, L, n: DecimalNumber) = G, L, dec(n),
        where dec is the decimal decoding function

Funzioni di Conversione tra Tipi
--------------------------------

Yul non fornisce il supporto per la conversione implicita tra tipi. Esistono
però alcune funzioni per permettere la conversione esplicita. Durante la conversione
tra tipi, possono presentarsi delle runtime exception in caso di overflow.

Le coonversioni sono supportate tra i seguenti tipi di dato:
 - ``bool``
 - ``u32``
 - ``u64``
 - ``u256``
 - ``s256``

Per ognuno di questi, esiste una funzione di conversione con il prototipo della forma
``<input_type>to<output_type>(x:<input_type>) -> y:<output_type>``,
come per esempio ``u32tobool(x:u32) -> y:bool``, ``u256tou32(x:u256) -> y:u32`` o 
``s256tou256(x:s256) -> y:u256``.

.. note::

    ``u32tobool(x:u32) -> y:bool`` può essere implemeentata come ``y := not(iszerou256(x))`` and
    ``booltou32(x:bool) -> y:u32`` può essere implemeentata come ``switch x case true:bool { y := 1:u32 } case false:bool { y := 0:u32 }``

Funzioni di Basso Livello
-------------------------

Le seguenti funzioni devono essere disponibili:

+---------------------------------------------------------------------------------------------------------------------------+
| *Logica*                                                                                                                  |
+---------------------------------------------+-----------------------------------------------------------------------------+
| not(x:bool) ‑> z:bool                       | not logico                                                                  |
+---------------------------------------------+-----------------------------------------------------------------------------+
| and(x:bool, y:bool) ‑> z:bool               | and logico                                                                  |
+---------------------------------------------+-----------------------------------------------------------------------------+
| or(x:bool, y:bool) ‑> z:bool                | or logico                                                                   |
+---------------------------------------------+-----------------------------------------------------------------------------+
| xor(x:bool, y:bool) ‑> z:bool               | xor                                                                         |
+---------------------------------------------+-----------------------------------------------------------------------------+
| *Aritmetica*                                                                                                              |
+---------------------------------------------+-----------------------------------------------------------------------------+
| addu256(x:u256, y:u256) ‑> z:u256           | x + y                                                                       |
+---------------------------------------------+-----------------------------------------------------------------------------+
| subu256(x:u256, y:u256) ‑> z:u256           | x - y                                                                       |
+---------------------------------------------+-----------------------------------------------------------------------------+
| mulu256(x:u256, y:u256) ‑> z:u256           | x * y                                                                       |
+---------------------------------------------+-----------------------------------------------------------------------------+
| divu256(x:u256, y:u256) ‑> z:u256           | x / y                                                                       |
+---------------------------------------------+-----------------------------------------------------------------------------+
| divs256(x:s256, y:s256) ‑> z:s256           | x / y, per numeri con segno in complemento a due                            |
+---------------------------------------------+-----------------------------------------------------------------------------+
| modu256(x:u256, y:u256) ‑> z:u256           | x % y                                                                       |
+---------------------------------------------+-----------------------------------------------------------------------------+
| mods256(x:s256, y:s256) ‑> z:s256           | x % y, per numeri con segno in complemento a due                            |
+---------------------------------------------+-----------------------------------------------------------------------------+
| signextendu256(i:u256, x:u256) ‑> z:u256    | estensione di segno dal (i*8+7)-esimo bit partendo dal meno significativo   |
+---------------------------------------------+-----------------------------------------------------------------------------+
| expu256(x:u256, y:u256) ‑> z:u256           | x alla y                                                                    |
+---------------------------------------------+-----------------------------------------------------------------------------+
| addmodu256(x:u256, y:u256, m:u256) ‑> z:u256| (x + y) % m con aritmetica di precisione arbitraria                         |
+---------------------------------------------+-----------------------------------------------------------------------------+
| mulmodu256(x:u256, y:u256, m:u256) ‑> z:u256| (x * y) % m con aritmetica di precisione arbitraria                         |
+---------------------------------------------+-----------------------------------------------------------------------------+
| ltu256(x:u256, y:u256) ‑> z:bool            | true se x < y, false altrimenti                                             |
+---------------------------------------------+-----------------------------------------------------------------------------+
| gtu256(x:u256, y:u256) ‑> z:bool            | true se x > y, false altrimenti                                             |
+---------------------------------------------+-----------------------------------------------------------------------------+
| lts256(x:s256, y:s256) ‑> z:bool            | true se x < y, false altrimenti                                             |
|                                             | (per numeri con segno in complemento a due)                                 |
+---------------------------------------------+-----------------------------------------------------------------------------+
| gts256(x:s256, y:s256) ‑> z:bool            | true se x > y, false altrimenti                                             |
|                                             | (per numeri con segno in complemento a due)                                 |
+---------------------------------------------+-----------------------------------------------------------------------------+
| equ256(x:u256, y:u256) ‑> z:bool            | true se x == y, false altrimenti                                            |
+---------------------------------------------+-----------------------------------------------------------------------------+
| iszerou256(x:u256) ‑> z:bool                | true se x == 0, false altrimenti                                            |
+---------------------------------------------+-----------------------------------------------------------------------------+
| notu256(x:u256) ‑> z:u256                   | ~x, ogni bit di x è negato                                                  |
+---------------------------------------------+-----------------------------------------------------------------------------+
| andu256(x:u256, y:u256) ‑> z:u256           | bitwise and di x e y                                                        |
+---------------------------------------------+-----------------------------------------------------------------------------+
| oru256(x:u256, y:u256) ‑> z:u256            | bitwise or di x e y                                                         |
+---------------------------------------------+-----------------------------------------------------------------------------+
| xoru256(x:u256, y:u256) ‑> z:u256           | bitwise xor di x e y                                                        |
+---------------------------------------------+-----------------------------------------------------------------------------+
| shlu256(x:u256, y:u256) ‑> z:u256           | shift a sinistra logico di x di y                                           |
+---------------------------------------------+-----------------------------------------------------------------------------+
| shru256(x:u256, y:u256) ‑> z:u256           | shift a destra logico di x di y                                             |
+---------------------------------------------+-----------------------------------------------------------------------------+
| sars256(x:s256, y:u256) ‑> z:u256           | shift a destra aritmetico di x di y                                         |
+---------------------------------------------+-----------------------------------------------------------------------------+
| byte(n:u256, x:u256) ‑> v:u256              | n-esimo byte di x, dove il byte più significativo è il byte 0.              |
|                                             |                                                                             |
+---------------------------------------------+-----------------------------------------------------------------------------+
| *Memoria e storage*                                                                                                       |
+---------------------------------------------+-----------------------------------------------------------------------------+
| mload(p:u256) ‑> v:u256                     | mem[p..(p+32))                                                              |
+---------------------------------------------+-----------------------------------------------------------------------------+
| mstore(p:u256, v:u256)                      | mem[p..(p+32)) := v                                                         |
+---------------------------------------------+-----------------------------------------------------------------------------+
| mstore8(p:u256, v:u256)                     | mem[p] := v & 0xff    - only modifies a single byte                         |
+---------------------------------------------+-----------------------------------------------------------------------------+
| sload(p:u256) ‑> v:u256                     | storage[p]                                                                  |
+---------------------------------------------+-----------------------------------------------------------------------------+
| sstore(p:u256, v:u256)                      | storage[p] := v                                                             |
+---------------------------------------------+-----------------------------------------------------------------------------+
| msize() ‑> size:u256                        | size of memory, i.e. largest accessed memory index, albeit due              |
|                                             | due to the memory extension function, which extends by words,               |
|                                             | this will always be a multiple of 32 bytes                                  |
+---------------------------------------------+-----------------------------------------------------------------------------+
| *Execution control*                                                                                                       |
+---------------------------------------------+-----------------------------------------------------------------------------+
| create(v:u256, p:u256, n:u256)              | create new contract with code mem[p..(p+n)) and send v wei                  |
|                                             | and return the new address                                                  |
+---------------------------------------------+-----------------------------------------------------------------------------+
| create2(v:u256, p:u256, n:u256, s:u256)     | create new contract with code mem[p...(p+n)) at address                     |
|                                             | keccak256(0xff . this . s . keccak256(mem[p...(p+n)))                       |
|                                             | and send v wei and return the new address, where ``0xff`` is a              |
|                                             | 8 byte value, ``this`` is the current contract's address                    |
|                                             | as a 20 byte value and ``s`` is a big-endian 256-bit value                  |
+---------------------------------------------+-----------------------------------------------------------------------------+
| call(g:u256, a:u256, v:u256, in:u256,       | call contract at address a with input mem[in..(in+insize))                  |
| insize:u256, out:u256,                      | providing g gas and v wei and output area                                   |
| outsize:u256)                               | mem[out..(out+outsize)) returning 0 on error (eg. out of gas)               |
| ‑> r:u256                                   | and 1 on success                                                            |
+---------------------------------------------+-----------------------------------------------------------------------------+
| callcode(g:u256, a:u256, v:u256, in:u256,   | identical to ``call`` but only use the code from a                          |
| insize:u256, out:u256,                      | and stay in the context of the                                              |
| outsize:u256) ‑> r:u256                     | current contract otherwise                                                  |
+---------------------------------------------+-----------------------------------------------------------------------------+
| delegatecall(g:u256, a:u256, in:u256,       | identical to ``callcode``,                                                  |
| insize:u256, out:u256,                      | but also keep ``caller``                                                    |
| outsize:u256) ‑> r:u256                     | and ``callvalue``                                                           |
+---------------------------------------------+-----------------------------------------------------------------------------+
| abort()                                     | abort (equals to invalid instruction on EVM)                                |
+---------------------------------------------+-----------------------------------------------------------------------------+
| return(p:u256, s:u256)                      | end execution, return data mem[p..(p+s))                                    |
+---------------------------------------------+-----------------------------------------------------------------------------+
| revert(p:u256, s:u256)                      | end execution, revert state changes, return data mem[p..(p+s))              |
+---------------------------------------------+-----------------------------------------------------------------------------+
| selfdestruct(a:u256)                        | end execution, destroy current contract and send funds to a                 |
+---------------------------------------------+-----------------------------------------------------------------------------+
| log0(p:u256, s:u256)                        | log without topics and data mem[p..(p+s))                                   |
+---------------------------------------------+-----------------------------------------------------------------------------+
| log1(p:u256, s:u256, t1:u256)               | log with topic t1 and data mem[p..(p+s))                                    |
+---------------------------------------------+-----------------------------------------------------------------------------+
| log2(p:u256, s:u256, t1:u256, t2:u256)      | log with topics t1, t2 and data mem[p..(p+s))                               |
+---------------------------------------------+-----------------------------------------------------------------------------+
| log3(p:u256, s:u256, t1:u256, t2:u256,      | log with topics t, t2, t3 and data mem[p..(p+s))                            |
| t3:u256)                                    |                                                                             |
+---------------------------------------------+-----------------------------------------------------------------------------+
| log4(p:u256, s:u256, t1:u256, t2:u256,      | log with topics t1, t2, t3, t4 and data mem[p..(p+s))                       |
| t3:u256, t4:u256)                           |                                                                             |
+---------------------------------------------+-----------------------------------------------------------------------------+
| *State queries*                                                                                                           |
+---------------------------------------------+-----------------------------------------------------------------------------+
| blockcoinbase() ‑> address:u256             | current mining beneficiary                                                  |
+---------------------------------------------+-----------------------------------------------------------------------------+
| blockdifficulty() ‑> difficulty:u256        | difficulty of the current block                                             |
+---------------------------------------------+-----------------------------------------------------------------------------+
| blockgaslimit() ‑> limit:u256               | block gas limit of the current block                                        |
+---------------------------------------------+-----------------------------------------------------------------------------+
| blockhash(b:u256) ‑> hash:u256              | hash of block nr b - only for last 256 blocks excluding current             |
+---------------------------------------------+-----------------------------------------------------------------------------+
| blocknumber() ‑> block:u256                 | current block number                                                        |
+---------------------------------------------+-----------------------------------------------------------------------------+
| blocktimestamp() ‑> timestamp:u256          | timestamp of the current block in seconds since the epoch                   |
+---------------------------------------------+-----------------------------------------------------------------------------+
| txorigin() ‑> address:u256                  | transaction sender                                                          |
+---------------------------------------------+-----------------------------------------------------------------------------+
| txgasprice() ‑> price:u256                  | gas price of the transaction                                                |
+---------------------------------------------+-----------------------------------------------------------------------------+
| gasleft() ‑> gas:u256                       | gas still available to execution                                            |
+---------------------------------------------+-----------------------------------------------------------------------------+
| balance(a:u256) ‑> v:u256                   | wei balance at address a                                                    |
+---------------------------------------------+-----------------------------------------------------------------------------+
| this() ‑> address:u256                      | address of the current contract / execution context                         |
+---------------------------------------------+-----------------------------------------------------------------------------+
| caller() ‑> address:u256                    | call sender (excluding delegatecall)                                        |
+---------------------------------------------+-----------------------------------------------------------------------------+
| callvalue() ‑> v:u256                       | wei sent together with the current call                                     |
+---------------------------------------------+-----------------------------------------------------------------------------+
| calldataload(p:u256) ‑> v:u256              | call data starting from position p (32 bytes)                               |
+---------------------------------------------+-----------------------------------------------------------------------------+
| calldatasize() ‑> v:u256                    | size of call data in bytes                                                  |
+---------------------------------------------+-----------------------------------------------------------------------------+
| calldatacopy(t:u256, f:u256, s:u256)        | copy s bytes from calldata at position f to mem at position t               |
+---------------------------------------------+-----------------------------------------------------------------------------+
| codesize() ‑> size:u256                     | size of the code of the current contract / execution context                |
+---------------------------------------------+-----------------------------------------------------------------------------+
| codecopy(t:u256, f:u256, s:u256)            | copy s bytes from code at position f to mem at position t                   |
+---------------------------------------------+-----------------------------------------------------------------------------+
| extcodesize(a:u256) ‑> size:u256            | size of the code at address a                                               |
+---------------------------------------------+-----------------------------------------------------------------------------+
| extcodecopy(a:u256, t:u256, f:u256, s:u256) | like codecopy(t, f, s) but take code at address a                           |
+---------------------------------------------+-----------------------------------------------------------------------------+
| extcodehash(a:u256)                         | code hash of address a                                                      |
+---------------------------------------------+-----------------------------------------------------------------------------+
| *Others*                                                                                                                  |
+---------------------------------------------+-----------------------------------------------------------------------------+
| discard(unused:bool)                        | discard value                                                               |
+---------------------------------------------+-----------------------------------------------------------------------------+
| discardu256(unused:u256)                    | discard value                                                               |
+---------------------------------------------+-----------------------------------------------------------------------------+
| splitu256tou64(x:u256) ‑> (x1:u64, x2:u64,  | split u256 to four u64's                                                    |
| x3:u64, x4:u64)                             |                                                                             |
+---------------------------------------------+-----------------------------------------------------------------------------+
| combineu64tou256(x1:u64, x2:u64, x3:u64,    | combine four u64's into a single u256                                       |
| x4:u64) ‑> (x:u256)                         |                                                                             |
+---------------------------------------------+-----------------------------------------------------------------------------+
| keccak256(p:u256, s:u256) ‑> v:u256         | keccak(mem[p...(p+s)))                                                      |
+---------------------------------------------+-----------------------------------------------------------------------------+
| *Object access*                             |                                                                             |
+---------------------------------------------+-----------------------------------------------------------------------------+
| datasize(name:string) ‑> size:u256          | size of the data object in bytes, name has to be string literal             |
+---------------------------------------------+-----------------------------------------------------------------------------+
| dataoffset(name:string) ‑> offset:u256      | offset of the data object inside the data area in bytes,                    |
|                                             | name has to be string literal                                               |
+---------------------------------------------+-----------------------------------------------------------------------------+
| datacopy(dst:u256, src:u256, len:u256)      | copy len bytes from the data area starting at offset src bytes              |
|                                             | to memory at position dst                                                   |
+---------------------------------------------+-----------------------------------------------------------------------------+

Backend
-------

Backend o target sono convertitori tra Yul ed uno specifico bytecode. 
Ogni backend può mettere a disposizione funzioni che hanno come prefisso
il nome del backend. I prefissi ``evm_`` e ``ewasm_`` sono riservati per
i due backend proposti.

Backend: EVM
------------
Il target EVM ha tutti gli EVM opcodes esposti con il prefisso `evm_`.

Backend: "EVM 1.5"
------------------

TBD

Backend: eWASM
--------------

TBD

Specifiche di Oggetti Yul
=========================
Gli oggetti yul sono utilizzati per raggruppare codice e sezioni dati.
Le funzioni ``datasize``, ``dataoffset`` e ``datacopy``
possono essere utilizzate per accedere dall'interno 
del codice a queste sezioni.
Stringhe esadecimali possono essere utilizzate per specificare dati in 
formato esadecimale, stringhe normali con l'encoding nativo. 
``datacopy`` accede alla rappresentazione binaria del codice.

Grammatica::

    Object = 'object' StringLiteral '{' Code ( Object | Data )* '}'
    Code = 'code' Block
    Data = 'data' StringLiteral ( HexLiteral | StringLiteral )
    HexLiteral = 'hex' ('"' ([0-9a-fA-F]{2})* '"' | '\'' ([0-9a-fA-F]{2})* '\'')
    StringLiteral = '"' ([^"\r\n\\] | '\\' .)* '"'

Nel riquadro sopra, ``Block`` si riferisce a ``Block`` nella grammatica Yul 
descritta precedentemente.

Un esempio di oggetto Yul è il seguente:

.. code::

    // Code consiste di un singolo oggetto. Un singolo nodo "code" è il code dell'oggetto.
    // Ogni altro oggetto o sezione dati è serializzata e resa accessibile
    // alle speciali funzioni built-in datacopy / dataoffset / datasize.
    // L'accesso ad oggetti innestati può essere effettuato unendo i nomi utilizzando ``.``.
    // L'oggetto corrente, i sotto oggetti e i dati all'interno dell'oggetto corrente
    // sono già all'interno dello scope senza accesso innestato.
    object "Contract1" {
        code {
            function allocate(size) -> ptr {
                ptr := mload(0x40)
                if iszero(ptr) { ptr := 0x60 }
                mstore(0x40, add(ptr, size))
            }

            // first create "runtime.Contract2"
            let size := datasize("runtime.Contract2")
            let offset := allocate(size)
            // This will turn into a memory->memory copy for eWASM and
            // a codecopy for EVM
            datacopy(offset, dataoffset("runtime.Contract2"), size)
            // constructor parameter is a single number 0x1234
            mstore(add(offset, size), 0x1234)
            pop(create(offset, add(size, 32), 0))

            // now return the runtime object (this is
            // constructor code)
            size := datasize("runtime")
            offset := allocate(size)
            // This will turn into a memory->memory copy for eWASM and
            // a codecopy for EVM
            datacopy(offset, dataoffset("runtime"), size)
            return(offset, size)
        }

        data "Table2" hex"4123"

        object "runtime" {
            code {
                function allocate(size) -> ptr {
                    ptr := mload(0x40)
                    if iszero(ptr) { ptr := 0x60 }
                    mstore(0x40, add(ptr, size))
                }

                // runtime code

                let size := datasize("Contract2")
                let offset := allocate(size)
                // This will turn into a memory->memory copy for eWASM and
                // a codecopy for EVM
                datacopy(offset, dataoffset("Contract2"), size)
                // constructor parameter is a single number 0x1234
                mstore(add(offset, size), 0x1234)
                pop(create(offset, add(size, 32), 0))
            }

            // Embedded object. Use case is that the outside is a factory contract,
            // and Contract2 is the code to be created by the factory
            object "Contract2" {
                code {
                    // code here ...
                }

                object "runtime" {
                    code {
                        // code here ...
                    }
                }

                data "Table1" hex"4123"
            }
        }
    }
