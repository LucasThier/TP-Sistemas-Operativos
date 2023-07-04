#include "../include/FileSystemMain.h"

void sighandler(int s) {

	TerminarModulo();
	exit(0);
}

int main(int argc, char* argv[])
{
	signal(SIGINT, sighandler);
	signal(SIGSEGV, sighandler);

	FS_Logger = log_create("FileSystem.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);
	ListaFCBs = list_create();

	//leer las config
	LeerConfigs(argv[1],argv[2]);

	if(InicializarBitmap() == 1)
	{
		return EXIT_FAILURE;
	}

	if(InicializarArchivoDeBloques() == 1)
	{
		return EXIT_FAILURE;
	}

	if(InicializarFCBs() == 1)
	{
		return EXIT_FAILURE;
	}

	CrearFCB("HolaQionda", 100, 41, 55);
	CrearFCB("HolaQionda2", 101, 42, 56);
	CrearFCB("HolaQionda3", 102, 43, 57);

	InicializarConexiones();

	while(true){
		log_info(FS_Logger, "hilo principal esta ejecutando");
		sleep(10);
	}

	return EXIT_SUCCESS;
}

//Crea un hilo de escucha para el kernel
void InicializarConexiones()
{
	SocketMemoria = conectar_servidor(FS_Logger, "Memoria", IP_MEMORIA , PUERTO_MEMORIA);

    if (pthread_create(&HiloEscucha, NULL, EscuchaKernel, NULL) != 0) {
        exit(EXIT_FAILURE);
    }

}

//Crea un servidor y espera al kernel, luego recibe mensajes del mismo
void* EscuchaKernel()
{
	SocketFileSystem = iniciar_servidor(FS_Logger, NOMBRE_PROCESO, "0.0.0.0", PUERTO_ESCUCHA);
	
	if(SocketFileSystem == 0)
	{
		liberar_conexion(SocketFileSystem);
		return (void*)EXIT_FAILURE;
	}

	int SocketKernel = esperar_cliente(FS_Logger, NOMBRE_PROCESO, SocketFileSystem);

	if (SocketKernel == 0)
	{
		liberar_conexion(SocketFileSystem);
		liberar_conexion(SocketKernel);
		return (void*)EXIT_FAILURE;
	}
	
	while(true)
	{
		char* PeticionRecibida = (char*)recibir_paquete(SocketKernel);

		char* Pedido = strtok(PeticionRecibida, " ");

		if(strcmp(Pedido, "ABRIR_ARCHIVO")==0)
		{
			char* NombreArchivo = strtok(NULL, " ");
			
			//verificar si existe el archivo,
			//si existe enviar "OK" si no enviar cualquier otra cadena
			if(BuscarFCB(NombreArchivo) != NULL)
			{
				EnviarMensage("OK", SocketKernel);
			}
			else
			{
				EnviarMensage("NO", SocketKernel);
			}			
		}

		else if(strcmp(Pedido, "CREAR_ARCHIVO")==0)
		{
			char* NombreArchivo = strtok(NULL, " ");

			//crear el archivo y enviar un "OK" al finalizar
			CrearArchivo(NombreArchivo);

			EnviarMensage("OK", SocketKernel);
		}

		else if(strcmp(Pedido, "TRUNCAR_ARCHIVO") == 0)
		{
			char* NombreArchivo = strtok(NULL, " ");
			char* NuevoTamanoArchivo = strtok(NULL, " ");
			
			//modificar el tamao del archivo y avisar con un "TERMINO" cuando termine la operacion
			TruncarArchivo(NombreArchivo, atoi(NuevoTamanoArchivo));

			EnviarMensage("TERMINO", SocketKernel);
		}

		else if(strcmp(Pedido, "LEER_ARCHIVO") == 0)
		{
			char* NombreArchivo = strtok(NULL, " ");
			char* PID = strtok(NULL, " ");
			char* NumSegmento = strtok(NULL, " ");
			char* Offset = strtok(NULL, " ");
			char* CantBytesALeer = strtok(NULL, " ");
			char* PunteroArchivo = strtok(NULL, " ");

			//leer la cantidad de bytes pedidos a partir de la pos del puntero
			//y gardar lo leido en la direccion fisica de la Memoria.
			//avisar con un "TERMINO" cuando termine la operacion.

			char* DatosLeidos = LeerArchivo(NombreArchivo, atoi(PunteroArchivo), atoi(CantBytesALeer));

			char Mensaje[800];
			sprintf(Mensaje, "MOV_OUT %s %s %s %s FS\0", PID, NumSegmento, Offset, DatosLeidos);
			EnviarMensage(Mensaje, SocketMemoria);
			

			EnviarMensage("TERMINO", SocketKernel);
		}

		else if(strcmp(Pedido, "ESCRIBIR_ARCHIVO") == 0)
		{
			char* NombreArchivo = strtok(NULL, " ");
			char* PID = strtok(NULL, " ");
			char* NumSegmento = strtok(NULL, " ");
			char* Offset = strtok(NULL, " ");
			char* CantBytesAEscribir = strtok(NULL, " ");
			char* Puntero = strtok(NULL, " ");
			
			//leer la cantidad de bytes pedidos en la direccion fisica de la Memoria
			//y guardar lo leido a partir de la pos del puntero
			//avisar con un "TERMINO" cuando termine la operacion.
			
			char Mensaje[800];
			sprintf(Mensaje, "MOV_IN %s %s %s %s FS\0", PID, NumSegmento, Offset, CantBytesAEscribir);
			EnviarMensage(Mensaje, SocketMemoria);
		
			char* Contenido = (char*)recibir_paquete(SocketMemoria);

			EscribirArchivo(NombreArchivo, atoi(Puntero), Contenido, atoi(CantBytesAEscribir));
			
			EnviarMensage("TERMINO", SocketKernel);
		}
	}
	return NULL;	
}

void LeerConfigs(char* path, char* path_superbloque)
{
	config = config_create(path);

	IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");

	PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");

	PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");

	PATH_FCB = config_get_string_value(config, "PATH_FCB");

	PATH_BLOQUES = config_get_string_value(config, "PATH_BLOQUES");


	configSB = config_create(path_superbloque);

	BLOCK_SIZE = config_get_string_value(configSB, "BLOCK_SIZE");
	TAMANO_BLOQUES = atoi(BLOCK_SIZE);

	BLOCK_COUNT = config_get_string_value(configSB, "BLOCK_COUNT");
	CANTIDAD_BLOQUES = strtoul(BLOCK_COUNT, NULL, 10);

	PunterosPorBloque = TAMANO_BLOQUES / sizeof(uint32_t);
}




//crea/abre el archivo, lo mapea, y crea el Bitarray usando las commons
int InicializarBitmap()
{
	printf("Inicializando Bitmap\n");
	char* PathArchivo = "PERSISTIDOS/Bitmap.dat";
	bool ArchivoExiste = false;

	//calcular el tamano en bytes del archivo
	size_t CantidadBytes = CANTIDAD_BLOQUES / 8;
    if (CANTIDAD_BLOQUES % 8 != 0)
	{
    	CantidadBytes++;
	}

	//Si el archivo SI existe
	if(access(PathArchivo, F_OK) == 0)
	{
		ArchivoExiste = true;
	}

	// Abrir/crear el archivo
    ArchivoBitarray = open(PathArchivo, O_RDWR | O_CREAT, 0644);
    if (ArchivoBitarray == -1) {
        log_error(FS_Logger, "Error al abrir/crear el bitmap");
        return 1;
    }

	// Truncar el archivo a la cantidad de bloques
	if(ftruncate(ArchivoBitarray, CantidadBytes) == -1)
	{
		log_error(FS_Logger, "Error al truncar el bitmap");
		return 1;
	}

	// Mapear el archivo a la memoria
    bitarray = mmap(NULL, CantidadBytes, PROT_READ | PROT_WRITE, MAP_SHARED, ArchivoBitarray, 0);
    if (bitarray == MAP_FAILED) {
        perror("Error al mapear el archivo a la memoria");
        close(ArchivoBitarray);
        return 1;
    }

	// Crear el bitmap utilizando la biblioteca bitarray de las commons
    bitmap = bitarray_create(bitarray, CantidadBytes);

	// Si el archivo no existia, inicializar el bitmap en 0
	if(!ArchivoExiste)
	{
		for(int i = 0; i < CANTIDAD_BLOQUES; i++)
		{
			bitarray_clean_bit(bitmap, i);
		}
		BitmapOcuparBloque(0);
	}

	return 0;
}

//imprime el bitmap en pantalla
void ImprimirBitmap()
{
	uint32_t inicioIntervalo = 0;
	bool anterior = bitarray_test_bit(bitmap, 0);

	for(uint32_t i = 1; i < CANTIDAD_BLOQUES; i++)
	{
		if(bitarray_test_bit(bitmap, i) != anterior)
		{
			printf("El intervalo [%d, %d] es %s\n", inicioIntervalo, i - 1, anterior ? "1" : "0");
			anterior = !anterior;
			inicioIntervalo = i;
		}
		if(i == CANTIDAD_BLOQUES - 1)
		{
			printf("El intervalo [%d, %d] es %s\n", inicioIntervalo, i, anterior ? "1" : "0");
		}
	}
}

//devuelve true si el bloque esta ocupado, false si esta libre
bool BitmapEstaOcupado(uint32_t i)
{
	return bitarray_test_bit(bitmap, i);
}

//devuelve true si el bloque esta libre, false si esta ocupado
bool BitmapEstaLibre(uint32_t i)
{
	return !bitarray_test_bit(bitmap, i);
}

//Marca como ocupado un bloque (lo pone en 1)
void BitmapOcuparBloque(uint32_t IndiceBloque)
{
	bitarray_set_bit(bitmap, IndiceBloque);

	//vaciar el bloque
	char* bloque = ObtenerBloque(IndiceBloque);
	memset(bloque, 0, TAMANO_BLOQUES);
}

//Marca como libre un bloque (lo pone en 0)
void BitmapLiberarBloque(uint32_t IndiceBloque)
{
	bitarray_clean_bit(bitmap, IndiceBloque);

	//vaciar el bloque
	char* bloque = ObtenerBloque(IndiceBloque);
	memset(bloque, 0, TAMANO_BLOQUES);
}

//retorna el "puntero" al bloque vacio
uint32_t BitmapBuscarBloqueVacio()
{
	for(int i = 0; i < CANTIDAD_BLOQUES; i++)
	{
		if(BitmapEstaLibre(i))
		{
			return i;
		}
	}
	return 0;
}




//crea/abre el archivo y lo mapea
int InicializarArchivoDeBloques()
{
	printf("Inicializando archivo de bloques\n");
	char* PathArchivo = "PERSISTIDOS/Bloques.dat";

	//calcular el tamano en bytes del archivo
	size_t CantidadBytes = CANTIDAD_BLOQUES * TAMANO_BLOQUES;

	// Abrir/crear el archivo
    ArchivoBloques = open(PathArchivo, O_RDWR | O_CREAT, 0644);
    if (ArchivoBloques == -1) {
        log_error(FS_Logger, "Error al abrir/crear el MapArchivoBloques");
        return 1;
    }

	// Truncar el archivo a la cantidad de bloques
	if(ftruncate(ArchivoBloques, CantidadBytes) == -1)
	{
		log_error(FS_Logger, "Error al truncar el MapArchivoBloques");
		return 1;
	}

	// Mapear el archivo a la memoria
    MapArchivoBloques = mmap(NULL, CantidadBytes, PROT_READ | PROT_WRITE, MAP_SHARED, ArchivoBloques, 0);
    if (MapArchivoBloques == MAP_FAILED) {
        perror("Error al mapear el archivo a la memoria");
        close(ArchivoBloques);
        return 1;
    }
	return 0;
}

//retorna un puntero al bloque i
char* ObtenerBloque(uint32_t IndiceBloque)
{
	return MapArchivoBloques + (IndiceBloque * TAMANO_BLOQUES);
}

//imprime el contenido de N bytes de un bloque a partir de un determinado desplazamiento
void ImprimirBloque(uint32_t IndiceBloque, int Desplazamiento, int CantidadAImpimir)
{
	if(Desplazamiento + CantidadAImpimir > TAMANO_BLOQUES)
	{
		log_error(FS_Logger, "Intentando imprimir mas alla del bloque");
		return;
	}
	char* bloque = ObtenerBloque(IndiceBloque);
	printf("El bloque %d contiene: %.*s\n", IndiceBloque, CantidadAImpimir, bloque + Desplazamiento);
}

//escribe el contenido de un CHAR en un bloque a partir de un determinado desplazamiento
void EscribirBloqueDeChar(uint32_t IndiceBloque, int Desplazamiento, char* ContenidoAEscribir)
{
	if(strlen(ContenidoAEscribir) > TAMANO_BLOQUES - Desplazamiento)
	{
		log_error(FS_Logger, "Intentando escribir mas alla del final del bloque");
		return;
	}
	char* bloque = ObtenerBloque(IndiceBloque);
	memcpy(bloque, ContenidoAEscribir, strlen(ContenidoAEscribir));
}

//lee el contenido de N bytes de un bloque a partir de un determinado desplazamiento
char* LeerBloqueDeChar(uint32_t IndiceBloque, int Desplazamiento, int CantidadALeer)
{
	if(Desplazamiento + CantidadALeer > TAMANO_BLOQUES)
	{
		log_error(FS_Logger, "Intentando leer mas alla del bloque");
		return NULL;
	}
	char* bloque = ObtenerBloque(IndiceBloque);
	char* contenido = malloc(CantidadALeer + 1);
	memcpy(contenido, bloque + Desplazamiento, CantidadALeer);
	contenido[CantidadALeer] = '\0';
	return contenido;
}

//escribe un "puntero" (a un bloque) en un bloque a partir de un determinado desplazamiento
void EscribirBloqueDePunteros(uint32_t IndiceBloque, int IndicePuntero, uint32_t ContenidoAEscribir)
{
	if((IndicePuntero + 1) * sizeof(uint32_t) > TAMANO_BLOQUES)
	{
		log_error(FS_Logger, "Intentando escribir mas alla del final del bloque");
		return;
	}

	char* bloque = ObtenerBloque(IndiceBloque);
	memcpy(bloque + (IndicePuntero * sizeof(uint32_t)), &ContenidoAEscribir, sizeof(uint32_t));
}

//retorna el "puntero" (a un bloque) en un bloque a partir de un determinado desplazamiento
uint32_t LeerBloqueDePunteros(uint32_t IndiceBloque, int IndicePuntero)
{
	if((IndicePuntero + 1) * sizeof(uint32_t) > TAMANO_BLOQUES)
	{
		log_error(FS_Logger, "Intentando leer mas alla del bloque");
		return 0;
	}

	char* bloque = ObtenerBloque(IndiceBloque);
	uint32_t contenido;
	memcpy(&contenido, bloque + (IndicePuntero * sizeof(uint32_t)), sizeof(uint32_t));
	return contenido;
}





int InicializarFCBs()
{
	printf("Inicializando FCBs\n");

	//abrir el config base para la creacion de nuevos FCBs
	t_config* FCB = config_create("Base.FCB");
	list_add(ListaFCBs, FCB);

	//cargar todos los FCB que ya existen
	DIR* dir;
    struct dirent* entrada;

    // Abrir el directorio
    dir = opendir(PathDirectorioFCBs);
    if (dir == NULL) {
        printf("No se pudo abrir el directorio de FCBs.\n");
        return 1;
    }

    // Leer las entradas del directorio
    while ((entrada = readdir(dir)) != NULL) {
        // Excluir las entradas "." y ".."
        if (strcmp(entrada->d_name, ".") != 0 && strcmp(entrada->d_name, "..") != 0 && strcmp(entrada->d_name, "borrar.txt") != 0) {            
			
			//armar el path del archivo
			char path[257];
			sprintf(path, "%s/%s", PathDirectorioFCBs, entrada->d_name);

			//crear el config para ese archivo y agregarlo a la lista
			t_config* FCB = config_create(path);
			if(FCB != NULL)
				list_add(ListaFCBs, FCB);
        }
    }

    // Cerrar el directorio
    closedir(dir);

	return 0;
}

void CrearFCB(char* NombreArchivo, int TamanoArchivo, uint32_t Puntero_Directo, uint32_t Puntero_Indirecto)
{
	printf("Creando FCB para %s\n", NombreArchivo);

	//armar el path del archivo
	char path[257];
	sprintf(path, "%s/%s.FCB", PathDirectorioFCBs, NombreArchivo);

	//Si el FCB no existe
	if(access(path, F_OK) != 0)
	{
		//buscar el FCB base
		t_config* FCBAClonar = list_get(ListaFCBs, 0);

		//clonar el config para ese archivo y agregarlo a la lista
		config_save_in_file(FCBAClonar, path);
		t_config* FCB = config_create(path);
		list_add(ListaFCBs, FCB);

		//setear los valores del FCB
		config_set_value(FCB, "NOMBRE_ARCHIVO", NombreArchivo);
		
		char TamanoArchivoChar[15];
		sprintf(TamanoArchivoChar, "%d", TamanoArchivo);
		config_set_value(FCB, "TAMANIO_ARCHIVO", TamanoArchivoChar);

		char PunteroDirectoChar[15];
		sprintf(PunteroDirectoChar, "%u", Puntero_Directo);
		config_set_value(FCB, "PUNTERO_DIRECTO", PunteroDirectoChar);

		char PunteroIndirectoChar[15];
		sprintf(PunteroIndirectoChar, "%u", Puntero_Indirecto);
		config_set_value(FCB, "PUNTERO_INDIRECTO", PunteroIndirectoChar);

		config_save(FCB);
	}
	else
	{
		log_error(FS_Logger, "El FCB para %s ya existe", NombreArchivo);
	}
}

t_config* BuscarFCB(char* NombreArchivo)
{
	for(int i = 0; i < list_size(ListaFCBs); i++)
	{
		t_config* FCB = list_get(ListaFCBs, i);
		if(strcmp(config_get_string_value(FCB, "NOMBRE_ARCHIVO"), NombreArchivo) == 0)
			return FCB;
	}
	return NULL;
}

void ModificarValorFCB(t_config* FCB, char* KEY, char* Valor)
{
	config_set_value(FCB, KEY, Valor);
	config_save(FCB);
}




void CrearArchivo(char* NombreArchivo)
{
	uint32_t PunteroDirecto = BitmapBuscarBloqueVacio();

	if(PunteroDirecto == 0)
	{
		log_error(FS_Logger, "No hay bloques disponibles para crear el archivo");
		return;
	}
	
	BitmapOcuparBloque(PunteroDirecto);

	CrearFCB(NombreArchivo, 0, PunteroDirecto, 0);
}

void TruncarArchivo(char* NombreArchivo, int NuevoTamanoArchivo)
{
	t_config* FCB = BuscarFCB(NombreArchivo);
	if(FCB == NULL)
	{
		log_error(FS_Logger, "No se encontro el archivo %s", NombreArchivo);
		return;
	}

	int TamanoActualArchivo = atoi(config_get_string_value(FCB, "TAMANIO_ARCHIVO"));
	int PunteroIndirecto = atoi(config_get_string_value(FCB, "PUNTERO_INDIRECTO"));

	//determino cuantos bloques esta usando el archivo actualmente
	int CantBloquesActuales = TamanoActualArchivo / TAMANO_BLOQUES;
	if((TamanoActualArchivo % TAMANO_BLOQUES != 0) || CantBloquesActuales == 0)
		CantBloquesActuales++;

	//determino cuantos bloques va a usar el archivo luego del truncado
	int NuevaCantBloques = NuevoTamanoArchivo / TAMANO_BLOQUES;
	if((NuevoTamanoArchivo % TAMANO_BLOQUES != 0) || NuevaCantBloques == 0)
		NuevaCantBloques++;
	

	//Si hay que agrandar el Archivo, agregar los punteros a los nuevos bloques y ocupar los bloques
	if(NuevaCantBloques > CantBloquesActuales)
	{
		//si no tiene puntero indirecto al bloque de punteros indirectos,
		//creo el bloque que contendra los punteros
		if(PunteroIndirecto == 0)
		{
			//buscar un bloque vacio
			uint32_t NuevoPunteroIndirecto = BitmapBuscarBloqueVacio();
			if(NuevoPunteroIndirecto == 0)
			{
				log_error(FS_Logger, "No hay bloques disponibles para truncar el archivo");
				return;
			}

			//ocupar el bloque
			BitmapOcuparBloque(NuevoPunteroIndirecto);

			PunteroIndirecto = NuevoPunteroIndirecto;

			//agregar el bloque al archivo
			char NuevoPunteroIndirectoChar[15];
			sprintf(NuevoPunteroIndirectoChar, "%d", NuevoPunteroIndirecto);
			ModificarValorFCB(FCB, "PUNTERO_INDIRECTO", NuevoPunteroIndirectoChar);
		}

		//detemino cuantos bloques nuevos necesito
		int BloquesNecesarios = NuevaCantBloques - CantBloquesActuales;

		//por cada bloque nuevo que necesito agregar creo uno y lo agrego al bloque de punteros indirectos
		for(int i = 0; i < BloquesNecesarios; i++)
		{
			//buscar un bloque vacio
			uint32_t NuevoPuntero = BitmapBuscarBloqueVacio();
			if(NuevoPuntero == 0)
			{
				log_error(FS_Logger, "No hay bloques disponibles para truncar el archivo");
				return;
			}

			//ocupar el bloque
			BitmapOcuparBloque(NuevoPuntero);
			
			//buscar en que posicion del bloque de punteros indirectos tengo que agregar el puntero
			int IndicePuntero = (CantBloquesActuales + i - 1);

			//agrego el puntero al bloque de punteros indirectos
			EscribirBloqueDePunteros(PunteroIndirecto, IndicePuntero, NuevoPuntero);
		}
	}
	//si hay que achicar el archivo, liberar los bloques que no se usan mas
	else if(NuevaCantBloques < CantBloquesActuales)
	{
		//si no tiene puntero indirecto a un bloque de punteros, Error...
		if(PunteroIndirecto == 0)
		{
			log_error(FS_Logger, "El archivo no tiene bloques de punteros indirectos");
			return;
		}

		//por cada bloque que hay que liberar, libero el bloque y lo saco del bloque de punteros indirectos
		for(int i = CantBloquesActuales; i> NuevaCantBloques; i--)
		{
			//obtener el indice del puntero a liberar
			int IndicePuntero = i - 2;

			//busco el puntero al bloque a liberar
			uint32_t PunteroBloqueALiberar = LeerBloqueDePunteros(PunteroIndirecto, IndicePuntero);

			//libero el bloque
			BitmapLiberarBloque(PunteroBloqueALiberar);

			//lo borro de la lista de punteros indirectos
			EscribirBloqueDePunteros(PunteroIndirecto, IndicePuntero, 0);
		}

		//si no quedan bloques apuntados por el puntero indirecto,
		//libero el bloque de punteros indirectos y lo saco del FCB
		if(NuevaCantBloques == 1)
		{
			BitmapLiberarBloque(PunteroIndirecto);

			ModificarValorFCB(FCB, "PUNTERO_INDIRECTO", "0");
		}
	}

	//cambiar el tamaño del archivo en el FCB
	char NuevoTamanoArchivoChar[15];
	sprintf(NuevoTamanoArchivoChar, "%d", NuevoTamanoArchivo);
	ModificarValorFCB(FCB, "TAMANO", NuevoTamanoArchivoChar);		
}

//busca el puntero(uint32_t) de un bloque en funcion de un puntero de archivo(marca la posicion a modificar dentro del archivo)
uint32_t CalcularPunteroABloqueDePunteroArchivo(char* NombreArchivo, int PunteroArchivo, int* DesplazamientoEnBloqueBuscado)
{
	//obtener el FCB del archivo
	t_config* FCB = BuscarFCB(NombreArchivo);
	if(FCB == NULL)
	{
		log_error(FS_Logger, "No se encontro el archivo %s", NombreArchivo);
		return 0;
	}

	uint32_t BloqueBuscado = PunteroArchivo / TAMANO_BLOQUES;

	if(BloqueBuscado ==  0)
	{
		BloqueBuscado = strtoul(config_get_string_value(FCB, "PUNTERO_DIRECTO"), NULL, 10);
	}
	else
	{
		int IndicePuntero = BloqueBuscado - 1;

		uint32_t PunteroIndirecto = strtoul(config_get_string_value(FCB, "PUNTERO_INDIRECTO"), NULL, 10);

		BloqueBuscado = LeerBloqueDePunteros(PunteroIndirecto, IndicePuntero);
	}
	
	*DesplazamientoEnBloqueBuscado = PunteroArchivo % TAMANO_BLOQUES;

	return BloqueBuscado;
}

void EscribirArchivo(char* NombreArchivo, int PunteroArchivo, char* ContenidoAEscribir, int CantBytesAEscribir)
{
	//obtener el FCB del archivo
	t_config* FCB = BuscarFCB(NombreArchivo);
	if(FCB == NULL)
	{
		log_error(FS_Logger, "No se encontro el archivo %s", NombreArchivo);
		return;
	}

	if(PunteroArchivo + CantBytesAEscribir > atoi(config_get_string_value(FCB, "TAMANIO_ARCHIVO")))
	{
		log_error(FS_Logger, "El archivo %s no tiene espacio suficiente para escribir %d bytes", NombreArchivo, CantBytesAEscribir);
		return;
	}

	int CantBytesEscritos = 0;
	int PunteroAux = PunteroArchivo;

	while(CantBytesEscritos < CantBytesAEscribir)
	{
		int DesplazamientoEnBloqueBuscado;
		uint32_t BloqueBuscado = CalcularPunteroABloqueDePunteroArchivo(NombreArchivo, PunteroAux, &DesplazamientoEnBloqueBuscado);
		
		int CantBytesAEscribirEnEsteBloque = TAMANO_BLOQUES - DesplazamientoEnBloqueBuscado;

		char ContenidoAEscribirEnEsteBloque[CantBytesAEscribirEnEsteBloque+1];
		strncpy(ContenidoAEscribirEnEsteBloque, ContenidoAEscribir + CantBytesEscritos, CantBytesAEscribirEnEsteBloque);
		ContenidoAEscribirEnEsteBloque[CantBytesAEscribirEnEsteBloque] = '\0';

		EscribirBloqueDeChar(BloqueBuscado, DesplazamientoEnBloqueBuscado, ContenidoAEscribirEnEsteBloque);

		PunteroAux = 0;
		CantBytesEscritos += CantBytesAEscribirEnEsteBloque;
	}
	return;
}


char* LeerArchivo(char* NombreArchivo, int PunteroArchivo, int CantBytesALeer)
{
	//obtener el FCB del archivo
	t_config* FCB = BuscarFCB(NombreArchivo);
	if(FCB == NULL)
	{
		log_error(FS_Logger, "No se encontro el archivo %s", NombreArchivo);
		return NULL;
	}

	if(PunteroArchivo + CantBytesALeer > atoi(config_get_string_value(FCB, "TAMANIO_ARCHIVO")))
	{
		log_error(FS_Logger, "El archivo %s no tiene espacio suficiente para leer %d bytes", NombreArchivo, CantBytesALeer);
		return NULL;
	}

	char* ContenidoLeido = malloc(CantBytesALeer+1);
	ContenidoLeido[CantBytesALeer] = '\0';

	int CantBytesLeidos = 0;
	int PunteroAux = PunteroArchivo;

	while(CantBytesLeidos < CantBytesALeer)
	{
		int DesplazamientoEnBloqueBuscado;
		uint32_t BloqueBuscado = CalcularPunteroABloqueDePunteroArchivo(NombreArchivo, PunteroAux, &DesplazamientoEnBloqueBuscado);
		
		int CantBytesALeerEnEsteBloque = TAMANO_BLOQUES - DesplazamientoEnBloqueBuscado;

		char* ContenidoLeidoEnEsteBloque = LeerBloqueDeChar(BloqueBuscado, DesplazamientoEnBloqueBuscado, CantBytesALeerEnEsteBloque);

		strncpy(ContenidoLeido + CantBytesLeidos, ContenidoLeidoEnEsteBloque, CantBytesALeerEnEsteBloque);

		PunteroAux = 0;
		CantBytesLeidos += CantBytesALeerEnEsteBloque;
	}

	return ContenidoLeido;
}






//cierra y persiste el archivo de bitmap, desmapea la memoria y libera el bitmap
void CerrarArchivoDeBitmap()
{
	printf("Cerrando Bitmap\n");

	bitarray_destroy(bitmap);

    // Sincronizar los datos pendientes en el bitmap
    if (msync(bitarray, CANTIDAD_BLOQUES / 8, MS_SYNC) == -1) {
        perror("Error al sincronizar el bitmap en la memoria");
    }

    // Desmapear el bitmap de la memoria
    if (munmap(bitarray, CANTIDAD_BLOQUES / 8) == -1) {
        perror("Error al desmapear el bitmap de la memoria");
    }

    // Sincronizar los datos pendientes en el archivo
    if (fsync(ArchivoBitarray) == -1) {
        perror("Error al sincronizar el archivo del bitmap");
    }

    // Cerrar el archivo
    if (close(ArchivoBitarray) == -1) {
        perror("Error al cerrar el archivo del bitmap");
    }
}

//cierra y persiste el archivo de lloques y desmapea la memoria
void CerrarArchivoDeBloques()
{
	printf("Cerrando archivo de bloques\n");

	// Sincronizar los datos pendientes en el archivo
	if (msync(MapArchivoBloques, CANTIDAD_BLOQUES * TAMANO_BLOQUES, MS_SYNC) == -1) {
		perror("Error al sincronizar el MapArchivoBloques en la memoria");
	}

	// Desmapear el MapArchivoBloques de la memoria
	if (munmap(MapArchivoBloques, CANTIDAD_BLOQUES * TAMANO_BLOQUES) == -1) {
		perror("Error al desmapear el MapArchivoBloques de la memoria");
	}

	// Sincronizar los datos pendientes en el archivo
	if (fsync(ArchivoBloques) == -1) {
		perror("Error al sincronizar el archivo del MapArchivoBloques");
	}

	// Cerrar el archivo
	if (close(ArchivoBloques) == -1) {
		perror("Error al cerrar el archivo del MapArchivoBloques");
	}
}

//limpia la lista que contiene los punteros a cada archivo FCB
void CerrarFCBs()
{
	printf("Cerrando FCBs\n");

	while(!list_is_empty(ListaFCBs))
	{
		t_config* FCB = list_remove(ListaFCBs, 0);
		config_destroy(FCB);
	}
	list_destroy(ListaFCBs);
}

void PersistirDatos()
{
	printf("Persistiendo datos...\n");
	CerrarArchivoDeBitmap();
	CerrarArchivoDeBloques();
	CerrarFCBs();
}

void TerminarModulo()
{
	printf("\nTerminando modulo...\n");

	PersistirDatos();

	pthread_cancel(HiloEscucha);
	
	config_destroy(config);
	config_destroy(configSB);

	log_destroy(FS_Logger);
	
	liberar_conexion(SocketKernel);
	liberar_conexion(SocketMemoria);
	liberar_conexion(SocketFileSystem);
}