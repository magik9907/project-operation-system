#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "function.h"
int main(int argc, char *argv[])
{
    if (argc < 5)
    {
        exitFailure("Not given arguments\n");
    }

    init(argc, argv);
    syncDir();
    return 0;
}
// TODO: ERRORS catch
//example command after compile:
//      ./function -s ./test -d ./test_copy -t 3 -m 3 -R
//moze na pipe zrobic

// Demon synchronizujący dwa podkatalogi

// [DONE]Program który otrzymuje co najmniej dwa argumenty: ścieżkę źródłową, ścieżkę docelowa.

//[DONE] Jeżeli któraś ze ścieżek nie jest katalogiem program powraca natychmiast z komunikatem błędu.
//   [TODO]  W przeciwnym wypadku staje się demonem.

//[DONE]Synchronizacja folderów (nie rekursyjna)

// Demon wykonuje następujące czynności:
//     śpi przez piec minut (czas spania można zmieniać przy pomocy dodatkowego opcjonalnego argumentu) (parametr -t),
//     po czym po obudzeniu się porównuje katalog źródłowy z katalogiem docelowym.
// [DONE?]    Pozycje, które nie są zwykłymi plikami są ignorowane (np. katalogi i dowiązania symboliczne).

//     Jeżeli demon
//         {
//        [DONE]     (a) napotka na nowy plik w katalogu źródłowym,  i tego pliku brak w katalogu docelowym
//         [DONE]    (b) plik w katalogu docelowym ma późniejsza datę ostatniej modyfikacji
//         }=>demon wykonuje kopie pliku z katalogu źródłowego do katalogu docelowego
//             -ustawiając w katalogu docelowym datę modyfikacji tak, aby przy kolejnym obudzeniu nie trzeba było wykonać kopii (chyba ze plik w katalogu źródłowym zostanie ponownie zmieniony).
//          [DONE]   Jeżeli zaś odnajdzie plik w katalogu docelowym, którego nie ma w katalogu źródłowym to usuwa ten plik z katalogu docelowego.

//     Możliwe jest również natychmiastowe obudzenie się demona poprzez wysłanie mu sygnału SIGUSR1.

// Wyczerpująca informacja o każdej akcji typu uśpienie/obudzenie się demona (naturalne lub w wyniku sygnału), wykonanie kopii lub usuniecie pliku jest przesłana do logu systemowego.
// Informacja ta powinna zawierać aktualna datę.
// [DONE]Operacje kopiowania mają być wykonane za pomocą niskopoziomowych operacji read/write. (14p). Dodatkowo:

//[DONE]Opcja -R pozwalająca na rekurencyjną synchronizację katalogów (teraz pozycje będące katalogami nie są ignorowane). W szczególności jeżeli demon stwierdzi w katalogu docelowym podkatalog którego brak w katalogu źródłowym powinien usunąć go wraz z zawartością. (8p)

//[TODO] W zależności od rozmiaru plików {
//     dla małych plików wykonywane jest kopiowanie przy pomocy read/write a
//     w przypadku dużych używany jest bardziej efektywny mechanizm,
//         np.: przy pomocy mmap/write (plik źródłowy zostaje zamapowany w całości w pamięci)
//         lub za pomocą dedykowanego wywołania (np. sendfile czy copy_file_range).

//     Próg dzielący pliki małe od dużych może być przekazywany jako opcjonalny argument.
//     Wykonaj analizę wpływu danej metody na szybkość kopiowania plików i przedstaw wyniki w dokumentacji. (12p)
// }