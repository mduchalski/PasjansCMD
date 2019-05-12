#include<iostream>
#include<queue>
#include<algorithm>
#include<iomanip>
#include<cstdlib>
#include<ctime>
#include<exception>
#include<stack>
#include<string>
#include<sstream>

using namespace std;

// własności kart
enum KolorTyp { TREFL, KARO, KIER, PIK };
enum FiguraTyp {
	AS, DWA, TRZY, CZTERY, PIEC, SZESC, SIEDEM, OSIEM, DZIEWIEC,
	DZIESIEC, WALET, DAMA, KROL
};
bool czerwona(KolorTyp);

const unsigned int ILE_WIDOK = 3; // ilość kart w widoku
class Karta
{
public:
	Karta() {}
	Karta(KolorTyp, FiguraTyp);

	void ukryj() { ukryta = true; }
	void odkryj() { ukryta = false; }
	bool czyUkryta() { return ukryta; }

	FiguraTyp jakaFigura() { return figura; }
	KolorTyp jakiKolor() { return kolor; }

	friend bool operator ==(const Karta, const Karta);
	friend bool operator !=(const Karta, const Karta);
	friend ostream& operator <<(ostream&, const Karta);
	friend istream& operator >>(istream&, Karta&);

protected:
	KolorTyp kolor;
	FiguraTyp figura;
	bool ukryta;
};

// obsługa talii
class Talia : protected queue<Karta>
{
public:
	Talia();

	void odswiezWidok();

	Karta pierwsza();
	void zdejmij();

	friend ostream& operator <<(ostream&, const Talia);

	friend vector<vector<Karta> > initKolumny(Talia&);
protected:
	vector<Karta> widok; // widoczne dla gracza karty talii
};

bool rozmiarCmp(const vector<Karta>, const vector<Karta>);

// obsługa wyświetlania i rozgrywki
ostream& operator <<(ostream&, vector<stack<Karta> >);
void wyswietl(Talia, vector<vector<Karta> >, vector<stack<Karta> >);
void wykPolecenie(string, Talia&, vector<vector<Karta> >&, vector<stack<Karta> >&);
bool koniecGry(vector<stack<Karta> >);

// przenoszenie kart
void kartaDoKol(Karta, vector<Karta>&);
void kartaDoStosu(Karta, stack<Karta>&);


int main()
{
	Talia talia;
	vector<vector<Karta> > kolumny = initKolumny(talia);
	vector<stack<Karta> > stosy(4);
	string polecenie;

	while (!koniecGry(stosy))
	{
		// wyświetlanie aktualnego stanu kart gracza
		wyswietl(talia, kolumny, stosy);

		// wczytywanie polecenia
		cout << endl << ">> ";
		getline(cin, polecenie);

		// usiłownie wykonania polecenia
		try
		{
			wykPolecenie(polecenie, talia, kolumny, stosy);
		}
		catch (string& blad)
		{
			cout << blad << endl;
			system("pause");
		}

		// czyszczenie ekranu
		system("cls");
	}

	cout << "Koniec gry!" << endl;
	system("pause");

	return 0;
}


bool czerwona(KolorTyp kolor)
{
    // funkcja zwraca prawdę dla kart czerwonych, fałsz dla czarnych
    return kolor == KARO || kolor == KIER;
}

Karta::Karta(KolorTyp _kolor, FiguraTyp _figura)
{
    // konstruktor tworzy kartę o zadanych kolorze i figurze
    kolor = _kolor;
    figura = _figura;
    ukryta = false;
}

bool operator ==(const Karta a, const Karta b)
{
    return a.kolor == b.kolor && a.figura == b.figura && a.ukryta == b.ukryta;
}
bool operator !=(const Karta a, const Karta b)
{
    return !(a == b);
}

ostream& operator <<(ostream& strumien, const Karta k)
{
    // operator wyświetla kartę
    // pominięcie wyświetlania dla ukrytych kart
    if (k.ukryta)
        strumien << "===";
    else
    {
        // wyświetlanie figury karty
        switch (k.figura)
        {
        case WALET:
            strumien << "J"; break;
        case DAMA:
            strumien << "Q"; break;
        case KROL:
            strumien << "K"; break;
        case AS:
            strumien << "A"; break;
        default:
            // karta o wartości liczbowej
            strumien << k.figura + 1; break;
        }

        // wyświetlanie koloru karty
        switch (k.kolor)
        {
        case TREFL:
            strumien << "c"; break;
        case KARO:
            strumien << "d"; break;
        case KIER:
            strumien << "h"; break;
        case PIK:
            strumien << "s"; break;
        }
    }

    return strumien;
}
istream& operator >>(istream& strumien, Karta& t)
{
    // operator wczytuje kartę ze strumienia, lub wyrzuca wyjątek
    // wczytywanie danych ze strumienia
    string bufor;
    strumien >> bufor;

    // ustawianie koloru karty
    switch (bufor[bufor.size() - 1])
    {
    case 'c':
        t.kolor = TREFL; break;
    case 'd':
        t.kolor = KARO; break;
    case 'h':
        t.kolor = KIER; break;
    case 's':
        t.kolor = PIK; break;
    default:
        throw string("Niepoprawny format karty"); break;
    }

    // ustawianie figury karty
    bufor = bufor.substr(0, bufor.size() - 1);
    if (bufor == "J")
        t.figura = WALET;
    else if (bufor == "Q")
        t.figura = DAMA;
    else if (bufor == "K")
        t.figura = KROL;
    else if (bufor == "A")
        t.figura = AS;
    else if (bufor == "10")
        t.figura = DZIESIEC;
    else if (bufor.size() == 1 && bufor[0] - '0' >= 2 && bufor[0] - '0' < 10)
        t.figura = FiguraTyp(bufor[0] - '0' - 1);
    else throw string("Niepoprawny format karty");

    t.ukryta = false;

    return strumien;
}

