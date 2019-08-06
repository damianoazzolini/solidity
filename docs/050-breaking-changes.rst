************************************************
Solidity v0.5.0 Cambiamenti non Retrocompatibili
************************************************

Questa sezione evidenzia i principali cambiamenti non retrocompatibili introdotti
nella versione Solidity 0.5.0, assieme alle motivazioni e alla spiegazione su come 
aggiornare il codice. Per una lista completa consultare:
`il log dei cambiamenti nelle release <https://github.com/ethereum/solidity/releases/tag/v0.5.0>`_.

.. note::
   I contratti compilati con Solidity v0.5.0 possono ancora interfacciarsi con 
   contratti e persino librerie compilate con versioni precedenti senza bisogno di ricompilarle 
   o ridistribuirle. È sufficiente modificare le interfacce per includere la posizione 
   dei dati e gli specificatori di visibilità e mutabilità. Vedere la sezione sottostante
   :ref:`Interoperabilità con Contratti Precedenti <interoperability>`.

Cambiamenti Semantici
=====================


Questa sezione elenca le modifiche che sono solo semantiche, nascondendo quindi potenzialmente 
comportamenti nuovi e diversi nel codice esistente.

* Lo shift a destra con segno ora usa un vero shift aritmetico, i.e. arrotondamento 
  a meno infinito invece di arrotondamento a zero. Gli shift con e senza segno 
  avranno degli opcode dedicati in Constantinople, e per ora sono emulati da Solidity.

* Lo statement ``continue`` in un ciclo ``do...while`` ora salta alla condizione,
  che è il comportamento comune in questo caso. Viene utilizzato per saltare nel
  body del loop. Quindi, se la condizione è falsa, il loop termina.

* Le funzioni ``.call()``, ``.delegatecall()`` e ``.staticcall()`` non effettuano 
  più il padding quando viene passato un singolo ``byte`` come parametro.

* Le funzioni pure e view sono chiamate attraverso l'opcode ``STATICCALL``
  invece di ``CALL`` se la versione di EVM è Byzantium o successiva. Questo impedisce
  cambi di stato a livello EVM.

* L'encoder ABI ora effettua il padding di array di byte e string dai calldata
  (``msg.data`` e parametri di funzioni esterne) quando vengono utilizzati in 
  chiamate a funzioni external e in ``abi.encode``. Per un encoding senza padding usare
  ``abi.encodePacked``.

* Il decoder ABI ritorna all'inizio della funzione e ad
  ``abi.decode()`` se i calldata passati sono troppo corti o puntano out of bound.
  Notare che i dirty higher order bits sono ignorati.

* L'inoltro di tutto il gas disponibile con chiamate a funzioni external
  inizia con Tangerine Whistle.

Cambiamenti Sintattici e Semantici
==================================

Questa sezione evidenzia i cambiamenti che riguardano sia la sintassi che la 
semantica.

