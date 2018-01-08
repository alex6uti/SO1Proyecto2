/*
 ============================================================================
 Name        : pro2_201020781.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <ncurses.h>
#include <unistd.h>
#include <signal.h>

#define WIDTH 80
#define HEIGHT 40

#define CLAN_DEFENSOR 1
#define CLAN_INVASOR 2

#define PAIR_SELECCION_SI 1
#define PAIR_SELECCION_NO 2

#define JUGADOR_ACTIVO  1

#define JUGADOR_PASO_INICIANDO  1
#define JUGADOR_PASO_SELECCIONANDO  2
#define JUGADOR_PASO_ESPERANDO  3
#define JUGADOR_PASO_JUGANDO  4

#define MAPA_X 60
#define MAPA_Y 12
#define MAPA_X_CENTRO 29
#define MAPA_NAVES_L1_Y  4
#define MAPA_NAVES_L2_Y  5
#define MAPA_NAVES_L3_Y  6
#define MAPA_NAVES_L4_Y  7

#define MAPA_COMANDANTE_Y  0
#define MAPA_DEFENSOR_Y  11
#define LONGITUD_JUGADOR 2

#define NAVES_DIR_DER 0
#define NAVES_DIR_IZQ 1
#define NAVES_MARGEN 10
#define LOG_DEKKER_Y 15
#define LOG_TIEMPO_Y 16
#define MOVIMIENTOS_NAVES 10

struct info {
	int jugador1Estado;
	int jugador2Estado;

	int jugador1Paso;
	int jugador2Paso;

	int procesoDekker; //para saber si instancia se configura como proceso 1 o proceso 2
	bool p1_puede_entrar;
	bool p2_puede_entrar;
	int turno;
	int regionCriticDekker;

	/***variables para naves**/
	int comandante_x;
	int defensonr_x;
	//lnea 1
	int l1_navc1_x;
	int l1_navc2_x;
	int l1_navc3_x;
	int l1_navc4_x;
	int l1_navef_x;
	//linea 2
	int l2_navc1_x;
	int l2_navc2_x;
	int l2_navc3_x;
	int l2_navc4_x;
	int l2_navef_x;
	//linea 3
	int l3_navc1_x;
	int l3_navc2_x;
	int l3_navc3_x;
	int l3_navc4_x;
	int l3_navef_x;
	//linea 4
	int l4_navc1_x;
	int l4_navc2_x;
	int l4_navc3_x;
	int l4_navc4_x;
	int l4_navef_x;

	int direccionNaves;
	int movimientosNaves;
	int tiempo;

};

//#define KEY (key_t)1243;

void cargarMemoriaCompartida();
void verificarDisponibilidadJugadores();

void doSignal(int sId, int sNum);
void doWait(int sId, int sNum);
void initSem(int sId, int numSem, int valor);
void liberarSemaforo();
void configurarSemaforo();
void configurarSemaforoJugador1();
void configurarSemaforoJugador2();

void configurarPantallaInicio();
void seleccionarClan();
void guardarSeleccion(int clan);
void cambioPantallaInicioClan(int clan);
void pantallaEmpezandoJuego();
void cerrarPantalla();

void configurarProcesoDekker();
void dekkerProceso1();
void dekkerProceso2();
void imprimirPantallaDekkerEntro(int proceso);
void imprimirPantallaDekkerSalio(int proceso);
void iniciarNaves();
void imprimirComandante();
void imprimirDefensor();
void imprimirNaves();
void imprimirNaveComun(int y, int centroX);
void imprimirNaveFuerte(int y, int centroX, int numero);
void refrescarPantallaAutomaticamente();
void imprimirPantallaJuego();
void escucharTeclaProcesoDekker();
void moverNaves();

WINDOW *menu_win;
struct info *mInfo;

int mTipoClan = CLAN_DEFENSOR;
bool mClanSeleccionado = false;
int mIdJugador;
int mIdProceso;

