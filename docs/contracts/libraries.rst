.. index:: ! library, callcode, delegatecall

.. _libraries:

********
Librerie
********

Le librerie sono simili a contratti ma il loro scopo è quello di essere caricate
una sola volta ad uno specifico indirizzo e di essere riutilizzate attraverso
``DELEGATECALL`` (``CALLCODE`` fino a Homestead). 
Questo significa che se le funzioni di libreria sono chiamate, il loro
codice viene eseguito nel contesto del contratto chiamante, i.e. ``this`` 
punta al contratto chiamante e in particolare, lo storage del contratto chiamante 
può essere acceduto.
Poiché una libreria è una parte isolata del codice sorgente, può accedere alle 
variabili di stato del contratto chiamante solo se vengono fornite esplicitamente 
(non avrebbe alcun modo di nominarle altrimenti).
Le funzioni di libreria possono essere chiamate solamente in modo diretto 
(senza l'uso di ``DELEGATECALL``) se non modificano lo stato
(cioè se sono funzioni ``view`` o ``pure``), perché si assume
che le librerie siano prive di stato. In particolare, non è possibile cancellare
una libreria.

.. note::
    Fino alla versione 0.4.20 era possibile cancellare una libreria aggirando
    il type system di Solidity. A partire da quella versione, le librerie contengono
    un :ref:`meccanismo <call-protection>` che non consente la chiamata diretta 
    (senza ``DELEGATECALL``) di funzioni che modificano lo stato.

Le librerie possono essere viste come contratti base impliciti dei contratti 
che le utilizzano. Non sono esplicitamente visibili nella gerarchia dell'ereditarietà, 
ma le chiamate a funzioni di libreria assomigliano a chiamate a funzioni 
di contratti di base espliciti (``L.f()`` se ``L`` è il nome della libreria). 
Inoltre, funzioni ``internal`` di libreriasono visibili in tutti i contratti,
come se la libreria fosse con contratto base. Chiaramente, una chiamata ad una funzione
internal utilizza la convenzione per le chiamate a funzioni internal, ciò significa 
che tutti i tipi internal possono essere passati e i tipi :ref:`salvati in memoria <data-location>` 
saranno passati per riferimento e non per copia.
Per realizzare ciò nella EVM, il codice delle funzioni internal di libreria e tutte le 
funzionichiamate da esso, sono inserite nel contratto chiamante a tempo di 
compilazione. Quindi viene utilizzata una chiamata ``JUMP`` invece di ``DELEGATECALL``.

.. index:: using for, set

L'esempio seguente mostra come utilizzare le librerie (per un esempio più
avanzato su come implementare i set, vedere :ref:`l'utilizzo del for <using-for>`).

::

    pragma solidity >=0.4.22 <0.7.0;


    library Set {
        // Definiamo un nuovo tipo di dati di struttura che verrà utilizzato 
        // per conservare i dati nel contratto di chiamata.
        struct Data { mapping(uint => bool) flags; }

        // Si noti che il primo parametro è di tipo  "storage
        // reference" ae quindi solo il suo indirizzo di memoria 
        // e non il suo contenuto viene passato come parte della chiamata. 
        // Questa è una caratteristica speciale delle funzioni di libreria.
        // È idiomatico chiamare il primo parametro `self`, se la funzione 
        // può essere vista come un metodo di quell'oggetto.
        function insert(Data storage self, uint value)
            public
            returns (bool)
        {
            if (self.flags[value])
                return false; // qua
            self.flags[value] = true;
            return true;
        }

        function remove(Data storage self, uint value)
            public
            returns (bool)
        {
            if (!self.flags[value])
                return false; // non qua
            self.flags[value] = false;
            return true;
        }

        function contains(Data storage self, uint value)
            public
            view
            returns (bool)
        {
            return self.flags[value];
        }
    }


    contract C {
        Set.Data knownValues;

        function register(uint value) public {
            // Le funzioni di libreria possono essere chiamate senza una 
            // specifica istanza della libreria, dato che l'istanza
            // è il contratto corrente.
            require(Set.insert(knownValues, value));
        }
        // In questo contratto, possiamo anche accedere direttamente a ``knownValues.flags``.
    }

Chiaramente, non è necessario utilizzare questo metodo per usare le librerie: 
possono anche essere usate senza definire i tipi di dati di struct. 
Le funzioni funzionano anche senza parametri di riferimento di storage e 
possono avere più parametri di riferimento storage e in qualsiasi posizione.

