#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_NAMES 4
#define MAX_LIVES 127
#define PRECIO_VIDA 5
#define PRECIO_ITEM_PRIMERA_MAZMORRA 25
#define PRECIO_VIDA_ADICIONAL 100
#define DINERO_CAMINO 10

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
    Item *item_oculto;
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
    int mazmorras_derrotadas_superior;
    int mazmorras_derrotadas_paralelo;
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
void imprimir_mundo(Aldea *aldea);
bool perder_vida_aleatorio();
void agregar_item_inventario(Jugador *jugador, Item *item);
bool tiene_item(Jugador *jugador, char *nombre_item);

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
    if (jugador->ubicacion_actual->mazmorra_asociada) {
        // Estamos en una mazmorra
        Mazmorra *mazmorra = jugador->ubicacion_actual->mazmorra_asociada;
        
        if (mazmorra->item_oculto) {
            if (mazmorra->item_oculto->encontrado) {
                printf("Ya encontraste el ítem oculto en esta mazmorra.\n");
                if (perder_vida_aleatorio()) {
                    jugador->vidas--;
                    printf("¡Perdiste una vida por buscar repetidamente! Vidas restantes: %d\n", jugador->vidas);
                }
            } else {
                mazmorra->item_oculto->encontrado = true;
                agregar_item_inventario(jugador, mazmorra->item_oculto);
                printf("¡Encontraste el ítem %s en la mazmorra!\n", mazmorra->item_oculto->nombre);
            }
        } else {
            jugador->dinero += DINERO_CAMINO;
            printf("No hay ítem oculto aquí. Encontraste $%d en el camino.\n", DINERO_CAMINO);
            if (perder_vida_aleatorio()) {
                jugador->vidas--;
                printf("¡Perdiste una vida en el camino! Vidas restantes: %d\n", jugador->vidas);
            }
        }
    } else {
        // Estamos en una aldea
        if (jugador->ubicacion_actual->item_oculto) {
            if (jugador->ubicacion_actual->item_oculto->encontrado) {
                printf("El ítem oculto de esta aldea ya fue encontrado.\n");
                printf("El ítem para derrotar la mazmorra asociada es: %s\n", 
                       jugador->ubicacion_actual->mazmorra_asociada->item_requerido->nombre);
            } else {
                jugador->ubicacion_actual->item_oculto->encontrado = true;
                agregar_item_inventario(jugador, jugador->ubicacion_actual->item_oculto);
                printf("¡Encontraste el ítem %s en la aldea!\n", jugador->ubicacion_actual->item_oculto->nombre);
            }
        } else {
            printf("No hay ítem oculto en esta aldea.\n");
            printf("El ítem para derrotar la mazmorra asociada es: %s\n", 
                   jugador->ubicacion_actual->mazmorra_asociada->item_requerido->nombre);
        }
    }
}

// Luego corregimos la función entrar_mazmorra
void entrar_mazmorra(Jugador *jugador) {
    if (jugador->ubicacion_actual->mazmorra_asociada) {
        printf("Ya estás en una mazmorra.\n");
        return;
    }
    
    // Necesitamos mantener un puntero a la aldea actual para poder volver
    Aldea *aldea_actual = jugador->ubicacion_actual;
    jugador->ubicacion_actual = NULL; // Temporalmente NULL
    
    printf("Has entrado a la mazmorra %s\n", aldea_actual->mazmorra_asociada->nombre);
    
    // Aquí necesitaríamos una forma de manejar que ahora estamos en una mazmorra
    // Podríamos añadir un campo en Jugador para saber si está en aldea o mazmorra
    // O modificar la estructura para manejar ambos casos
}

void comprar(Jugador *jugador) {
    if (jugador->ubicacion_actual->mazmorra_asociada) {
        printf("No puedes comprar en una mazmorra.\n");
        return;
    }
    
    printf("Tienda de la aldea:\n");
    printf("1. Recuperar vida ($%d)\n", PRECIO_VIDA);
    printf("2. Ítem de la primera mazmorra ($%d)\n", PRECIO_ITEM_PRIMERA_MAZMORRA);
    printf("3. Vida adicional ($%d)\n", PRECIO_VIDA_ADICIONAL);
    printf("Selecciona una opción (1-3): ");
    
    int opcion;
    scanf("%d", &opcion);
    
    switch(opcion) {
        case 1:
            if (jugador->dinero >= PRECIO_VIDA) {
                jugador->dinero -= PRECIO_VIDA;
                jugador->vidas = (jugador->vidas < MAX_LIVES) ? jugador->vidas + 1 : MAX_LIVES;
                printf("Vida recuperada. Vidas actuales: %d\n", jugador->vidas);
            } else {
                printf("No tienes suficiente dinero.\n");
            }
            break;
        case 2:
            if (jugador->dinero >= PRECIO_ITEM_PRIMERA_MAZMORRA) {
                jugador->dinero -= PRECIO_ITEM_PRIMERA_MAZMORRA;
                Item *item = crear_item();
                strcpy(item->nombre, nombres_items[0]); // Ítem de la primera mazmorra
                agregar_item_inventario(jugador, item);
                printf("Has comprado el ítem %s\n", item->nombre);
            } else {
                printf("No tienes suficiente dinero.\n");
            }
            break;
        case 3:
            if (jugador->dinero >= PRECIO_VIDA_ADICIONAL && jugador->vidas < MAX_LIVES) {
                jugador->dinero -= PRECIO_VIDA_ADICIONAL;
                jugador->vidas++;
                printf("Vida adicional adquirida. Vidas actuales: %d\n", jugador->vidas);
            } else if (jugador->vidas >= MAX_LIVES) {
                printf("Ya tienes el máximo de vidas.\n");
            } else {
                printf("No tienes suficiente dinero.\n");
            }
            break;
        default:
            printf("Opción inválida.\n");
    }
}

