#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	int cod;
	char* denumire;
	float profit;
	char* adresa;
	int nrClienti;
} Agentie;

typedef struct Nod {
	Agentie info;
	struct Nod* next;
} Nod;

typedef struct Grup {
	char categorie[16];
	Nod* agentii;
	struct Grup* next;
} Grup;

char* copiereSir(const char* s) {
	char* r = (char*)malloc(strlen(s) + 1);
	strcpy(r, s);
	return r;
}

Agentie copiereAgentie(Agentie a) {
	Agentie c = a;
	c.denumire = copiereSir(a.denumire);
	c.adresa = copiereSir(a.adresa);
	return c;
}

void eliberareAgentie(Agentie* a) {
	free(a->denumire);
	free(a->adresa);
}

Nod* inserareFinal(Nod* cap, Agentie a) {
	Nod* nou = (Nod*)malloc(sizeof(Nod));
	nou->info = copiereAgentie(a);
	nou->next = NULL;
	if (cap == NULL) {
		return nou;
	}
	Nod* t = cap;
	while (t->next != NULL) {
		t = t->next;
	}
	t->next = nou;
	return cap;
}

void scriereAgentie(FILE* out, Agentie a) {
	fprintf(out, "Cod: %d, Denumire: %s, Profit: %.2f, Adresa: %s, Clienti: %d\n",
		a.cod, a.denumire, a.profit, a.adresa, a.nrClienti);
}

void scriereLista(FILE* out, Nod* cap) {
	for (Nod* t = cap; t != NULL; t = t->next) {
		scriereAgentie(out, t->info);
	}
}

Nod* citireFisier(const char* numeFisier) {
	FILE* f = fopen(numeFisier, "r");
	if (f == NULL) {
		printf("Fisierul %s nu a putut fi deschis.\n", numeFisier);
		return NULL;
	}
	Nod* cap = NULL;
	Agentie a;
	char denumire[100];
	char adresa[100];
	while (fscanf(f, "%d %99s %f %99s %d", &a.cod, denumire, &a.profit, adresa, &a.nrClienti) == 5) {
		a.denumire = denumire;
		a.adresa = adresa;
		cap = inserareFinal(cap, a);
	}
	fclose(f);
	return cap;
}

void stergereCoduriImpare(Nod** cap, const char* numeFisier) {
	FILE* f = fopen(numeFisier, "w");
	Nod* t = *cap;
	Nod* anterior = NULL;
	while (t != NULL) {
		Nod* urm = t->next;
		if (t->info.cod % 2 != 0) {
			if (anterior == NULL) {
				*cap = urm;
			}
			else {
				anterior->next = urm;
			}
			eliberareAgentie(&t->info);
			free(t);
		}
		else {
			if (f != NULL) {
				scriereAgentie(f, t->info);
			}
			anterior = t;
		}
		t = urm;
	}
	if (f != NULL) {
		fclose(f);
	}
}

const char* categorieClienti(int nrClienti) {
	if (nrClienti < 1000) {
		return "mica";
	}
	if (nrClienti < 5000) {
		return "medie";
	}
	return "mare";
}

Grup* grupareDupaClienti(Nod* cap) {
	Grup* grupuri = NULL;
	for (Nod* t = cap; t != NULL; t = t->next) {
		const char* cat = categorieClienti(t->info.nrClienti);
		Grup* g = grupuri;
		while (g != NULL && strcmp(g->categorie, cat) != 0) {
			g = g->next;
		}
		if (g == NULL) {
			g = (Grup*)malloc(sizeof(Grup));
			strcpy(g->categorie, cat);
			g->agentii = NULL;
			g->next = grupuri;
			grupuri = g;
		}
		g->agentii = inserareFinal(g->agentii, t->info);
	}
	return grupuri;
}

void afisareGrupuri(FILE* out, Grup* grupuri) {
	for (Grup* g = grupuri; g != NULL; g = g->next) {
		fprintf(out, "Agentii cu numar de clienti %s:\n", g->categorie);
		scriereLista(out, g->agentii);
	}
}

void dezalocareLista(Nod** cap) {
	while (*cap != NULL) {
		Nod* urm = (*cap)->next;
		eliberareAgentie(&(*cap)->info);
		free(*cap);
		*cap = urm;
	}
}

void salvareSiDezalocareLista(Nod** cap, const char* numeFisier) {
	FILE* f = fopen(numeFisier, "w");
	if (f != NULL) {
		scriereLista(f, *cap);
		fclose(f);
	}
	dezalocareLista(cap);
}

void salvareSiDezalocareGrupuri(Grup** grupuri, const char* numeFisier) {
	FILE* f = fopen(numeFisier, "w");
	if (f != NULL) {
		afisareGrupuri(f, *grupuri);
		fclose(f);
	}
	while (*grupuri != NULL) {
		Grup* urm = (*grupuri)->next;
		dezalocareLista(&(*grupuri)->agentii);
		free(*grupuri);
		*grupuri = urm;
	}
}

int main() {
	Nod* cap = citireFisier("agentii.txt");

	printf("Agentiile bancii:\n");
	scriereLista(stdout, cap);

	stergereCoduriImpare(&cap, "agentii_pare.txt");
	printf("\nDupa stergerea agentiilor cu cod impar (salvate in agentii_pare.txt):\n");
	scriereLista(stdout, cap);

	Grup* grupuri = grupareDupaClienti(cap);
	printf("\nAgentii grupate dupa numarul de clienti:\n");
	afisareGrupuri(stdout, grupuri);

	salvareSiDezalocareGrupuri(&grupuri, "grupuri_dezalocate.txt");
	salvareSiDezalocareLista(&cap, "lista_dezalocata.txt");
	printf("\nStructurile au fost salvate in fisiere si dezalocate.\n");
	return 0;
}
