// POC.cpp : définit le point d'entrée pour l'application console.

#include "stdafx.h"
#include "Entete.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>

using namespace std;


//Prototype des fonctions locales

//Descente par permutation des gênes
TSolution Descente(TSolution &Individu, TProblem &LeProb, TRecuit LeRecuit, int &tache_a_developper);

//Copie d'une solution dans une autre (utilisé pour garder le Best)
void CopierSolution(TSolution Solution, TSolution &LeBest, TProblem LeProb);

//Affichage d'une population de solution avec Fitness
void AfficherSolution(TSolution &Solution, TProblem LeProb, int cptEval);

//Vérifie si une tache est déjà présente dans une sequence
bool contains(vector<int> sequence, int number);

//Generation d'une population initiale en faisant appel à la NEH
void GenererSolutionValide(TSolution &UneSolution, TProblem LeProb, TRecuit &LeRecuit);

//Construction d'une solution en l'instanciant sur une ligne de production
vector<int> ConstruitTache(string temps_tache, int NbMachines);

//Heuristique de construction priorisant les tâches par ordre décroissant de durée d'execution des tâches
vector<int> NEH_Heuristique(TProblem &Probleme);

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
int EvaluerSolution(TProblem &LeProb, TSolution &UneSol, TRecuit &LeRecuit);

//A partir d'une solution (Sequence de tache) execution des taches dans chaques machines
void ConstruireSolution(TProblem &LeProb, TSolution &UneSol, TRecuit &LeRecuit);


int main(int NbParam, char *Param[])
{
	TProblem LeProb;
	TSolution Courante;
	string NomFichier;


	TRecuit LeRecuit;
	TSolution Next;

	//Définition des paramètres du problème
	NomFichier.assign(Param[1]);
	LeRecuit.TempInit = atoi(Param[2]);
	LeRecuit.Alpha = atof(Param[3]);
	LeRecuit.NbPalier = atoi(Param[4]);
	LeRecuit.NB_EVAL_MAX = atoi(Param[5]);
	LeRecuit.cptEval = 0;

	//Lecture et affichage du probleme
	LectureProbleme(NomFichier, LeProb);

	//Constuire solution initiale valide
	GenererSolutionValide(Courante, LeProb, LeRecuit);
	AfficherSolution(Courante, LeProb, LeRecuit.cptEval);

	//Initialisation de la meilleure solution
	CopierSolution(Courante, Next, LeProb);

	int tache_a_developper = Courante.Seq[0];
	int nb_iteration = 0;

	//Boucle principale de l'algorithme génétique
	do
	{

		Next = Descente(Next, LeProb, LeRecuit, tache_a_developper);
		ConstruireSolution(LeProb, Next, LeRecuit);
		LeRecuit.Delta = Next.FctObj - Courante.FctObj;

		nb_iteration++;

		//Vérification pour ne pas perdre la meilleure solution connue avant mutation
		if (LeRecuit.Delta <= 0) {
			CopierSolution(Next, Courante, LeProb);
			cout << "Meilleure solution trouve (iteration #" << LeRecuit.cptEval << "): " << Next.FctObj << endl;
			tache_a_developper = Courante.Seq[0];

			//reinitialisation des tours
			nb_iteration = 0;
		}

		if (nb_iteration == (LeProb.NbTaches)) {
			Next = Courante;
			nb_iteration = 0;
			int index = 0;

			for (int i = 0; i < LeProb.NbTaches; i++) {
				if (Courante.Seq[i] == tache_a_developper) {
					index = (i + 1) % LeProb.NbTaches;
				}
			}

			tache_a_developper = Courante.Seq[index];
		}

		//AfficherSolution(Next, LeProb, LaDesc.cptEval);

	} while (LeRecuit.cptEval < LeRecuit.NB_EVAL_MAX);

	AfficherSolution(Courante, LeProb, LeRecuit.cptEval);

	system("PAUSE");
	return 0;
}


