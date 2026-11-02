// SISTEMA DE GESTION DE PROCESOS - ESTRUCTURA DE DATOS
// Lista enlazada (procesos), cola con prioridad (por atender) y pila (memoria)
// Memoria total: 1 GB = 1024 MB

#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;

#define MEMORIA_TOTAL 1024

// ---------- Estructuras ----------

// Nodo de la lista enlazada: un proceso registrado
// estado: 0 NUEVO, 1 LISTO, 2 EN EJECUCION, 3 BLOQUEADO, 4 TERMINADO
struct Proceso {
    int id;
    char nombre[30];
    int prioridad;      // 1 = mayor prioridad, 5 = menor
    int estado;
    int memoria;        // memoria que necesita (MB)
    Proceso *sig;
};

// Nodo de la cola: proceso por atender
struct NodoCola {
    int id;
    int prioridad;
    NodoCola *sig;
};

// Nodo de la pila: bloque de memoria asignado
struct Bloque {
    int numero;
    int idProceso;
    int tamano;
    Bloque *sig;
};

Proceso *cabeza = NULL;     // inicio de la lista
NodoCola *frente = NULL;    // inicio de la cola
Bloque *tope = NULL;        // tope de la pila
int memoriaAsignada = 0;
int contadorBloques = 1;

// ---------- Funciones auxiliares ----------

// Pide un numero y lo repite hasta que este dentro del rango
int leerNumero(const char *mensaje, int minimo, int maximo) {
    int n;
    cout << mensaje;
    cin >> n;
    while (cin.fail() || n < minimo || n > maximo) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Dato invalido. Ingrese un numero entre " << minimo << " y " << maximo << ": ";
        cin >> n;
    }
    cin.ignore(1000, '\n');
    return n;
}

const char *nombreEstado(int e) {
    if (e == 0) return "NUEVO";
    if (e == 1) return "LISTO";
    if (e == 2) return "EN EJECUCION";
    if (e == 3) return "BLOQUEADO";
    return "TERMINADO";
}

int memoriaDisponible() {
    return MEMORIA_TOTAL - memoriaAsignada;
}

void resumenMemoria() {
    cout << "Memoria total: " << MEMORIA_TOTAL << " MB | Asignada: " << memoriaAsignada
         << " MB | Pendiente: " << memoriaDisponible() << " MB" << endl;
}

// ---------- Lista enlazada ----------

Proceso *buscarPorId(int id) {
    Proceso *p = cabeza;
    while (p != NULL && p->id != id) {
        p = p->sig;
    }
    return p;
}

Proceso *buscarPorNombre(char *nombre) {
    Proceso *p = cabeza;
    while (p != NULL && strcmp(p->nombre, nombre) != 0) {
        p = p->sig;
    }
    return p;
}

// Agrega un nodo al final de la lista
void insertarAlFinal(Proceso *nuevo) {
    nuevo->sig = NULL;
    if (cabeza == NULL) {
        cabeza = nuevo;
    } else {
        Proceso *aux = cabeza;
        while (aux->sig != NULL) {
            aux = aux->sig;
        }
        aux->sig = nuevo;
    }
}

// ---------- Cola con prioridad ----------

// Inserta ordenado: la prioridad 1 sale primero; si hay empate, el que llego antes
void encolar(int id, int prioridad) {
    NodoCola *nuevo = new NodoCola;
    nuevo->id = id;
    nuevo->prioridad = prioridad;
    nuevo->sig = NULL;

    if (frente == NULL || prioridad < frente->prioridad) {
        nuevo->sig = frente;
        frente = nuevo;
    } else {
        NodoCola *aux = frente;
        while (aux->sig != NULL && aux->sig->prioridad <= prioridad) {
            aux = aux->sig;
        }
        nuevo->sig = aux->sig;
        aux->sig = nuevo;
    }
}

// Saca el primero de la cola
void desencolar() {
    if (frente != NULL) {
        NodoCola *aux = frente;
        frente = frente->sig;
        delete aux;
    }
}

// Quita un proceso de cualquier posicion de la cola
// Devuelve 1 si lo encontro y 0 si no estaba
int quitarDeCola(int id) {
    NodoCola *anterior = NULL;
    NodoCola *aux = frente;
    while (aux != NULL && aux->id != id) {
        anterior = aux;
        aux = aux->sig;
    }
    if (aux == NULL) {
        return 0;
    }
    if (anterior == NULL) {
        frente = aux->sig;
    } else {
        anterior->sig = aux->sig;
    }
    delete aux;
    return 1;
}

