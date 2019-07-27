#################
Contract Metadata
#################

.. index:: metadata, contract verification

Il compilatore di Solidity genera automaticamente un file JSON che contiene 
i metadati del contratto che rappresentano informazioni sul contratto stesso. 
Questo file può essere usato per scoprire la versione del compilatore, 
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

Il file coi metadati ha il seguente formato. L'esempio è presentato in a
modo human-readable. I metadati correttamente formattati dovrebbero usare le
virgolette in maniera corretta, ridurre gli spazi al minimo ed ordinare le chiavi di 
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
        // Richiesto per Solidity: lista ordinata di remapping
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
          // Riflette le impostazioni di input json, default a false
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

Perché in futuro potranno essere supportati altri modi per recuperare il file dei metadati,
il mapping `` {"bzzr0": <Swarm hash>, "solc": <versione del compilatore>} `` è memorizzato
con encoding `CBOR <https://tools.ietf.org/html/rfc7049>`_. 
Dal momento che la mappatura potrebbe contiene più chiavi (vedi sotto) e 
l'inizio di questa codifica non è facile da trovare, la sua lunghezza viene aggiunta 
in un encoding a 2 byte big-endian. 
La versione corrente del compilatore di Solidity di solito aggiunge quanto segue
alla fine del bytecode ::

    0xa2
    0x65 'b' 'z' 'z' 'r' '0' 0x58 0x20 <32 bytes swarm hash>
    0x64 's' 'o' 'l' 'c' 0x43 <3 byte version encoding>
    0x00 0x32

Quindi, per recuperare i dati, è possibile controllare la fine del bytecode, 
cercare il pattern precedente ed utilizzare l'hash Swarm 
per recuperare il file.

Mentre le release di solc usano una codifica a 3 byte della versione come mostrato
sopra (un byte ciascuno per il numero di versione major, minor e patch), 
le prerelease utilizzano invece una stringa di versione completa che include l'hash 
del commit e la data di compilazione.

.. note::
  La mappatura CBOR può contenere anche altre chiavi, quindi è meglio
  decodificare completamente i dati invece di fare affidamento sul fatto che 
  inizi con ``0xa265``.
  Ad esempio, se vengono utilizzate alcune funzionalità sperimentali 
  che influiscono sulla generazione del codice, la mappatura conterrà 
  anche `` "experimental": true``

.. note::
  Il compilatore utilizza attualmente l'hash "swarm version 0" dei metadati,
  ma questa convenzione potrebbe cambiare in futuro. Non fare quindi affidamento 
  sulla sequenza iniziale `` 0xa2 0x65 'b' 'z' 'z' 'r' '0'``. Potrebbero essere aggiunti
  anche ulteriori dati alla struttura CBOR, quindi
  l'opzione migliore è usare un parser per CBOR.


Utilizzo per Automatic Interface Generation e NatSpec
=====================================================

I metadati vengono utilizzati nel modo seguente: un componente che desidera interagire
con un contratto (ad esempio Mist od un qualsiasi wallet) recupera il codice del contratto, 
dal quale recupera l'hash Swarm del file che viene quindi recuperato.
Quel file è decodificato da JSON in una struttura come sopra.

Il componente quindi usa l'ABI per generare automaticamente una rudimentale interfaccia
per il contratto.

Inoltre, il wallet può usare la documentazione utente NatSpec per mostrare un messaggio di
conferma ogni volta che interagisce con il contratto, insieme alla richiesta di autorizzazione
per la firma della transazione.

Per maggiori informazioni leggere :doc:`Ethereum Natural Language Specification Format (NatSpec) <natspec-format>`.

Utilizzo per Verifica del Codice Sorgente
=========================================

Per verificare la compilazione, i sorgenti possono essere recuperati da Swarm
tramite il link nel file contenente i metadati. 
Il compilatore della versione corretta (viene controllato che faccia parte 
dei compilatori "ufficiali") viene invocato su quell'input con le impostazioni specificate.
Il bytecode risultato viene confrontato con i dati della transazione di creazione 
o con i dati opcode `` CREATE``.
Questo processo verifica automaticamente i metadati poiché l'hash è parte del bytecode.
I dati in eccesso corrispondono ai dati di input del costruttore, che devono essere decodificati
secondo l'interfaccia e presentati all'utente.

Nel repository `source-verify <https://github.com/ethereum/source-verify>`_
(`npm package <https://www.npmjs.com/package/source-verify>`_) è presente un codice
di esempio che mostra questa funzionalità.