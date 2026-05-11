#include "tdas/extra.h"
#include "tdas/list.h"
#include "tdas/map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//Definicion del tipo pelicula para poder almacenar cada pelicula con su informacion completa
typedef struct{
  char id[100];
  char titulo[100];
  List *generos;
  List *directores;
  int year;
  float IMBD;
} Pelicula;

//Funcion utilizada para que todas las palabras sean iguales y no haya errores de comparacion a futuro 
//en caso de haberse ingresado una completamente en minusculas y despues mezclando minusculas y mayusculas
//En resumen: DRAMA = draMA = Drama = drama = dRaMA....etc
void normalizarPalabras(char *str){
  if (str == NULL) return;
  for (int i = 0; str[i]; i++){
    str[i] = tolower((unsigned char)str[i]);
  }
}

int is_equal_str(void *key1, void *key2) {
  if (key1 == NULL || key2 == NULL) return 0;
  return strcmp((char *)key1, (char *)key2) == 0;
}

int is_equal_int(void *key1, void *key2) {
  if (key1 == NULL || key2 == NULL) return 0;
  return (*(int *)key1 == *(int *)key2);
}

//Funciones para mostrar las opciones
void mostrarMenu(){
  printf("SELECCIONA UNA OPCION PARA COMENZAR\n");
  printf("===================================\n\n");
  printf("1 - Cargar Catalogo\n");
  printf("2 - Buscar por Genero\n");
  printf("3 - Buscar por Director\n");
  printf("4 - Buscar por Decada\n");
  printf("5 - Busqueda Avanzada\n");
  printf("6 - Gestionar WatchList\n");
  printf("0 - Para Terminar\n");
  printf("===================================\n");
}

void mostrarOpcionesWatchList(){
  printf("\n--- MENU WATCHLIST ---\n");
  printf("1. AGREGAR PELICULA\n");
  printf("2. ELIMINAR PELICULA\n");
  printf("3. MOSTRAR WATCHLIST\n");
  printf("0. VOLVER AL MENU PRINCIPAL\n");
}

//Funcion para cargar el archivo "Top1500.csv"
//mapa_genero: mapa que asocia nombres de generos con listas de peliculas 
//mapa_director: mapa que asocia nombres de directores con listas de peliculas 
//mapa_decada: mapa que asocia años de inicio de decadas con listas de peliculas 
//mapa_id: mapa que asocia IDs unicos con estructuras de peliculas 

void cargarPeliculas(Map *mapa_genero, Map *mapa_director, Map *mapa_decada, Map *mapa_id){
  FILE *archivo = fopen("data/Top1500.csv", "r");
  if (archivo == NULL){
    perror("No se pudo abrir el archivo");
    return;
  }
  printf("\n--- ARCHIVO CARGADO CORRECTAMENTE ---\n\n");
  char **campos; //Arreglo de caracteres para cada columna del archivo
  campos = leer_linea_csv(archivo, ',');

  while((campos = leer_linea_csv(archivo, ',')) != NULL){
    Pelicula *peli = (Pelicula *)malloc(sizeof(Pelicula));

    strcpy(peli -> id, campos[1]);
    strcpy(peli -> titulo, campos[5]);
    peli -> IMBD = atof(campos[8]);
    peli -> year = atoi(campos[10]);

    peli -> generos = split_string(campos[11], ",");
    peli -> directores = split_string(campos[14], ",");

    map_insert(mapa_id, strdup(peli -> id), peli);

    char *gen = list_first(peli -> generos);
    while(gen != NULL){
      if (gen[0] == ' ') gen ++; 

      char *genANormalizar = strdup(gen);
      normalizarPalabras(genANormalizar);
      
      MapPair *pair = map_search(mapa_genero, genANormalizar);
      if (pair == NULL){
        List *nuevaLista = list_create();
        list_pushBack(nuevaLista, peli);
        map_insert(mapa_genero, genANormalizar, nuevaLista);
      } else {
        List *listaExistente = (List *)pair -> value;
        list_pushBack(listaExistente, peli);
        free(genANormalizar);
      }
      gen = list_next(peli -> generos);
    }

    char *direct = list_first(peli -> directores);
    while(direct != NULL){
      if (direct[0] == ' ') direct ++;

      char *directANormalizar = strdup(direct);
      normalizarPalabras(directANormalizar);

      MapPair *pair = map_search(mapa_director, directANormalizar);
      if(pair == NULL){
        List *nuevaLista = list_create();
        list_pushBack(nuevaLista, peli);
        map_insert(mapa_director, directANormalizar, nuevaLista);
      } else{
        List *listaExistente = (List *)pair -> value;
        list_pushBack(listaExistente, peli);
        free(directANormalizar);
      }
      direct = list_next(peli -> directores);
    }

    int *decada = (int*)malloc(sizeof(int));
    *decada = peli -> year - (peli -> year % 10);
    MapPair *pairDec = map_search(mapa_decada, decada);
    if (pairDec == NULL){
      List *nuevaL = list_create();
      list_pushBack(nuevaL, peli);
      map_insert(mapa_decada, decada, nuevaL);
    } else{
      list_pushBack((List *)pairDec -> value, peli);
      free(decada);
    }
  }
  fclose(archivo);
}

