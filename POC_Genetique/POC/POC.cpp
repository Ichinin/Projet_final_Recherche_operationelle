// POC.cpp : définit le point d'entrée pour l'application console.

#include "stdafx.h"
#include "Entete.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <time.h>

using namespace std; 


//Prototype des fonctions locales

//Remplacement élitiste avec maintiens du meilleur individu
void Remplacement(std::vector<TSolution> & Parents, std::vector<TSolution> Enfants, TProblem unProb, TGenetic unGen);

//Mutation par permutation des gênes
void Mutation(TSolution &Individu, TProblem &LeProb, TGenetic LeGen);

//Croisement Ox des parents
TSolution Croisement(TSolution Parent1, TSolution Parent2, TProblem unProb, TGenetic & unGen);

//Selection des parents pour croisement (Selection aléatoire puis roulette)
int Selection(vector<TSolution> Population, int taillePop, TProblem LeProb, int Generation);

//Copie d'une solution dans une autre (utilisé pour garder le Best)
void CopierSolution(TSolution Solution, TSolution &LeBest, TProblem LeProb);

//Affichage d'une population de solution avec Fitness
void AfficherSolutions(vector<TSolution> &Population, TProblem LeProb, int TaillePop, TGenetic &LeGen);

//Tri de la population par ordre décroissant de fitness
void TrierPopulation(vector<TSolution> &Population, int TaillePop);

//Vérifie si une tache est déjà présente dans une sequence
bool contains(vector<int> sequence, int number);

//Generation d'une population initiale en faisant appel à la NEH
void GenererPopInitialeValide(vector<TSolution>& Pop, TProblem LeProb, TGenetic & LeGenetic);

//Construction d'une solution en l'instanciant sur une ligne de production
vector<int> ConstruitTache(string temps_tache, int NbMachines);

//Chargement de la matrice de temps de réglage 
vector<int> SetupTimeParser(string ligne, int nbtaches);

//Chargement des instances d'une problème dans la structure TProblem
void LectureProbleme(string NomFichier, TProblem &leProb);

//Retourne le temps de réglage d'une tâche à l'autre sur une machine
int getTempsReglage(TProblem Probleme, int nMachine, int i, int j);

//Heuristique de construction priorisant les tâches par ordre décroissant de durée d'execution des tâches
vector<int> NEH_Heuristique(TProblem &Probleme);

//Date à laquelle une tache est disponible pour entrer passer dans la prochaine machine
int getDispoTache(TProblem & LeProb, int numero_tache);

//Estime la fitness d'une solution construite
int EvaluerSolution(TProblem &LeProb, TSolution &UneSol, TGenetic &LeGen);

//A partir d'une solution (Sequence de tache) execution des taches dans chaques machines
void ConstruireSolution(TProblem &LeProb,TSolution &UneSol, TGenetic &UnGenetic);


