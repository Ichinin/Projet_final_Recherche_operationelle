// POC.cpp�: d�finit le point d'entr�e pour l'application console.

#include "stdafx.h"
#include "Entete.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>

using namespace std;


//Prototype des fonctions locales

//Descente par permutation des g�nes
TSolution Descente(TSolution &Individu, TProblem &LeProb, TDescente LaDesc, int &tache_a_developper);

//Copie d'une solution dans une autre (utilis� pour garder le Best)
void CopierSolution(TSolution Solution, TSolution &LeBest, TProblem LeProb);

//Affichage d'une population de solution avec Fitness
void AfficherSolution(TSolution &Solution, TProblem LeProb, int cptEval);

//V�rifie si une tache est d�j� pr�sente dans une sequence
bool contains(vector<int> sequence, int number);

//Generation d'une population initiale en faisant appel � la NEH
void GenererSolutionValide(TSolution &UneSolution, TProblem LeProb, TDescente &LaDesc);

//Construction d'une solution en l'instanciant sur une ligne de production
vector<int> ConstruitTache(string temps_tache, int NbMachines);

//Heuristique de construction priorisant les t�ches par ordre d�croissant de dur�e d'execution des t�ches
vector<int> NEH_Heuristique(TProblem &Probleme);

//Chargement de la matrice de temps de r�glage 
vector<int> SetupTimeParser(string ligne, int nbtaches);

//Chargement des instances d'une probl�me dans la structure TProblem
void LectureProbleme(string NomFichier, TProblem &leProb);

//Retourne le temps de r�glage d'une t�che � l'autre sur une machine
int getTempsReglage(TProblem Probleme, int nMachine, int i, int j);

//Heuristique de construction priorisant les t�ches par ordre d�croissant de dur�e d'execution des t�ches
vector<int> NEH_Heuristique(TProblem &Probleme);

//Date � laquelle une tache est disponible pour entrer passer dans la prochaine machine
int getDispoTache(TProblem & LeProb, int numero_tache);

//Estime la fitness d'une solution construite
int EvaluerSolution(TProblem &LeProb, TSolution &UneSol, TDescente &LaDesc);

//A partir d'une solution (Sequence de tache) execution des taches dans chaques machines
void ConstruireSolution(TProblem &LeProb, TSolution &UneSol, TDescente &LaDesc);


int main(int NbParam, char *Param[])
{
	TProblem LeProb;
	TSolution Courante;
	string NomFichier;


	TDescente LaDesc;
	TSolution Next;

	//D�finition des param�tres du probl�me
	NomFichier.assign(Param[1]);
	LaDesc.NB_EVAL_MAX = atoi(Param[2]);
	LaDesc.cptEval = 0;

	//Lecture et affichage du probleme
	LectureProbleme(NomFichier, LeProb);

	//Constuire solution initiale valide
	GenererSolutionValide(Courante, LeProb, LaDesc);
	AfficherSolution(Courante, LeProb, LaDesc.cptEval);

	//Initialisation de la meilleure solution
	CopierSolution(Courante, Next, LeProb);
	
	int tache_a_developper = Courante.Seq[0];
	int nb_iteration = 0;

	//Boucle principale de l'algorithme g�n�tique
	do
	{	

		Next = Descente(Next, LeProb, LaDesc, tache_a_developper);
		ConstruireSolution(LeProb, Next, LaDesc);
		nb_iteration++;

		//V�rification pour ne pas perdre la meilleure solution connue avant mutation
		if (Next.FctObj <= Courante.FctObj) {
			CopierSolution(Next, Courante, LeProb);
			cout << "Meilleure solution trouve (iteration #" << LaDesc.cptEval << "): " << Next.FctObj << endl;
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
					index = (i+1)%LeProb.NbTaches;
				}
			}
			
			tache_a_developper = Courante.Seq[index];
		}
	
		//AfficherSolution(Next, LeProb, LaDesc.cptEval);

	} while (LaDesc.cptEval < LaDesc.NB_EVAL_MAX);

	AfficherSolution(Courante, LeProb, LaDesc.cptEval);

	system("PAUSE");
	return 0;
}