//Funcion creada para filtrar peliculas por el genero que ingrese el usuario
//mapa_genero: al ser el mapa que asocia genero con listas de peliculas este es usado para buscar la existencia
//de un genero, y en caso de haberla mosrtar por pantalla las peliculas de la lista asociada 
void buscarPorGenero(Map *mapaGenero){
  printf("\nIngresa el genero que deseas buscar : \n");
  char genero[50];

  int c;
  while ((c = getchar()) != '\n' && c != EOF);
  
  if (fgets(genero, 50, stdin) == NULL) return;
  genero[strcspn(genero, "\n")] = 0;
  normalizarPalabras(genero);

  MapPair *par = map_search(mapaGenero, genero);
  if (par != NULL) {
    List *listaPelis = (List *)par -> value;
    printf("\nPeliculas del genero '%s':\n", genero);

    Pelicula *aux = list_first(listaPelis);
    while(aux != NULL){
      printf("\nID: %-10s | Titulo: %s", aux -> id, aux -> titulo);
      printf("\nAño: %-9d | IMDb: %.1f", aux -> year, aux -> IMBD);
      printf("\nGenero(s): ");
      char *generosPelicula = list_first(aux -> generos);
      while(generosPelicula != NULL){
        printf("%s ", generosPelicula);
        generosPelicula = list_next(aux -> generos);
      }
      printf("\nDirector(es): ");
      char *directoresPelicula = list_first(aux -> directores);
      while(directoresPelicula != NULL) {
        printf("%s", directoresPelicula);
        directoresPelicula = list_next(aux -> directores);
        if (directoresPelicula != NULL) printf(", ");
      }
      printf("\n-------------------------\n\n");
      aux = list_next(listaPelis);
    }
  }else{
    printf("No se encontraron peliculas para el genero: %s\n", genero);
  }
}

//Funcion para filtrar peliculas por el director ingresado por el usuario
//mapaDirector: al ser el mapa que asocia director con lista de peliculas este es usado para buscar la existencia 
//del director deseado, y en caso de existir mostrar la lista de peliculas asociadas al mismo
void buscarPorDirector(Map *mapaDirector) {
  printf("\nIngresa el director que deseas buscar :\n");
  char director[50];

  int c;
  while((c = getchar()) != '\n' && c != EOF);

  if (fgets(director, 50, stdin) == NULL) return;
  director[strcspn(director, "\n")] = 0;
  normalizarPalabras(director);
  
  MapPair *par = map_search(mapaDirector, director);
  if (par != NULL) {
    List *listaPelis = (List *)par -> value;
    printf("\nPeliculas del director '%s':\n", director);

    Pelicula *aux = list_first(listaPelis);
    while(aux != NULL){
      printf("- %s (%d)\n", aux -> titulo, aux -> year);
      aux = list_next(listaPelis);
    }
    printf("-------------------------\n\n");
  } else{
    printf("No se encontraron peliculas del director: %s\n", director);
  }
}

//Funcion para filtrar peliculas segun la decada escogida por el usuario 
//mapaDecada: al ser el mapa que asocia la decada con una lista de peliculas, aqui todas las peliculas ingresadas 
//fueron asignadas a la decada = añoPelicula - añoPelicula % 10, se busca la decada ingresada por el usuario 
//y se muestran las peliculas de cuya decada
void buscarPorDecada(Map *mapaDecada){
  printf("Ingresa la decada que quieres investigar :\n");
  int decada;
  scanf("%i", &decada);
  decada -= decada%10; //garantizar que el usuario ingreso una decada y no un año especifico
  
  MapPair *par = map_search(mapaDecada, &decada);
  if (par != NULL){
    List *listaPelis = (List *)par -> value;
    Pelicula *aux = list_first(listaPelis);
    
    while(aux != NULL){
      printf("- %s (%d)\n", aux -> titulo, aux -> year);
      aux = list_next(listaPelis);
    }
    printf("-------------------------\n\n");
  } else{
    printf("No hay peliculas de la decada: %i\n", decada);
  }
}

