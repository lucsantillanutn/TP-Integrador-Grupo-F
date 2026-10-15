#include <iostream>
#include <cstdio>
using namespace std;

struct Mozo {
    int idMozo;
    char nombre[50];
    char password[20];
    float totalComision;
};

struct Comanda {
    int idMozo;
    int codigoproducto;
    int cantidad;
    float comision;
};

void generarResumen(const char* archivo) {
    FILE* f = fopen(archivo, "rb");
    if (f == NULL) {
        cout << "Error: No se pudo abrir el archivo " << archivo << "\n";
        return;
    }

    Comanda c;
    int ventasBuffet = 0;
    int leido = fread(&c, sizeof(Comanda), 1, f);

    while (leido == 1) {
        int idActual = c.idMozo;
        int ventasMozo = 0;
        float comisionTotal = 0;
        
        while (leido == 1 && idActual == c.idMozo) {
            comisionTotal += c.comision;
            ventasMozo += c.cantidad;
            leido = fread(&c, sizeof(Comanda), 1, f);
        }
        
        ventasBuffet += ventasMozo;
        cout << "Vendedor N°: " << idActual 
             << " Vendió un total de: " << ventasMozo 
             << " unidades. Comisionando: $" << comisionTotal << endl;
    }
    
    cout << "El buffet vendió un total de " << ventasBuffet << " unidades." << endl;
    fclose(f);
}

int main() {
    char nombreArchivo[50];

    cout << "Ingrese el nombre del archivo semanal (ej. comandas_semana.dat): ";
    cin >> nombreArchivo;

    generarResumen(nombreArchivo);

    return 0;
}
