#ifndef MOD_FS_H_
#define MOD_FS_H_

#include "../../shared/include/shared_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <dirent.h>
#include <sys/stat.h>


t_log* FS_Logger;

int SocketFileSystem;
int SocketMemoria;
int SocketKernel;

pthread_t HiloEscucha;
pthread_t HiloAccionKernel;

void* RealizarPeticion(void* arg);

t_config* ConfigsIps;
t_config *config, *configSB;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA;
char* BLOCK_SIZE;
int TAMANO_BLOQUES;
char* BLOCK_COUNT;
uint32_t CANTIDAD_BLOQUES;
void* BLOQUES;
char* PATH_FCB;
char* PATH_BLOQUES;
char* PATH_BITMAP;
char* PATH_SUPERBLOQUE;
int RETARDO_ACCESO_BLOQUE;
int fd;

void InicializarConexiones();
void* EscuchaKernel();
void LeerConfigs(char*);
void LiberarMemoria();
void CrearArchivo(char*);

char NOMBRE_PROCESO[11] = "FileSystem";


//BITMAP:
int ArchivoBitarray;
char* bitarray;
t_bitarray *bitmap;
int InicializarBitmap();
void ImprimirBitmap();
bool BitmapEstaOcupado(uint32_t i);
bool BitmapEstaLibre(uint32_t i);
void BitmapOcuparBloque(uint32_t i);
void BitmapLiberarBloque(uint32_t i);


//BLOQUES
int ArchivoBloques;
char* MapArchivoBloques;
int PunterosPorBloque;
int InicializarArchivoDeBloques();
char* ObtenerBloque(uint32_t IndiceBloque);
void ImprimirBloque(uint32_t IndiceBloque, int Desplazamiento, int CantidadAImpimir);
void EscribirBloqueDeChar(uint32_t IndiceBloque, int Desplazamiento, char* ContenidoAEscribir);
char* LeerBloqueDeChar(uint32_t IndiceBloque, int Desplazamiento, int CantidadALeer);
void EscribirBloqueDePunteros(uint32_t IndiceBloque, int IndicePuntero, uint32_t ContenidoAEscribir);
uint32_t LeerBloqueDePunteros(uint32_t IndiceBloque, int IndicePuntero);


//FCBs
const char* PathDirectorioFCBs = "PERSISTIDOS/FCB/";
t_list* ListaFCBs;
int InicializarFCBs();
void CrearFCB(char* NombreArchivo, int TamanoArchivo, uint32_t Puntero_Directo, uint32_t Puntero_Indirecto);
t_config* BuscarFCB(char* NombreArchivo);
void ModificarValorFCB(t_config* FCB, char* KEY, char* Valor);


//Funciones de Archivos
void CrearArchivo(char* NombreArchivo);
void TruncarArchivo(char* NombreArchivo, int NuevoTamanoArchivo);
uint32_t CalcularPunteroABloqueDePunteroArchivo(char* NombreArchivo, int PunteroArchivo, int* DesplazamientoEnBloqueBuscado);
void EscribirArchivo(char* NombreArchivo, int PunteroArchivo, char* ContenidoAEscribir, int CantBytesAEscribir);
char* LeerArchivo(char* NombreArchivo, int PunteroArchivo, int CantBytesALeer);


//FUNCIONES DE FINALIZACION DEL MODULO
void TerminarModulo();
void PersistirDatos();
void CerrarArchivoDeBloques();
void CerrarArchivoDeBitmap();
void CerrarFCBs();

#endif