//Funcion busquedaAvanzada mezcla la busqueda por genero con la busqueda por decada, el usuario ingresa un genero que quiere 
//filtrar, y posteriormente una decada la cual le gustaria ver, con esto por ejemplo se mostraria por pantalla 
//todas las peliculas de horror en la decada del 2000
void busquedaAvanzada(Map *mapaDecada, Map *mapaGenero){
  int c; 
  while ((c = getchar()) != '\n' && c != EOF);

  printf("Ingrese el genero que quiere revisar:\n");
  char genero[50];
  fgets(genero, 50, stdin);
  genero[strcspn(genero, "\n")] = 0;
  normalizarPalabras(genero);

  printf("Ingrese la decada: \n");
  int decada;
  scanf("%i", &decada);
  decada = decada - (decada%10); //para confirmar que sea una decada y no un año especifico
  MapPair *par = map_search(mapaGenero, genero);
  if (par != NULL) {
    List *listaPelis = (List *)par -> value;
    Pelicula *aux = list_first(listaPelis);

    printf("\n--- %s en la decada del %i ---\n", genero, decada);
    int encontrados = 0;
    while(aux != NULL){
      int decadaPeli = aux -> year - (aux ->year % 10);
      if (decadaPeli == decada){
        printf("- %s (%i)\n", aux -> titulo, aux -> year);
        encontrados ++;
      }
      aux = list_next(listaPelis);
    }
    if (encontrados != 0) printf("-------------------------\n\n");
    if (encontrados == 0) {
      printf("No se hallaron peliculas de %s en la decada %i\n", genero, decada);
    }
  } else{
    printf("El genero '%s' no fue encontrado\n", genero);
  }
}

//Funcion para la lista de peliculas creada por el usuario
//watch: una lsita creada en el main, es esta la que sera rellenada, se le borraran datos o sera mostrada
//mapa_id: como se pide ingresar el ID de la pelicula que se quiere agregar/eliminar, este mapa es usado 
//para buscar la existencia de ese ID y asi agregar a la lista watch la pelicula correspondiente
void procesarWatchList(List* watch, Map *mapa_id){
  int opcion;
  char Id[100];
  
  do {
    mostrarOpcionesWatchList();
    scanf("%i", &opcion);

    switch(opcion){
      case 1: {
        printf("Ingrese ID de la pelicula para añadir:\n");
        int c;
        while((c = getchar()) != '\n' && c != EOF);
        
        if (fgets(Id, sizeof(Id), stdin) != NULL){
          Id[strcspn(Id, "\n")] = 0;
          MapPair *par = map_search(mapa_id, Id);
          if (par != NULL){
            list_pushBack(watch, par -> value);
            printf("Pelicula añadida\n");
          } else {
            printf("ID no encontrado\n");
          }
        }
        break;
      }
      case 2: {
        printf("Ingrese ID de la pelicula para eliminar:\n");
        int c;
        while((c = getchar()) != '\n' && c != EOF);
        
        if (fgets(Id, sizeof(Id), stdin) != NULL) {
          Id[strcspn(Id, "\n")] = 0;
          Pelicula *aux = list_first(watch);
          int eliminado = 0;
          while(aux != NULL) {
            if (strcmp(aux -> id, Id) == 0) {
              list_popCurrent(watch);
              eliminado = 1;
              break;
            }
            aux = list_next(watch);
          }
          if(eliminado) printf("Pelicula eliminada del watchList\n");
          else printf("No se encontro ese ID en la watchList\n");
        }
        break;
      }
      case 3: {
        printf("\n--- WATCHLIST ACTUAL ---\n");
        Pelicula *aux = list_first(watch);
        if (aux == NULL){
          printf("\nLa lista esta vacia\n");
        } else{
          int i = 1;
          while(aux != NULL){
            printf("%i. [%s] %s (%i)\n", i, aux -> id, aux -> titulo, aux -> year);
            i ++;
            aux = list_next(watch);
          }
        }
        break;
      }

      default:{
        printf("Opcion invalida \n");
        break;
      }
    }
  } while (opcion != 0);
  printf("Proceso Finalizado\n");
}

//Funcion main
int main(){
  int opcion;
  
  Map *peliculasGenero = map_create(is_equal_str);
  Map *peliculasDirector = map_create(is_equal_str);
  Map *peliculasDecada = map_create(is_equal_int);
  Map *peliculasId= map_create(is_equal_str);
  List *watchList = list_create();
  
  do {
    mostrarMenu();
    scanf("%i", &opcion);
    switch(opcion) {
      case 1:{
        //Cargar archivo
        cargarPeliculas(peliculasGenero, peliculasDirector, peliculasDecada, peliculasId);
        break;
      }
      
      case 2: {
        //BUSCAR POR GENERO
        buscarPorGenero(peliculasGenero);
        break;
      }
      
      case 3: {
        //BUSCAR POR DIRECTOR
        buscarPorDirector(peliculasDirector);
        break;
      }

      case 4:{
        //BUSCAR POR DECADA
        buscarPorDecada(peliculasDecada);
        break;
      }

      case 5:{
        //BUSQUEDA AVANZADA
        busquedaAvanzada(peliculasDecada, peliculasGenero);
        break;
      }

      case 6:{
        //WATCHLIST
        procesarWatchList(watchList, peliculasId);
      }
      
      default: {
        printf("Opcion Invalida\n");
        break;
      }
    }
  } while(opcion != 0);
  printf("Proceso Finalizado\n");
  return 0;
}