int main(void) {
	cargarMemoriaCompartida();
	verificarDisponibilidadJugadores();

	//pantalla entrada
	menu_win = initscr();
	keypad(menu_win, TRUE);
	cbreak();
	start_color();
	clear();
	init_pair(10, COLOR_WHITE, COLOR_CYAN);
	init_pair(11, COLOR_WHITE, COLOR_GREEN);
	attron(COLOR_PAIR(10));
	mvprintw(4, 28, "                    ");
	mvprintw(5, 28, "   SPACE INVADER    ");
	mvprintw(6, 28, "                    ");
	attroff(COLOR_PAIR(10));

	attron(COLOR_PAIR(11));
	mvprintw(10, 20, "  Mario Alexander Gutierrez Hernandez  ");
	mvprintw(11, 20, "              201020781                ");
	attroff(COLOR_PAIR(11));

	refresh();
	wgetch(menu_win);

	configurarSemaforo();
	return EXIT_SUCCESS;
}

void cargarMemoriaCompartida() {
	int id = shmget(1243l, sizeof(struct info), IPC_CREAT | 0666);

	if (id < 0) {
		printf("Problemas al cargar shmget\n");
		exit(0);
	} else {
		printf("shmget exito!");
	}

	mInfo = (struct info *) shmat(id, 0, 0);
	if (mInfo < (struct info *) 0) {
		printf("Error en shmat\n");
		exit(2);
	} else {
		printf("shmat exito!");
	}
}

void verificarDisponibilidadJugadores() {
	if (mInfo->jugador1Estado != JUGADOR_ACTIVO) {
		mInfo->jugador1Estado = JUGADOR_ACTIVO;
		mInfo->jugador1Paso = JUGADOR_PASO_INICIANDO;
		mIdJugador = 1;
		printf("entro jugador 1");
		//configurarSemaforoJugador1();
		//printf("jugador1 Press [Enter] to continue . . .\n");
		//fflush( stdout);
		//getchar();
	} else if (mInfo->jugador2Estado != JUGADOR_ACTIVO) {
		mInfo->jugador2Estado = JUGADOR_ACTIVO;
		mInfo->jugador2Paso = JUGADOR_PASO_INICIANDO;
		mIdJugador = 2;
		printf("Entro jugador 2");
		//configurarSemaforoJugador2();

		//printf("jugador2 Press [Enter] to continue . . .\n");
		//fflush( stdout);
		//getchar();
	} else {
		liberarSemaforo();
		printf("Lo sentimos, no hay turnos disponibles!\n");
	}

}

//CONFIGURAR SECCION SEMAFOROS*******************************************
//***********************************************************************
//***********************************************************************
//***********************************************************************

void doSignal(int sId, int sNum) {
	struct sembuf lsembuf;
	lsembuf.sem_num = sNum;
	lsembuf.sem_op = 1;
	lsembuf.sem_flg = 0;

	if (semop(sId, &lsembuf, 1) == -1) {
		perror(NULL);
		printf("Error al hacer Signal");
	}
}

void doWait(int sId, int sNum) {
	struct sembuf sops;
	sops.sem_num = sNum;
	sops.sem_op = -1;
	sops.sem_flg = 0;

	if (semop(sId, &sops, 1) == -1) {
		perror(NULL);
		printf("Error al hacer el Wait");
	}
}

void initSem(int sId, int numSem, int valor) { //iniciar un semaforo

	if (semctl(sId, numSem, SETVAL, valor) < 0) {
		perror(NULL);
		printf("Error iniciando semaforo");
	}
}

void liberarSemaforo() {
	int sId = semget(4321, 1, IPC_CREAT | 0700);

	//Creamos un semaforo y damos permisos para compartirlo
	if (sId < 0) {
		perror(NULL);
		printf("Semaforo: semget");
	}

	//Liberacion del semaforo
	if ((semctl(sId, 0, IPC_RMID)) == -1) {
		perror(NULL);
		printf("Semaforo borrando");
	}
}

void configurarSemaforo() {
	if (mIdJugador == 1) {
		configurarSemaforoJugador1();
	} else if (mIdJugador == 2) {
		configurarSemaforoJugador2();
	}
}

