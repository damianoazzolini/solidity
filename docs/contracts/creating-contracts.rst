.. index:: ! contract;creation, constructor

**********************
Creazione di Contratti
**********************

I contratti possono essere creati "dall'esterno" attraverso transazioni Ethereum 
o attraverso contratti Solidity.

IDE come `Remix <https://remix.ethereum.org/>`_, facilitano il processo di creazione 
attraverso l'utilizzo di elementi UI.

La creazione di contratti in maniera programmatica se Ethereum
è eseguita attraverso le JavaScript API `web3.js <https://github.com/ethereum/web3.js>`_.
Offrono una funzione chiamata 
`web3.eth.Contract <https://web3js.readthedocs.io/en/1.0/web3-eth-contract.html#new-contract>`_
per facilitare la creazione di un contratto.

Quando un contratto viene creato, la funzione :ref:`constructor <constructor>` 
(funzione dichiarata con la keyword ``constructor``) viene eseguita una sola volta.

Un costruttore è facoltativo. È consentito un solo costruttore, il che significa
l'overloading non è supportato.

Dopo l'esecuzione del costruttore, il codice finale del contratto viene caricato nella
blockchain. Questo codice include tutte le funzioni pubbliche ed esterne e tutte le funzioni
che sono raggiungibili tramite chiamate di funzione. Il codice caricato non
include il codice del costruttore o le funzioni interne chiamate solo dal costruttore.

.. index:: constructor;arguments

Internamente, gli argomenti del costruttore vengono passati :ref:`ABI encoded <ABI>`
dopo il codice del contratto stesso, ma bisogna preoccuparsene se viene utilizzato `` web3.js``.

Se un contratto vuole creare un altro contratto, il codice sorgente
(e binario) del contratto creato deve essere noto al creatore.
Ciò significa che la creazione di dipendenze cicliche sono impossibili.

::

    pragma solidity >=0.4.22 <0.7.0;


    contract OwnedToken {
        // `TokenCreator` è un tipo di contratto definito sotto.
        // è possibile referenziarlo se non viene utilizzato per creare
        // un nuovo contratto.
        TokenCreator creator;
        address owner;
        bytes32 name;

        // Costruttore che registra il creatore e 
        // il nome assegnatogli.
        constructor(bytes32 _name) public {
            // Le variabili di stato possono essere accedute attraverso il loro nome 
            // e non con, per esempio, `this.owner`. Le funzioni possono essere accedute
            // direttamente o attraverso `this.f`,
            // ma il secondo metodo offre una external view
            // della funzione. Specialmente nel construttore,
            // le funzioni non dovrebbero essere accedute esternamente,
            // perché le funzioni non esistono ancora.
            // Vedere la prossima sezione per i dettagli.
            owner = msg.sender;

            // Conversione esplicita da `address`
            // a `TokenCreator` e si assume che il tipo di contratto 
            // chiamante è `TokenCreator`, non c'è un vero modo per 
            // controllarlo.
            creator = TokenCreator(msg.sender);
            name = _name;
        }

        function changeName(bytes32 newName) public {
            // Solo il creatore può cambiarne il nome.
            // Il confronto è possibile solamente perché 
            // i contratti possono essere convertiti esplicitamente in indirizzi.
            if (msg.sender == address(creator))
                name = newName;
        }

        function transfer(address newOwner) public {
            // Solo il proprietario corrente può trasferire i token.
            if (msg.sender != owner) return;

            // Richiediamo al contratto creatore se il trasferimento
            // può essere effettuato utilizzando una funzione del contratto 
            // `TokenCreator` definito sotto. Se la chiamata fallisce
            // (per esempio out-of-gas),
            // l'esecuzione fallisce anche qui.
            if (creator.isTokenTransferOK(owner, newOwner))
                owner = newOwner;
        }
    }


    contract TokenCreator {
        function createToken(bytes32 name)
            public
            returns (OwnedToken tokenAddress)
        {
            // Crea un nuovo contratto `Token` e ne restituisce l'indirizzo.
            // Dal lato JavaScript, il tipo di ritorno è
            // `address`, poiché è il tipo più simile disponibile nell'ABI
            return new OwnedToken(name);
        }

        function changeName(OwnedToken tokenAddress, bytes32 name) public {
            // L'external type di `tokenAddress` è
            // semplicemente `address`.
            tokenAddress.changeName(name);
        }

        // Effettua un controllo per determinare se il trasferimento al contratto
        // `OwnedToken` può essere effettuato
        function isTokenTransferOK(address currentOwner, address newOwner)
            public
            pure
            returns (bool ok)
        {
            // Controlla una condizione arbitraria per verificare se il trasferimento 
            // si può effettuare
            return keccak256(abi.encodePacked(currentOwner, newOwner))[0] == 0x7f;
        }
    }
