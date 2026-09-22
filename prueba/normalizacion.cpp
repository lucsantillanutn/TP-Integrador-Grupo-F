#include <iostream>
#include <cstring>
using namespace std;

struct ComandaHistorica {
    char fecha[11];
    char nombreMozo[50];
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

    if (archivoComandas == NULL) {
        cout << "No se pudo abrir el archivo de comandas." << endl;
        return;
    }

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
            Mozo nuevoMozo;
            fseek(archivoMozos, 0, SEEK_END);

            nuevoMozo.idMozo = mozoID;
            mozoID++;

            strcpy(nuevoMozo.nombre, comandaHistorica.nombreMozo);
            strcpy(nuevoMozo.password, "default");
            nuevoMozo.totalComision = comandaHistorica.comision;

            fwrite(&nuevoMozo, sizeof(Mozo), 1, archivoMozos);
        }
    }

    fclose(archivoComandas);
    fclose(archivoMozos);
}

void descontarStockViejo() {

    FILE* archivoComandas = fopen("comandas_historicas.dat", "rb");

    if (archivoComandas == NULL) {
        cout << "No se pudo abrir el archivo de comandas." << endl;
        return;
    }

    FILE* archivoInventario = fopen("inventario.dat", "rb+");

    if (archivoInventario == NULL) {
        cout << "No se pudo abrir el archivo del inventario." << endl;
        return;
    }

    ComandaHistorica comandaHistorica;

    while(fread(&comandaHistorica, sizeof(ComandaHistorica), 1, archivoComandas) == 1) {

        int codigoProducto = comandaHistorica.codigoProducto;

        fseek(archivoInventario, 0, SEEK_END);
        long cantidadProductos = ftell(archivoInventario) / sizeof(Producto);
        long primero = 0, ultimo = cantidadProductos - 1;
        bool encontrado = false;
        Producto producto;
        
        while (primero <= ultimo && !encontrado) {
            long medio = (primero + ultimo) / 2;
            fseek(archivoInventario, medio * sizeof(Producto), SEEK_SET);
            if(fread(&producto, sizeof(Producto), 1, archivoInventario) != 1) {
                break;
            }
            if (producto.codigo == codigoProducto) {
                encontrado = true;
                if(producto.stockActual > 0) {
                    producto.stockActual -= comandaHistorica.cantidad;
                    fseek(archivoInventario, -(long)sizeof(Producto), SEEK_CUR);
                    fwrite(&producto, sizeof(Producto), 1, archivoInventario);
                }
            }
            else if (codigoProducto > producto.codigo) primero = medio + 1;
            else ultimo = medio - 1;
        }
    }
    fclose(archivoComandas);
    fclose(archivoInventario);
}

int main() {
    //PRUEBAS PARA VER SI NORMALIZA BIEN A LOS MOZOS
    normalizarMozos();
    Mozo mozoPrueba;
    FILE* archivoMozos = fopen("mozos.dat", "rb");
    while (fread(&mozoPrueba, sizeof(Mozo), 1, archivoMozos) == 1) {
        cout << "ID: " << mozoPrueba.idMozo << ", Nombre: " << mozoPrueba.nombre << ", Comision: " << mozoPrueba.totalComision << endl;
    }
    fclose(archivoMozos);

    //PRUEBAS PARA VER SI NORMALIZA EL INVENTARIO
    descontarStockViejo();
    Producto producto;
    FILE* archivoInventario = fopen("inventario.dat", "rb");
    while (fread(&producto, sizeof(Producto), 1, archivoInventario) == 1) {
        cout << "Codigo: " << producto.codigo << ", Stock: " << producto.stockActual << endl;
    }
    fclose(archivoInventario);
}