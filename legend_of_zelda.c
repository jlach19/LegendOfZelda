#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_NAMES 4
#define MAX_LIVES 127

// Nombres predefinidos
char *nombres_mazmorras[] = {"Agua", "Tierra", "Fuego", "Aire"};
char *nombres_aldeas[] = {"Kakariko", "Hateno", "Gerudo", "Zora"};
char *nombres_items[] = {"Espada", "Escudo", "Arco", "Bomba"};

typedef struct Item {
    char nombre[50];
    bool encontrado;
    struct Item *siguiente;
} Item;

typedef struct Mazmorra {
    char nombre[50];
    Item *item_requerido;
    bool derrotada;
    struct Mazmorra *siguiente;
} Mazmorra;

typedef struct Aldea {
    char nombre[50];
    Mazmorra *mazmorra_asociada;
    Item *item_oculto;
    struct Aldea *anterior;
    struct Aldea *siguiente;
    struct Aldea *paralela;
} Aldea;

typedef struct Jugador {
    int vidas;
    int dinero;
    Item *inventario;
    Aldea *ubicacion_actual;
    bool mundo_paralelo_desbloqueado;
} Jugador;

// Prototipos de funciones
char* combinar_nombres(int indice, char **base);
Item* crear_item();
Item* asignar_item_aleatorio();
Aldea* crear_aldeas(int num_aldeas);
void asignar_mazmorras(Aldea *aldea, int num_aldeas);
void enlazar_mundos(Aldea *mundo_superior, Aldea *mundo_paralelo);
void buscar_item(Jugador *jugador);
void entrar_mazmorra(Jugador *jugador);
void comprar(Jugador *jugador);
void transportar(Jugador *jugador);
void mover(Jugador *jugador, char *direccion);
bool verificar_victoria(Jugador *jugador, int num_aldeas);
void liberar_memoria(Aldea *aldea);

// Implementación de funciones

char* combinar_nombres(int indice, char **base) {
    char *nombre = malloc(100);
    strcpy(nombre, base[indice % MAX_NAMES]);
    if (indice >= MAX_NAMES) {
        for (int i = 1; i <= indice / MAX_NAMES; i++) {
            strcat(nombre, "-");
            strcat(nombre, base[(indice + i) % MAX_NAMES]);
        }
    }
    return nombre;
}

Item* crear_item() {
    Item *item = malloc(sizeof(Item));
    strcpy(item->nombre, nombres_items[rand() % MAX_NAMES]);
    item->encontrado = false;
    item->siguiente = NULL;
    return item;
}

Item* asignar_item_aleatorio() {
    return crear_item();
}

Aldea* crear_aldeas(int num_aldeas) {
    Aldea *head = NULL;
    Aldea *current = NULL;
    for (int i = 0; i < num_aldeas; i++) {
        Aldea *aldea = malloc(sizeof(Aldea));
        strcpy(aldea->nombre, combinar_nombres(i, nombres_aldeas));
        aldea->mazmorra_asociada = NULL;
        aldea->item_oculto = (rand() % 2 == 0) ? crear_item() : NULL;
        aldea->anterior = current;
        aldea->siguiente = NULL;
        aldea->paralela = NULL;
        if (!head) head = aldea;
        else current->siguiente = aldea;
        current = aldea;
    }
    return head;
}

void asignar_mazmorras(Aldea *aldea, int num_aldeas) {
    for (int i = 0; i < num_aldeas; i++) {
        Mazmorra *mazmorra = malloc(sizeof(Mazmorra));
        strcpy(mazmorra->nombre, combinar_nombres(i, nombres_mazmorras));
        mazmorra->item_requerido = (i == 0) ? crear_item() : asignar_item_aleatorio();
        mazmorra->derrotada = false;
        mazmorra->siguiente = NULL;
        aldea->mazmorra_asociada = mazmorra;
        aldea = aldea->siguiente;
    }
}

void enlazar_mundos(Aldea *mundo_superior, Aldea *mundo_paralelo) {
    Aldea *sup = mundo_superior;
    Aldea *par = mundo_paralelo;
    while (sup && par) {
        sup->paralela = par;
        par->paralela = sup;
        sup = sup->siguiente;
        par = par->siguiente;
    }
}

void buscar_item(Jugador *jugador) {
    printf("Buscando item...\n");
    // Implementación real aquí
}

void entrar_mazmorra(Jugador *jugador) {
    printf("Entrando a mazmorra...\n");
    // Implementación real aquí
}

void comprar(Jugador *jugador) {
    printf("Comprando items...\n");
    // Implementación real aquí
}

void transportar(Jugador *jugador) {
    printf("Transportando entre mundos...\n");
    // Implementación real aquí
}

void mover(Jugador *jugador, char *direccion) {
    printf("Moviendo %s...\n", direccion);
    // Implementación real aquí
}

bool verificar_victoria(Jugador *jugador, int num_aldeas) {
    printf("Verificando victoria...\n");
    return false; // Implementación real aquí
}

void liberar_memoria(Aldea *aldea) {
    printf("Liberando memoria...\n");
    // Implementación real aquí
}

// Función principal del juego
void jugar(int num_aldeas) {
    srand(time(NULL));
    Aldea *mundo_superior = crear_aldeas(num_aldeas);
    Aldea *mundo_paralelo = crear_aldeas(num_aldeas);
    enlazar_mundos(mundo_superior, mundo_paralelo);

    Jugador jugador = {3, 0, NULL, mundo_superior, false};

    while (jugador.vidas > 0) {
        printf("Estás en: %s\n", jugador.ubicacion_actual->nombre);
        printf("Vidas: %d | Dinero: $%d\n", jugador.vidas, jugador.dinero);
        printf("Comandos: busq, maz, compr, trans, ant, sig\n> ");
        
        char comando[10];
        scanf("%s", comando);
        
        if (strcmp(comando, "busq") == 0) {
            buscar_item(&jugador);
        } else if (strcmp(comando, "maz") == 0) {
            entrar_mazmorra(&jugador);
        } else if (strcmp(comando, "compr") == 0) {
            comprar(&jugador);
        } else if (strcmp(comando, "trans") == 0) {
            transportar(&jugador);
        } else if (strcmp(comando, "ant") == 0 || strcmp(comando, "sig") == 0) {
            mover(&jugador, comando);
        } else {
            printf("Comando inválido.\n");
        }
        
        if (verificar_victoria(&jugador, num_aldeas)) {
            printf("¡Has derrotado todas las mazmorras!\n");
            break;
        }
    }
    
    if (jugador.vidas <= 0) printf("Game Over.\n");
    liberar_memoria(mundo_superior);
}

int main() {
    int num_aldeas;
    printf("Ingrese el número de aldeas: ");
    scanf("%d", &num_aldeas);
    jugar(num_aldeas);
    return 0;
}