* Le funzioni ``.call()``, ``.delegatecall()``, ``staticcall()``,
  ``keccak256()``, ``sha256()`` e ``ripemd160()`` ora accettano solo un singolo argomento 
  ``bytes``. Inoltre, all'argomento non è applicato il padding. Questo cambiamento è
  stato introdotto per rendere più esplicito e chiaro come gli argomenti vengono concatenati. 
  È necessario sostituire ogni ``.call()`` (e tutte le funzioni della stessa famiglia) con ``.call("")`` 
  ed ogni ``.call(signature, a,  b, c)`` con ``.call(abi.encodeWithSignature(signature, a, b, c))`` 
  (l'ultimo funziona solo con i tipi value). Cambiare inoltre ogni ``keccak256(a, b, c)`` 
  con ``keccak256(abi.encodePacked(a, b, c))``. Anche se non è un cambio retrocompatibile, 
  è consigliato sostituire ``x.call(bytes4(keccak256("f(uint256)"), a, b)`` con
  ``x.call(abi.encodeWithSignature("f(uint256)", a, b))``.

* Le funzioni ``.call()``, ``.delegatecall()`` e ``.staticcall()`` ora restituiscono 
  ``(bool, bytes memory)`` per fornire accesso ai dati di ritorno. Sostituire 
  ``bool success = otherContract.call("f")`` con ``(bool success, bytes memory
  data) = otherContract.call("f")``.

* Solidity ora implementa le regole di scoping C99-style per le variabili locali delle
  funzioni. Ora le variabili possono essere utilizzare solamente la loro
  dichiarazione ande solamente nello stesso scope innestato. Le variabili dichiarate nel
  blocco di inizializzazione di un ciclo ``for`` sono valide in ogni punto all'interno del loop.

Requisiti di Chiarezza
======================

Questa sezione elenca i cambiamenti per i quali ora il codice deve essere
più esplicito. Per alcuni di questi il compilatore lascia dei suggerimenti.

* La visibilità per le funzioni è obbligatoria. Aggiungere ``public`` ad ogni funzione
  e costruttore, e ``external`` ad ogni fallback o interfaccia che non specifica già 
  una visibilità.

* È obbligatorio specificare la posizione in memoria di tutte le variabili o strutture, 
  array o mapping. Questa regola viene anche applicata ai parametri delle funzioni e 
  alle variabili di ritorno. Per esempio, cambiare change ``uint[] x = m_x`` con 
  ``uint[] storage x = m_x``, e ``function f(uint[][] x)`` con 
  ``function f(uint[][] memory x)`` dove ``memory`` è la posizione dei dati e 
  può essere sostituita con ``storage`` o ``calldata``. Notare che le funzioni ``external``
  richiedono che i parametri siano ``calldata``.

* I contract types non includono più ``address`` per distinguere i namespace.  
  Ora è quindi necessario convertire esplicitamente i valori in address prima
  di utilizzare ``address``. Per esempio, se ``c`` è un contratto, cambiare
  ``c.transfer(...)`` con ``address(c).transfer(...)``,
  e ``c.balance`` con ``address(c).balance``. 

* Conversioni esplicite tra unrelated contract type non sono più consentite. 
  È possibile convertire solo da un tipo di contratto a uno dei suoi tipi base o antenati. 
  Se si è sicuri che un contratto sia compatibile con il tipo di contratto in cui 
  si vuole convertirlo, anche se non lo eredita, si può aggirare questo limite 
  convertendo prima in ``address``. Esempio: se ``A`` e ``B`` sono tipi di contratto, 
  ``B`` non eredita da ``A`` e ``b`` è un contratto di tipo ``B``, si può ancora 
  convertire ``b`` nel tipo ``A`` usando ``A (address(b))``.
  È ancora necessario fare attenzione alle corrispondenti funzioni di fallback payable, 
  come spiegato di seguito.

* Il tipo ``address`` è stato diviso in ``address`` e ``address payable``,
  dove solo ``address payable`` fornisce la funzione ``transfer``.
  Un ``address payable`` può essere convertito direttamente in ``address``, ma
  non viceversa. Convertire ``address`` in ``address payable`` è possibile con 
  una conversione a ``uint160``. Se ``c`` è un contract, ``address(c)`` 
  risulta in ``address payable`` solo se ``c`` ha una funzione di fallback payable. 
  Se vengono seguiti i :ref:`withdraw pattern<withdrawal_pattern>`,
  probabilmente non bisogna cambiare il codice peché ``transfer``
  viene solamente utilizzato su ``msg.sender`` invece di indirizzi salvati in 
  memoria e ``msg.sender`` è un ``address payable``.

* Conversioni tra ``bytesX`` e ``uintY`` di diverse dimensioni non sono più consentite
  a causa del padding a destra di ``bytesX`` e a sinistra di ``uintY``che potrebbero
  causare risultati di conversione inaspettati. La dimensione ora deve essere sistemata
  prima della conversione. Per esempio, si può convertire ``bytes4`` (4 byte) in 
  ``uint64`` (8 byte) convertendo prima ``bytes4`` in ``bytes8`` e poi in ``uint64``. 
  Si può ottenere il padding opposto quando si converte attraverso ``uint32``.

* L'utilizzo di ``msg.value`` in funzioni non payable (o l'introduzione attraverso un
  modifier) non è più consentita per questioni di sicurezza. Convertire una funzione in
  ``payable`` o creare una nuova funzione internal function per le parti del programma 
  che usano ``msg.value``.

* Per motivi di chiarezza, l'interfaccia a linea di comando ora richiede ``-`` se viene
  utilizzato come input lo standard input.

Elementi Deprecati
===================

Questa sezione elenca i cambiamenti che rendono deprecate le caratteristiche
precedenti o la sintassi. Notare che questi cambiamenti erano già attivati nella 
modalità experimental ``v0.5.0``.

Linea di Comando ed Interfacce JSON
-----------------------------------

* L'opzione a linea di comando ``--formal`` (utilizzata per generare l'outpu Why3 
  per una ulteriore verifica formale) è stata deprecata ed ora rimossa. Un nuovo modulo
  per la verifica formale è disponibile, chiamato SMTChecker, e può essere attivato 
  con ``pragma experimental SMTChecker;``.