// ---------- Pila de memoria ----------

// push: asigna un bloque de memoria a un proceso
void push(int idProceso, int tamano) {
    Bloque *nuevo = new Bloque;
    nuevo->numero = contadorBloques;
    contadorBloques++;
    nuevo->idProceso = idProceso;
    nuevo->tamano = tamano;
    nuevo->sig = tope;
    tope = nuevo;
    memoriaAsignada = memoriaAsignada + tamano;
}

// Busca si un proceso tiene memoria asignada
Bloque *buscarBloque(int idProceso) {
    Bloque *b = tope;
    while (b != NULL && b->idProceso != idProceso) {
        b = b->sig;
    }
    return b;
}

// ---------- Operaciones sobre procesos ----------

// Ciclo de vida permitido: devuelve 1 si el cambio de estado es valido
int transicionValida(int actual, int nuevo) {
    if (actual == 0 && nuevo == 1) return 1;
    if (actual == 1 && nuevo == 2) return 1;
    if (actual == 2 && (nuevo == 1 || nuevo == 3 || nuevo == 4)) return 1;
    if (actual == 3 && nuevo == 1) return 1;
    return 0;
}

void registrarProceso() {
    Proceso *nuevo = new Proceso;
    nuevo->id = leerNumero("ID del proceso (1-9999): ", 1, 9999);
    if (buscarPorId(nuevo->id) != NULL) {
        cout << "Ya existe un proceso con ese ID." << endl;
        delete nuevo;
        return;
    }
    do {
        cout << "Nombre del proceso (maximo 29 letras): ";
        cin.getline(nuevo->nombre, 30);
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
        }
        if (strlen(nuevo->nombre) == 0) {
            cout << "El nombre no puede estar vacio." << endl;
        }
    } while (strlen(nuevo->nombre) == 0);
    nuevo->prioridad = leerNumero("Prioridad (1 = mayor, 5 = menor): ", 1, 5);
    nuevo->memoria = leerNumero("Memoria requerida en MB (1-1024): ", 1, MEMORIA_TOTAL);
    nuevo->estado = 0;
    insertarAlFinal(nuevo);
    cout << "Proceso registrado con estado NUEVO." << endl;
}

void mostrarFila(Proceso *p) {
    cout << p->id << "\t" << p->nombre << "\t" << p->prioridad << "\t"
         << nombreEstado(p->estado) << "\t" << p->memoria << "\t";
    if (buscarBloque(p->id) != NULL) {
        cout << "Si" << endl;
    } else {
        cout << "No" << endl;
    }
}

void buscarProceso() {
    int tipo = leerNumero("Buscar por 1) ID  2) Nombre: ", 1, 2);
    Proceso *p;
    if (tipo == 1) {
        p = buscarPorId(leerNumero("ID a buscar: ", 1, 9999));
    } else {
        char nombre[30];
        cout << "Nombre a buscar: ";
        cin.getline(nombre, 30);
        p = buscarPorNombre(nombre);
    }
    if (p == NULL) {
        cout << "Proceso no encontrado." << endl;
    } else {
        cout << "ID\tNombre\tPrior.\tEstado\tMem(MB)\tMemoria asignada" << endl;
        mostrarFila(p);
    }
}

void modificarPrioridad() {
    Proceso *p = buscarPorId(leerNumero("ID del proceso: ", 1, 9999));
    if (p == NULL) {
        cout << "El proceso no existe." << endl;
        return;
    }
    p->prioridad = leerNumero("Nueva prioridad (1-5): ", 1, 5);
    // si ya estaba en la cola, se vuelve a colocar segun su nueva prioridad
    if (quitarDeCola(p->id) == 1) {
        encolar(p->id, p->prioridad);
    }
    cout << "Prioridad actualizada." << endl;
}

