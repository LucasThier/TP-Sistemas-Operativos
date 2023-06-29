#include "../include/shared_Kernel_CPU.h"


t_Segmento* CrearSegmento(t_list* tabla, int id, int limite)
{
    t_Segmento* segmento = malloc(sizeof(t_Segmento));
    segmento->id = id;
    segmento->limite = limite;
    if(tabla != NULL)       
        list_add(tabla, segmento);
    return segmento;
}


void EliminarSegmento(t_list* tabla, int id)
{
    for(int i = 0; i < list_size(tabla); i++)
    { 
        t_Segmento* segmento = (t_Segmento*)list_get(tabla, i);
        if(segmento->id == id)
        {
            list_remove(tabla, i);
            free(segmento);
            break;
        }
    }
}

t_list* ObtenerTablaSegmentosDelPaquete(t_list* DatosRecibidos)
{
    t_list* Tabla = list_create();

    int* aux2 = (int*)list_remove(DatosRecibidos, 0);
    int CantSegmentos = *aux2;
    free(aux2);

    for(int i=0; i < CantSegmentos; i++)
    {
        aux2 = (int*)list_remove(DatosRecibidos, 0);
        int ID = *aux2;
        free(aux2);

        aux2 = (int*)list_remove(DatosRecibidos, 0);
        int Limite = *aux2;
        free(aux2);

        CrearSegmento(Tabla, ID, Limite);
    }
    return Tabla;
}

t_paquete* AgregarTablaSegmentosAlPaquete(t_paquete* paquete, t_list* tabla)
{
    t_paquete* pqt = paquete;
    if (paquete == NULL)
        pqt = crear_paquete(CPU_PCB);
    
    int cantSegmentos = list_size(tabla);
    agregar_a_paquete(pqt, &cantSegmentos, sizeof(int));

    for(int i=0; i < cantSegmentos; i++)
    {
        t_Segmento* segmento = (t_Segmento*)list_get(tabla, i);
        agregar_a_paquete(pqt, &(segmento->id), sizeof(int));
        agregar_a_paquete(pqt, &(segmento->limite), sizeof(int));
    }
    return pqt;
}

int ObtenerTamanoSegmento(int IdSegmento, t_list* TablaSegmentos)
{
    for(int i = 0; i < list_size(TablaSegmentos); i++)
    {
        t_Segmento* segmento = (t_Segmento*)list_get(TablaSegmentos, i);
        if(segmento->id == IdSegmento)
            return segmento->limite;
    }
    return -1;
}