Talia::Talia()
{
    // konstruktor generuje i tasuje talię
    // tworzenie tymczasowego wektora kart celem i jego potasowanie
    vector <Karta> bufor;
    for (unsigned int i = TREFL; i <= PIK; i++)
        for (unsigned int j = AS; j <= KROL; j++)
            bufor.push_back(Karta(KolorTyp(i), FiguraTyp(j)));

    srand(time(NULL));
    random_shuffle(bufor.begin(), bufor.end());

    // kopiowanie zawartości bufora do kolejki
    for (vector<Karta>::iterator it = bufor.begin(); it != bufor.end(); it++)
        push(*it);
}

void Talia::odswiezWidok()
{
    // funkcja odświeża "widok" talii
    // przełożenie widocznych kart na koniec talii
    for (vector<Karta>::iterator it = widok.begin(); it != widok.end(); it++)
        push(*it);
    widok.clear();

    // uzupełnienie widoku nowymi kartami
    for (unsigned int i = 0; i < ILE_WIDOK && i < size(); i++)
    {
        widok.push_back(front());
        pop();
    }
}

Karta Talia::pierwsza()
{
    // funkcja zwraca pierwszą widoczną kartę z talii
    if (widok.empty())
        throw  string("Usilowano zdjac z talii niewidoczna karte");
    else return widok[widok.size() - 1];
}

void Talia::zdejmij()
{
    // funkcja zdejmuje kartę z tali
    if (widok.empty())
        throw string("Usilowano zdjac z talii niewidoczna karte");

    widok.erase(widok.end());
}

ostream& operator <<(ostream& strumien, const Talia t)
{
    // operator wyświetla talię gracza
    for(unsigned int i = 0; i < ILE_WIDOK ; i++)
        if(i < t.widok.size())
            strumien << setw(3) << t.widok[i];
        else strumien << string(4, ' ');

    return strumien;
}

vector<vector<Karta> > initKolumny(Talia& t)
{
	// funkcja przenosi karty z potasowanej talii do kolumn
	const unsigned int ILE_KOLUMN = 7;
	vector<vector<Karta> > kolumny(ILE_KOLUMN);

	for (unsigned int i = 0; i < ILE_KOLUMN; i++)
		for (unsigned int j = 0; j <= i; j++)
		{
			kolumny[i].push_back(t.front());
			t.pop();

			// ukrywanie niektórych kart
			if (j != i)
				kolumny[i][j].ukryj();
		}

	return kolumny;
}

bool rozmiarCmp(const vector<Karta> a, const vector<Karta> b)
{
	// funkcja porównuje rozmiary wektorów
	return a.size() < b.size();
}

ostream& operator <<(ostream& strumien, vector<stack<Karta> > stosy)
{
    // operator wyświetla wektor stosów gracza
    for(vector<stack<Karta> >::iterator it = stosy.begin(); it != stosy.end(); it++)
        if(!it->empty())
            strumien << setw(3) << it->top();
        else strumien << string(4, ' ');

    return strumien;
}

void wyswietl(Talia talia, vector<vector<Karta> > kolumny,
	vector<stack<Karta> > stosy)
{
	// funkcja wyświetla aktualny układ kart gracza
	// wyświetlanie kart talii i stosów
	cout << string(6, ' ') << '0' << string(34, ' ');
	for(unsigned int i = 0; i < stosy.size(); i++)
        cout << setw(4) << i + 8;
    cout << endl;

    cout << string(57, '-') << endl << talia << string(29, ' ') << stosy << endl
         << string(57, '-') << endl;

    // ustalenie ilości kart w największej kolumnie
    unsigned int rozmiarMax = max_element(kolumny.begin(), kolumny.end(), rozmiarCmp)->size();

    // wyświetlanie kolumn kart gracza
    for(unsigned int i = 1; i <= rozmiarMax; i++)
    {
        for(unsigned int j = 0; j < kolumny.size(); j++)
        {
            cout << '|';

            if (kolumny[j].size() >= i)
                cout << "  " << setw(2) << kolumny[j][i - 1] << "  ";
            else cout << string(7, ' ');
        }
        cout << '|' << endl;
    }

    // wyświetlanie numerów kolumn
    for(unsigned int i = 1; i <= kolumny.size(); i++)
        cout << string((i == 1) ? 4 : 7, ' ') << i;
    cout << endl;
}

