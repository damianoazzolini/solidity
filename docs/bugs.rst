.. index:: Bugs

.. _known_bugs:

##################
Lista di bug noti
##################

In questa sezione si trova una lista in formato JSON con elencati alcuni dei bachi di sicurezza
del compilatore Solidity noti. 
Il file si trova nel `repository Github
<https://github.com/ethereum/solidity/blob/develop/docs/bugs.json>`_.
La lista parte dalla versione 0.3.0, i bug noti presenti nelle versioni 
precedenti non sono elencati.

È presente anche un altro file chiamato `bugs_by_version.json
<https://github.com/ethereum/solidity/blob/develop/docs/bugs_by_version.json>`_,
che può essere utilizzato per controllare se un baco affligge una specifica versione
del compilatore.

Gli strumenti di verifica di codice sorgente ed altri strumenti che interagiscono
con i contratti dovrebbero consultare questa lista considerando che:

 - È abbastanza sospettoso che un contratto sia compilato con una versione nightly
   del compilatore invece di una versione rilasciata. Questa lista non mantiene 
   traccia delle versioni non rilasciate o nightly.
 - È altrettanto sospettoso che un contratto sia compilato
   con una versione del compilatore che non è la più recente al tempo della creazione del
   contratto stesso. Per contratti creati da altri contratti, per risalire alla versione
   del compilatore bisogna risalire alla transazione che ha creato il primo contratto ed
   utilizzare quella data come data di creazione.
 - È altamente sospettoso il fatto che un contratto sia compilato con una versione
   di un compilatore che contiene un baco noto e che una versione del compilatore con quel
   baco risolto, al tempo della creazione del contratto, fosse già disponibile.

Il file JSON per i bug noti è composto da un array di oggetti, uno per ogni baco,
con le seguenti chiavi:

name
    Nome unico per il bug 
summary
    Breve descrizione del baco
description
    Descrizione dettagliata del baco
link
    URL del sito con informazioni più dettagliate
introduced
    La prima versione pubblicata del compilatore che introduce quel baco
fixed
    La prima versione del compilatore pubblicata che non contiene più il baco
publish
    La data in cui il baco è diventato noto pubblicamente, facoltativo
severity
    Severità del baco: molto bassa (very low), bassa (low), media (medium), alta (high). Tiene conto della
    rilevabilità nei test dei contratti, della probabilità che accada e del
    potenziale danno causato da un exploit.
conditions
    Condizioni che devono essere soddisfatte per far si che il bug si verifichi. 
    Le seguenti chiavi possono essere utilizzate:
    `` optimizer``, valore booleano che
    significa che l'ottimizzatore deve essere attivo per far si che il bug si verifichi.
    `` evmVersion``, una stringa che indica quali impostazioni della versione del compilatore 
    EVM scatenano il bug. La stringa può contenere operatori di confronto. 
    Ad esempio, `` "> = constantinople"`` significa che il bug
    è presente quando la versione EVM è impostata su `` constantinople`` o
    una più recente.
    Se non viene indicata nessuna condizione, si suppone che il bug sia presente.
check
    Questo campo contiene diversi controlli che segnalano se lo smart contract
    contiene il bug o no. Il primo tipo di controllo è rappresentato da espressioni regolari Javascript
    che devono essere confrontate con il codice sorgente ("source-regex"). 
    Se non c'è corrispondenza, il bug molto probabilmente
    non presente. Se c'è una corrispondenza, il bug potrebbe essere presente. 
    Per migliorare l'accuratezza, i controlli dovrebbero essere applicati 
    al codice sorgente dopo la rimozione dei commenti.
    Il secondo tipo di controllo è costituito da pattern che dovrebbero essere controllati 
    sul compact AST del programma Solidity ("ast-compact-json-path"). 
    La query di ricerca specificata è un'espressione 
    `JsonPath <https://github.com/json-path/JsonPath>` _.
    Se almeno un percorso di Solidity AST corrisponde alla query, il bug è
    probabilmente presente.

.. literalinclude:: bugs.json
   :language: js
