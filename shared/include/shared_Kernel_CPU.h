#define _POSIX_C_SOURCE 200112L

#ifndef KernelCPU_H_
#define KernelCPU_H_

#include "shared_utils.h"

typedef struct
{
    int id;
    int limite;
} t_Segmento;

/**
 * Crea un segmento, lo agrega a la tabla y lo retorna
 * @param tabla Pasar NULL si no se quiere agregar a ninguna tabla
 */
t_Segmento* CrearSegmento(t_list* tabla, int id, int limite);
void EliminarSegmento(t_list* tabla, int id);

//Crea una tabla de segmentos a partir de los datos recibidos
t_list* ObtenerTablaSegmentosDelPaquete(t_list* DatosRecibidos);

/*
*   Agregar una tabla de segmentos a un paquete. EL PAQUETE DEBE SER DE TIPO PCB_CPU
*   @param paquete: Si pasas NULL se crea un paquete nuevo
*/
t_paquete* AgregarTablaSegmentosAlPaquete(t_paquete* paquete, t_list* tabla);

int ObtenerTamanoSegmento(int IdSegmento, t_list* TablaSegmentos);

#endif