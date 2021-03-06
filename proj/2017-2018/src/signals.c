#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "signals.h"
#include "helpers.h"

/*--------------------------------------------------------------------
| Private functions
---------------------------------------------------------------------*/
// Operting System-specific macros
#ifdef __APPLE__
	#define checkerr(cb, msg) cb
#else
	#define checkerr(cb, msg) if (cb) { fprintf(stderr, msg); exit(EXIT_FAILURE); }
#endif

// SIGINT items
int interrupted;
// SIGALRM items
int alarmed;
int alarm_interval;

// Signal items
int my_signals[] = { SIGINT, SIGALRM };
sigset_t set;

// General
#define test_signal(sig, a) { \
	if (sig != a) { \
		fprintf(stderr, "Signal other than "#sig" was deployed.\n"); \
		exit(EXIT_FAILURE); \
	} \
}

void manage_signal(int signum) {
	switch (signum) {
	case SIGINT:
		test_signal(SIGINT, signum);
		interrupted = 1;
		break;
	case SIGALRM:
		test_signal(SIGALRM, signum);
		alarmed = 1;
		break;
	}
}

/*--------------------------------------------------------------------
| Public functions
---------------------------------------------------------------------*/
void signals_block(void) {
	checkerr(sigemptyset(&set), "Não foi possível esvaziar o conjunto de sinais\n");

	for (size_t idx = 0; idx < array_len(my_signals); idx++) {
		int signum = my_signals[idx];
		checkerr(sigaddset(&set, signum), "Não foi possível adicionar um sinal ao conjunto de sinais\n");
	}

	if (pthread_sigmask(SIG_BLOCK, &set, NULL)) {
		fprintf(stderr, "Não foi possível bloquear os sinais\n");
		exit(EXIT_FAILURE);
	}
}

void signals_unblock(void) {
	if (pthread_sigmask(SIG_UNBLOCK, &set, NULL)) {
		fprintf(stderr, "Não foi possível desbloquear os sinais\n");
		exit(EXIT_FAILURE);
	}

	checkerr(sigemptyset(&set), "Não foi possível esvaziar o conjunto de sinais\n");
}

void signals_init(int interval) {
	/* Criando o sigaction geral para todos os signals */
	struct sigaction mysigaction = {
		.sa_handler   = manage_signal,
		.sa_mask      = set
	};

	for (size_t idx = 0; idx < array_len(my_signals); idx++) {
		int signum = my_signals[idx];
		if (sigaction(signum, &mysigaction, NULL)) {
			fprintf(stderr, "Erro ao definir sinal.\n");
			exit(EXIT_FAILURE);
		}
	}

	/* Inicializar variáveis deste scope */
	interrupted = 0;
	alarmed = 0;
	alarm_interval = interval;

	if (interval > 0) {
		signals_reset_alarm();
	}
}

int signals_was_interrupted(void) {
	return interrupted;
}

int signals_was_alarmed(void) {
	return alarmed;
}

void signals_reset_alarm(void) {
	alarmed = 0;
	alarm(alarm_interval);
}