void configurarSemaforoJugador1() {
	int sId = semget(4321, 1, IPC_CREAT | 0700);
	//Manera de usar semget http://pubs.opengroup.org/onlinepubs/7908799/xsh/semget.html
	//Creamos un semaforo y damos permisos para compartirlo
	if (sId < 0) {
		perror(NULL);
		printf("error semaforo: semget");
	}

	initSem(sId, 0, 1);

	bool flag = true;
	while (flag) {
		doWait(sId, 0);
		puts("Entra>jugador1");
		if (mInfo->jugador1Paso == JUGADOR_PASO_INICIANDO) {//si esta iniciando lanzar seleccion de clan
			configurarPantallaInicio();
			seleccionarClan();
		} else if (mInfo->jugador1Paso == JUGADOR_PASO_ESPERANDO) {	//si esta esperando
			if (mInfo->jugador2Paso == JUGADOR_PASO_ESPERANDO
					|| mInfo->jugador2Paso == JUGADOR_PASO_JUGANDO) {//si el jugador 2 ya esta esperando o jugando.... iniciar juego para jugador 1
				flag = false;
				//EMPEZAR JUEGO
				pantallaEmpezandoJuego();
			}
		}

		//puts("Entra>jugador1");
		sleep(1);
		//puts("Sale>jugador1");
		doSignal(sId, 0);
	}

	configurarProcesoDekker();
}

void configurarSemaforoJugador2() {
	int sId = semget(4321, 1, IPC_CREAT | 0700);
	//Manera de usar semget http://pubs.opengroup.org/onlinepubs/7908799/xsh/semget.html
	//Creamos un semaforo y damos permisos para compartirlo
	if (sId < 0) {
		perror(NULL);
		printf("error semaforo: semget");
	}

	bool flag = true;
	while (flag) {

		/*
		 doWait(sId, 0);
		 puts("Entra>jugador2");
		 sleep(3);
		 puts("Sale>jugador2")
		 doSignal(sId, 0);
		 */

		doWait(sId, 0);
		puts("Entra>jugador2");
		if (mInfo->jugador2Paso == JUGADOR_PASO_INICIANDO) {//si esta iniciando lanzar seleccion de clan
			configurarPantallaInicio();
			seleccionarClan();
		} else if (mInfo->jugador2Paso == JUGADOR_PASO_ESPERANDO) {	//si esta esperando
			if (mInfo->jugador1Paso == JUGADOR_PASO_ESPERANDO
					|| mInfo->jugador1Paso == JUGADOR_PASO_JUGANDO) {//si el jugador 1 ya esta esperando o jugando.... iniciar juego para jugador 1
				flag = false;
				//EMPEZAR JUEGO
				pantallaEmpezandoJuego();
			}
		}

		sleep(1);

		doSignal(sId, 0);
	}

	configurarProcesoDekker();
}

//CONFIGURAR SECCION PANTALLA INICIO*******************************************
//***********************************************************************
//***********************************************************************
//***********************************************************************
void configurarPantallaInicio() {

	//menu_win = initscr();
	//keypad(menu_win, TRUE);
	cbreak();
	start_color();
	init_pair(PAIR_SELECCION_SI, COLOR_WHITE, COLOR_GREEN);
	init_pair(PAIR_SELECCION_NO, COLOR_WHITE, COLOR_BLACK);

	clear();
	cambioPantallaInicioClan(CLAN_DEFENSOR);

	refresh();
}

void cerrarPantalla() {
	endwin();
}

void seleccionarClan(int pasoJugador) {
	cambioPantallaInicioClan(mTipoClan);
	bool flag = true;
	while (flag) {
		int tecla = wgetch(menu_win);

		if (tecla == KEY_UP || tecla == KEY_DOWN) {
			if (mTipoClan == CLAN_DEFENSOR)
				mTipoClan = CLAN_INVASOR;
			else
				mTipoClan = CLAN_DEFENSOR;

			cambioPantallaInicioClan(mTipoClan);
		} else if (tecla == 10) {
			guardarSeleccion(mTipoClan);
			if (mTipoClan == CLAN_DEFENSOR) {
				mvprintw(0, 0, ">Selecciono DEFENSOR ");
			} else {
				mvprintw(0, 0, ">Selecciono INVASOR ");
			}
			mvprintw(1, 0, ">Esperando oponente");
			mClanSeleccionado = true;
			flag = false;
		} else if (tecla == 27) {
			flag = false;
		}

		refresh();

	}
}