void eliminarProceso() {
    int id = leerNumero("ID del proceso a eliminar: ", 1, 9999);
    Proceso *anterior = NULL;
    Proceso *p = cabeza;
    while (p != NULL && p->id != id) {
        anterior = p;
        p = p->sig;
    }
    if (p == NULL) {
        cout << "El proceso no existe." << endl;
        return;
    }
    if (buscarBloque(id) != NULL) {
        cout << "El proceso tiene memoria asignada. Libere la memoria primero (opcion 8)." << endl;
        return;
    }
    quitarDeCola(id);
    if (anterior == NULL) {
        cabeza = p->sig;
    } else {
        anterior->sig = p->sig;
    }
    delete p;
    cout << "Proceso eliminado." << endl;
}

// Cambia el estado a LISTO y lo envia a la cola
void planificarProceso(int id) {
    Proceso *p = buscarPorId(id);
    if (p == NULL) {
        cout << "El proceso no existe." << endl;
        return;
    }
    if (transicionValida(p->estado, 1) == 0) {
        cout << "Transicion no permitida: " << nombreEstado(p->estado) << " -> LISTO." << endl;
        return;
    }
    p->estado = 1;
    encolar(p->id, p->prioridad);
    cout << "Proceso enviado a la cola segun su prioridad (LISTO)." << endl;
}

void cambiarEstado() {
    Proceso *p = buscarPorId(leerNumero("ID del proceso: ", 1, 9999));
    if (p == NULL) {
        cout << "El proceso no existe." << endl;
        return;
    }
    cout << "Estado actual: " << nombreEstado(p->estado) << endl;
    cout << "1) LISTO  2) EN EJECUCION  3) BLOQUEADO  4) TERMINADO" << endl;
    int nuevo = leerNumero("Nuevo estado: ", 1, 4);
    if (nuevo == 1) {
        planificarProceso(p->id);
    } else if (nuevo == 2) {
        cout << "Para ejecutar un proceso use la opcion 7 (verifica la memoria)." << endl;
    } else if (nuevo == 4) {
        cout << "Para terminar un proceso use la opcion 8 (libera su memoria)." << endl;
    } else {
        if (transicionValida(p->estado, 3) == 1) {
            p->estado = 3;
            cout << "Estado actualizado a BLOQUEADO." << endl;
        } else {
            cout << "Transicion no permitida: " << nombreEstado(p->estado) << " -> BLOQUEADO." << endl;
        }
    }
}

// Ejecuta el proceso del frente de la cola verificando la memoria
void ejecutarSiguiente() {
    if (frente == NULL) {
        cout << "No hay procesos por atender." << endl;
        return;
    }
    Proceso *p = buscarPorId(frente->id);
    cout << "Proceso a ejecutar: [" << p->id << "] " << p->nombre << endl;

    if (buscarBloque(p->id) == NULL) {
        cout << "Memoria requerida : " << p->memoria << " MB" << endl;
        cout << "Memoria disponible: " << memoriaDisponible() << " MB" << endl;
        if (p->memoria > memoriaDisponible()) {
            cout << ">> MEMORIA INSUFICIENTE: faltan " << p->memoria - memoriaDisponible() << " MB." << endl;
            cout << ">> El proceso sigue en la cola. Libere memoria con la opcion 8." << endl;
            return;
        }
        cout << ">> MEMORIA SUFICIENTE: se asigna el bloque." << endl;
        push(p->id, p->memoria);
    } else {
        cout << "El proceso ya tiene memoria asignada." << endl;
    }
    desencolar();
    p->estado = 2;
    cout << "Proceso en ejecucion." << endl;
    resumenMemoria();
}

// pop: libera el ultimo bloque asignado y termina ese proceso
void finalizarProceso() {
    if (tope == NULL) {
        cout << "No hay memoria asignada que liberar." << endl;
        return;
    }
    Bloque *b = tope;
    tope = tope->sig;
    memoriaAsignada = memoriaAsignada - b->tamano;
    Proceso *p = buscarPorId(b->idProceso);
    if (p != NULL) {
        quitarDeCola(p->id);
        p->estado = 4;
        cout << "Se libero el bloque #" << b->numero << " (" << b->tamano
             << " MB) del proceso [" << p->id << "] " << p->nombre << "." << endl;
        cout << "El proceso paso a TERMINADO." << endl;
    }
    delete b;
    resumenMemoria();
}

// ---------- Mostrar informacion ----------

void mostrarRegistrados() {
    cout << "=== PROCESOS REGISTRADOS ===" << endl;
    if (cabeza == NULL) {
        cout << "(no hay procesos registrados)" << endl;
        return;
    }
    cout << "ID\tNombre\tPrior.\tEstado\tMem(MB)\tMemoria asignada" << endl;
    Proceso *p = cabeza;
    while (p != NULL) {
        mostrarFila(p);
        p = p->sig;
    }
}

