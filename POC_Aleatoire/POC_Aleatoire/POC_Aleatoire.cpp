// POC_Aleatoire.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"
#include "Entete.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>

using namespace std;
using namespace std::chrono;

//Prototype des fonctions locales

//Copie d'une solution dans une autre (utilisé pour garder le Best)
void CopierSolution(TSolution Solution, TSolution &LeBest, TProblem LeProb);

//Affichage d'une population de solution avec Fitness
void AfficherSolution(TSolution &Solution, TProblem LeProb, int cptEval);

//Vérifie si une tache est déjà présente dans une sequence
bool contains(vector<int> sequence, int number);

//Generation d'une solution valide
void GenererSolutionValide(TSolution& Pop, TProblem LeProb);

//Construction d'une solution en l'instanciant sur une ligne de production
vector<int> ConstruitTache(string temps_tache, int NbMachines);

//Chargement de la matrice de temps de réglage 
vector<int> SetupTimeParser(string ligne, int nbtaches);

//Chargement des instances d'une problème dans la structure TProblem
void LectureProbleme(string NomFichier, TProblem &leProb);

//Retourne le temps de réglage d'une tâche à l'autre sur une machine
int getTempsReglage(TProblem Probleme, int nMachine, int i, int j);

//Date à laquelle une tache est disponible pour entrer passer dans la prochaine machine
int getDispoTache(TProblem & LeProb, int numero_tache);

//Estime la fitness d'une solution construite
int EvaluerSolution(TProblem &LeProb, TSolution &UneSol);

//A partir d'une solution (Sequence de tache) execution des taches dans chaques machines
void ConstruireSolution(TProblem &LeProb, TSolution &UneSol);


int main(int NbParam, char *Param[])
{
	TProblem LeProb;
	TSolution UneSol;
	string NomFichier;

	TRandom UnAleat;
	TSolution Best;

	//Definition des parametres du probleme
	NomFichier.assign(Param[1]);
	UnAleat.NB_EVAL_MAX = atoi(Param[2]);
	UnAleat.cptEval = 0;

	//Construire population initiale valide
	LectureProbleme(NomFichier, LeProb);
	
	GenererSolutionValide(Best, LeProb);
	AfficherSolution(Best, LeProb, UnAleat.cptEval);

	do
	{
		UnAleat.cptEval++;

		GenererSolutionValide(UneSol, LeProb);
		

		if (Best.FctObj > UneSol.FctObj) {
			CopierSolution(UneSol, Best,LeProb);
			AfficherSolution(Best, LeProb, UnAleat.cptEval);
		}

	} while (UnAleat.cptEval < UnAleat.NB_EVAL_MAX);
	
	system("PAUSE");
    return 0;
}

void AfficherSolution(TSolution &Solution, TProblem LeProb, int cptEval) {
	cout << "Solution (cpt "<<cptEval<<") : " << endl;

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
void GenererSolutionValide(TSolution &UneSol, TProblem LeProb) {
	int temp0, temp1, index_tache0;

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
	temp0 = Sequence.at(LeProb.NbTaches - 1);
	temp1 = Sequence.at(index_tache0);
	
	Sequence.at(index_tache0) = temp0;
	Sequence.at(LeProb.NbTaches - 1) = temp1;

	UneSol.Seq = Sequence;
	ConstruireSolution(LeProb, UneSol);
		
	}

void CopierSolution(TSolution Solution, TSolution &LeBest, TProblem LeProb) {
	LeBest = Solution;
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
	else {
		cerr << "Impossible d'ouvrir le fichier" << endl;
		system("PAUSE");
	}
		
}

// Donne la date de début de disponibilité d'une tache pour un temps d'execution donné.
int getDispoTache(TProblem & LeProb, int numero_tache) {

	Taches tache = LeProb.Liste_taches.at(numero_tache);
	int derniere_date = tache.Schedule.back().at(1);

	return derniere_date;
}

//Lis la matrice de réglage et retourne le temps de réglage depuis la tache i vers la tache j pour la machine nMachine
int getTempsReglage(TProblem Probleme, int nMachine, int i, int j) {
	Machine machine = Probleme.Ensemble_machines.at(nMachine);
	vector<int> tps_rgl_tache_precedente = machine.Temps_reglage.at(i);
	int tps_rgl_tache_prochaine = tps_rgl_tache_precedente.at(j);

	return tps_rgl_tache_prochaine;

}

//Incrémente le compteur d'évaluation d'une solution 
int EvaluerSolution(TProblem &LeProb, TSolution &UneSol) {

	int fonction_obj;

	fonction_obj = LeProb.Ensemble_machines.back().temps_total;

	UneSol.FctObj = fonction_obj;
	//cout << "eval : " << LeGen.cptEval << " fonction objectif : " << fonction_obj << endl;

	return fonction_obj;
}

//Construction d'une solution avec un atelier seriel (TProblem) et une liste de tâches (TSolution)
void ConstruireSolution(TProblem &LeProb, TSolution &UneSol) {


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

	UneSol.FctObj = EvaluerSolution(LeProb, UneSol);

}