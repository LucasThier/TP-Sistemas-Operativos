#include "../include/KernelMain.h"


int main(void)
{
	int SocketServidor = iniciar_servidor_sin_logger("0.0.0.0", "3568");

	int SocketCliente = esperar_cliente_sin_logger(SocketServidor);

	int recibido;
	do
	{
		recibido = recibir_int(SocketCliente);
		printf("El valor recivido es %d\n", recibido);

	}while(recibido != 0);
	
	/*liberar_conexion(SocketServidor);
	liberar_conexion(SocketCliente);*/

	printf("El valor recivido es \n");

	return EXIT_SUCCESS;
}
