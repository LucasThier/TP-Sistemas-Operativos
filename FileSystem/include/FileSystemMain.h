#ifndef MOD_FS_H_
#define MOD_FS_H_

#include "../../shared/include/shared_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <dirent.h>


t_log* FS_Logger;

int SocketFileSystem;
int SocketMemoria;
int SocketKernel;

pthread_t HiloEscucha;

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
int fd;

void InicializarConexiones();
void* EscuchaKernel();
void LeerConfigs(char*, char*);
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
char* PathDirectorioFCBs = "PERSISTIDOS/FCB/";
t_list* ListaFCBs;
int InicializarFCBs();
void CrearFCB(char* NombreArchivo, int TamanoArchivo, uint32_t Puntero_Directo, uint32_t Puntero_Indirecto);
t_config* BuscarFCB(char* NombreArchivo);
void ModificarValorFCB(t_config* FCB, char* KEY, char* Valor);



//FUNCIONES DE FINALIZACION DEL MODULO
void TerminarModulo();
void PersistirDatos();
void CerrarArchivoDeBloques();
void CerrarArchivoDeBitmap();
void CerrarFCBs();

#endif
