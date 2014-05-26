
// Filho
typedef struct s_filho {
	char *nome;
	int pid;
	struct s_filho *next;
} filho;

// Freguesia
typedef struct s_freg {
	char *nome;
	int f_n_casos;
	struct s_freg *next;
} freg;

// Concelho
typedef struct s_conc {
	char *nome;
	int c_n_casos;
	struct s_freg *freguesias;
	struct s_conc *next;
} conc;

// Distrito
typedef struct s_dist {
	char *nome;
	int d_n_casos;
	struct s_conc *concelhos;
} dist;