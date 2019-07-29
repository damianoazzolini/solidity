.. index:: ! using for, library

.. _using-for:

***************
Utilizzo di For
***************

La direttiva ``using A for B;`` può essere usata per associare funzioni di 
libreria (dalla libreria ``A``) ad ogni tipo (``B``).
Queste funzioni ricevono l'oggetto sul quale sono chiamate come primo
parametro (come la variabile ``self`` in Python).

L'effetto di ``using A for *;`` è che le funzioni della libreria
``A`` sono associate a *qualsiasi* tipo.

In entrambe le situazioni, *tutte* le funzioni nella libreria sono 
associate, anche quelle in cui il tipo del primo parametro non corrisponde 
al tipo di oggetto. Il tipo viene verificato nel punto in cui viene chiamata 
la funzione e viene eseguita la risoluzione dell'overloading della funzione.

La direttiva ``using A for B;`` è attiva solamente nel contratto corrente,
incluso in tutte le sue funzioni e non ha alcun effetto al di fuori del contratto 
in cui viene utilizzata. La direttiva può essere utilizzata solo all'interno di 
un contratto, e non all'interno delle sue funzioni.


Includendo una libreria, i suoi tipi di dati, comprese le funzioni della libreria, 
sono disponibili senza dover aggiungere altro codice.

Riscriviamo l'esempio da :ref:`libraries` in questo modo: ::

    pragma solidity >=0.4.16 <0.7.0;


    // Codice di prima senza commenti
    library Set {
        struct Data { mapping(uint => bool) flags; }

        function insert(Data storage self, uint value)
            public
            returns (bool)
        {
            if (self.flags[value])
                return false;
            self.flags[value] = true;
            return true;
        }

        function remove(Data storage self, uint value)
            public
            returns (bool)
        {
            if (!self.flags[value])
                return false;
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
        using Set for Set.Data; // cambiamento
        Set.Data knownValues;

        function register(uint value) public {
            // Qui tutte le variabili di tipo Set.Data hanno la
            // corrispondente funzione member.
            // La seguente chiamata a funzione equivale a 
            // `Set.insert(knownValues, value)`
            require(knownValues.insert(value));
        }
    }

È anche possibile estendere i tipi elementari come segue: ::

    pragma solidity >=0.4.16 <0.7.0;

    library Search {
        function indexOf(uint[] storage self, uint value)
            public
            view
            returns (uint)
        {
            for (uint i = 0; i < self.length; i++)
                if (self[i] == value) return i;
            return uint(-1);
        }
    }

    contract C {
        using Search for uint[];
        uint[] data;

        function append(uint value) public {
            data.push(value);
        }

        function replace(uint _old, uint _new) public {
            // Effettua la chiamata alla funzione di libreria
            uint index = data.indexOf(_old);
            if (index == uint(-1))
                data.push(_new);
            else
                data[index] = _new;
        }
    }

Si noti che tutte le chiamate in libreria sono effettive chiamate di funzione EVM. 
Ciò significa che se vengono passati tipi memory o values, verrà eseguita una copia, 
anche della variabile `` self``. 
L'unica situazione in cui non verrà eseguita alcuna copia è quando vengono utilizzate 
le variabili di riferimento allo storage.