void guardarSeleccion(int clan) {
	if (mIdJugador == 1) {
		mInfo->jugador1Paso = JUGADOR_PASO_ESPERANDO;
	} else {
		mInfo->jugador2Paso = JUGADOR_PASO_ESPERANDO;
	}
}

void cambioPantallaInicioClan(int clan) {
	mTipoClan = clan;
	if (clan == CLAN_DEFENSOR) {
		//init_pair(1, COLOR_WHITE, COLOR_GREEN);
		attron(COLOR_PAIR(PAIR_SELECCION_SI));
		mvprintw(4, 20, "                    ");
		mvprintw(5, 20, "      DEFENSOR      ");
		mvprintw(6, 20, "                    ");
		attroff(COLOR_PAIR(PAIR_SELECCION_SI));

		attron(COLOR_PAIR(PAIR_SELECCION_NO));
		mvprintw(9, 20, "                    ");
		mvprintw(10, 20, "       INVASOR      ");
		mvprintw(11, 20, "                    ");
		attroff(COLOR_PAIR(PAIR_SELECCION_NO));
	} else {
		attron(COLOR_PAIR(PAIR_SELECCION_NO));
		mvprintw(4, 20, "                    ");
		mvprintw(5, 20, "      DEFENSOR      ");
		mvprintw(6, 20, "                    ");
		attroff(COLOR_PAIR(PAIR_SELECCION_NO));

		attron(COLOR_PAIR(PAIR_SELECCION_SI));
		mvprintw(9, 20, "                    ");
		mvprintw(10, 20, "       INVASOR      ");
		mvprintw(11, 20, "                    ");
		attroff(COLOR_PAIR(PAIR_SELECCION_SI));
	}
}

void pantallaEmpezandoJuego() {
	clear();
	attron(COLOR_PAIR(PAIR_SELECCION_SI));
	mvprintw(8, 20, "          INICIANDO          ");
	mvprintw(9, 20, "PRESION UNA TECLA PARA ENTRAR");
	attroff(COLOR_PAIR(PAIR_SELECCION_SI));

	refresh();
	wgetch(menu_win);
}

//CONFIGURAR DEKKER V5***************************************************
//***********************************************************************
//***********************************************************************
//***********************************************************************

void configurarProcesoDekker() {
	//iniciarNaves();

	switch (mInfo->procesoDekker) {
	case 0:
		mIdProceso = 1;
		mInfo->procesoDekker = 1;
		mInfo->p1_puede_entrar = false;
		mInfo->p2_puede_entrar = false;
		mInfo->turno = 1;
		mInfo->tiempo = 1;
		iniciarNaves();

		switch (fork()) {
		case -1:
			printf("Error al hacer fork");
			break;
		case 0:
			//hijo
			while (true) {
				imprimirPantallaJuego();
				//sleep(1);
				usleep(100 * 1000);
			}
			break;
		default:
			//padre
			dekkerProceso1();
			break;
		}
		//crearHiloImpresion();

		break;
	case 1:
		mIdProceso = 2;
		mInfo->procesoDekker = 2;
		//crearHiloImpresion();
		//dekkerProceso2();

		switch (fork()) {
		case -1:
			printf("Error al hacer fork");
			break;
		case 0:
			//hijo
			while (true) {
				imprimirPantallaJuego();
				//sleep(1);
				usleep(100 * 1000);
			}
			break;
		default:
			//padre
			dekkerProceso2();
			break;
		}
		break;
	default:
		clear();
		refresh();
		cerrarPantalla();
		exit(0);
	}

}

void dekkerProceso1() {
	while (true) {
		//[REALIZA_TAREAS_INICIALES]
		//imprimirPantallaJuego();
		mInfo->p1_puede_entrar = true;
		while (mInfo->p2_puede_entrar) {
			if (mInfo->turno == 2) {
				mInfo->p1_puede_entrar = false;
				while (mInfo->turno == 2) {
				}
				mInfo->p1_puede_entrar = true;
			}
		}
		//[REGION_CRITICA]
		mInfo->regionCriticDekker = 1;
		mInfo->tiempo = mInfo->tiempo + 1;

		int pidForkProceso1 = fork();
		bool flagActivo = true;
		int auxiliar;

		switch (pidForkProceso1) {
		case -1:
			printf("Error al hacer fork");
			break;
		case 0:
			//hijo
			auxiliar = 0;
			while (flagActivo) {
				usleep(100 * 1000);
				auxiliar++;
				mInfo->tiempo++;
				if (auxiliar == 10) {
					moverNaves();
					auxiliar = 0;
				}
			}
			break;
		default:
			escucharTeclaProcesoDekker();
			flagActivo = false;
			break;
		}

		while (flagActivo) {
			usleep(100 * 1000);
		}

		kill(pidForkProceso1, SIGKILL);

		mInfo->turno = 2;
		mInfo->p1_puede_entrar = false;
		//imprimirPantallaJuego();
	}
}

