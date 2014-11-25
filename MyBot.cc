#include <iostream>
#include <vector>
#include <time.h> 
#include "PlanetWars.h"

#if DEBUGFILE
#include <fstream>

int nTimestamp = time(NULL);
#endif

//turn number of current game
int nTurn = 0;
double dInitTime = 0.0;

int max( int i1, int i2)
{
	return (i1 > i2) ? i1 : i2;	
}
std::vector<Planet> knapsack01(std::vector<Planet> planets, int maxWeight) 
{
            std::vector<int> weights;
            std::vector<int> values;
           
            // solve 0-1 knapsack problem
            for (std::vector<Planet>::iterator it = planets.begin(); it != planets.end(); ++it) {
                // here weights and values are numShips and growthRate respectively
                // you can change this to something more complex if you like...
                Planet p = *it;       
                weights.push_back(p.NumShips() + 1);
                values.push_back(p.GrowthRate());
            }
           
            //int K[weights.size()+1][maxWeight];
			int **K = new int*[weights.size()+1];
			int i = 0;
			for( i = 0; i<weights.size()+1; i++)
			{
				K[i] = new int[maxWeight];
			}
           
            for (i = 0; i < maxWeight; i++) {
                K[0][i] = 0;
            }
           
            for (int k = 1; k <= weights.size(); k++) {
                for (int y = 1; y <= maxWeight; y++) {
               
                    if (y < weights[k-1]){
                      K[k][y-1] = K[k-1][y-1];
                    } 
                    else if (y > weights[k-1]){
                      K[k][y-1] = max( K[k-1][y-1], K[k-1][y-1-weights[k-1]] + values[k-1]);
                    }
                    else
                      K[k][y-1] = max(K[k-1][y-1], values[k-1]);
                }
            }

            // get the planets in the solution
            i = weights.size();
            int currentW = maxWeight-1;
            std::vector<Planet> markedPlanets;
           
            while ((i > 0) && (currentW >= 0)) {
                if (((i == 0) && (K[i][currentW] > 0)) || (K[i][currentW] != K[i-1][currentW])) {
                    markedPlanets.push_back(planets[i-1]);
                    currentW = currentW - weights[i-1];
                }
                i--;
            }

			if (K)
			{
				for( int i = 0; i<weights.size()+1; i++)
				{
					delete [] K[i];
				}
				delete [] K;
			}

            return markedPlanets;
}

// The DoTurn function is where your code goes. The PlanetWars object contains
// the state of the game, including information about all planets and fleets
// that currently exist. Inside this function, you issue orders using the
// pw.IssueOrder() function. For example, to send 10 ships from planet 3 to
// planet 8, you would say pw.IssueOrder(3, 8, 10).
//
// There is already a basic strategy in place here. You can use it as a
// starting point, or you can throw it out entirely and replace it with your
// own. Check out the tutorials and articles on the contest website at
// http://www.ai-contest.com/resources.

void DoTurn(PlanetWars& pw)
{
	//first turn? use knapsack01 to optimize growth
	if ( pw.MyPlanets().size() == 1 && pw.EnemyPlanets().size() == 1 && pw.NumFleets() == 0)
	{
		int nMyNumShips = pw.MyPlanets()[0].NumShips();
		int nReplaceShips = pw.MyPlanets()[0].GrowthRate() * pw.Distance(pw.MyPlanets()[0].PlanetID(), pw.EnemyPlanets()[0].PlanetID());
		int ships_available = (nMyNumShips < nReplaceShips) ? nMyNumShips : nReplaceShips;
		
		//simetric maps, avoid stupiud things like select a far planet (enemy conquer first surely!)
		std::vector<Planet> possiblePlanets;
		for ( int j = 0; j<pw.NeutralPlanets().size(); j++)
		{
			if ( pw.Distance(pw.MyPlanets()[0].PlanetID(), pw.NeutralPlanets()[j].PlanetID())+1 < 
					pw.Distance(pw.EnemyPlanets()[0].PlanetID(), pw.NeutralPlanets()[j].PlanetID()))
			{
				possiblePlanets.insert(possiblePlanets.end(), pw.NeutralPlanets()[j]);
			}
		}
		
		std::vector<Planet> planets = knapsack01(possiblePlanets, ships_available);
		
		//attack those neutral planets!
		for (int i = 0; i < planets.size(); i++)
		{
			pw.IssueOrder(pw.MyPlanets()[0].PlanetID(), planets[i].PlanetID(), planets[i].NumShips()+1);
		}
		return;
	}

	std::vector<Planet> myplanets = pw.MyPlanets();
	
	Order* currentOrder = NULL;
	pw.CalculateFPS();
	
	bool bStop = false;
	bool bParanoicMode = false;
	bool bEnteringParanoic = false;
	std::vector<Planet> planets = pw.Planets();
	do
	{
		if ( currentOrder)
			pw.ReCalculateFPS(currentOrder);

		pw.ClearAttackOrders();
		bEnteringParanoic = false;

		for ( int i = 0; i < planets.size(); i++)
		{
			const Planet& p = planets[i];
			pw.DefendPossibleLostPlanets(p);
			if ( p.Owner() == 1)
			{
				if ( pw.IsFrontPlanet(p.PlanetID(), pw.GetMaxDistance(p.PlanetID())))
					pw.GenerateAttackMovements(p, bParanoicMode);
				else
					pw.GenerateSupplyMovements(p);
			}
		}

		if ( pw.GetAttackOrders() == 0 && pw.IamGoingToLose() && !bParanoicMode)
		{
			//attack everywhere, do something... i don't wanna loseeeee
			bParanoicMode = true;
			bEnteringParanoic = true;
		}

		currentOrder = pw.GetFirstVirtualOrder();
		if ( currentOrder)
			pw.AddDefinitiveOrder(currentOrder);
		
	}while(!bStop && (currentOrder ||bEnteringParanoic));
	
	pw.ExecuteOrders();
}


// This is just the main game loop that takes care of communicating with the
// game engine for you. You don't have to understand or change the code below.
int main(int argc, char *argv[]) {
	std::string current_line;
	std::string map_data;

       	PlanetWars pw;

	while (true) 
	{
	    int c = std::cin.get();
	    current_line += (char)c;
	    if (c == '\n') 
	    {
	    	if (current_line.length() >= 2 && current_line.substr(0, 2) == "go") 
		{
	        	
	        	pw.NewTurn(map_data);
		        map_data = "";
		        DoTurn(pw);
			pw.FinishTurn();
#if DEBUGFILE

		char szFileName[256];
		sprintf(szFileName, "%s_%d.log", "Xayide", nTimestamp);
	  	std::ofstream  file_op(szFileName, std::fstream::app);
		file_op.close();
#endif

		}
		else 
		{
	        	map_data += current_line;
	      	}
	      	current_line = "";
	    }
	}
	return 0;
}