int main(int NbParam, char *Param[])
{
	TProblem LeProb;
	TSolution UneSol;
	string NomFichier;

	bool stopped;
	timespec start_time;
	timespec stop_time;
	int clock_id;
	
	//Chrono chrono(Clock::Thread);

	TGenetic LeGenetic;
	vector<TSolution> Pop;
	vector<TSolution> PopEnfant;
	TSolution Best;

	int Pere, Mere;
	double Alea;

	//Définition des paramètres du problème
	NomFichier.assign(Param[1]);
	LeGenetic.TaillePop = atoi(Param[2]);
	LeGenetic.ProbCr = atof(Param[3]);
	LeGenetic.ProbMut = atof(Param[4]);
	LeGenetic.NB_EVAL_MAX = atoi(Param[5]);
	LeGenetic.TaillePopEnfant = (int)ceil(LeGenetic.TaillePop*LeGenetic.ProbCr);
	LeGenetic.Gen = 0;
	LeGenetic.cptEval = 0;

	//Définition de la taille des populations 
	Pop.resize(LeGenetic.TaillePop);
	PopEnfant.resize(LeGenetic.TaillePopEnfant);

	//Lecture et affichage du probleme
	LectureProbleme(NomFichier, LeProb);

	//Constuire population initiale valide
	GenererPopInitialeValide(Pop, LeProb, LeGenetic);

	//Tri de la population par fitness croissante
	TrierPopulation(Pop, LeGenetic.TaillePop);
	AfficherSolutions(Pop, LeProb, LeGenetic.TaillePop, LeGenetic);

	//Initialisation de la meilleure solution
	CopierSolution(Pop[0], Best, LeProb);
	cout << endl << "Meilleure solution de la population initiale : " << Best.FctObj << endl << endl;

	//Boucle principale de l'algorithme génétique
	do 
	{
		LeGenetic.Gen++;

		//Selection et croisement
		for (int i = 0; i < LeGenetic.TaillePopEnfant; i++) {

			//Croisement entre deux parents avec création d'UN enfant
			Pere = Selection(Pop, LeGenetic.TaillePop, LeProb, LeGenetic.Gen);
			Mere = Selection(Pop, LeGenetic.TaillePop, LeProb, LeGenetic.Gen);

			PopEnfant[i] = Croisement(Pop[Pere], Pop[Mere], LeProb, LeGenetic);

			//Mutation d'une solution
			Alea = double(rand()) / double(RAND_MAX);
			if (Alea < LeGenetic.ProbMut)
			{
				//Vérification pour ne pas perdre la meilleure solution connue avant mutation
				if (Best.FctObj > PopEnfant[i].FctObj) {
					CopierSolution(PopEnfant[i], Best, LeProb);
				}
					Mutation(PopEnfant[i], LeProb, LeGenetic);
			}
		}

		//Remplacement de la population pour la prochaine génération
		Remplacement(Pop, PopEnfant, LeProb, LeGenetic);

		//Conservation de la meilleure solution
		TrierPopulation(Pop, LeGenetic.TaillePop);
		if (Best.FctObj > Pop[0].FctObj) {
			CopierSolution(Pop[0], Best, LeProb);
		}
		cout << "Meilleure solution trouvee (Generation# " << LeGenetic.Gen << "): " << Best.FctObj << endl;

	} while (LeGenetic.cptEval < LeGenetic.NB_EVAL_MAX);


	system("PAUSE");
	return 0;
}


void Remplacement(std::vector<TSolution> & Parents, std::vector<TSolution> Enfants, TProblem unProb, TGenetic unGen)
{
	//**Déclaration et dimension dynamique d'une population temporaire pour contenir tous les parents et les enfants
	std::vector<TSolution> Temporaire;
	std::vector<TSolution> Temporaire_parents;

	for (int i = 0; i < Parents.size(); i++)
	{
		Temporaire.push_back(Parents[i]);
	}

	for (int i = 0; i < Enfants.size(); i++)
	{
		Temporaire.push_back(Enfants[i]);
	}

	TrierPopulation(Temporaire, Parents.size() + Enfants.size());

	int taille = Temporaire.size();
	int proportion_elite = 1;
	int proportion_proletaires = Parents.size() - proportion_elite;

	for (int i = 0; i < proportion_elite; i++)
	{
		//Temporaire_parents.push_back(Temporaire[i]);
		Temporaire_parents.push_back(Temporaire[0]);
		Temporaire.erase(Temporaire.begin());
	}

	for (int i = 0; i < proportion_proletaires; i++)
	{
		int index = rand() % Temporaire.size();
		Temporaire_parents.push_back(Temporaire[index]);
		Temporaire.erase(Temporaire.begin() + index);

		//Temporaire_parents.push_back(Temporaire[taille - (i+1)]);
	}

	Parents = Temporaire_parents;
}

void Mutation(TSolution &Individu, TProblem &LeProb, TGenetic LeGen) {
	int tache1, tache2, temp;

	tache1 = rand() % LeProb.NbTaches;
	tache2 = rand() % LeProb.NbTaches;

	temp = Individu.Seq[tache1];
	Individu.Seq[tache1] = Individu.Seq[tache2];
	Individu.Seq[tache2] = temp;
}