void transportar(Jugador *jugador) {
    if (!jugador->mundo_paralelo_desbloqueado) {
        printf("Aún no has desbloqueado el mundo paralelo.\n");
        return;
    }
    
    if (jugador->ubicacion_actual->mazmorra_asociada) {
        printf("No puedes transportarte desde una mazmorra.\n");
        return;
    }
    
    jugador->ubicacion_actual = jugador->ubicacion_actual->paralela;
    printf("Te has transportado al mundo paralelo. Ahora estás en %s\n", jugador->ubicacion_actual->nombre);
}

void mover(Jugador *jugador, char *direccion) {
    Aldea *nueva_ubicacion = NULL;
    
    if (strcmp(direccion, "ant") == 0) {
        if (jugador->ubicacion_actual->anterior) {
            nueva_ubicacion = jugador->ubicacion_actual->anterior;
        } else {
            printf("No puedes ir a la anterior, ¡aquí naciste!\n");
            return;
        }
    } else if (strcmp(direccion, "sig") == 0) {
        if (jugador->ubicacion_actual->siguiente) {
            nueva_ubicacion = jugador->ubicacion_actual->siguiente;
        } else {
            printf("No hay aldea siguiente, ¡este es el fin del camino!\n");
            return;
        }
    }
    
    if (nueva_ubicacion) {
        jugador->ubicacion_actual = nueva_ubicacion;
        jugador->dinero += DINERO_CAMINO;
        printf("En el camino encontraste $%d. Dinero actual: $%d\n", DINERO_CAMINO, jugador->dinero);
        
        if (perder_vida_aleatorio()) {
            jugador->vidas--;
            printf("¡Perdiste una vida en el camino! Vidas restantes: %d\n", jugador->vidas);
        }
        
        printf("Ahora estás en %s\n", jugador->ubicacion_actual->nombre);
    }
}

bool verificar_victoria(Jugador *jugador, int num_aldeas) {
    if (jugador->mazmorras_derrotadas_superior >= num_aldeas && 
        jugador->mazmorras_derrotadas_paralelo >= num_aldeas) {
        return true;
    }
    return false;
}

void liberar_memoria(Aldea *aldea) {
    while (aldea) {
        Aldea *temp_aldea = aldea;
        aldea = aldea->siguiente;
        
        if (temp_aldea->mazmorra_asociada) {
            free(temp_aldea->mazmorra_asociada->item_requerido);
            free(temp_aldea->mazmorra_asociada);
        }
        
        if (temp_aldea->item_oculto) {
            free(temp_aldea->item_oculto);
        }
        
        free(temp_aldea);
    }
}

void imprimir_mundo(Aldea *aldea) {
    printf("\n=== Mundo del Juego ===\n");
    while (aldea) {
        printf("Aldea: %s\n", aldea->nombre);
        if (aldea->mazmorra_asociada) {
            printf("  Mazmorra: %s\n", aldea->mazmorra_asociada->nombre);
            printf("  Ítem requerido: %s\n", aldea->mazmorra_asociada->item_requerido->nombre);
        }
        if (aldea->item_oculto) {
            printf("  Ítem oculto: %s (%s)\n", aldea->item_oculto->nombre, 
                   aldea->item_oculto->encontrado ? "encontrado" : "no encontrado");
        }
        aldea = aldea->siguiente;
    }
    printf("======================\n\n");
}

bool perder_vida_aleatorio() {
    return (rand() % 4) == 0; // 25% de probabilidad de perder vida
}

void agregar_item_inventario(Jugador *jugador, Item *item) {
    item->siguiente = jugador->inventario;
    jugador->inventario = item;
}

bool tiene_item(Jugador *jugador, char *nombre_item) {
    Item *actual = jugador->inventario;
    while (actual) {
        if (strcmp(actual->nombre, nombre_item) == 0) {
            return true;
        }
        actual = actual->siguiente;
    }
    return false;
}

// Función principal del juego
void jugar(int num_aldeas) {
    srand(time(NULL));
    Aldea *mundo_superior = crear_aldeas(num_aldeas);
    Aldea *mundo_paralelo = crear_aldeas(num_aldeas);
    
    asignar_mazmorras(mundo_superior, num_aldeas);
    asignar_mazmorras(mundo_paralelo, num_aldeas);
    enlazar_mundos(mundo_superior, mundo_paralelo);

    Jugador jugador = {3, 0, NULL, mundo_superior, false, 0, 0};

    // Imprimir mundo generado (para depuración)
    imprimir_mundo(mundo_superior);
    imprimir_mundo(mundo_paralelo);

    while (jugador.vidas > 0) {
        printf("\nEstás en: %s\n", jugador.ubicacion_actual->nombre);
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
            printf("\n¡Felicidades! Has derrotado todas las mazmorras y ganado el juego.\n");
            break;
        }
    }
    
    if (jugador.vidas <= 0) {
        printf("\n¡Game Over! Te has quedado sin vidas.\n");
    }
    
    liberar_memoria(mundo_superior);
    liberar_memoria(mundo_paralelo);
}

int main() {
    int num_aldeas;
    printf("Ingrese el número de aldeas: ");
    scanf("%d", &num_aldeas);
    
    if (num_aldeas < 1) {
        printf("Debe haber al menos 1 aldea.\n");
        return 1;
    }
    
    jugar(num_aldeas);
    return 0;
}