TSolution Descente(TSolution &Individu, TProblem &LeProb, TDescente LaDesc, int &tache_a_developper) {

	int temp;
	int index, index_prochain;

	for (int i = 0; i < LeProb.NbTaches; i++) {
		if (Individu.Seq[i] == tache_a_developper) {
			index = i;
		}
	}
	
	if (index == (LeProb.NbTaches - 1)) {
		index_prochain = index;
	}
	index_prochain = (index + 1) % LeProb.NbTaches;
	temp = Individu.Seq[index];
	Individu.Seq[index] = Individu.Seq[index_prochain];
	Individu.Seq[index_prochain] = temp;

	return Individu;
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
void GenererSolutionValide(TSolution &UneSol, TProblem LeProb, TDescente &UneDescente) {
	int temp0, temp1, index_tache0;

	vector<int> Sequence;

	Sequence = NEH_Heuristique(LeProb);
	UneSol.Seq = Sequence;
	ConstruireSolution(LeProb, UneSol, UneDescente);

}


//Extrait les temps d'executions de chaque machine sur les machine depuis une ligne (correspondant � une tache)
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



//Extrait les informations de temps de r�glage ligne par ligne
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


//Chargement des donn�es du fichier et construction de la solution initiale.
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

		//Construction des machines avec temps de r�glage 
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




//Lis la matrice de r�glage et retourne le temps de r�glage depuis la tache i vers la tache j pour la machine nMachine
int getTempsReglage(TProblem Probleme, int nMachine, int i, int j) {
	Machine machine = Probleme.Ensemble_machines.at(nMachine);
	vector<int> tps_rgl_tache_precedente = machine.Temps_reglage.at(i);
	int tps_rgl_tache_prochaine = tps_rgl_tache_precedente.at(j);

	return tps_rgl_tache_prochaine;

}


// Donne la date de d�but de disponibilit� d'une tache pour un temps d'execution donn�.
int getDispoTache(TProblem & LeProb, int numero_tache) {

	Taches tache = LeProb.Liste_taches.at(numero_tache);
	int derniere_date = tache.Schedule.back().at(1);

	return derniere_date;
}


//Incr�mente le compteur d'�valuation d'une solution 
int EvaluerSolution(TProblem &LeProb, TSolution &UneSol, TDescente &UneDesc) {


	int fonction_obj;

	fonction_obj = LeProb.Ensemble_machines.back().temps_total;

	UneSol.FctObj = fonction_obj;
	
	//cout << "eval : " << LeGen.cptEval << " fonction objectif : " << fonction_obj << endl;
	UneDesc.cptEval++;
	return fonction_obj;
}

//Construction d'une solution avec un atelier seriel (TProblem) et une liste de t�ches (TSolution)
void ConstruireSolution(TProblem &LeProb, TSolution &UneSol, TDescente &UneDescente) {


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

		//Passage de la tache dans chaques machines avec ajout du temps de r�glage pour prochaine tache
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

				//Gestion des disponibilit� taches/machines
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

	UneSol.FctObj = EvaluerSolution(LeProb, UneSol, UneDescente);

}

/*
Construction d'une premiere suite de tache selon le proc�d� suivant :
- Calcul du temps total d'execution des taches (tps_total) sur toutes les machines
- Tri dans l'ordre d�croissant de tps_total
- Retourne une liste d'index correspondant aux taches class�es par tps_total d�croissant
*/
vector<int> NEH_Heuristique(TProblem &Probleme) {
	vector<vector<int>> Tps_total_exec;
	Taches tache;

	int temps_maximal, tps_i, tps_j;

	vector<int> tps_tache, sup, inf, tps_exec_decroissant;

	//Calcul du temps total d'execution de chaques t�ches
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


	//Classement par ordre d�croissant des tps_max
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

	//Recuperation des index de tache tri� par ordre decroissant

	for (int i = 0; i < Probleme.NbTaches; i++) {
		tps_exec_decroissant.push_back(Tps_total_exec.at(i).at(0));
	}

	return tps_exec_decroissant;
}