TSolution Croisement(TSolution Parent1, TSolution Parent2, TProblem unProb, TGenetic & unGen)
{
	//**INDICE: Le sous-programme rand() génère aléatoirement un nombre entier entre 0 et RAND_MAX (i.e., 32767) inclusivement.
	//**Pour tirer un nombre aléatoire entier entre 0 et MAX-1 inclusivement, il suffit d'utiliser l'instruction suivante : NombreAleatoire = rand() % MAX;

	TSolution Enfant;
	TSolution Pere;
	TSolution Mere;
	/*//METHODE BIDON: Recopie les genes du Parent1 dans l'enfant
	CopierSolution(Parent1, Enfant, unProb);*/

	int Coupure1 = (int)(unProb.NbTaches * .33);
	int Coupure2 = (int)(unProb.NbTaches * .66);

	double aleat = (double)rand() / RAND_MAX;

	if (aleat < .5) {
		CopierSolution(Parent1, Mere, unProb);
		CopierSolution(Parent2, Pere, unProb);
	}
	else {
		CopierSolution(Parent2, Mere, unProb);
		CopierSolution(Parent1, Pere, unProb);
	}

	//Heritage correspond au corps du chromosome qui est transmis a l'enfant
	vector<int> heritage_maman;
	vector<int> heritage_papa = Pere.Seq;

	vector<int> seq_enfant(unProb.NbTaches, 0);

	for (int i = Coupure1 + 1; i < (Coupure2 + 1); i++)
	{
		heritage_maman.push_back(Mere.Seq.at(i));
	}

	for (int i = 0; i < heritage_maman.size(); i++)
	{
		for (int j = 0; j < heritage_papa.size(); j++)
		{
			if (heritage_maman[i] == heritage_papa[j]) {

				heritage_papa.erase(heritage_papa.begin() + j);

			}
		}
	}

	for (size_t i = 0; i < unProb.NbTaches; i++)
	{
		int Locus = i;

		if ((Coupure1 < Locus) && (Locus <= Coupure2)) {
			seq_enfant[Locus] = heritage_maman.at(Locus - (Coupure1 + 1));
		}
		else if (Locus <= Coupure1) {
			seq_enfant[Locus] = heritage_papa[i];
		}
		else if (Coupure2 < Locus) {
			seq_enfant[Locus] = heritage_papa[i - (Coupure1 + 1)];
		}
	}

	Enfant.Seq = seq_enfant;

	//**NE PAS ENLEVER
	ConstruireSolution(unProb, Enfant, unGen);
	EvaluerSolution(unProb, Enfant, unGen);

	return (Enfant);
}

//RWS : Roulette wheel selection (F = Fmax-F)
int Selection(vector<TSolution> Population, int taillePop, TProblem LeProb, int Generation) {
	
	int selected = 0;

	if (Generation < taillePop) {
		//Un trop grand écars de fitness genere une "consanguinité" de la population
		selected = rand() % taillePop;
	}
	else {
		// RWS : Roulette Wheel selection
		int Fmax = Population[taillePop - 1].FctObj;
		int selected;
		vector<int> Cumulatif;

		Cumulatif.resize(taillePop);

		for (int i = 0; i < taillePop; i++) {

			int proportion = Fmax - Population[i].FctObj;

			if (i == 0) {
				Cumulatif[i] = proportion;
			}
			else {
				Cumulatif[i] = Cumulatif[i - 1] + proportion;
			}
		}

		int rang = rand() % Cumulatif[taillePop - 1];

		for (int i = 0; i < taillePop; i++) {
			if ((i == 0) && (rang <= Cumulatif[i])) {
				selected = i;
			}
			else if (i != (taillePop - 1)) {
				if ((Cumulatif[i] <= rang) && (rang <= Cumulatif[i + 1]))
				{
					selected = i;
				}
			}
			else if (rang == Cumulatif[i]) {
				selected = i;
			}
		}

	}
	
	return selected;
}

void CopierSolution(TSolution Solution, TSolution &LeBest, TProblem LeProb) {
	LeBest = Solution;
}

