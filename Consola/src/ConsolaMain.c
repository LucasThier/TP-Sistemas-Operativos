#include "../include/ConsolaMain.h"

int main(void)
{
	int SocketServidor = crear_conexion_sin_logger("0.0.0.0", "3568");

	int leido;

	do
	{
		leido = readline("> ");
		enviar_int(SocketServidor, leido);
		printf("El valor enviado es %d\n", leido);

	} while (leido != 0);

	return EXIT_SUCCESS;
}