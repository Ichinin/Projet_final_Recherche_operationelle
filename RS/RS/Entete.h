#pragma once

#include <fstream>
#include <iostream>
#include <vector>
using namespace std;


struct Machine
{
	vector<vector<int>> Temps_reglage;		//Temps de reglage d'une machine entre deux taches (passage de taches i -> j : [i,j])
	int temps_total;						//Makespan de la machine
};

struct Taches
{
	vector<int> TempsProcess;				//Temps de process sur chaque machines
	vector<vector<int>> Schedule;			//Liste des instants de début et fin sur une machine

};

struct TProblem								//Définition du probleme
{
	string Nom;								//Nom du fichier de probleme
	int NbTaches;							//Nombre de taches indiqué dans le fichier
	int NbMachines;							//Nombre de machines indiqué dans le fichier
	vector<Taches> Liste_taches;			//Liste des taches avec leurs temps max de passage sur les n machines
	vector<Machine> Ensemble_machines;		//Ligne de production sur laquelle les tâches doivent passer		

};


struct TSolution
{
	vector<int> Seq;						//Sequence d'ordonnancement des commandes
	long FctObj;							//Valeur de la fonction objective : sommation des retards
};

struct TRecuit
{
	double Temperature;						//Temperature courante du systeme
	int TempInit;							//Temperature initiale du systeme
	int NoPalier;							//Compteur de palier
	
	int Delta;								//Dégradation courante entre la solution courante et la solution Next
	double Alpha;							//Schema de "reduction de la température" : Temp(t+1) = Alpha*Temp(t)
	int NbPalier;							//Schema de "reduction de la température" : Duree = NB_EVAL_MAX/NbPalier

	int cptEval;							//Compteur du nombre de solutions evaluees. Point de sortie du programme
	int NB_EVAL_MAX;						//Critère d'arrêt

};