* L'opzione a linea di comando ``--julia`` è stata rinominata in ``--yul`` peché
  il linguaggio intermedio ``Julia`` è stato rinominato in ``Yul``.

* Le opzioni a linea di comando ``--clone-bin`` e ``--combined-json clone-bin``
  sono state rimosse.

* Il remapping con un prefisso vuoto non è consentito.

* I campi JSON AST ``constant`` e ``payable`` sono stati rimossi. Le informazioni
  ora sono presenti nel campo ``stateMutability``.

* Il campo JSON AST ``isConstructor`` di ``FunctionDefinition``
  è stato sostituito da un campo ``kind`` che può assumere i valori
  ``"constructor"``, ``"fallback"`` o ``"function"``.

* Nei file esadecimali unlinked, i placeholder per gli indirizzi di librerie sono ora
  i primi 36 caratteri esadecimali dell'hash keccak256 del fully qualified
  name della libreria, circondato da ``$...$``. Precedentemente, solamente il fully 
  qualified name della libreria veniva usato. Questo riduce la possiblità di
  collisione, specialmente quando vengono utilizzati dei percorsi lunghi.
  I file binari ora contengono anche una lista di mapping da questi placeholder
  verso il fully qualified names.

Costruttori
-----------

* I costruttori devono essere definiti con la keyword ``constructor``.

* La chiamata ad un costruttore base senza parentesi non è più supportata.

* Non è più consentito specificare gli argomenti del costruttore base più volte
  nella stessa gerarchia di ereditarietà.

* La chiamata ad un costruttore col numero sbagliato di argomenti non è più supportata.  
  Se si vuole specificare una relazione di ereditarietà senza fornire argomenti, non
  inserire le parentesi.

Funzioni
--------

* La funzione ``callcode`` non è più supportata (a favore di ``delegatecall``). Si può
  ancora utilizzare con inline assembly.

* ``suicide`` non è più supportato (a favore di ``selfdestruct``).

* ``sha3`` non è più supportato (a favore di ``keccak256``).

* ``throw`` non è più supportato (a favore di ``revert``, ``require`` e ``assert``).

Conversioni
-----------

* Conversioni esplicite ed implicite da decimal literals a tipi ``bytesXX`` non sono più supportate.
  
* Conversioni esplicite ed implicite da hex literals a tipi ``bytesXX`` di dimensione
  diversa non sono più supportate.

Letterali e Suffissi
--------------------

* La dicitura ``years`` non è più supportata a causa di confusioni create da 
  anni bisestili.

* I puntini (trailing dots) che non sono seguiti da un numero ora non sono consentiti.

* La combinazione di numeri esadecimali con unità (e.g. ``0x1e wei``) non è più consentito.

* Il prefisso ``0X`` per i numeri decimali non è più consentito, è consentito solo ``0x``.

Variabili
---------

* La dichiarazione di strutture vuote non è più consentito.

* La keyword ``var`` non è più consentita.

* Assegnamenti tra tuple con numero diverso di componenti non è più consentito.

* Valori per costanti che non sono costanti a tempo di compilazione non sono più consentiti.

* La dichiarazione di multi-variabili con un numero diverso di valori non è più consentito.

* Le dichiarazione di variabili storage vuote non è più consentito.

* Componenti di tuple vuoti non sono più consentiti.

* L'individuazione di dipendenze cicliche tra variabili e strutture è limitata nella ricorsione a 256.

* Array di dimensione costante con una lunghezza pari a 0 non sono più consentiti.

Sintassi
--------

* L'utilizzo di ``constant`` come modificatore di stato per le funzioni non è più consentito.

* Espressioni booleane non possono utilizzare operazioni aritmetiche.

* L'operatore unario ``+`` non è più consentito.

