#include <iostream>
#include <cstring>
using namespace std;

struct ComandaHistorica {
    char fecha[11]; // "DD-MM-AAAA"
    char nombreMozo[50]; // el nombre completo, repetido en cada venta
    int codigoProducto; 
    int cantidad; 
    float comision;
};

struct Producto {
    int codigo; 
    char descripcion[50]; 
    float precio; 
    int stockActual;
};

struct Mozo { 
    int idMozo; 
    char nombre[50]; 
    char password[20]; 
    float totalComision; 
};

struct Comanda { 
    int idMozo; 
    int codigoProducto; 
    int cantidad; 
    float comision;
};

void normalizarMozos() {

    FILE* archivoComandas = fopen("comandas_historicas.dat", "rb");

    FILE* archivoMozos = fopen("mozos.dat", "rb+");
    if (!archivoMozos) archivoMozos = fopen("mozos.dat", "wb+");

    int mozoID = 1;
    ComandaHistorica comandaHistorica;
    Mozo mozo;
    bool encontrado;

    while (fread(&comandaHistorica, sizeof(ComandaHistorica), 1, archivoComandas) == 1) {

        encontrado = false;
        rewind(archivoMozos);

        while (fread(&mozo, sizeof(Mozo), 1, archivoMozos) == 1) {

            if (strcmp(comandaHistorica.nombreMozo, mozo.nombre) == 0) {

                mozo.totalComision += comandaHistorica.comision;

                fseek(archivoMozos, -(long)sizeof(Mozo), SEEK_CUR);
                fwrite(&mozo, sizeof(Mozo), 1, archivoMozos);

                encontrado = true;
                break;
            }
        }

        if (!encontrado) {
            Mozo nuevo;
            fseek(archivoMozos, 0, SEEK_END);

            nuevo.idMozo = mozoID;
            mozoID++;

            strcpy(nuevo.nombre, comandaHistorica.nombreMozo);
            strcpy(nuevo.password, "default");
            nuevo.totalComision = comandaHistorica.comision;

            fwrite(&nuevo, sizeof(Mozo), 1, archivoMozos);
        }
    }

    fclose(archivoComandas);
    fclose(archivoMozos);
}


int main() {
    normalizarMozos();
    Mozo mozoPrueba;
    FILE* archivoMozos = fopen("mozos.dat", "rb");
    while (fread(&mozoPrueba, sizeof(Mozo), 1, archivoMozos) == 1) {
        cout << "ID: " << mozoPrueba.idMozo << ", Nombre: " << mozoPrueba.nombre << ", Comision: " << mozoPrueba.totalComision << endl;
    }
    fclose(archivoMozos);
    return 0;
}