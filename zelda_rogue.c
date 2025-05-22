#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Definiciones de constantes para el juego
#define MAX_NAMES 4
#define MAX_LIVES 127
#define PRECIO_VIDA 5
#define PRECIO_ITEM_PRIMERA_MAZMORRA 25
#define PRECIO_VIDA_ADICIONAL 100
#define DINERO_CAMINO 10
int MAZMORRAS_PARA_PARALELO; // Mazmorras necesarias para desbloquear el mundo paralelo

// Nombres predefinidos para mazmorras, aldeas e ítems
char *nombres_mazmorras[] = {"Agua", "Tierra", "Fuego", "Aire"};
char *nombres_aldeas[] = {"Kakariko", "Hateno", "Gerudo", "Zora"};
char *nombres_items[] = {"Espada", "Escudo", "Arco", "Bomba"};

// Estructura para los ítems del juego
typedef struct Item {
    char nombre[50];
    bool encontrado;
    struct Item *siguiente;
} Item;

// Estructura para las mazmorras
typedef struct Mazmorra {
    char nombre[50];
    Item *item_requerido;   // Ítem necesario para derrotar la mazmorra
    Item *item_oculto;      // Ítem oculto dentro de la mazmorra
    bool derrotada;
    struct Mazmorra *siguiente;
} Mazmorra;

// Estructura para las aldeas
typedef struct Aldea {
    char nombre[50];
    Mazmorra *mazmorra_asociada;
    Item *item_oculto;      // Ítem oculto en la aldea
    struct Aldea *anterior;
    struct Aldea *siguiente;
    struct Aldea *paralela; // Aldea correspondiente en el otro mundo
    bool es_mundo_superior; // Flag para saber en qué mundo está
} Aldea;

// Apuntadores globales a las cabezas de los mundos
Aldea *cabeza_mundo_superior = NULL;
Aldea *cabeza_mundo_paralelo = NULL;

// Estructura para el jugador
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

// Prototipos de funciones (declaraciones)
char* combinar_nombres(int indice, char **base);
Item* crear_item();
Item* asignar_item_aleatorio();
Aldea* crear_aldeas(int num_aldeas, bool es_superior);
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