// Atendidos: los que ya se ejecutaron (EN EJECUCION o TERMINADO)
void mostrarAtendidos() {
    int cantidad = 0;
    cout << "=== PROCESOS ATENDIDOS ===" << endl;
    Proceso *p = cabeza;
    while (p != NULL) {
        if (p->estado == 2 || p->estado == 4) {
            if (cantidad == 0) {
                cout << "ID\tNombre\tPrior.\tEstado\tMem(MB)\tMemoria asignada" << endl;
            }
            mostrarFila(p);
            cantidad++;
        }
        p = p->sig;
    }
    if (cantidad == 0) {
        cout << "(aun no se ha atendido ningun proceso)" << endl;
    }
}

// Por atender: los que esperan en la cola
void mostrarPorAtender() {
    cout << "=== PROCESOS POR ATENDER ===" << endl;
    if (frente == NULL) {
        cout << "(la cola esta vacia)" << endl;
        return;
    }
    cout << "Orden\tID\tNombre\tPrior.\tMem(MB)" << endl;
    int orden = 1;
    NodoCola *n = frente;
    while (n != NULL) {
        Proceso *p = buscarPorId(n->id);
        cout << orden << "\t" << p->id << "\t" << p->nombre << "\t" << p->prioridad
             << "\t" << p->memoria << endl;
        orden++;
        n = n->sig;
    }
}

void mostrarMemoria() {
    int llenos = (memoriaAsignada * 40) / MEMORIA_TOTAL;
    cout << "=== ESTADO DE LA MEMORIA (PILA) ===" << endl;
    cout << "Memoria total            : " << MEMORIA_TOTAL << " MB (1 GB)" << endl;
    cout << "Memoria asignada         : " << memoriaAsignada << " MB" << endl;
    cout << "Memoria pendiente (libre): " << memoriaDisponible() << " MB" << endl;
    cout << "[";
    for (int i = 0; i < 40; i++) {
        if (i < llenos) {
            cout << "#";
        } else {
            cout << ".";
        }
    }
    cout << "]" << endl;
    if (tope == NULL) {
        cout << "(no hay bloques asignados)" << endl;
        return;
    }
    cout << "Bloques (del tope a la base):" << endl;
    cout << "Bloque\tID\tTamano(MB)" << endl;
    Bloque *b = tope;
    while (b != NULL) {
        cout << b->numero << "\t" << b->idProceso << "\t" << b->tamano;
        if (b == tope) {
            cout << "\t<- TOPE";
        }
        cout << endl;
        b = b->sig;
    }
}

// ---------- Guardar y cargar ----------

void guardarDatos() {
    ofstream archivo("datos.txt");
    int cantidad = 0;
    Proceso *p = cabeza;
    while (p != NULL) {
        cantidad++;
        p = p->sig;
    }
    archivo << cantidad << endl;
    p = cabeza;
    while (p != NULL) {
        archivo << p->id << " " << p->prioridad << " " << p->estado << " " << p->memoria << endl;
        archivo << p->nombre << endl;
        p = p->sig;
    }
    cantidad = 0;
    NodoCola *n = frente;
    while (n != NULL) {
        cantidad++;
        n = n->sig;
    }
    archivo << cantidad << endl;
    n = frente;
    while (n != NULL) {
        archivo << n->id << " " << n->prioridad << endl;
        n = n->sig;
    }
    cantidad = 0;
    Bloque *b = tope;
    while (b != NULL) {
        cantidad++;
        b = b->sig;
    }
    archivo << cantidad << " " << contadorBloques << endl;
    b = tope;                       // se guarda del tope a la base
    while (b != NULL) {
        archivo << b->numero << " " << b->idProceso << " " << b->tamano << endl;
        b = b->sig;
    }
    archivo.close();
    cout << "Datos guardados en datos.txt" << endl;
}

// Borra todo lo que hay en memoria antes de cargar
void limpiarTodo() {
    while (cabeza != NULL) {
        Proceso *aux = cabeza;
        cabeza = cabeza->sig;
        delete aux;
    }
    while (frente != NULL) {
        desencolar();
    }
    while (tope != NULL) {
        Bloque *aux = tope;
        tope = tope->sig;
        delete aux;
    }
    memoriaAsignada = 0;
    contadorBloques = 1;
}