void AfficherSolutions(vector<TSolution> &Population, TProblem LeProb, int TaillePop, TGenetic &LeGen) {
	cout << "Population initiale : " << endl;

	for (int i = 0; i < TaillePop; i++) {
		for (int j = 0; j < LeProb.NbTaches; j++) {
			cout << Population.at(i).Seq.at(j) <<"   ";
		}

		cout << " : solution " << i << " (fitness : " << Population.at(i).FctObj << ") " << endl;
	}
}

void TrierPopulation(vector<TSolution> &Population, int TaillePop) {
	 
	for (int i = 0; i < TaillePop; i++) {
		
		for (int j = i; j < TaillePop; j++) {
			if (Population.at(i).FctObj>Population.at(j).FctObj) {
				int Fitness_j = Population.at(i).FctObj;
				int Fitness_i = Population.at(j).FctObj;
				Population.at(i).FctObj = Fitness_i;
				Population.at(j).FctObj = Fitness_j;
			}
		}

	}

	
}

bool contains(vector<int> sequence, int number) {
	bool contain = false;

	for each (int var in sequence)
	{
		if (var == number) {
			contain = true;
		}
	}

	return contain;
}

//Construction d'une solution initiale à partir de la heuristique de construction NEH 
void GenererPopInitialeValide(vector<TSolution>& Pop, TProblem LeProb, TGenetic & LeGenetic) {
	

	int taillePop = Pop.size();
	TSolution UneSol;
	vector<int> Sequence_initiale;

	int tache_courante, FctObj, temp0, temp1, index_tache0;

	Sequence_initiale = NEH_Heuristique(LeProb);
	UneSol.Seq = Sequence_initiale;
	ConstruireSolution(LeProb, UneSol, LeGenetic);
	Pop.at(0) = UneSol;

	for (int i = 1; i < taillePop; i++) {
		vector<int> Sequence;
		Sequence.resize(LeProb.NbTaches);
		int tache;
		
		for (int j = 0; j < LeProb.NbTaches; j++) {
			Sequence.at(j) = -1;
			do
			{
				tache = rand() % LeProb.NbTaches;
			} while (contains(Sequence, tache));
			Sequence.at(j) = tache;
		}

		//Amélioration : l'index 0 etant généralement le dernier sorti (defaut de la méthode rand()), echange de cet index avec un autre
		index_tache0 = rand() % LeProb.NbTaches;
		temp0 = Sequence.at(LeProb.NbTaches-1);
		temp1 = Sequence.at(index_tache0);

		Sequence.at(index_tache0) = temp0;
		Sequence.at(LeProb.NbTaches-1) = temp1;

		UneSol.Seq = Sequence;
		ConstruireSolution(LeProb, UneSol, LeGenetic);
		Pop.at(i) = UneSol;
	}

}


//Extrait les temps d'executions de chaque machine sur les machine depuis une ligne (correspondant à une tache)
vector<int> ConstruitTache(string temps_tache, int NbMachines) {
	int index_mchn;
	int date_lim;
	vector<int> TempsTaches;
	istringstream buffer(temps_tache);
	//cout << temps_tache << endl;

	for (int j = 0; j < NbMachines; j++)
	{
		buffer >> index_mchn >> date_lim;
		if (index_mchn == j) {
			TempsTaches.push_back(date_lim);
			//cout << "[Mch, tache] : [" << index_mchn << "," << "] Temps :" << TempsTaches.at(j) << endl;
		}
	}

	buffer.clear();
	return TempsTaches;
}



//Extrait les informations de temps de réglage ligne par ligne
vector<int> SetupTimeParser(string ligne, int nbtaches) {
	//cout << ligne << endl;
	istringstream buffer(ligne);

	vector<int> setup_time_row;
	int setup_time;

	for (int i = 0; i < nbtaches; i++) {
		buffer >> setup_time;
		setup_time_row.push_back(setup_time);
	}

	return setup_time_row;
}