Le chiamate a ``Set.contains``, ``Set.insert`` e ``Set.remove``
sono tutte compilate come chiamate (``DELEGATECALL``) ad un
contratto/libreria esterno. Se vengono utilizzate le librerie, viene effettuata
una vera chiamata a funzione esterna.
``msg.sender``, ``msg.value`` e ``this`` mantengono il loro valore nella
chiamata (prima di Homestead, a causa di ``CALLCODE``, ``msg.sender`` e
``msg.value`` avrebbe potuto cambiare).

Il seguente esempio mostra come utilizzare :ref:`i tipi salvati in memoria <data-location>` 
e le funzioni interne nelle librerie per implementare dei tipi personalizzati 
senza l'overhead di una chiamata a funzione esterna:

::

    pragma solidity >=0.4.16 <0.7.0;

    library BigInt {
        struct bigint {
            uint[] limbs;
        }

        function fromUint(uint x) internal pure returns (bigint memory r) {
            r.limbs = new uint[](1);
            r.limbs[0] = x;
        }

        function add(bigint memory _a, bigint memory _b) internal pure returns (bigint memory r) {
            r.limbs = new uint[](max(_a.limbs.length, _b.limbs.length));
            uint carry = 0;
            for (uint i = 0; i < r.limbs.length; ++i) {
                uint a = limb(_a, i);
                uint b = limb(_b, i);
                r.limbs[i] = a + b + carry;
                if (a + b < a || (a + b == uint(-1) && carry > 0))
                    carry = 1;
                else
                    carry = 0;
            }
            if (carry > 0) {
                // too bad, we have to add a limb
                uint[] memory newLimbs = new uint[](r.limbs.length + 1);
                uint i;
                for (i = 0; i < r.limbs.length; ++i)
                    newLimbs[i] = r.limbs[i];
                newLimbs[i] = carry;
                r.limbs = newLimbs;
            }
        }

        function limb(bigint memory _a, uint _limb) internal pure returns (uint) {
            return _limb < _a.limbs.length ? _a.limbs[_limb] : 0;
        }

        function max(uint a, uint b) private pure returns (uint) {
            return a > b ? a : b;
        }
    }

    contract C {
        using BigInt for BigInt.bigint;

        function f() public pure {
            BigInt.bigint memory x = BigInt.fromUint(7);
            BigInt.bigint memory y = BigInt.fromUint(uint(-1));
            BigInt.bigint memory z = x.add(y);
            assert(z.limb(1) > 0);
        }
    }

Poiché il compilatore non può sapere dove la libreria verrà caricata
questi indirizzi devono essere inseriti nel bytecode finale da un linker
(vedere :ref:`commandline-compiler` per vedere come utilizzare il compilatore
a linea di comando per il linking). Se gli indirizzi non sono dati come 
argomenti al compilatore, il codice esadecimale conterrà dei placeholders
della forma ``__Set______`` (dove ``Set`` è il nome della libreria). 
Gli indirizzi possono essere inseriti manualmente sostituendo tutti
i 40 simboli con la codifica esadecimale degli indirizzi dei contratti delle librerie.

.. note::
    Il linking manuale di librerie nel bytecode generato è sconsigliato perché
    è limitato a 36 caratteri. Si dovrebbe chiedere al compilatore per effettuare 
    il link alle librerie quando il contratto è compilato utilizzando 
    l'opzione ``--libraries`` di ``solc`` o la chiave ``libraries`` se vengono utilizzate
    le interfacce del compilatore standard JSON.

Restrizioni delle librerie rispetto ai contratti (queste possono essere cambiate 
in futuro):

- No variabili di stato
- Non possono ereditare o essere ereditate
- Non possono ricevere Ether

.. _call-protection:

Protezione di Chiamate per Librerie
===================================

Come specificato nell'introduzione, se il codice di una libreria viene eseguito
utilizzando ``CALL`` invece di ``DELEGATECALL`` o ``CALLCODE``, fallirà se non 
viene chiamata una funzione ``view`` o ``pure``.

L'EVM non fornisce un modo diretto per un contratto per rilevare se è stato 
chiamato usando ``CALL`` o no, ma un contratto può usare il codice operativo 
``ADDRESS`` per scoprire "dove" è attualmente in esecuzione. 
Il codice generato confronta questo indirizzo con l'indirizzo utilizzato in fase 
di costruzione per determinare la modalità di chiamata.

Più specificamente, il codice di runtime di una libreria inizia sempre con 
un'istruzione push, che è uno zero di 20 byte al momento della compilazione. 
Quando viene eseguito il codice caricato, questa costante viene sostituita 
in memoria dall'indirizzo corrente e questo codice modificato viene archiviato 
nel contratto. In fase di esecuzione, questo fa sì che l'indirizzo 
di caricamento sia la prima costante da inserire nello stack 
e il codice dispatcher confronta l'indirizzo corrente con questa costante 
per qualsiasi funzione non view e non pure.