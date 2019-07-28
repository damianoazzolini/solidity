.. index:: ! event

.. _events:

******
Eventi
******

Gli eventi Solidity offrono un'astrazione in aggiunta alla funzionalità di logging di EVM. 
Le applicazioni possono iscriversi ed ascoltare questi eventi tramite una interfaccia 
RPC di un client Ethereum.

Gli eventi sono membri ereditabili di un contratto.

Quando li chiami, causano l'archiviazione degli argomenti nel registro delle transazioni 
(transaction's log), una struttura di dati speciale nella blockchain.
Questi registri sono associati all'indirizzo del contratto,
sono incorporati nella blockchain e rimangono lì fino a quando è un blocco
accessibile (per sempre dalle versioni Frontier e Homestead, ma questo potrebbe 
cambiare con Serenity). 
Il registro e i relativi dati dell'evento non sono accessibili dall'interno
contratti (nemmeno dal contratto che li ha creati).

È possibile richiedere una semplice verifica del pagamento (SPV) per i log, 
quindi se un'entità esterna fornisce ad un contratto tale verifica, 
può verificare che il log esista effettivamente all'interno della blockchain. 
Devi essere fornite le intestazioni di blocco (block header) perché il contratto 
può vedere solo gli ultimi 256 hash di un blocco.

Può essere inoltre aggiunto l'attributo ``indexed`` che supporta
fino a tre parametri, che aggiunge le informazioni
in una struttura dati speciale nota come :ref:`"topics" <abi_events>` 
invece che nella parte dati del log. 
Se vengono utlizzati array (compresi ``string`` e ``bytes``)
come indexed arguments, il suo Keccak-256 hash è salvato come topic, 
questo perché un topic può contenere al solo una sigola word (32 byte).

Tutti i parametri senza l'attributo ``indexed`` sono :ref:`ABI-encoded <ABI>`
all'interno della parte dati del log.


Gli argomenti consentono di cercare eventi, ad esempio quando si filtra una 
sequenza di blocchi per determinati eventi. Puoi anche filtrare gli eventi in 
base all'indirizzo del contratto che ha emesso l'evento.

Per esempio, il codice sottostante usa il web3.js ``subscribe("logs")``
`method <https://web3js.readthedocs.io/en/1.0/web3-eth-subscribe.html#subscribe-logs>`_ 
per filtrare log che riguardano un argomento con un certo indirizzo:

.. code-block:: javascript

    var options = {
        fromBlock: 0,
        address: web3.eth.defaultAccount,
        topics: ["0x0000000000000000000000000000000000000000000000000000000000000000", null, null]
    };
    web3.eth.subscribe('logs', options, function (error, result) {
        if (!error)
            console.log(result);
    })
        .on("data", function (log) {
            console.log(log);
        })
        .on("changed", function (log) {
    });


L'hash della firma dell'evento è uno degli argomenti, 
tranne l'evento è dichiarato con lo specificatore ``anonymous``. 
Ciò significa che non è possibile filtrare eventi anonimi per nome, ma
è possibile filtrare solo per l'indirizzo del contratto.
Il vantaggio di eventi anonimi è che sono più economici da caricare e chiamare.

::

    pragma solidity >=0.4.21 <0.7.0;

    contract ClientReceipt {
        event Deposit(
            address indexed _from,
            bytes32 indexed _id,
            uint _value
        );

        function deposit(bytes32 _id) public payable {
            // Gli eventi vengono emessi con la parola chiave `emit`, seguita dal nome
            // dell'evento e dagli argomenti tra parentesi (se ci sono).
            // Ogni invocazione di questo tipo
            // (anche innestata) viene rilevata dalle 
            // API JavaScript filtrando `Deposit`.
            emit Deposit(msg.sender, _id, msg.value);
        }
    }

L'utilizzo con le JavaScript API è il seguente:

::

    var abi = /* Abi come generato dal compilatore */;
    var ClientReceipt = web3.eth.contract(abi);
    var clientReceipt = ClientReceipt.at("0x1234...ab67" /* address */);

    var event = clientReceipt.Deposit();

    // Controllo di eventiali cambiamenti
    event.watch(function(error, result){
        // Il risultato contiene argomenti non indicizzati e topics
        // forniti alla chiamata `Deposit`.
        if (!error)
            console.log(result);
    });


    // Callback per iniziare il controllo immediatamente
    var event = clientReceipt.Deposit(function(error, result) {
        if (!error)
            console.log(result);
    });

L'output del comando precedente è il seguente (troncato):

.. code-block:: json

  {
     "returnValues": {
         "_from": "0x1111…FFFFCCCC",
         "_id": "0x50…sd5adb20",
         "_value": "0x420042"
     },
     "raw": {
         "data": "0x7f…91385",
         "topics": ["0xfd4…b4ead7", "0x7f…1a91385"]
     }
  }

.. index:: ! log

Interfaccia di Basso Livello ai Log
===================================

È anche possibile accedere all'interfaccia di basso livello del meccanismo
di logging attraverso le funzioni ``log0``, ``log1``, ``log2``, ``log3`` e ``log4``.
``logi`` accetta ``i + 1`` parametri di tipo ``bytes32``, dove il primo argomento
viene utilizzato per la parte di dati del log ed i rimanenti come topic. 
La chiamata all'evento sopra può essere eseguita allo stesso modo con

::

    pragma solidity >=0.4.10 <0.7.0;

    contract C {
        function f() public payable {
            uint256 _id = 0x420042;
            log3(
                bytes32(msg.value),
                bytes32(0x50cb9fe53daa9737b786ab3646f04d0150dc50ef4e75f59509d83667ad5adb20),
                bytes32(uint256(msg.sender)),
                bytes32(_id)
            );
        }
    }

dove il numero esadecimale è uguale a 
``keccak256("Deposit(address,bytes32,uint256)")``, la signature dell'evento.

Risorse Aggiuntive per Approfondire gli Eventi
==============================================

- `Documentazione Javascript <https://github.com/ethereum/wiki/wiki/JavaScript-API#contract-events>`_
- `Esempio di utilizzo di eventi <https://github.com/debris/smart-exchange/blob/master/lib/contracts/SmartExchange.sol>`_
- `COme accedere agli eventi utilizzando js <https://github.com/debris/smart-exchange/blob/master/lib/exchange_transactions.js>`_