//Chargement des données du fichier et construction de la solution initiale.
void LectureProbleme(string NomFichier, TProblem &leProb) {
	leProb.Nom = NomFichier;
	Taches tache;

	fstream fichier(NomFichier, ios::in);
	
	string prochaine_ligne;
	if (fichier.is_open()) {

		cout << NomFichier << endl;

		int Nbtaches, NbMachines;

		getline(fichier, prochaine_ligne);
		//cout << prochaine_ligne << endl;
		istringstream buffer(prochaine_ligne);
		
		//Chargement taille du probleme
		buffer >> Nbtaches >> NbMachines;
		//cout << NbMachines << endl;

		leProb.NbMachines = NbMachines;
		leProb.NbTaches = Nbtaches;

		buffer.clear();

		//Construction des taches par instant de passage sur machine 
		for (int i = 0; i < Nbtaches; i++)
		{
			getline(fichier, prochaine_ligne);
			
			vector<int> TpsTaches = ConstruitTache(prochaine_ligne, NbMachines);
			tache.TempsProcess = TpsTaches;
			leProb.Liste_taches.push_back(tache);	
		}

		cout << "taches construites" << endl;
		
		getline(fichier, prochaine_ligne);	

		//Construction des machines avec temps de réglage 
		for (int j = 0; j < NbMachines; j++) {
			
			getline(fichier, prochaine_ligne);

			vector<vector<int>> Setup_time;

			for (int i = 0; i < Nbtaches; i++) {

				getline(fichier, prochaine_ligne);
				vector<int> setup_time_ligne = SetupTimeParser(prochaine_ligne, Nbtaches);
				Setup_time.push_back(setup_time_ligne);
			}

			Machine machine;
			machine.Temps_reglage = Setup_time;
			machine.temps_total = 0;
			leProb.Ensemble_machines.push_back(machine);
		}	

		fichier.close();
		
	}
	else
		cerr << "Impossible d'ouvrir le fichier" << endl;
}




//Lis la matrice de réglage et retourne le temps de réglage depuis la tache i vers la tache j pour la machine nMachine
int getTempsReglage(TProblem Probleme, int nMachine, int i, int j) {
	Machine machine = Probleme.Ensemble_machines.at(nMachine);
	vector<int> tps_rgl_tache_precedente = machine.Temps_reglage.at(i);
	int tps_rgl_tache_prochaine = tps_rgl_tache_precedente.at(j);

	return tps_rgl_tache_prochaine;

}


/*
Construction d'une premiere suite de tache selon le procédé suivant : 
- Calcul du temps total d'execution des taches (tps_total) sur toutes les machines
- Tri dans l'ordre décroissant de tps_total 
- Retourne une liste d'index correspondant aux taches classées par tps_total décroissant
*/
vector<int> NEH_Heuristique(TProblem &Probleme) {
	vector<vector<int>> Tps_total_exec;
	Taches tache;

	int temps_maximal, tps_i, tps_j;

	vector<int> tps_tache, sup, inf, tps_exec_decroissant;

	//Calcul du temps total d'execution de chaques tâches
	for (int i = 0; i < Probleme.NbTaches; i++) {
		tache = Probleme.Liste_taches.at(i);
		temps_maximal = 0;
		for (int j = 0; j < Probleme.NbMachines; j++) {
			temps_maximal += tache.TempsProcess.at(j);
		}
		//cout << "tache " << i << " " << temps_maximal << endl;
		tps_tache.clear();
		
		tps_tache.push_back(i);
		tps_tache.push_back(temps_maximal);

		Tps_total_exec.push_back(tps_tache);
	}

	
	//Classement par ordre décroissant des tps_max
	for (int i = 0; i < (Probleme.NbTaches); i++) {
		for (int j = i; j < (Probleme.NbTaches); j++) {
			
			//Comparaison des temps dexecution
			tps_i = Tps_total_exec.at(i).at(1);
			tps_j = Tps_total_exec.at(j).at(1);

			if (tps_i < tps_j) {
								
				sup = Tps_total_exec.at(j);
				inf = Tps_total_exec.at(i);
				Tps_total_exec.at(i) = sup;
				Tps_total_exec.at(j) = inf;
			}
		}
	}

	//Recuperation des index de tache trié par ordre decroissant
	
	for (int i = 0; i < Probleme.NbTaches; i++) {
		tps_exec_decroissant.push_back(Tps_total_exec.at(i).at(0));
	}

	return tps_exec_decroissant;
}


