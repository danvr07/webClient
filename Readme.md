# Client Web - Comunicație cu REST API

Pentru implementarea acestei teme, am început cu scheletul laboratorului 9, care m-a ajutat foarte mult să înțeleg conceptele folosite în temă. Am folosit limbajul C deoarece sunt foarte multe chestii simple cu string-urile și nu am simțit nevoia de a folosi C++.

## Implementarea

Pentru a afla ce comenzi există, puteți folosi comanda `help`.

Pentru toate comenzile de mai jos am folosit funcțiile din laborator pentru a crea un mesaj, pentru a trimite mesajul și pentru a primi un mesaj de la server. Pe lângă implementarea care era deja, am ajustat codul pentru lucrul cu token-uri.

Pentru token cat si pentru cookie am 2 functii care extrag aceste date din raspunsul primit de la server. Toate functiile ajutatoare se afla in helpers.c

**Notă:** M-am gândit că ar fi corect ca după ce suntem logați să nu avem voie să ne înregistrăm din nou, astfel întorcând un mesaj că trebuie să ne delogăm mai întâi. Nu știu dacă acest lucru este corect, dar în viziunea mea așa este!

Pentru a primi comenzile, am creat o buclă infinită care așteaptă comenzile și se oprește doar în cazul în care se dă comanda `exit`.

### Comenzi

#### `register`
Pentru a înregistra un user, mai întâi citesc de la tastatură username-ul și password-ul. Verific cazurile să nu fie parola sau username-ul gol sau să conțină spații. Dacă se întâmplă acest lucru, întorc un mesaj de eroare. Dacă aceste credențiale nu au mai fost folosite, user-ul este înregistrat cu succes. Dacă username-ul a mai fost folosit, se afișează un mesaj precum că acest username este ocupat.

#### `login`
Pentru login, am făcut aceleași verificări ca și la `register`. Dacă credențialele sunt corecte și acest user există pe server, se loghează cu succes și se întoarce cookie-ul de autorizare. Dacă parola este greșită, se afișează un mesaj precum credențialele sunt incorecte, iar dacă username-ul nu există, se afișează un mesaj precum că nu există acest utilizator.

#### `enter_library`
Dacă user-ul nu este autorizat, nu poate avea acces. Dacă răspunsul primit de la server conține un token, putem ști sigur că am primit acces la bibliotecă. Am tratat și cazul în care, dacă am primit acces, după ce dăm comanda `enter_library` din nou să afișeze mesaj precum "Acces deja permis". Am verificat doar dacă token-ul nu este gol când se dă comanda. Dacă nu este gol, deja avem acces.

#### `get_books`
Pentru `get_books`, verific mai întâi dacă user-ul este logat, adică dacă cookie-ul este gol, întoarce un mesaj de eroare. Dacă trece peste această verificare, se verifică dacă user-ul are acces, astfel se verifică dacă token-ul nu este gol. Dacă este gol, se întoarce un mesaj de eroare precum că user-ul nu are acces. Pentru aceste verificări, am făcut funcții ajutătoare pe care le folosesc și la alte comenzi în temă, precum `get_book_id`, `delete_book`, `logout`.

#### `get_book_id`
Pentru această comandă, concatenez URL-ul cu ID-ul cărții, trimit către server și verific răspunsul primit de la server pentru a trata toate cazurile.

Pentru `get_books` și `get_book_id` am funcții ajutătoare care îmi afișează informațiile dorite în formatul scris în condiție (puteți verifica manual).

#### `add_book`
Pentru această comandă, citesc toate câmpurile necesare de la tastatură, verific ca `page_count` să fie număr și verific să nu existe câmpuri goale. Dacă toate datele sunt valide, creez un obiect JSON și îl trimit către server, verific răspunsul primit și afișez mesajele.

#### `delete_book`
Pentru `delete_book`, pe lângă funcțiile de get și post din laborator, am mai creat o funcție similară pentru delete. Este o logică similară cu `add_book`, doar că se folosește `compute_delete_request` în loc de post.

#### `logout`
Pentru logout, verific mai întâi dacă sunt logat. Dacă nu, afișez un mesaj de eroare. Dacă sunt logat, golesc cookie-ul și token-ul.

Acesta tema folosește biblioteca parson pentru a crea și a manipula obiecte JSON în diverse funcții. Biblioteca parson este utilizată pentru a facilita comunicarea cu serverul prin crearea de cereri HTTP în format JSON și interpretarea răspunsurilor JSON primite.
În funcțiile care comunică cu serverul (cum ar fi register_user, login_user, și add_book), se inițializează obiecte JSON și li se adaugă perechi cheie-valoare pentru a reprezenta datele trimise către server.
Obiectele JSON create sunt convertite în șiruri de caractere pentru a fi incluse în cererile HTTP POST către server, iar răspunsurile JSON primite sunt parsate pentru a extrage informațiile relevante. Aceasta se face folosind funcții precum json_parse_string și json_value_get_object.

---

Vrinceanu Dan 325CD
