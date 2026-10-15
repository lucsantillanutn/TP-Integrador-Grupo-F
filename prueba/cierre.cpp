

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

int main() {
    int semana;
    cout << "Numero de semana: ";
    cin >> semana;

    int dd=0;
    int mm=0;
    int aaaa=0;
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

    for (int i = 0; i < DIAS_SEMANA; i++) {
        char nombreDia[40];
        sprintf(nombreDia, "comandas_%02d-%02d-%04d.dat", dd, mm, aaaa);

        FILE *f = fopen(nombreDia, "rb");
        if (f == NULL) {
            cout << nombreDia << ": no existe, se saltea" << endl;
        } else {
            int n = cantidadRegistros(nombreDia);
            cout << nombreDia << ": " << n << " ventas" << endl;
            dias[cantDias] = f;
            cantDias++;
        }
        siguienteDia(dd, mm, aaaa);
    }

    cout << "Se encontraron " << cantDias << " planillas. Archivo a generar: " << nombreSemana << endl;
    for (int i = 0; i < cantDias; i++) fclose(dias[i]);
    return 0;
}