// Donne la date de début de disponibilité d'une tache pour un temps d'execution donné.
int getDispoTache(TProblem & LeProb, int numero_tache) {
	
	Taches tache = LeProb.Liste_taches.at(numero_tache);
	int derniere_date = tache.Schedule.back().at(1);

	return derniere_date;
}


//Incrémente le compteur d'évaluation d'une solution 
int EvaluerSolution(TProblem &LeProb, TSolution &UneSol, TGenetic &LeGen) {

	
	int fonction_obj;

	fonction_obj = LeProb.Ensemble_machines.back().temps_total;
	
	UneSol.FctObj = fonction_obj;
	LeGen.cptEval++;
	//cout << "eval : " << LeGen.cptEval << " fonction objectif : " << fonction_obj << endl;

	return fonction_obj;
}

//Construction d'une solution avec un atelier seriel (TProblem) et une liste de tâches (TSolution)
void ConstruireSolution(TProblem &LeProb, TSolution &UneSol, TGenetic &UnGenetic) {
	

	int NbTaches = LeProb.NbTaches;
	int NbMachines = LeProb.NbMachines;
	vector<int> ordre_tache = UneSol.Seq;

	int numero_tache_courante, numero_tache_prochaine, date_dispo_machine, tps_execution, fin_tache_machine, date_dispo_tache, tps_reglage;

	vector <int> execution_tache;
	//Passages des taches dans les machines
	for (int i = 0; i < NbTaches; i++) {

		numero_tache_courante = ordre_tache[i];
		numero_tache_prochaine = -1;
		
		if (i < (NbTaches-1)) {
			numero_tache_prochaine = ordre_tache[i + 1];
		}

		Taches tache_actuelle = LeProb.Liste_taches.at(numero_tache_courante);

		//Passage de la tache dans chaques machines avec ajout du temps de réglage pour prochaine tache
		for (int j = 0; j < NbMachines; j++)
		{	
			date_dispo_machine = LeProb.Ensemble_machines[j].temps_total;
			tps_execution = tache_actuelle.TempsProcess[j];
			fin_tache_machine = date_dispo_machine + tps_execution;
			date_dispo_tache = 0;
			
			if (j == 0) {
				//ajout du temps de traitement
				LeProb.Ensemble_machines[j].temps_total += tps_execution;

				execution_tache.clear();
				execution_tache.push_back(date_dispo_machine);
				execution_tache.push_back(fin_tache_machine);
				LeProb.Liste_taches.at(numero_tache_courante).Schedule.push_back(execution_tache);
			}
			else {
				date_dispo_tache = getDispoTache(LeProb, numero_tache_courante);

				//Gestion des disponibilité taches/machines
				if (date_dispo_tache < date_dispo_machine) {
					LeProb.Ensemble_machines[j].temps_total += tps_execution;

					execution_tache.clear();
					execution_tache.push_back(date_dispo_machine);
					execution_tache.push_back(fin_tache_machine);
					LeProb.Liste_taches[numero_tache_courante].Schedule.push_back(execution_tache);
				}
				else {
					LeProb.Ensemble_machines[j].temps_total = (date_dispo_tache + tps_execution);

					execution_tache.clear();
					execution_tache.push_back(date_dispo_tache);
					execution_tache.push_back((date_dispo_tache+tps_execution));
					LeProb.Liste_taches.at(numero_tache_courante).Schedule.push_back(execution_tache);

				}
				
			}
			
			if (i < (NbTaches - 1)) {
				tps_reglage = getTempsReglage(LeProb, j, numero_tache_courante, numero_tache_prochaine);
				LeProb.Ensemble_machines[j].temps_total += tps_reglage;
			}	

			if (j == 0) {
				//cout << "Machine " << j << " tache "<<numero_tache_courante << " "<<LeProb.Ensemble_machines.Machines.at(j).temps_total << endl;
			}
			
		}
	}

	UneSol.FctObj = EvaluerSolution(LeProb, UneSol, UnGenetic);
	
}