// Función para obtener un número aleatorio entre min y max
int aleatorio(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Combina nombres para generar nombres únicos de aldeas o mazmorras
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

// Crea un nuevo ítem aleatorio
Item* crear_item() {
    Item *item = malloc(sizeof(Item));
    strcpy(item->nombre, nombres_items[rand() % MAX_NAMES]);
    item->encontrado = false;
    item->siguiente = NULL;
    return item;
}

// Asigna un ítem aleatorio (wrapper de crear_item)
Item* asignar_item_aleatorio() {
    return crear_item();
}

// Crea una lista enlazada de aldeas para un mundo
Aldea* crear_aldeas(int num_aldeas, bool es_superior_flag_val) {
    Aldea *head = NULL;
    Aldea *current = NULL;
    for (int i = 0; i < num_aldeas; i++) {
        Aldea *aldea = malloc(sizeof(Aldea));
        if (!aldea) { return NULL; }

        char* nombre_base_temp = combinar_nombres(i, nombres_aldeas);

        strncpy(aldea->nombre, nombre_base_temp, sizeof(aldea->nombre) - 1);
        aldea->nombre[sizeof(aldea->nombre) - 1] = '\0';
        free(nombre_base_temp);

        aldea->mazmorra_asociada = NULL;
        aldea->item_oculto = (rand() % 2 == 0) ? crear_item() : NULL;
        aldea->anterior = current;
        aldea->siguiente = NULL;
        aldea->paralela = NULL;
        aldea->es_mundo_superior = es_superior_flag_val;

        if (!head) head = aldea;
        else current->siguiente = aldea;
        current = aldea;
    }
    return head;
}

// Asigna una mazmorra a cada aldea
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

// Enlaza aldeas paralelas entre los dos mundos
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

// Permite buscar un ítem en la ubicación actual (aldea o mazmorra)
void buscar_item(Jugador *jugador) {
    if (jugador->mazmorra_actual) {
        // Lógica para buscar en mazmorra
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
        // Lógica para buscar en aldea
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

// Permite entrar a la mazmorra asociada a la aldea actual
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

// Permite salir de la mazmorra y volver a la aldea
void salir_mazmorra(Jugador *jugador) {
    if (!jugador->mazmorra_actual) {
        printf("No estás en una mazmorra.\n");
        return;
    }
    // Busca la aldea asociada a la mazmorra actual
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

// Permite atacar la mazmorra actual, verifica si el jugador tiene el ítem necesario
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
        if (jugador->vidas <= 0) return;
    }
    if (tiene_item(jugador, jugador->mazmorra_actual->item_requerido->nombre)) {
        jugador->mazmorra_actual->derrotada = true;
        printf("¡Has derrotado la mazmorra %s!\n", jugador->mazmorra_actual->nombre);

        // Imprime el estado actualizado del mundo
        printf("\n=== Estado actualizado del mundo ===\n");
        if (jugador->aldea_actual->es_mundo_superior) {
            imprimir_mundo(cabeza_mundo_superior);
        } else {
            imprimir_mundo(cabeza_mundo_paralelo);
        }
        
        // Actualiza el contador de mazmorras derrotadas
        if (jugador->aldea_actual->es_mundo_superior) {
            jugador->mazmorras_derrotadas_superior++;
        } else {
            jugador->mazmorras_derrotadas_paralelo++;
        }
        
        // Desbloquea el mundo paralelo si corresponde
        if (jugador->aldea_actual->es_mundo_superior && 
            jugador->mazmorras_derrotadas_superior >= MAZMORRAS_PARA_PARALELO &&
            !jugador->mundo_paralelo_desbloqueado) {
            
            jugador->mundo_paralelo_desbloqueado = true;
            printf("¡Has desbloqueado el mundo paralelo!\n");
            if (jugador->aldea_actual->paralela) {
                printf("Saliendo de la mazmorra %s para el transporte...\n", jugador->mazmorra_actual->nombre);
                jugador->mazmorra_actual = NULL; 
                jugador->aldea_actual = jugador->aldea_actual->paralela;
                printf("Has sido transportado automáticamente al %s. Ahora estás en %s\n",
                       (jugador->aldea_actual->es_mundo_superior) ? "mundo superior" : "mundo paralelo",
                       jugador->aldea_actual->nombre);

                // Imprime el estado del mundo al que llegas
                if (jugador->aldea_actual->es_mundo_superior) {
                    printf("\n=== Mundo Superior ===\n");
                    imprimir_mundo(cabeza_mundo_superior);
                    printf("======================\n");
                } else {
                    printf("\n=== Mundo Paralelo ===\n");
                    imprimir_mundo(cabeza_mundo_paralelo);
                    printf("======================\n");
                }
            }
        } 
    } else {
        printf("No tienes el ítem necesario (%s) para derrotar esta mazmorra.\n", 
               jugador->mazmorra_actual->item_requerido->nombre);
    }
}

// Permite comprar vidas o ítems en la tienda de la aldea
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
                // Compra el ítem requerido para la primera mazmorra
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

// Permite transportarse entre mundos si está desbloqueado
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
        printf("Error crítico: No hay aldea paralela físicamente asociada a esta aldea.\n");
        return;
    }
    if (!jugador->aldea_actual->es_mundo_superior && jugador->mazmorras_derrotadas_paralelo == 0) {
        printf("Debes derrotar al menos una mazmorra en el mundo paralelo antes de regresar al mundo superior.\n");
        return;
    }
    jugador->aldea_actual = jugador->aldea_actual->paralela;
    printf("Te has transportado al %s. Ahora estás en %s\n",
           (jugador->aldea_actual->es_mundo_superior) ? "mundo superior" : "mundo paralelo",
           jugador->aldea_actual->nombre);

    // Imprime el estado del mundo al que llegas
    if (jugador->aldea_actual->es_mundo_superior) {
        imprimir_mundo(cabeza_mundo_superior);
    } else {
        imprimir_mundo(cabeza_mundo_paralelo);
    }
}