void dekkerProceso2() {
	while (true) {
		//[REALIZA_TAREAS_INICIALES]
		//imprimirPantallaJuego();
		mInfo->p2_puede_entrar = true;
		while (mInfo->p1_puede_entrar) {
			if (mInfo->turno == 1) {
				mInfo->p2_puede_entrar = false;
				while (mInfo->turno == 1) {
				}
				mInfo->p2_puede_entrar = true;
			}
		}
		//[REGION_CRITICA]
		mInfo->regionCriticDekker = 2;
		mInfo->tiempo = mInfo->tiempo + 1;

		int pidForkProceso2 = fork();
		bool flagActivo = true;
		int auxiliar;

		switch (pidForkProceso2) {
		case -1:
			printf("Error al hacer fork");
			break;
		case 0:
			//hijo
			auxiliar = 0;
			while (flagActivo) {
				usleep(100 * 1000);
				auxiliar++;
				mInfo->tiempo++;
				if (auxiliar == 10) {
					moverNaves();
					auxiliar = 0;
				}
			}
			break;
		default:
			escucharTeclaProcesoDekker();
			flagActivo = false;
			break;
		}

		while (flagActivo) {
			usleep(100 * 1000);
		}

		kill(pidForkProceso2, SIGKILL);

		mInfo->turno = 1;
		mInfo->p2_puede_entrar = false;
		//[REALIZA_TAREAS_FINALES]
		//imprimirPantallaJuego();
	}
}

void imprimirPantallaDekkerEntro(int proceso) {
	clear();
	attron(COLOR_PAIR(PAIR_SELECCION_SI));
	mvprintw(8, 20, "     ENTRO-PROCESO_%d            ", proceso);
	mvprintw(9, 20, "PRESION UNA TECLA PARA CAMBIAR");
	attroff(COLOR_PAIR(PAIR_SELECCION_SI));

	refresh();
	wgetch(menu_win);

	imprimirPantallaDekkerSalio(proceso);
}

void escucharTeclaProcesoDekker() {
	bool flag = true;
	while (flag) {
		int tecla = wgetch(menu_win);

		if (mIdProceso == mInfo->regionCriticDekker) {
			if (tecla == KEY_LEFT) {

				if (mTipoClan == CLAN_DEFENSOR) {
					mInfo->defensonr_x = mInfo->defensonr_x - 1;
				} else if (mTipoClan == CLAN_INVASOR) {
					mInfo->comandante_x = mInfo->comandante_x - 1;
				}
				flag = false;

			} else if (tecla == KEY_RIGHT) {
				if (mTipoClan == CLAN_DEFENSOR) {
					mInfo->defensonr_x = mInfo->defensonr_x + 1;
				} else if (mTipoClan == CLAN_INVASOR) {
					mInfo->comandante_x = mInfo->comandante_x + 1;
				}
				flag = false;
			}
		}
	}

}

void imprimirPantallaDekkerSalio(int proceso) {
	clear();
	attron(COLOR_PAIR(PAIR_SELECCION_SI));
	mvprintw(8, 20, "     SALIO-PROCESO_%d            ", proceso);
	attroff(COLOR_PAIR(PAIR_SELECCION_SI));

	refresh();
	sleep(1);
//wgetch(menu_win);
}

void imprimirLogDekker() {
	mvprintw(LOG_DEKKER_Y, 0, "ID_P: %d  | RC: %d", mIdProceso,
			mInfo->regionCriticDekker);
}

