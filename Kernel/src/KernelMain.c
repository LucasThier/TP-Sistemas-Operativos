#include "../include/KernelMain.h"

int SocketServidor;
int SocketCliente;


void sighandler(int s) {
	liberar_conexion(SocketServidor);
	liberar_conexion(SocketCliente);
	exit(0);
}

int main(void)
{
	t_log* ConexionesLogger = log_create("Kernel.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);

	SocketServidor = iniciar_servidor(ConexionesLogger, NOMBRE_PROCESO, "0.0.0.0", "33668");
	if(SocketServidor != 0)
	{
		SocketCliente = esperar_cliente(ConexionesLogger, NOMBRE_PROCESO, SocketServidor);

		if(SocketCliente != 0)
		{
			int recibido = 100;
			do
			{
				recibido = recibir_int(ConexionesLogger, NOMBRE_PROCESO, SocketCliente);
				log_info(ConexionesLogger, "Numero recibido: %d\n", recibido);

			}while(recibido != 0);
		}
	}
	
	liberar_conexion(SocketServidor);
	liberar_conexion(SocketCliente);
	return EXIT_SUCCESS;
}