void wykPolecenie(string polecenie, Talia& talia,
	vector<vector<Karta> >& kolumny, vector<stack<Karta> >& stosy)
{
    // funckja wykonuje polecenie dane jako ciąg znaków odpowiednio modyfikując
    // stan gry, lub wyrzuca wyjątek
	if (polecenie == "0")
		talia.odswiezWidok();
	else
	{
		istringstream strumien(polecenie);
		int skad, dokad;
		Karta jaka;

		strumien >> skad >> jaka >> dokad;
		if (skad == dokad || skad > 11 || dokad > 11)
			throw string("Niepoprawne polecenie");

        if(skad == 0)
        {
            if(talia.pierwsza() != jaka)
                throw string("Usilowano zabrac niepoprawna karte z talii");

            if(dokad >= 1 && dokad <= 7)
                kartaDoKol(talia.pierwsza(), kolumny[dokad - 1]);
            else if(dokad >= 8 && dokad <= 11)
                kartaDoStosu(talia.pierwsza(), stosy[dokad - 8]);

            talia.zdejmij();
        }
		else if (skad >= 8 && skad <= 11)
		{
		    if(stosy[skad - 8].empty() || stosy[skad - 8].top() != jaka)
                throw string("Usilowano zabrac niepoporawna karte ze stosu");

			if(dokad >= 1 && dokad <= 7)
                kartaDoKol(stosy[skad - 8].top(), kolumny[dokad - 1]);
            else if(dokad >= 8 && dokad <= 11)
                kartaDoStosu(stosy[skad - 8].top(), stosy[dokad - 8]);

			stosy[skad - 8].pop();
		}
		else if (skad >= 1 && skad <= 7 && dokad >= 1 && dokad <= 11) // z kolumn do innych kolumn lub stosów
		{
			vector<Karta>::iterator it = find(kolumny[skad - 1].begin(),
				kolumny[skad - 1].end(), jaka);
			if (it == kolumny[skad - 1].end() || it->czyUkryta())
				throw string("Usilowano zabrac niepoprawna karte z kolumny");

			// kopiowanie kolejnych kart
			for (; it != kolumny[skad - 1].end(); it++)
				if (dokad >= 1 && dokad <= 7)
					kartaDoKol(*it, kolumny[dokad - 1]);
				else if (dokad >= 8 && dokad <= 11)
					kartaDoStosu(*it, stosy[dokad - 8]);

			// czyszczenie pobranych kart z kolumny
			kolumny[skad - 1].erase(find(kolumny[skad - 1].begin(),
				kolumny[skad - 1].end(), jaka), kolumny[skad - 1].end());

			// odkrywanie kolejnego elementu kolumny
			if (!kolumny[skad - 1].empty())
				kolumny[skad - 1][kolumny[skad - 1].size() - 1].odkryj();
		}
	}
}

bool koniecGry(vector<stack<Karta> > stosy)
{
	// funkcja sygnalizuje poprawne ułożenie kart na stosach
	for (vector<stack<Karta> >::iterator it = stosy.begin(); it != stosy.end(); it++)
		if (it->empty() || it->top().jakaFigura() != KROL) return false;
	return true;
}


void kartaDoKol(Karta karta, vector<Karta>& kolumna)
{
    // funkcja dodaje wskazaną kartę do wskazanej kolumny, lub wyrzuca wyjątek
	if ((kolumna.empty() && karta.jakaFigura() == KROL) ||
		(kolumna[kolumna.size() - 1].jakaFigura() - karta.jakaFigura() == 1 &&
        czerwona(kolumna[kolumna.size() - 1].jakiKolor()) != czerwona(karta.jakiKolor()) ))
		kolumna.push_back(karta);
	else throw string("Usilowano dodac niepoprawna karte do kolumny");
}

void kartaDoStosu(Karta karta, stack<Karta>& stos)
{
    // funkcja dodaje wskazaną kartę do wskazanego stosu, lub wyrzuca wyjątek
	if ((stos.empty() && karta.jakaFigura() == AS) ||
        (!stos.empty() && karta.jakaFigura() - stos.top().jakaFigura() == 1 &&
		karta.jakiKolor() == stos.top().jakiKolor() ))
		stos.push(karta);
	else throw string("Usilowano dodac niepoprawna karte do stosu");
}
