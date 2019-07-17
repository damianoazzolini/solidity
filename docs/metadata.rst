#################
Contract Metadata
#################

.. index:: metadata, contract verification

Il compilatore di Solidity genera automaticamente un file JSON che contiene 
i metadati del contratto che rappresentano informazioni sul contratto corrente. 
Questo file può essere usato per interrogare la versione del compilatore, 
i sorgeti utilizzati, l'ABI e la documentazione NatSpec
per interagire in modo più sicuro con il contratto e verificarne il codice.

Il compilatore aggiunge un hash Swarm del file coi metadati alla fine del file
bytecode (per i dettagli, vedi sotto) di ciascun contratto, in modo da poterlo 
recuperare in modo autenticato senza dover ricorrere ad una soluzione centralizzata.


Il file dei metadati deve essere pubblicato su Swarm (o su un altro servizio) 
in modo che altri possono accedervi. Si può creare il file usando il
comando `` solc --metadata`` che genera un file chiamato "ContractName_meta.json``. 
Questo file contiene i riferimenti Swarm al codice sorgente, quindi bisogna caricare 
tutti i file sorgente e il file contenente i metadati.

Il file coi metadati ha il seguente formato. L'esempio seguente è presentato in a
modo human-readable. I metadati correttamente formattati dovrebbero usare le
virgolette correttamente, ridurre gli spazi al minimo e ordinare le chiavi di 
tutti gli oggetti per arrivare ad una formattazione unica. 
I commenti non sono consentiti e vengono usati qui solo per scopi esplicativi.

.. code-block:: none

    {
      // Richiesto: la versione del formato dei metadati
      version: "1",
      // Richiesto: source code language
      language: "Solidity",
      // Richiesto: dettagli riguardanti il compilatore, i contenuti sono
      // specifici per ogni linguaggio
      compiler: {
        // Richiesto per Solidity: versione del compilatore
        version: "0.4.6+commit.2dabbdf0.Emscripten.clang",
        // Opzionale: hash dei binari del compilatore che produce questo output
        keccak256: "0x123..."
      },
      // Richiesto: file sorgenti, le key sono nomi di file
      sources:
      {
        "myFile.sol": {
          // Richiesto: hash keccak256 del file sorgente
          "keccak256": "0x123...",
          // Richiesto (a meno che "content" sia, veid sotto): URL ordinati
          // ai file sorgenti, il protocollo è arbitrario, ma un URL
          // Swarm è consigliato
          "urls": [ "bzzr://56ab..." ]
        },
        "mortal": {
          // Richiesto: hash keccak256 del file sorgente
          "keccak256": "0x234...",
          // Richiesto (a meno che non venga usato "url"): contenuto del file sorgente
          "content": "contract mortal is owned { function kill() { if (msg.sender == owner) selfdestruct(owner); } }"
        }
      },
      // Richiesto: impostazioni del compilatore
      settings:
      {
        // Richiesto per Solidity: lista ordinata di remappings
        remappings: [ ":g=/dir" ],
        // Opzionale: impostazioni di ottimizzazione. Il campo "enabled" e "runs" sono deprecati
        // e sono mantenuti solo per retrocompatibilità.
        optimizer: {
          enabled: true,
          runs: 500,
          details: {
            // peephole default a "true"
            peephole: true,
            // jumpdestRemover default a "true"
            jumpdestRemover: true,
            orderLiterals: false,
            deduplicate: false,
            cse: false,
            constantOptimizer: false,
            yul: false,
            yulDetails: {}
          }
        },
        metadata: {
          // Riflette le impostazioni di input json, defaults a false
          useLiteralContent: true
        }
        // Richiesto per Solidity: file e nome del contratto o libreria per il quale
        // questi metadata sono creati.
        compilationTarget: {
          "myFile.sol": "MyContract"
        },
        // Richiesto per Solidity: indirizzi delle librerie utilizzate
        libraries: {
          "MyLib": "0x123123..."
        }
      },
      // Richiesto: informazioni generate sul contratto.
      output:
      {
        // Richiesto: definizione ABI del contratto.
        abi: [ ... ],
        // Richiesto: documentazione utente NatSpec del contratto
        userdoc: [ ... ],
        // Richiesto: documentazione sviluppatore NatSpec del contratto
        devdoc: [ ... ],
      }
    }