// Permite moverse entre aldeas (anterior o siguiente)
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

// Verifica si el jugador ha ganado (derrotó todas las mazmorras en ambos mundos)
bool verificar_victoria(Jugador *jugador, int num_aldeas) {
    if (jugador->mazmorras_derrotadas_superior >= num_aldeas && 
        jugador->mazmorras_derrotadas_paralelo >= num_aldeas) {
        return true;
    }
    return false;
}

// Libera toda la memoria dinámica usada por las aldeas y sus estructuras asociadas
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

// Imprime el estado de todas las aldeas y mazmorras de un mundo
void imprimir_mundo(Aldea *aldea) {
    while (aldea) {
        printf("Aldea: %s\n", aldea->nombre);
        if (aldea->mazmorra_asociada) {
            printf("  Mazmorra: %s\n", aldea->mazmorra_asociada->nombre);
            printf("  Ítem requerido: %s\n", aldea->mazmorra_asociada->item_requerido->nombre);
            if (aldea->mazmorra_asociada->item_oculto) {
                printf("  Ítem oculto: %s (%s)\n",
                    aldea->mazmorra_asociada->item_oculto->nombre,
                    aldea->mazmorra_asociada->item_oculto->encontrado ? "encontrado" : "no encontrado"
                );
            } else {
                printf("  Ítem oculto: Ninguno\n");
            }
            printf("  Derrotada: %s\n", aldea->mazmorra_asociada->derrotada ? "Sí" : "No");
        }
        if (aldea->item_oculto) {
            printf("Ítem oculto en aldea: %s (%s)\n", aldea->item_oculto->nombre, 
                   aldea->item_oculto->encontrado ? "encontrado" : "no encontrado");
        }
        aldea = aldea->siguiente;
    }
    printf("======================\n\n");
}

// 25% de probabilidad de perder una vida en ciertas acciones
bool perder_vida_aleatorio() {
    return (rand() % 4) == 0;
}

// Agrega un ítem al inventario del jugador
void agregar_item_inventario(Jugador *jugador, Item *item) {
    item->siguiente = jugador->inventario;
    jugador->inventario = item;
}

// Verifica si el jugador tiene un ítem específico en su inventario
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

// Muestra el inventario del jugador
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

// Función principal del juego: ciclo de comandos y lógica principal
void jugar(int num_aldeas) {
    srand(time(NULL));
    Aldea *mundo_superior = crear_aldeas(num_aldeas, true);
    Aldea *mundo_paralelo = crear_aldeas(num_aldeas, false);

    // Asigna las cabezas globales
    cabeza_mundo_superior = mundo_superior;
    cabeza_mundo_paralelo = mundo_paralelo;
    
    asignar_mazmorras(mundo_superior, num_aldeas);
    asignar_mazmorras(mundo_paralelo, num_aldeas);
    enlazar_mundos(mundo_superior, mundo_paralelo);

    // Inicializa el jugador
    Jugador jugador = {3, 0, NULL, mundo_superior, NULL, false, 0, 0};

    // Imprime los mundos generados (para depuración)
    printf("\n=== Mundo Superior ===\n");
    imprimir_mundo(mundo_superior);
    printf("\n=== Mundo Paralelo ===\n");
    imprimir_mundo(mundo_paralelo);

    // Bucle principal del juego
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
        // Procesa el comando ingresado por el usuario
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
        // Verifica si el jugador ha ganado o perdido
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

// Punto de entrada del programa
int main() {
    int num_aldeas;
    printf("Ingrese el número de aldeas: ");
    scanf("%d", &num_aldeas);
    if (num_aldeas < 1) {
        printf("Debe haber al menos 1 aldea.\n");
        return 1;
    }
    srand(time(NULL));
    MAZMORRAS_PARA_PARALELO = aleatorio(1, num_aldeas); // Número aleatorio de mazmorras para desbloquear el mundo paralelo
    jugar(num_aldeas);
    return 0;
}