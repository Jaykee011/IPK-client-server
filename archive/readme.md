NAME
--------

>ftrest, ftrestd

SYNOPSIS
--------
>ftrest COMMAND REMOTE-PATH [LOCAL-PATH]  
>frestd [-p PORT] [-r ROOT-FOLDER]

DESCRIPTION
--------
>**ftrest** spustí klienta pro komunikaci v protokolu HTTP a vyšle zprávu s RESTful operací podle atributu COMMAND na server specifikovaný v REMOTE-PATH. V případě chyby vypíše hlášku na stderr a ukončí se s chybovou hodnotou.

>**ftrestd** spustí souborový server na portu PORT (implicitně *6677*) s kořenovým souborem ROOT-FOLDER (implicitně *./*) pro komunikaci v protokolu HTTP, zpracovávající RESTful operace.

OPTIONS
-------
>**COMMAND** specifikuje operaci, která má být provedena. Možné operace jsou:

>>lst \- vypíše obsah adresáře

>>mkd \- vytvoří ardresář

>>del \- smaže soubor

>>rmd \- smaže prázdný adresář

>>put \- nahraje soubor na server

>>get \- stáhne soubor ze serveru

>**REMOTE-PATH** URI, specifikující server, port a cestu, ve tvaru *http://SERVER-NAME:PORT/USER/PATH* nebo *http://SERVER-NAME/USER/PATH*, port je implicitně *6677*

>**LOCAL-PATH** relativní cesta k souboru. Povinné pro operaci put, volitelné pro operaci get (implicitně ./)

>**-p PORT** port na kterém má server naslouchat. Implicitně *6677*.

>**-r ROOT-FOLDER** kořen adresářového stromu serveru. Implicitně *./*

EXIT STATUS
--------

>Vrácená hodnota je 0 v případě úspěchu. V případě neúspěchu, 1 značí pokus o vytvoření již existujícího adresáře, 2 neznámého uživatele, 3 že adresář nebyl nalezen, 4 že mazaný adresář není prázdný, 5 pokus o provedení adresářové operace nad souborem, 6 pokus o provedení souborové operace nad adresářem, 7 nenalezený soubor a 127 jinou chybu.

DIAGNOSTICS
-----------

>Následující chybová hlášení se mohou vyskytnout na stderr:

>Not a file.
>>Pokus o provedení adresářové operace nad souborem.

>Not a directory.
>>Pokus o provedení souborové operace nad adresářem.

>Directory not empty.
>>Mazaný adresář není prázdný.

>File not found.
>>Soubor nebyl nalezen.

>Directory not found.
>>Adresář nebyl nalezen.

>User Account Not Found.
>>Specifikovaný uživatel na serveru neexistuje.

>Unknown error.
>>Nastala jiná chyba.