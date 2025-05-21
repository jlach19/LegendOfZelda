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
int MAZMORRAS_PARA_PARALELO;

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
    Aldea *aldea_actual;
    Mazmorra *mazmorra_actual;
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
void salir_mazmorra(Jugador *jugador);
void atacar_mazmorra(Jugador *jugador);
void comprar(Jugador *jugador);
void transportar(Jugador *jugador);
void mover(Jugador *jugador, char *direccion);
bool verificar_victoria(Jugador *jugador, int num_aldeas);
void liberar_memoria(Aldea *aldea);
void imprimir_mundo(Aldea *aldea);
bool perder_vida_aleatorio();
void agregar_item_inventario(Jugador *jugador, Item *item);
bool tiene_item(Jugador *jugador, char *nombre_item);
void mostrar_inventario(Jugador *jugador);

// Implementación de funciones

int aleatorio(int min, int max) {
    return min + rand() % (max - min + 1);
}

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
        mazmorra->item_oculto = (rand() % 2 == 0) ? crear_item() : NULL;
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
    if (jugador->mazmorra_actual) {
        // Estamos en una mazmorra
        if (jugador->mazmorra_actual->item_oculto) {
            if (jugador->mazmorra_actual->item_oculto->encontrado) {
                printf("Ya encontraste el ítem oculto en esta mazmorra.\n");
                if (perder_vida_aleatorio()) {
                    jugador->vidas--;
                    printf("¡Perdiste una vida por buscar repetidamente! Vidas restantes: %d\n", jugador->vidas);
                }
            } else {
                jugador->mazmorra_actual->item_oculto->encontrado = true;
                agregar_item_inventario(jugador, jugador->mazmorra_actual->item_oculto);
                printf("¡Encontraste el ítem %s en la mazmorra!\n", jugador->mazmorra_actual->item_oculto->nombre);
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
        if (jugador->aldea_actual->item_oculto) {
            if (jugador->aldea_actual->item_oculto->encontrado) {
                printf("El ítem oculto de esta aldea ya fue encontrado.\n");
                printf("El ítem para derrotar la mazmorra asociada es: %s\n", 
                       jugador->aldea_actual->mazmorra_asociada->item_requerido->nombre);
            } else {
                jugador->aldea_actual->item_oculto->encontrado = true;
                agregar_item_inventario(jugador, jugador->aldea_actual->item_oculto);
                printf("¡Encontraste el ítem %s en la aldea!\n", jugador->aldea_actual->item_oculto->nombre);
            }
        } else {
            printf("No hay ítem oculto en esta aldea.\n");
            printf("El ítem para derrotar la mazmorra asociada es: %s\n", 
                   jugador->aldea_actual->mazmorra_asociada->item_requerido->nombre);
        }
    }
}

void entrar_mazmorra(Jugador *jugador) {
    if (jugador->mazmorra_actual) {
        printf("Ya estás en una mazmorra.\n");
        return;
    }
    
    if (!jugador->aldea_actual || !jugador->aldea_actual->mazmorra_asociada) {
        printf("No hay mazmorra asociada a esta aldea.\n");
        return;
    }
    
    jugador->mazmorra_actual = jugador->aldea_actual->mazmorra_asociada;
    printf("Has entrado a la mazmorra %s\n", jugador->mazmorra_actual->nombre);
}

void salir_mazmorra(Jugador *jugador) {
    if (!jugador->mazmorra_actual) {
        printf("No estás en una mazmorra.\n");
        return;
    }
    
    // Buscar la aldea asociada a esta mazmorra
    Aldea *aldea = jugador->aldea_actual;
    while (aldea && aldea->mazmorra_asociada != jugador->mazmorra_actual) {
        aldea = aldea->siguiente;
    }
    
    if (aldea) {
        jugador->aldea_actual = aldea;
        jugador->mazmorra_actual = NULL;
        printf("Has vuelto a la aldea %s\n", aldea->nombre);
    } else {
        printf("Error: No se encontró la aldea asociada a esta mazmorra.\n");
    }
}

void atacar_mazmorra(Jugador *jugador) {
    if (!jugador->mazmorra_actual) {
        printf("No estás en una mazmorra.\n");
        return;
    }
    
    if (jugador->mazmorra_actual->derrotada) {
        printf("Esta mazmorra ya ha sido derrotada.\n");
        return;
    }
    
    if (perder_vida_aleatorio()) {
        jugador->vidas--;
        printf("¡Perdiste una vida en el ataque! Vidas restantes: %d\n", jugador->vidas);
    }
    
    if (tiene_item(jugador, jugador->mazmorra_actual->item_requerido->nombre)) {
        jugador->mazmorra_actual->derrotada = true;
        printf("¡Has derrotado la mazmorra %s!\n", jugador->mazmorra_actual->nombre);
        
        // Actualizar contadores de mazmorras derrotadas
        if (!jugador->aldea_actual->paralela) {
            jugador->mazmorras_derrotadas_paralelo++;
        } else {
            jugador->mazmorras_derrotadas_superior++;
        }
        
        // Lógica para mundo paralelo
        if (jugador->mazmorras_derrotadas_superior >= MAZMORRAS_PARA_PARALELO) {
            jugador->mundo_paralelo_desbloqueado = true;
            printf("¡Has desbloqueado el mundo paralelo!\n");
        }
    } else {
        printf("No tienes el ítem necesario (%s) para derrotar esta mazmorra.\n", 
               jugador->mazmorra_actual->item_requerido->nombre);
    }
}

void comprar(Jugador *jugador) {
    if (jugador->mazmorra_actual) {
        printf("No puedes comprar en una mazmorra.\n");
        return;
    }
    
    printf("Tienda de la aldea:\n");
    printf("1. Recuperar vida ($%d)\n", PRECIO_VIDA);
    printf("2. Ítem de la primera mazmorra ($%d)\n", PRECIO_ITEM_PRIMERA_MAZMORRA);
    printf("3. Vida adicional ($%d) (máximo %d)\n", PRECIO_VIDA_ADICIONAL, MAX_LIVES);
    printf("4. Mostrar inventario\n");
    printf("Selecciona una opción (1-4): ");
    
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
                // Obtener la primera mazmorra del mundo superior
                Aldea *primera_aldea = jugador->aldea_actual;
                while (primera_aldea->anterior != NULL) {
                    primera_aldea = primera_aldea->anterior;
                }
                
                if (primera_aldea && primera_aldea->mazmorra_asociada) {
                    jugador->dinero -= PRECIO_ITEM_PRIMERA_MAZMORRA;
                    Item *item = malloc(sizeof(Item));
                    strcpy(item->nombre, primera_aldea->mazmorra_asociada->item_requerido->nombre);
                    item->encontrado = false;
                    item->siguiente = NULL;
                    agregar_item_inventario(jugador, item);
                    printf("Has comprado el ítem %s (requerido para la primera mazmorra)\n", item->nombre);
                } else {
                    printf("Error: No se pudo encontrar la primera mazmorra.\n");
                }
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
        case 4:
            mostrar_inventario(jugador);
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
    
    if (jugador->mazmorra_actual) {
        printf("No puedes transportarte desde una mazmorra.\n");
        return;
    }
    
    if (!jugador->aldea_actual->paralela) {
        printf("No hay aldea paralela asociada a esta aldea.\n");
        return;
    }
    
    jugador->aldea_actual = jugador->aldea_actual->paralela;
    printf("Te has transportado al mundo paralelo. Ahora estás en %s\n", jugador->aldea_actual->nombre);
}

void mover(Jugador *jugador, char *direccion) {
    if (jugador->mazmorra_actual) {
        printf("No puedes moverte entre aldeas desde una mazmorra. Usa 'ant' para volver.\n");
        return;
    }
    
    Aldea *nueva_ubicacion = NULL;
    
    if (strcmp(direccion, "ant") == 0) {
        if (jugador->aldea_actual->anterior) {
            nueva_ubicacion = jugador->aldea_actual->anterior;
        } else {
            printf("No puedes ir a la anterior, ¡aquí naciste!\n");
            return;
        }
    } else if (strcmp(direccion, "sig") == 0) {
        if (jugador->aldea_actual->siguiente) {
            nueva_ubicacion = jugador->aldea_actual->siguiente;
        } else {
            printf("No hay aldea siguiente, ¡este es el fin del camino!\n");
            return;
        }
    }
    
    if (nueva_ubicacion) {
        jugador->aldea_actual = nueva_ubicacion;
        jugador->dinero += DINERO_CAMINO;
        printf("En el camino encontraste $%d. Dinero actual: $%d\n", DINERO_CAMINO, jugador->dinero);
        
        if (perder_vida_aleatorio()) {
            jugador->vidas--;
            printf("¡Perdiste una vida en el camino! Vidas restantes: %d\n", jugador->vidas);
        }
        
        printf("Ahora estás en %s\n", jugador->aldea_actual->nombre);
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
            if (temp_aldea->mazmorra_asociada->item_requerido) {
                free(temp_aldea->mazmorra_asociada->item_requerido);
            }
            if (temp_aldea->mazmorra_asociada->item_oculto) {
                free(temp_aldea->mazmorra_asociada->item_oculto);
            }
            free(temp_aldea->mazmorra_asociada);
        }
        
        if (temp_aldea->item_oculto) {
            free(temp_aldea->item_oculto);
        }
        
        free(temp_aldea);
    }
}