* I letterali non possono più essere usati con ``abi.encodePacked`` senza una conversione precedente
  ad un tipo esplicito.

* Gli statement return vuoti per funzioni dove uno o più valori di ritorno non sono più consentiti.

* La sintassi "loose assembly" non è più completamente consentita: jump label,
  jumps ed instruzioni non funzionali non possono più essere usate. Utilizzare invece in nuovi costrutti
  ``while``, ``switch`` e ``if``.

* Funzioni senza implementazione non possono più usare i modificatori.

* I tipi di una funzione con valori di ritorno named non sono più consentiti.

* La dichiarazione di variabili dentro il corpo di if/while/for che non sono blocchi non è più consentita.
 
* Nuove keyword: ``calldata`` e ``constructor``.

* Nuove keyword riservate: ``alias``, ``apply``, ``auto``, ``copyof``,
  ``define``, ``immutable``, ``implements``, ``macro``, ``mutable``,
  ``override``, ``partial``, ``promise``, ``reference``, ``sealed``,
  ``sizeof``, ``supports``, ``typedef`` e ``unchecked``.

.. _interoperability:

Interoperabilità con Contratti Precedenti
=========================================

È ancora possibile interfacciarsi con i contratti scritti per le versioni di 
Solidity precedenti alla v0.5.0 (o viceversa) definendone le interfacce.
Considera di avere già implementato il seguente contratto precedente alla 0.5.0:

::

   // This will not compile with the current version of the compiler
   pragma solidity ^0.4.25;
   contract OldContract {
      function someOldFunction(uint8 a) {
         //...
      }
      function anotherOldFunction() constant returns (bool) {
         //...
      }
      // ...
   }

Questo non verrà più compilato con Solidity v0.5.0.
Tuttavia, è possibile definire un'interfaccia compatibile per esso:

::

   pragma solidity >=0.5.0 <0.7.0;
   interface OldContract {
      function someOldFunction(uint8 a) external;
      function anotherOldFunction() external returns (bool);
   }

Si noti che non abbiamo dichiarato ``anotherOldFunction`` come ``view``, 
nonostante sia stato dichiarato ``constant`` nel contratto originale. 
Ciò è dovuto al fatto che a partire da Solidity v0.5.0 ``staticcall`` viene 
utilizzato per chiamare le funzioni ``view``.
Prima della v0.5.0 la parola chiave ``costante`` non era applicata, quindi chiamare 
una funzione dichiarata ``costante`` con ``staticcall`` potrebbe ancora fallire, 
poiché la funzione `` costante`` potrebbe ancora tentare di modificare la memoria. 
Di conseguenza, quando si definisce un'interfaccia per contratti più vecchi, si dovrebbe 
usare solo `` view`` al posto di `` costante`` nel caso in cui si sia assolutamente sicuri
che la funzione funzionerà con `` staticcall``.

Data l'interfaccia sopra definita, ora è possibile utilizzare facilmente il contratto pre-0.5.0 già caricato:

::

   pragma solidity >=0.5.0 <0.7.0;

   interface OldContract {
      function someOldFunction(uint8 a) external;
      function anotherOldFunction() external returns (bool);
   }

   contract NewContract {
      function doSomething(OldContract a) public returns (bool) {
         a.someOldFunction(0x42);
         return a.anotherOldFunction();
      }
   }


Allo stesso modo, le librerie pre-0.5.0 possono essere usate definendo le 
funzioni della libreria senza implementazione e fornendo l'indirizzo della libreria 
pre-0.5.0 durante il collegamento (vedere :ref:`commandline-compiler` su come usare il 
compilatore da linea di comando per il linking):

::

   pragma solidity >=0.5.0 <0.7.0;

   library OldLibrary {
      function someFunction(uint8 a) public returns(bool);
   }

   contract NewContract {
      function f(uint8 a) public returns (bool) {
         return OldLibrary.someFunction(a);
      }
   }


Esempio
=======


L'esempio seguente mostra un contratto e la sua versione aggiornata per Solidity 
v0.5.0 con alcune delle modifiche elencate in questa sezione.

Versione vecchia:

