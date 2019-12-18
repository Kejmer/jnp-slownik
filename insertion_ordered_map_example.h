
/* TODO LIST
Konstruktor bezparametrowy O(1)
Konstruktor kopiujący O(1) O(n) copy-on-write
Konstruktor przenoszący O(1)
Operator przypisania O(1)
Wstawianie do słownika O(1) // smart pointer check
Usuwanie ze słownika O(1) // smart pointer check
Scalanie słowników O(n+m) // iterowanie się po drugim i wstawianie?
Referencja wartości O(1) // smart pointer check
Operator indeksowania O(1)
Rozmiar słownika O(1)
Sprawdzanie niepustości O(1)
Czyszczenie słownika O(n) // a nie da się w O(1)?
Sprawdzanie instnienia klucza O(1)
Klasa iteratora: O(1)
	konstruktor kopiujący
	konstruktor bezparametrowy
	operatory:
		++
		==
		!=
		* (dereferencja) 
Klasa lookup_error (na zewnątrz klasy, dziedziczy po std::exception)
Klasa insertion_ordered_map przeźroczysta na wyjątki
*/

/* INNE POMYSŁY 
Jeśli to możliwe sama operacja wstawiania będzie nieudostępnianą operacją
wykorzystywaną przez Wstawianie i Scalanie, aby tuż przed odpaleniem sprawdzać
czy należy kopiować tylko raz.
*/
