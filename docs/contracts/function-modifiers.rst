.. index:: ! function;modifier

.. _modifiers:

************************
Modificatori di Funzioni
************************

I modificatori possono essere utilizzati per modificare facilmente il comportamento delle funzioni.
Ad esempio, possono verificare automaticamente una condizione prima di eseguire la funzione.
I modificatori sono proprietà ereditabili dei contratti e possono essere sovrascritti da contratti derivati.

::

    pragma solidity >=0.5.0 <0.7.0;

    contract owned {
        constructor() public { owner = msg.sender; }
        address payable owner;

        // Questo contratto definisce un modificatore ma non lo utilizza:
        // verrà utilizzato nei contratti derivati.
        // Il corpo della funzione viene inserito al posto del simbolo
        // `_;` nella definizione del modificatore.
        // Questo significa che se il proprietario chiama questa funzione,
        // la funzione viene eseguita altrimenti viene lanciata una eccezione.
        modifier onlyOwner {
            require(
                msg.sender == owner,
                "Only owner can call this function."
            );
            _;
        }
    }

    contract mortal is owned {
        // Questo contratto eredita il modificatore `onlyOwner` da
        // `owned` e lo applica alla funzione `close`, che fa sì che
        // una chiamata a `close` abbia effetto solamente se fatta dal
        // proprietario.
        function close() public onlyOwner {
            selfdestruct(owner);
        }
    }

    contract priced {
        // I modificatori possono avere argomenti
        modifier costs(uint price) {
            if (msg.value >= price) {
                _;
            }
        }
    }

    contract Register is priced, owned {
        mapping (address => bool) registeredAddresses;
        uint price;

        constructor(uint initialPrice) public { price = initialPrice; }

        // è importante anche inserire la keyword
        // `payable`, altrimenti la funzione rigetterà automaticamente
        // automatically tutto l'Ether inviato.
        function register() public payable costs(price) {
            registeredAddresses[msg.sender] = true;
        }

        function changePrice(uint _price) public onlyOwner {
            price = _price;
        }
    }

    contract Mutex {
        bool locked;
        modifier noReentrancy() {
            require(
                !locked,
                "Reentrant call."
            );
            locked = true;
            _;
            locked = false;
        }

        /// Questa funzione è protetta da un mutex, il che significa che 
        /// chiamate reentranti a `msg.sender.call` non possono chiamare `f` nuovamente.
        /// Lo statement `return 7` assegna il valore 7 al valore di ritorno ma
        /// esegue comunque lo statement `locked = false` nel modificatore.
        function f() public noReentrancy returns (uint) {
            (bool success,) = msg.sender.call("");
            require(success);
            return 7;
        }
    }


Più modificatori vengono applicati a una funzione specificandoli in un elenco 
separato da spazi bianchi e vengono valutati nell'ordine presentato.

.. warning::
    Nelle versioni iniziali di Solidity, i ``return`` statement nelle funzioni
    possono far agire i modificatori in maniera diversa.

I return espliciti da un modificatore o da un corpo di funzione lasciano solo il 
modificatore o il corpo di funzione corrente. 
Le variabili di ritorno vengono assegnate e il flusso di controllo continua 
dopo "_" nel modificatore precedente.

Sono consentite espressioni arbitrarie per gli argomenti del modificatore e 
in questo contesto, tutti i simboli visibili dalla funzione sono visibili nel modificatore. 
I simboli introdotti nel modificatore non sono visibili nella funzione 
(poiché potrebbero cambiare per overriding).