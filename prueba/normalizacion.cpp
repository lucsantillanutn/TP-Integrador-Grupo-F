#include <iostream>
#include <cstring>
#include <array>
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

int obtenerIdMozo(const char nombreMozo[]) {

    FILE* archivoMozos = fopen("mozos.dat", "rb");

    if(archivoMozos == NULL) {
        return -1;
    }

    Mozo mozo;

    while(fread(&mozo, sizeof(Mozo), 1, archivoMozos) == 1) {

        if(strcmp(mozo.nombre, nombreMozo) == 0) {
            fclose(archivoMozos);
            return mozo.idMozo;
        }
    }

    fclose(archivoMozos);
    return -1;
}

void ordenarPorMozo(FILE* archivoDia) {

    ComandaHistorica c1;
    ComandaHistorica c2;

    fseek(archivoDia, 0, SEEK_END);
    long cantidad = ftell(archivoDia) / sizeof(ComandaHistorica);

    for(long i = 0; i < cantidad - 1; i++) {

        for(long j = 0; j < cantidad - i - 1; j++) {

            // Ir al registro j
            fseek(archivoDia, j * sizeof(ComandaHistorica), SEEK_SET);
            fread(&c1, sizeof(ComandaHistorica), 1, archivoDia);

            // Leer el registro j + 1
            fread(&c2, sizeof(ComandaHistorica), 1, archivoDia);

            int idMozo1 = obtenerIdMozo(c1.nombreMozo);
            int idMozo2 = obtenerIdMozo(c2.nombreMozo);

            if(idMozo1 > idMozo2) {

                // Volver al registro j
                fseek(archivoDia, j * sizeof(ComandaHistorica), SEEK_SET);

                // Escribirlos intercambiados
                fwrite(&c2, sizeof(ComandaHistorica), 1, archivoDia);
                fwrite(&c1, sizeof(ComandaHistorica), 1, archivoDia);
            }
        }
    }
}

void separarVentasPorDia() {

    FILE* archivoComandas = fopen("comandas_historicas.dat", "rb");

    if (archivoComandas == NULL) {
        cout << "No se pudo abrir el archivo de comandas." << endl;
        return;
    }

    ComandaHistorica comandaHistorica;
    string nombreArchivo;
    char fechaEnProceso[11];
    
    while(fread(&comandaHistorica, sizeof(ComandaHistorica), 1, archivoComandas) == 1) {
    
        nombreArchivo = string("comandas_") + comandaHistorica.fecha + ".dat";
        FILE* archivoDia = fopen(nombreArchivo.c_str(), "rb");

        if(archivoDia == NULL) {

            archivoDia = fopen(nombreArchivo.c_str(), "wb");
            strcpy(fechaEnProceso, comandaHistorica.fecha);

            long posicion = ftell(archivoComandas);
            fseek(archivoComandas, 0, SEEK_SET);

            while(fread(&comandaHistorica, sizeof(ComandaHistorica), 1, archivoComandas) == 1) {
                if(strcmp(comandaHistorica.fecha, fechaEnProceso) == 0) {
                    fwrite(&comandaHistorica, sizeof(ComandaHistorica), 1, archivoDia);
                }
            }
            
            fseek(archivoComandas, posicion, SEEK_SET);

        }

        fclose(archivoDia);
        archivoDia = fopen(nombreArchivo.c_str(), "rb+");
    
        if(archivoDia != NULL) {
            ordenarPorMozo(archivoDia);
            fclose(archivoDia);
        }
    }

    fclose(archivoComandas);
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
    cout << "Normalizacion de mozos completada." << endl;
    cout << endl;

    //PRUEBAS PARA VER SI NORMALIZA EL INVENTARIO
    descontarStockViejo();
    Producto producto;
    FILE* archivoInventario = fopen("inventario.dat", "rb");
    while (fread(&producto, sizeof(Producto), 1, archivoInventario) == 1) {
        cout << "Codigo: " << producto.codigo << ", Stock: " << producto.stockActual << endl;
    }
    fclose(archivoInventario);
    cout << "Descuento de stock completado." << endl;
    cout << endl;

    //PRUEBAS PARA VER SI SEPARA BIEN LAS VENTAS POR DIA
    separarVentasPorDia();
    ComandaHistorica comandaPrueba;
    FILE* archivoDia = fopen("comandas_02-06-2025.dat", "rb");
    while (fread(&comandaPrueba, sizeof(ComandaHistorica), 1, archivoDia) == 1) {
        cout << "Fecha: " << comandaPrueba.fecha << ", Nombre Mozo: " << comandaPrueba.nombreMozo << ", Codigo Producto: " << comandaPrueba.codigoProducto << ", Cantidad: " << comandaPrueba.cantidad << ", Comision: " << comandaPrueba.comision << endl;
    }
    cout << "--------------------------------" << endl;
    archivoDia = fopen("comandas_03-06-2025.dat", "rb");
    while (fread(&comandaPrueba, sizeof(ComandaHistorica), 1, archivoDia) == 1) {
        cout << "Fecha: " << comandaPrueba.fecha << ", Nombre Mozo: " << comandaPrueba.nombreMozo << ", Codigo Producto: " << comandaPrueba.codigoProducto << ", Cantidad: " << comandaPrueba.cantidad << ", Comision: " << comandaPrueba.comision << endl;
    }
    cout << "--------------------------------" << endl;
    archivoDia = fopen("comandas_04-06-2025.dat", "rb");
    while (fread(&comandaPrueba, sizeof(ComandaHistorica), 1, archivoDia) == 1) {
        cout << "Fecha: " << comandaPrueba.fecha << ", Nombre Mozo: " << comandaPrueba.nombreMozo << ", Codigo Producto: " << comandaPrueba.codigoProducto << ", Cantidad: " << comandaPrueba.cantidad << ", Comision: " << comandaPrueba.comision << endl;
    }
    cout << "--------------------------------" << endl;
    archivoDia = fopen("comandas_05-06-2025.dat", "rb");
    while (fread(&comandaPrueba, sizeof(ComandaHistorica), 1, archivoDia) == 1) {
        cout << "Fecha: " << comandaPrueba.fecha << ", Nombre Mozo: " << comandaPrueba.nombreMozo << ", Codigo Producto: " << comandaPrueba.codigoProducto << ", Cantidad: " << comandaPrueba.cantidad << ", Comision: " << comandaPrueba.comision << endl;
    }
    cout << "--------------------------------" << endl;
    archivoDia = fopen("comandas_06-06-2025.dat", "rb");
    while (fread(&comandaPrueba, sizeof(ComandaHistorica), 1, archivoDia) == 1) {
        cout << "Fecha: " << comandaPrueba.fecha << ", Nombre Mozo: " << comandaPrueba.nombreMozo << ", Codigo Producto: " << comandaPrueba.codigoProducto << ", Cantidad: " << comandaPrueba.cantidad << ", Comision: " << comandaPrueba.comision << endl;
    }
    cout << "--------------------------------" << endl;
    archivoDia = fopen("comandas_07-06-2025.dat", "rb");
    while (fread(&comandaPrueba, sizeof(ComandaHistorica), 1, archivoDia) == 1) {
        cout << "Fecha: " << comandaPrueba.fecha << ", Nombre Mozo: " << comandaPrueba.nombreMozo << ", Codigo Producto: " << comandaPrueba.codigoProducto << ", Cantidad: " << comandaPrueba.cantidad << ", Comision: " << comandaPrueba.comision << endl;
    }
    fclose(archivoDia);
    cout << "Separacion de ventas por dia completada." << endl;

    return 0;
}