::

   // Questo non compila
   pragma solidity ^0.4.25;

   contract OtherContract {
      uint x;
      function f(uint y) external {
         x = y;
      }
      function() payable external {}
   }

   contract Old {
      OtherContract other;
      uint myNumber;

      // Mutabilità della funzione non fornita, non un errore.
      function someInteger() internal returns (uint) { return 2; }

      // Visibilità della funzione non fornita, non un errore.
      // Mutabilità della funzione non fornita, non un errore.
      function f(uint x) returns (bytes) {
         // Var funzione in questa versione.
         var z = someInteger();
         x += z;
         // Throw funzione in questa versione.
         if (x > 100)
            throw;
         bytes b = new bytes(x);
         y = -3 >> 1;
         // y == -1 (sbagliato, dovrebbe essere -2)
         do {
            x += 1;
            if (x > 10) continue;
            // 'Continue' causa un loop infinito.
         } while (x < 11);
         // Call restituisce solamente un Bool.
         bool success = address(other).call("f");
         if (!success)
            revert();
         else {
            // Variabili locali possono essere dichiarate dopo l'uso.
            int y;
         }
         return b;
      }

      // Non è necessaria una locazione specifica per 'arr'
      function g(uint[] arr, bytes8 x, OtherContract otherContract) public {
         otherContract.transfer(1 ether);

         // Poiché uint32 (4 byte) è più piccolo di bytes8 (8 bytes),
         // i primi 4 byte di x verranno persi. Questo può causate un comportamento
         // inaspettato poiché i bytesX subiscono un padding a destra.
         uint32 y = uint32(x);
         myNumber += y + msg.value;
      }
   }

Nuova versione:

::

   pragma solidity >=0.5.0 <0.7.0;

   contract OtherContract {
      uint x;
      function f(uint y) external {
         x = y;
      }
      function() payable external {}
   }

   contract New {
      OtherContract other;
      uint myNumber;

      // La mutabilità della funzione deve essere specificata.
      function someInteger() internal pure returns (uint) { return 2; }

      // La visibilità della funzione deve essere specificata.
      // La mutabilità della funzione deve essere specificata.
      function f(uint x) public returns (bytes memory) {
         // Il tipo deve essere inserito esplicitamente.
         uint z = someInteger();
         x += z;
         // Throw non è consentito.
         require(x > 100);
         int y = -3 >> 1;
         // y == -2 (corretto)
         do {
            x += 1;
            if (x > 10) continue;
            // 'Continue' salta alla condizione sottostante.
         } while (x < 11);

         // Call restituisce (bool, bytes).
         // La posizione dei dati deve essere specificata.
         (bool success, bytes memory data) = address(other).call("f");
         if (!success)
            revert();
         return data;
      }

      using address_make_payable for address;
      // La posizione dei dati per 'arr' deve essere specificata
      function g(uint[] memory arr, bytes8 x, OtherContract otherContract, address unknownContract) public payable {
         // 'otherContract.transfer' non è fornito.
         // Poiché il codice di 'OtherContract' è noto e ha una funzione di fallback,
         // address(otherContract) è di tipo 'address payable'.
         address(otherContract).transfer(1 ether);

         // 'unknownContract.transfer' non è fornito.
         // 'address(unknownContract).transfer' non è fornito.
         // poiché 'address(unknownContract)' non è 'address payable'.
         // Se la funzione riceve un 'address' al quale si vogliono inviare fondi,
         // si può convertirlo in 'address payable' tramite 'uint160'.
         // Nota: questa operazione non è raccomandata ed il tipo esplicito
         // 'address payable' dovrebbe essere usato se possibile.
         // Per migliorare la chiarezza, suggeriamo l'uso di una libreria per 
         // la conversione (fornita dopo il contratto in questo esempio).
         address payable addr = unknownContract.make_payable();
         require(addr.send(1 ether));

         // Poiché uint32 (4 byte) è più piccolo di bytes8 (8 byte),
         // la conversione non è permessa.
         // Deve essere prima convertito prima ad una dimensione comune:
         bytes4 x4 = bytes4(x); // Padding a destra
         uint32 y = uint32(x4); // La conversione è consistente
         // 'msg.value' non può essere usato in una funzione 'non-payable'.
         // La funzione deve essere trasformata in payable.
         myNumber += y + msg.value;
      }
   }

   // Si può definire una libreria per la conversione da ``address``
   // a ``address payable`` come soluzione alternativa.
   library address_make_payable {
      function make_payable(address x) internal pure returns (address payable) {
         return address(uint160(x));
      }
   }
