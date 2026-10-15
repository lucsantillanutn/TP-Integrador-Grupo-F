

#include <iostream>
#include <cstdio>
using namespace std;

struct Comanda {
    int   idMozo;
    int   codigoProducto;
    int   cantidad;
    float comision;
};

const int DIAS_SEMANA = 7;


int diasDelMes(int mm) {
    int dias[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return dias[mm - 1];
}

bool fechaValida(int dd, int mm, int aaaa) {
    if (aaaa < 2000 || mm < 1 || mm > 12) return false;
    return dd >= 1 && dd <= diasDelMes(mm);
}

void siguienteDia(int &dd, int &mm, int &aaaa) {
    dd++;
    if (dd > diasDelMes(mm)) {
        dd = 1;
        mm++;
        if (mm > 12) {
            mm = 1;
            aaaa++;
        }
    }
}

int cantidadRegistros(const char NombreArchivo[]) {
    FILE *f = fopen(NombreArchivo, "rb");
    if (f == NULL) return 0;
    fseek(f, 0, SEEK_END);
    int n = ftell(f) / sizeof(Comanda);
    fclose(f);
    return n;
}

bool estaOrdenada(const char NombreArchivo[]) {
    FILE *f = fopen(NombreArchivo, "rb");
    if (f == NULL) return true;

    Comanda c;
    int anterior = -1;
    bool ok = true;
    while (ok && fread(&c, sizeof(Comanda), 1, f) == 1) {
        if (c.idMozo < anterior) ok = false;
        anterior = c.idMozo;
    }
    fclose(f);
    return ok;
}

int aparear(FILE *dias[], int cantDias, FILE *fSemana) {
    Comanda actual[DIAS_SEMANA];
    bool hay[DIAS_SEMANA];

    for (int i = 0; i < cantDias; i++) {
        hay[i] = (fread(&actual[i], sizeof(Comanda), 1, dias[i]) == 1);
    }

    int escritos = 0;
    bool quedan = true;
    while (quedan) {
        int menor = -1;
        for (int i = 0; i < cantDias; i++) {
            if (hay[i] && (menor == -1 || actual[i].idMozo < actual[menor].idMozo)) {
                menor = i;
            }
        }

        if (menor == -1) {
            quedan = false;
        } else {
            fwrite(&actual[menor], sizeof(Comanda), 1, fSemana);
            escritos++;
            hay[menor] = (fread(&actual[menor], sizeof(Comanda), 1, dias[menor]) == 1);
        }
    }
    return escritos;
}

int main() {
    int semana;
    cout << "Numero de semana: ";
    cin >> semana;

    int dd = 0;
    int mm = 0;
    int aaaa = 0;
    while (!fechaValida(dd, mm, aaaa)) {
        cout << "Fecha del primer dia de la semana (dd mm aaaa): ";
        cin >> dd; 
        cin >> mm; 
        cin >> aaaa;
        if (!fechaValida(dd, mm, aaaa)){ 
            cout << "Fecha invalida." << endl;
        }
    } 

    char nombreSemana[40];
    sprintf(nombreSemana, "comandas_semana_s%d-%02d.dat", semana, mm);

    FILE *dias[DIAS_SEMANA];
    int cantDias = 0;
    int totalEsperado = 0;
    bool todoOrdenado = true;

    for (int i = 0; i < DIAS_SEMANA; i++) {
        char nombreDia[40];
        sprintf(nombreDia, "comandas_%02d-%02d-%04d.dat", dd, mm, aaaa);

        FILE *f = fopen(nombreDia, "rb");
        if (f == NULL) {
            cout << nombreDia << ": no existe, se saltea" << endl;
        } else {
            int n = cantidadRegistros(nombreDia);
            cout << nombreDia << ": " << n << " ventas" << endl;
            if (!estaOrdenada(nombreDia)) {
                cout << "ATENCION: " << nombreDia << " no esta ordenada por mozo" << endl;
                todoOrdenado = false;
            }
            dias[cantDias] = f;
            cantDias++;
            totalEsperado += n;
        }
        siguienteDia(dd, mm, aaaa);
    }

    if (cantDias == 0) {
        cout << "No hay planillas para esa semana. No se genera " << nombreSemana << endl;
        return 1;
    }
    if (!todoOrdenado) {
        cout << "Hay planillas desordenadas. No se genera el cierre." << endl;
        for (int i = 0; i < cantDias; i++) fclose(dias[i]);
        return 1;
    }

    FILE *fSemana = fopen(nombreSemana, "wb");
    if (fSemana == NULL) {
        cout << "No se pudo crear " << nombreSemana << endl;
        for (int i = 0; i < cantDias; i++) fclose(dias[i]);
        return 1;
    }

    int escritos = aparear(dias, cantDias, fSemana);

    fclose(fSemana);
    for (int i = 0; i < cantDias; i++) fclose(dias[i]);

    cout << nombreSemana << " generado: " << escritos << " ventas de " << cantDias << " dias, ordenadas por mozo." << endl;
    if (escritos != totalEsperado) {
        cout << "ATENCION: se esperaban " << totalEsperado << " ventas." << endl;
    }
    return 0;
}