// Recherche de voisinage par échange de paires
TSolution Descente(TSolution &Individu, TProblem &LeProb, TRecuit LeRecuit, int &tache_a_developper) {

	TSolution Copie;
	int temp, index, index_prochain, Tmp;

	CopierSolution(Individu, Copie, LeProb);

	index = rand() % LeProb.NbTaches;
	do {
		index_prochain = rand() % LeProb.NbTaches;
	} while (index == index_prochain);

	Tmp = Copie.Seq[index];
	Copie.Seq[index] = Copie.Seq[index_prochain];
	Copie.Seq[index_prochain] = Copie.Seq[index];

	EvaluerSolution(LeProb, Copie, LeRecuit);
	return Copie;
}


void CopierSolution(TSolution Solution, TSolution &LeBest, TProblem LeProb) {
	LeBest = Solution;
}

void AfficherSolution(TSolution &Solution, TProblem LeProb, int cptEval) {
	cout << "Solution (cpt " << cptEval << ") : " << endl;

	for (int j = 0; j < LeProb.NbTaches; j++) {
		cout << Solution.Seq.at(j) << "   ";
	}
	cout << " (fitness : " << Solution.FctObj << ") " << endl;

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


//Construction d'une solution aleatoire
void GenererSolutionValide(TSolution &UneSol, TProblem LeProb, TRecuit &LeRecuit) {
	int temp0, temp1, index_tache0;

	vector<int> Sequence;

	Sequence = NEH_Heuristique(LeProb);
	UneSol.Seq = Sequence;
	ConstruireSolution(LeProb, UneSol, LeRecuit);

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
	{
		cerr << "Impossible d'ouvrir le fichier" << endl;
		system("PAUSE");
	}

}




//Lis la matrice de réglage et retourne le temps de réglage depuis la tache i vers la tache j pour la machine nMachine
int getTempsReglage(TProblem Probleme, int nMachine, int i, int j) {
	Machine machine = Probleme.Ensemble_machines.at(nMachine);
	vector<int> tps_rgl_tache_precedente = machine.Temps_reglage.at(i);
	int tps_rgl_tache_prochaine = tps_rgl_tache_precedente.at(j);

	return tps_rgl_tache_prochaine;

}


// Donne la date de début de disponibilité d'une tache pour un temps d'execution donné.
int getDispoTache(TProblem & LeProb, int numero_tache) {

	Taches tache = LeProb.Liste_taches.at(numero_tache);
	int derniere_date = tache.Schedule.back().at(1);

	return derniere_date;
}


//Incrémente le compteur d'évaluation d'une solution 
int EvaluerSolution(TProblem &LeProb, TSolution &UneSol, TRecuit &LeRecuit) {


	int fonction_obj;

	fonction_obj = LeProb.Ensemble_machines.back().temps_total;

	UneSol.FctObj = fonction_obj;

	//cout << "eval : " << LeGen.cptEval << " fonction objectif : " << fonction_obj << endl;
	LeRecuit.cptEval++;
	return fonction_obj;
}

//Construction d'une solution avec un atelier seriel (TProblem) et une liste de tâches (TSolution)
void ConstruireSolution(TProblem &LeProb, TSolution &UneSol, TRecuit &LeRecuit) {


	int NbTaches = LeProb.NbTaches;
	int NbMachines = LeProb.NbMachines;
	vector<int> ordre_tache = UneSol.Seq;

	int numero_tache_courante, numero_tache_prochaine, date_dispo_machine, tps_execution, fin_tache_machine, date_dispo_tache, tps_reglage;

	vector <int> execution_tache;
	//Passages des taches dans les machines
	for (int i = 0; i < NbTaches; i++) {

		numero_tache_courante = ordre_tache[i];
		numero_tache_prochaine = -1;

		if (i < (NbTaches - 1)) {
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
					execution_tache.push_back((date_dispo_tache + tps_execution));
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

	UneSol.FctObj = EvaluerSolution(LeProb, UneSol, LeRecuit);

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