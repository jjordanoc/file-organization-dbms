# Proyecto 1 BD2

## Introducción 


## Diccionario de datos

```c++
struct MovieRecord {
    int dataId{};
    char contentType[16]{'\0'};
    char title[256]{'\0'};
    short length{};
    short releaseYear{};
    short endYear{};
    int votes{};
    float rating{};
    int gross{};
    char certificate[16]{'\0'};
    char description[512]{'\0'};
```

|       Campo       |                         Descripción                          |
|:-----------------:|:------------------------------------------------------------:|
|   ```dataId```    | Id único de cada registro, sirve para identificar una tupla. |
| ```contentType``` |         Tipo de contenido, puede ser Movie or TvShow         |
|    ```title```    |                     Título del contenido                     |
|   ```length```    |              Longitud del contenido en minutos               |
| ```releaseYear``` |               Año de lanzamiento del contenido               |
|   ```endYear```   |      Finalización del contenido (en caso sea un TvShow)      |
|    ```votes```    |     Cantidad de personas que realizaron una calificación     |
|   ```rating```    |            La calificación promedio del contenido            |
|    ```gross```    |                El ingreso neto del contenido                 |
| ```certificate``` |                 Calificación del certificado                 |
| ```description``` |                  Descripción del contenido                   |
