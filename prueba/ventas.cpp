#include <iostream>
#include <cstdio>
#include <cstring>
using namespace std;

struct Producto {
    int   codigo;
    char  descripcion[50];
    float precio;
    int   stockActual;
};

struct Mozo {
    int   idMozo;
    char  nombre[50];
    char  password[20];
    float totalComision;
};

struct Comanda {
    int   idMozo;
    int   codigoProducto;
    int   cantidad;
    float comision;
};

const float TASA_COMISION = 0.10f;
const int   K = 5;   
const int   MAX_TANDA = 1000;   

int cantidadRegistros(const char nomArch[], int tamRegistro) {
    FILE *f = fopen(nomArch, "rb");
    if (f == NULL) return 0;
    fseek(f, 0, SEEK_END);
    int n = ftell(f) / tamRegistro;
    fclose(f);
    return n;
}

void encriptar(const char clave[], char resultado[]) {
    int i = 0;
    while (clave[i] != '\0') {
        resultado[i] = clave[i] + K;
        i++;
    }
    resultado[i] = '\0';
}

bool buscarMozo(int id, Mozo &m) {
    int cantMozos = cantidadRegistros("mozos.dat", sizeof(Mozo));
    if (id < 1 || id > cantMozos) return false;  

    FILE *f = fopen("mozos.dat", "rb");
    if (f == NULL) return false;

    fseek(f, (id - 1) * sizeof(Mozo), SEEK_SET);
    bool ok = (fread(&m, sizeof(Mozo), 1, f) == 1 && m.idMozo == id);
    fclose(f);
    return ok;
}

bool claveCorrecta(const Mozo &m, const char claveTipeada[]) {
    char encriptada[20];
    encriptar(claveTipeada, encriptada);
    return strcmp(encriptada, m.password) == 0;
}

int buscarProducto(FILE *fInv, int codigo, Producto &p) {
    fseek(fInv, 0, SEEK_END);
    int n = ftell(fInv) / sizeof(Producto);

    int desde = 0, hasta = n - 1;
    while (desde <= hasta) {
        int medio = (desde + hasta) / 2;
        fseek(fInv, medio * sizeof(Producto), SEEK_SET);
        fread(&p, sizeof(Producto), 1, fInv);

        if (p.codigo == codigo) return medio;
        if (p.codigo < codigo) desde = medio + 1;
        else                   hasta = medio - 1;
    }
    return -1;
}

void actualizarStock(FILE *fInv, int pos, Producto &p) {
    fseek(fInv, pos * sizeof(Producto), SEEK_SET);
    fwrite(&p, sizeof(Producto), 1, fInv);
    fflush(fInv);   
}

void ordenarTanda(Comanda v[], int n) {
    for (int i = 1; i < n; i++) {
        Comanda aux = v[i];
        int j = i - 1;
        while (j >= 0 && v[j].idMozo > aux.idMozo) {   
            v[j + 1] = v[j];
            j--;
        }
        v[j + 1] = aux;
    }
}

void dejarOrdenadoPorMozo(const char nomArch[], int nViejos) {
    int nTotal = cantidadRegistros(nomArch, sizeof(Comanda));
    int nNuevos = nTotal - nViejos;
    if (nNuevos == 0) return;   
    FILE *f = fopen(nomArch, "rb");

    Comanda nuevos[MAX_TANDA];
    fseek(f, nViejos * sizeof(Comanda), SEEK_SET);
    fread(nuevos, sizeof(Comanda), nNuevos, f);
    ordenarTanda(nuevos, nNuevos);

    FILE *fTemp = fopen("temp_ventas.dat", "wb");
    fseek(f, 0, SEEK_SET);

    Comanda viejo;
    int leidos = 0, i = 0;
    bool hayViejo = (leidos < nViejos && fread(&viejo, sizeof(Comanda), 1, f) == 1);

    while (hayViejo && i < nNuevos) {
        if (viejo.idMozo <= nuevos[i].idMozo) {
            fwrite(&viejo, sizeof(Comanda), 1, fTemp);
            leidos++;
            hayViejo = (leidos < nViejos && fread(&viejo, sizeof(Comanda), 1, f) == 1);
        } else {
            fwrite(&nuevos[i], sizeof(Comanda), 1, fTemp);
            i++;
        }
    }
    while (hayViejo) {
        fwrite(&viejo, sizeof(Comanda), 1, fTemp);
        leidos++;
        hayViejo = (leidos < nViejos && fread(&viejo, sizeof(Comanda), 1, f) == 1);
    }
    while (i < nNuevos) {
        fwrite(&nuevos[i], sizeof(Comanda), 1, fTemp);
        i++;
    }

    fclose(f);
    fclose(fTemp);

    remove(nomArch);
    rename("temp_ventas.dat", nomArch);
}