.. warning::
  Poiché il bytecode del contratto risultante contiene l'hash dei metadati, qualsiasi
  cambiamento dei metadati si riflette in una modifica del bytecode. Ciò comprende
  cambiamenti in un nome di un file o percorso, e dal momento che i metadati includono 
  un hash di tutte le fonti utilizzate, anche un singolo cambiamento di uno spazio bianco 
  produce metadati diversi e bytecode diverso.

.. note::
    Notare che la definizione ABI sopra non ha un ordine fisso. Può cambiare con le versioni del compilatore.

Encoding of the Metadata Hash in the Bytecode
=============================================

Because we might support other ways to retrieve the metadata file in the future,
the mapping ``{"bzzr0": <Swarm hash>, "solc": <compiler version>}`` is stored
`CBOR <https://tools.ietf.org/html/rfc7049>`_-encoded. Since the mapping might
contain more keys (see below) and the beginning of that
encoding is not easy to find, its length is added in a two-byte big-endian
encoding. The current version of the Solidity compiler usually adds the following
to the end of the deployed bytecode::

    0xa2
    0x65 'b' 'z' 'z' 'r' '0' 0x58 0x20 <32 bytes swarm hash>
    0x64 's' 'o' 'l' 'c' 0x43 <3 byte version encoding>
    0x00 0x32

So in order to retrieve the data, the end of the deployed bytecode can be checked
to match that pattern and use the Swarm hash to retrieve the file.

Whereas release builds of solc use a 3 byte encoding of the version as shown
above (one byte each for major, minor and patch version number), prerelease builds
will instead use a complete version string including commit hash and build date.

.. note::
  The CBOR mapping can also contain other keys, so it is better to fully
  decode the data instead of relying on it starting with ``0xa265``.
  For example, if any experimental features that affect code generation
  are used, the mapping will also contain ``"experimental": true``.

.. note::
  The compiler currently uses the "swarm version 0" hash of the metadata,
  but this might change in the future, so do not rely on this sequence
  to start with ``0xa2 0x65 'b' 'z' 'z' 'r' '0'``. We might also
  add additional data to this CBOR structure, so the
  best option is to use a proper CBOR parser.


Usage for Automatic Interface Generation and NatSpec
====================================================

The metadata is used in the following way: A component that wants to interact
with a contract (e.g. Mist or any wallet) retrieves the code of the contract, from that
the Swarm hash of a file which is then retrieved.
That file is JSON-decoded into a structure like above.

The component can then use the ABI to automatically generate a rudimentary
user interface for the contract.

Furthermore, the wallet can use the NatSpec user documentation to display a confirmation message to the user
whenever they interact with the contract, together with requesting
authorization for the transaction signature.

For additional information, read :doc:`Ethereum Natural Language Specification (NatSpec) format <natspec-format>`.

Usage for Source Code Verification
==================================

In order to verify the compilation, sources can be retrieved from Swarm
via the link in the metadata file.
The compiler of the correct version (which is checked to be part of the "official" compilers)
is invoked on that input with the specified settings. The resulting
bytecode is compared to the data of the creation transaction or ``CREATE`` opcode data.
This automatically verifies the metadata since its hash is part of the bytecode.
Excess data corresponds to the constructor input data, which should be decoded
according to the interface and presented to the user.

In the repository `source-verify <https://github.com/ethereum/source-verify>`_
(`npm package <https://www.npmjs.com/package/source-verify>`_) you can see
example code that shows how to use this feature.
