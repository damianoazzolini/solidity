Risorse
-------

Generali
~~~~~~~~

* `Ethereum <https://ethereum.org>`_

* `Changelog <https://github.com/ethereum/solidity/blob/develop/Changelog.md>`_

* `Source Code <https://github.com/ethereum/solidity/>`_

* `Ethereum Stackexchange <https://ethereum.stackexchange.com/>`_

* `Language Users Chat <https://gitter.im/ethereum/solidity/>`_

* `Compiler Developers Chat <https://gitter.im/ethereum/solidity-dev/>`_

Integrazioni con Solidity
~~~~~~~~~~~~~~~~~~~~~~~~~

* Generiche:

    * `EthFiddle <https://ethfiddle.com/>`_
        Solidity Browser IDE. Scrivi e condividi il tuo codice Solidity. Utilizza componenti server-side.

    * `Remix <https://remix.ethereum.org/>`_
        Browser-based IDE con un compilatore integrato ed un ambiente Solidity runtime senza componenti server-side.

    * `Solhint <https://github.com/protofire/solhint>`_
        Solidity linter che fornisce guide linea e best practice per la verifica di smart contract.

    * `Solidity IDE <https://github.com/System-Glitch/Solidity-IDE>`_
        Browser-based IDE con compilatore integrato e supporto per Ganache e local file system.

    * `Solium <https://github.com/duaraghav8/Solium/>`_
        Linter per identificare e sistemare problemi di stile e di sicurezza in Solidity.

    * `Superblocks Lab <https://lab.superblocks.com/>`_
        Browser-based IDE. Built-in browser-based VM ed integrazione con Metamask (one click deployment to Testnet/Mainnet).

* Atom:

    * `Etheratom <https://github.com/0mkara/etheratom>`_
        Plugin per l'editor Atom che fornisce syntax highlighting, compilazione ed un runtime environment (compatibile con backend node & VM).

    * `Atom Solidity Linter <https://atom.io/packages/linter-solidity>`_
        Plugin per Atom che fornisce linting per Solidity.

    * `Atom Solium Linter <https://atom.io/packages/linter-solium>`_
        Solidty linter configurabile per atom Atom basato su Solium.

* Eclipse:

   * `YAKINDU Solidity Tools <https://yakindu.github.io/solidity-ide/>`_
        IDE basato su Eclipse. Fornisce code completion context sensitive ed help, code navigation, syntax coloring, compilatore built in, quick fixes e template.

* Emacs:

    * `Emacs Solidity <https://github.com/ethereum/emacs-solidity/>`_
        Plugin per l'editor Emacs che fornisce syntax highlighting e report per errori di compilazione.

* IntelliJ:

    * `IntelliJ IDEA plugin <https://plugins.jetbrains.com/plugin/9475-intellij-solidity>`_
        Solidity plugin per IntelliJ IDEA (e tutti gli altri IDE JetBrains)

* Sublime:

    * `Package for SublimeText - Solidity language syntax <https://packagecontrol.io/packages/Ethereum/>`_
        Solidity syntax highlighting per l'editor SublimeText.

* Vim:

    * `Vim Solidity <https://github.com/tomlion/vim-solidity/>`_
        Plugin per Vim che fornisce syntax highlighting.

    * `Vim Syntastic <https://github.com/scrooloose/syntastic>`_
        Plugin per the Vim che fornisce compile checking.

* Visual Studio Code:

    * `Visual Studio Code extension <http://juan.blanco.ws/solidity-contracts-in-visual-studio-code/>`_
        Solidity plugin per Microsoft Visual Studio Code che include syntax highlighting e un compilatore Solidity.

Non più mantenuti:

* `Mix IDE <https://github.com/ethereum/mix/>`_
    Qt based IDE per il design, debugging e testing di smart contract Solidity.

* `Ethereum Studio <https://live.ether.camp/>`_
    Web IDE specializzato che fornisce anche accesso shell ad un completo Ethereum environment.

* `Visual Studio Extension <https://visualstudiogallery.msdn.microsoft.com/96221853-33c4-4531-bdd5-d2ea5acc4799/>`_
    Solidity plugin per Microsoft Visual Studio che include un Solidity compiler.

Strumenti per Solidity
~~~~~~~~~~~~~~~~~~~~~~

* `Dapp <https://dapp.tools/dapp/>`_
    Build tool, package manager, e deployment assistant per Solidity.

* `Solidity REPL <https://github.com/raineorshine/solidity-repl>`_
    Prova Solidity istantaneamente con una console command-line Solidity.

* `solgraph <https://github.com/raineorshine/solgraph>`_
    Visualizza il control flow Solidity ed evidenzia potenziali vulnerabilità di sicurezza.

* `Doxity <https://github.com/DigixGlobal/doxity>`_
    Generatore di documentazione per Solidity.

* `evmdis <https://github.com/Arachnid/evmdis>`_
    EVM Disassembler che esegue una analisi statica sul bytecode per fornire un livello di astrazione superiore rispetto a raw EVM operation.

* `ABI to solidity interface converter <https://gist.github.com/chriseth/8f533d133fa0c15b0d6eaf3ec502c82b>`_
    Script per generare interfacce per contratti partendo dall'ABI di uno smart contract.

* `Securify <https://securify.ch/>`_
    Static analyser completamente automatizzato ed accessibile online per smart contract, che fornisce security report basati su vulnerability patterns.

* `Sūrya <https://github.com/ConsenSys/surya/>`_
    Strumento che offre diverse informazioni riguardo la struttura di un contratto.

* `EVM Lab <https://github.com/ethereum/evmlab/>`_
    Serie di strumenti per interagire con la EVM. Include una VM, Etherchain API, ed un trace-viewer che mostra anche il costo in gas.

* `Universal Mutator <https://github.com/agroce/universalmutator>`_
    Un tool per generare mutation, con regole configurabili e supporto per Solidity e Vyper.

.. note::
  Informazioni come nomi di variabili, commenti e formattazione del codice sorgente
  sono perse durante la compilazione e non è possibile risalire al codice sorgente originale.
  Decompilare lo smart contract per analizzare il codice sorgente potrebbe non essere possibile
  o non essere così utile.
  
Solidity Parser Third-Party e Grammatiche
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* `solidity-parser <https://github.com/ConsenSys/solidity-parser>`_
    Solidity parser per JavaScript.

* `Solidity Grammar for ANTLR 4 <https://github.com/federicobond/solidity-antlr4>`_
    Grammatica Solidity per il ANTLR 4 parser generator.