void imprimirTiempo() {
	int segundos = mInfo->tiempo / 10;
	mvprintw(LOG_TIEMPO_Y, 0, "TIEMO: %d segundos", segundos);

}

void refrescarPantallaAutomaticamente() {
	/*
	 while (true) {
	 clear();
	 mvprintw(0, 0, "rpa=%d", mInfo->tiempo);
	 imprimirComandante();
	 imprimirDefensor();
	 //imprimirNaveComun(MAPA_NAVES_L1_Y, (NAVES_MARGEN + 3));
	 imprimirNaves();
	 refresh();
	 sleep(1);
	 mInfo->tiempo = mInfo->tiempo + 1;
	 }
	 */
}

void imprimirPantallaJuego() {
	clear();
	imprimirComandante();
	imprimirDefensor();
	imprimirNaves();
	imprimirLogDekker();
	imprimirTiempo();
	refresh();
}

void imprimirComandante() {
	if (mInfo->comandante_x == 0) {
		mInfo->comandante_x = MAPA_X_CENTRO;
	}

	int x = mInfo->comandante_x - LONGITUD_JUGADOR;
	attron(COLOR_PAIR(PAIR_SELECCION_SI));
	mvprintw(MAPA_COMANDANTE_Y, x, "OOOOO");
	attroff(COLOR_PAIR(PAIR_SELECCION_SI));

}

void imprimirDefensor() {
	if (mInfo->defensonr_x == 0) {
		mInfo->defensonr_x = MAPA_X_CENTRO;
	}

	int x = mInfo->defensonr_x - LONGITUD_JUGADOR;
	attron(COLOR_PAIR(PAIR_SELECCION_SI));
	mvprintw(MAPA_DEFENSOR_Y, x, "OOOOO");
	attroff(COLOR_PAIR(PAIR_SELECCION_SI));
}

void iniciarNaves() {
	mInfo->l1_navc1_x = NAVES_MARGEN + 3;
	mInfo->l1_navc2_x = NAVES_MARGEN + 11;
	mInfo->l1_navef_x = NAVES_MARGEN + 20;
	mInfo->l1_navc3_x = NAVES_MARGEN + 30;
	mInfo->l1_navc4_x = NAVES_MARGEN + 38;

	mInfo->l2_navef_x = NAVES_MARGEN + 3;
	mInfo->l2_navc1_x = NAVES_MARGEN + 11;
	mInfo->l2_navc2_x = NAVES_MARGEN + 20;
	mInfo->l2_navc3_x = NAVES_MARGEN + 30;
	mInfo->l2_navc4_x = NAVES_MARGEN + 38;

	mInfo->l3_navc1_x = NAVES_MARGEN + 3;
	mInfo->l3_navef_x = NAVES_MARGEN + 11;
	mInfo->l3_navc2_x = NAVES_MARGEN + 20;
	mInfo->l3_navc3_x = NAVES_MARGEN + 30;
	mInfo->l3_navc4_x = NAVES_MARGEN + 38;

	mInfo->l4_navc1_x = NAVES_MARGEN + 3;
	mInfo->l4_navc2_x = NAVES_MARGEN + 11;
	mInfo->l4_navc3_x = NAVES_MARGEN + 20;
	mInfo->l4_navef_x = NAVES_MARGEN + 30;
	mInfo->l4_navc4_x = NAVES_MARGEN + 38;
}