void imprimir_mundo(Aldea *aldea) {
    while (aldea) {
        printf("Aldea: %s\n", aldea->nombre);
        if (aldea->mazmorra_asociada) {
            printf("  Mazmorra: %s\n", aldea->mazmorra_asociada->nombre);
            printf("  Ítem requerido: %s\n", aldea->mazmorra_asociada->item_requerido->nombre);
            printf("  Ítem oculto: %s\n", aldea->mazmorra_asociada->item_oculto ? aldea->mazmorra_asociada->item_oculto->nombre : "Ninguno");
            printf("  Derrotada: %s\n", aldea->mazmorra_asociada->derrotada ? "Sí" : "No");
        }
        if (aldea->item_oculto) {
            printf("  Ítem oculto en aldea: %s (%s)\n", aldea->item_oculto->nombre, 
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

void mostrar_inventario(Jugador *jugador) {
    printf("\n=== Inventario ===\n");
    if (!jugador->inventario) {
        printf("No tienes ningún ítem.\n");
    } else {
        Item *actual = jugador->inventario;
        while (actual) {
            printf("- %s\n", actual->nombre);
            actual = actual->siguiente;
        }
    }
    printf("=================\n");
}

// Función principal del juego
void jugar(int num_aldeas) {
    srand(time(NULL));
    Aldea *mundo_superior = crear_aldeas(num_aldeas);
    Aldea *mundo_paralelo = crear_aldeas(num_aldeas);
    
    asignar_mazmorras(mundo_superior, num_aldeas);
    asignar_mazmorras(mundo_paralelo, num_aldeas);
    enlazar_mundos(mundo_superior, mundo_paralelo);

    Jugador jugador = {3, 0, NULL, mundo_superior, NULL, false, 0, 0};

    // Imprimir mundo generado (para depuración)
    printf("\n=== Mundo Superior ===\n");
    imprimir_mundo(mundo_superior);
    printf("\n=== Mundo Paralelo ===\n");
    imprimir_mundo(mundo_paralelo);

    while (jugador.vidas > 0) {
        if (jugador.mazmorra_actual) {
            printf("\nEstás en la mazmorra %s\n", jugador.mazmorra_actual->nombre);
            printf("Vidas: %d | Dinero: $%d\n", jugador.vidas, jugador.dinero);
            printf("Comandos: busq, atac, ant, inv\n> ");
        } else {
            printf("\nEstás en la aldea %s\n", jugador.aldea_actual->nombre);
            printf("Vidas: %d | Dinero: $%d\n", jugador.vidas, jugador.dinero);
            printf("Comandos: busq, maz, compr, trans, ant, sig, inv\n> ");
        }
        
        char comando[10];
        scanf("%s", comando);
        
        if (strcmp(comando, "busq") == 0) {
            buscar_item(&jugador);
        } else if (strcmp(comando, "maz") == 0 && !jugador.mazmorra_actual) {
            entrar_mazmorra(&jugador);
        } else if (strcmp(comando, "compr") == 0 && !jugador.mazmorra_actual) {
            comprar(&jugador);
        } else if (strcmp(comando, "trans") == 0 && !jugador.mazmorra_actual) {
            transportar(&jugador);
        } else if (strcmp(comando, "ant") == 0) {
            if (jugador.mazmorra_actual) {
                salir_mazmorra(&jugador);
            } else {
                mover(&jugador, comando);
            }
        } else if (strcmp(comando, "sig") == 0 && !jugador.mazmorra_actual) {
            mover(&jugador, comando);
        } else if (strcmp(comando, "atac") == 0 && jugador.mazmorra_actual) {
            atacar_mazmorra(&jugador);
        } else if (strcmp(comando, "inv") == 0) {
            mostrar_inventario(&jugador);
        } else {
            printf("Comando inválido.\n");
        }
        
        if (verificar_victoria(&jugador, num_aldeas)) {
            printf("\n¡Felicidades! Has derrotado todas las mazmorras y ganado el juego.\n");
            break;
        }
        
        if (jugador.vidas <= 0) {
            printf("\n¡Game Over! Te has quedado sin vidas.\n");
            break;
        }
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
    
    srand(time(NULL));
    MAZMORRAS_PARA_PARALELO = aleatorio(1, num_aldeas);

    jugar(num_aldeas);
    return 0;
}