bool fechaValida(int dd, int mm, int aaaa) {
    int diasMes[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (aaaa < 2000 || mm < 1 || mm > 12) return false;
    bool bisiesto = (aaaa % 4 == 0 && aaaa % 100 != 0) || aaaa % 400 == 0;
    int tope = diasMes[mm - 1] + (mm == 2 && bisiesto ? 1 : 0);
    return dd >= 1 && dd <= tope;
}

int main() {
    int dd, mm, aaaa;
    do {
        cout << "Fecha del dia (dd mm aaaa): ";
        cin >> dd >> mm >> aaaa;
        if (!fechaValida(dd, mm, aaaa)) cout << "Fecha invalida.\n";
    } while (!fechaValida(dd, mm, aaaa));

    char nomArch[40];
    sprintf(nomArch, "comandas_%02d-%02d-%04d.dat", dd, mm, aaaa);

    int nViejos = cantidadRegistros(nomArch, sizeof(Comanda));
    if (nViejos == 0) cout << "Se crea la planilla " << nomArch << "\n";
    else              cout << "La planilla ya tiene " << nViejos << " ventas, se agregan al final.\n";

    FILE *fComandas = fopen(nomArch, "ab");
    FILE *fInv      = fopen("inventario.dat", "r+b");
    if (fComandas == NULL || fInv == NULL) {
        cout << "Error: no se pudo abrir la planilla o inventario.dat\n";
        return 1;
    }
    if (cantidadRegistros("mozos.dat", sizeof(Mozo)) == 0) {
        cout << "Error: no existe mozos.dat (correr antes normalizacion.cpp)\n";
        return 1;
    }

    int cargadas = 0;
    int idMozo;
    cout << "\nNumero de mozo (0 para terminar): ";
    cin >> idMozo;

    while (idMozo != 0) {
        Mozo m;
        char clave[20];

        if (!buscarMozo(idMozo, m)) {
            cout << "El mozo " << idMozo << " no existe.\n";
        } else {
            cout << "Clave: ";
            cin.width(20);           
            cin >> clave;

            if (!claveCorrecta(m, clave)) {
                cout << "Clave incorrecta.\n";
            } else if (cargadas == MAX_TANDA) {
                cout << "Se alcanzo el maximo de ventas por tanda.\n";
            } else {
                Comanda c;
                Producto p;
                c.idMozo = idMozo;

                cout << "Codigo de producto: ";
                cin >> c.codigoProducto;
                cout << "Cantidad: ";
                cin >> c.cantidad;

                int pos = buscarProducto(fInv, c.codigoProducto, p);
                if (pos == -1) {
                    cout << "No existe el producto " << c.codigoProducto << ".\n";
                } else if (c.cantidad <= 0) {
                    cout << "Cantidad invalida.\n";
                } else if (p.stockActual < c.cantidad) {
                    cout << "Stock insuficiente de " << p.descripcion
                         << " (quedan " << p.stockActual << ").\n";
                } else {
                    c.comision = p.precio * c.cantidad * TASA_COMISION;
                    fwrite(&c, sizeof(Comanda), 1, fComandas);
                    cargadas++;

                    p.stockActual -= c.cantidad;
                    actualizarStock(fInv, pos, p);

                    cout << "OK: " << c.cantidad << " x " << p.descripcion
                         << " - comision $" << c.comision
                         << " - stock restante " << p.stockActual << "\n";
                }
            }
        }

        cout << "\nNumero de mozo (0 para terminar): ";
        cin >> idMozo;
    }

    fclose(fComandas);
    fclose(fInv);

    dejarOrdenadoPorMozo(nomArch, nViejos);
    cout << "Se cargaron " << cargadas << " ventas. " << nomArch
         << " quedo ordenada por mozo.\n";
    return 0;
}