void imprimirNaves() {
	imprimirNaveComun(MAPA_NAVES_L1_Y, mInfo->l1_navc1_x);
	imprimirNaveComun(MAPA_NAVES_L1_Y, mInfo->l1_navc2_x);
	imprimirNaveComun(MAPA_NAVES_L1_Y, mInfo->l1_navc3_x);
	imprimirNaveComun(MAPA_NAVES_L1_Y, mInfo->l1_navc4_x);
	imprimirNaveFuerte(MAPA_NAVES_L1_Y, mInfo->l1_navef_x, 1);

	imprimirNaveComun(MAPA_NAVES_L2_Y, mInfo->l2_navc1_x);
	imprimirNaveComun(MAPA_NAVES_L2_Y, mInfo->l2_navc2_x);
	imprimirNaveComun(MAPA_NAVES_L2_Y, mInfo->l2_navc3_x);
	imprimirNaveComun(MAPA_NAVES_L2_Y, mInfo->l2_navc4_x);
	imprimirNaveFuerte(MAPA_NAVES_L2_Y, mInfo->l2_navef_x, 2);

	imprimirNaveComun(MAPA_NAVES_L3_Y, mInfo->l3_navc1_x);
	imprimirNaveComun(MAPA_NAVES_L3_Y, mInfo->l3_navc2_x);
	imprimirNaveComun(MAPA_NAVES_L3_Y, mInfo->l3_navc3_x);
	imprimirNaveComun(MAPA_NAVES_L3_Y, mInfo->l3_navc4_x);
	imprimirNaveFuerte(MAPA_NAVES_L3_Y, mInfo->l3_navef_x, 3);

	imprimirNaveComun(MAPA_NAVES_L4_Y, mInfo->l4_navc1_x);
	imprimirNaveComun(MAPA_NAVES_L4_Y, mInfo->l4_navc2_x);
	imprimirNaveComun(MAPA_NAVES_L4_Y, mInfo->l4_navc3_x);
	imprimirNaveComun(MAPA_NAVES_L4_Y, mInfo->l4_navc4_x);
	imprimirNaveFuerte(MAPA_NAVES_L4_Y, mInfo->l4_navef_x, 4);

}

void imprimirNaveComun(int y, int centroX) {

	int inicioX = centroX - 2;
	if (inicioX >= 0) {
		attron(COLOR_PAIR(PAIR_SELECCION_NO));
		mvprintw(y, inicioX, "\\-.-/");
		attroff(COLOR_PAIR(PAIR_SELECCION_NO));
	}
}

void imprimirNaveFuerte(int y, int centroX, int numero) {
	int inicioX = centroX - 3;
	if (inicioX >= 0) {
		attron(COLOR_PAIR(PAIR_SELECCION_NO));
		mvprintw(y, inicioX, "(/-%d-\\)", numero);
		attroff(COLOR_PAIR(PAIR_SELECCION_NO));
	}
}

void moverNaves() {
	if (mInfo->movimientosNaves >= MOVIMIENTOS_NAVES) {
		mInfo->movimientosNaves = 0;
		if (mInfo->direccionNaves == NAVES_DIR_DER) {
			mInfo->direccionNaves = NAVES_DIR_IZQ;
		} else {
			mInfo->direccionNaves = NAVES_DIR_DER;
		}
	} else {
		mInfo->movimientosNaves++;
	}

	if (mInfo->direccionNaves == NAVES_DIR_DER) {
		mInfo->l1_navc1_x++;
		mInfo->l1_navc2_x++;
		mInfo->l1_navc3_x++;
		mInfo->l1_navc4_x++;
		mInfo->l1_navef_x++;

		mInfo->l2_navc1_x++;
		mInfo->l2_navc2_x++;
		mInfo->l2_navc3_x++;
		mInfo->l2_navc4_x++;
		mInfo->l2_navef_x++;

		mInfo->l3_navc1_x++;
		mInfo->l3_navc2_x++;
		mInfo->l3_navc3_x++;
		mInfo->l3_navc4_x++;
		mInfo->l3_navef_x++;

		mInfo->l4_navc1_x++;
		mInfo->l4_navc2_x++;
		mInfo->l4_navc3_x++;
		mInfo->l4_navc4_x++;
		mInfo->l4_navef_x++;
	} else {
		mInfo->l1_navc1_x--;
		mInfo->l1_navc2_x--;
		mInfo->l1_navc3_x--;
		mInfo->l1_navc4_x--;
		mInfo->l1_navef_x--;

		mInfo->l2_navc1_x--;
		mInfo->l2_navc2_x--;
		mInfo->l2_navc3_x--;
		mInfo->l2_navc4_x--;
		mInfo->l2_navef_x--;

		mInfo->l3_navc1_x--;
		mInfo->l3_navc2_x--;
		mInfo->l3_navc3_x--;
		mInfo->l3_navc4_x--;
		mInfo->l3_navef_x--;

		mInfo->l4_navc1_x--;
		mInfo->l4_navc2_x--;
		mInfo->l4_navc3_x--;
		mInfo->l4_navc4_x--;
		mInfo->l4_navef_x--;
	}

}