void cargarDatos() {
    ifstream archivo("datos.txt");
    if (!archivo) {
        cout << "No existe el archivo datos.txt. Guarde los datos primero." << endl;
        return;
    }
    limpiarTodo();
    int cantidad, i;

    archivo >> cantidad;
    for (i = 0; i < cantidad; i++) {
        Proceso *p = new Proceso;
        archivo >> p->id >> p->prioridad >> p->estado >> p->memoria;
        archivo.ignore();
        archivo.getline(p->nombre, 30);
        insertarAlFinal(p);
    }

    archivo >> cantidad;
    for (i = 0; i < cantidad; i++) {
        int id, prioridad;
        archivo >> id >> prioridad;
        // se agrega al final de la cola respetando el orden guardado
        NodoCola *nuevo = new NodoCola;
        nuevo->id = id;
        nuevo->prioridad = prioridad;
        nuevo->sig = NULL;
        if (frente == NULL) {
            frente = nuevo;
        } else {
            NodoCola *aux = frente;
            while (aux->sig != NULL) {
                aux = aux->sig;
            }
            aux->sig = nuevo;
        }
    }

    // los bloques se guardaron del tope a la base: se leen y se apilan al reves
    int numeros[200], ids[200], tamanos[200], siguienteBloque;
    archivo >> cantidad >> siguienteBloque;
    if (cantidad > 200) {
        cantidad = 200;
    }
    for (i = 0; i < cantidad; i++) {
        archivo >> numeros[i] >> ids[i] >> tamanos[i];
    }
    for (i = cantidad - 1; i >= 0; i--) {
        push(ids[i], tamanos[i]);
        tope->numero = numeros[i];
    }
    contadorBloques = siguienteBloque;
    archivo.close();
    cout << "Datos cargados correctamente." << endl;
}

// ---------- Menu ----------

void mostrarMenu() {
    cout << endl << "===== SISTEMA DE GESTION DE PROCESOS =====" << endl;
    cout << " 1. Registrar proceso" << endl;
    cout << " 2. Buscar proceso (por ID o nombre)" << endl;
    cout << " 3. Modificar prioridad" << endl;
    cout << " 4. Eliminar proceso" << endl;
    cout << " 5. Planificar proceso (enviar a la cola)" << endl;
    cout << " 6. Cambiar estado de un proceso" << endl;
    cout << " 7. Ejecutar siguiente proceso (verifica memoria)" << endl;
    cout << " 8. Finalizar proceso y liberar memoria (pop)" << endl;
    cout << " 9. Mostrar procesos registrados" << endl;
    cout << "10. Mostrar procesos por atender (cola)" << endl;
    cout << "11. Mostrar procesos atendidos" << endl;
    cout << "12. Mostrar estado de la memoria (1 GB)" << endl;
    cout << "13. Guardar datos" << endl;
    cout << "14. Cargar datos" << endl;
    cout << " 0. Salir" << endl;
    cout << "------------------------------------------" << endl;
    resumenMemoria();
}

int main() {
    int opcion;
    do {
        mostrarMenu();
        opcion = leerNumero("Seleccione una opcion: ", 0, 14);
        cout << endl;
        switch (opcion) {
            case 1:  registrarProceso(); break;
            case 2:  buscarProceso(); break;
            case 3:  modificarPrioridad(); break;
            case 4:  eliminarProceso(); break;
            case 5:  planificarProceso(leerNumero("ID del proceso a enviar a la cola: ", 1, 9999)); break;
            case 6:  cambiarEstado(); break;
            case 7:  ejecutarSiguiente(); break;
            case 8:  finalizarProceso(); break;
            case 9:  mostrarRegistrados(); break;
            case 10: mostrarPorAtender(); break;
            case 11: mostrarAtendidos(); break;
            case 12: mostrarMemoria(); break;
            case 13: guardarDatos(); break;
            case 14: cargarDatos(); break;
            case 0:  cout << "Saliendo del sistema..." << endl; break;
        }
        if (opcion != 0) {
            cout << endl << "Presione ENTER para continuar...";
            cin.get();
        }
    } while (opcion != 0);

    limpiarTodo();
    return 0;
}