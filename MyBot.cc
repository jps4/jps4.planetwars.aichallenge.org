#include <iostream>
#include <stdlib.h>
#include "PlanetWars.h"
#include "Centroid.h"

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
void DoTurn_new(const PlanetWars& pw)
{
	// (0) create main object for centroid's management
	Centroids c(pw);
	
	// (1) Calculate Centroids for me and enemy
	Centroid *c1 = c.GetCentroid(1, 0);
	Centroid *c2 = c.GetCentroid(2, 0);
	
	//future centroids (forecast)
	Centroid *c1f = c.GetCentroid(1, 10);
	Centroid *c2f = c.GetCentroid(2, 10);
	
	int nDistance = c.GetDistance(*c1, *c2);
	int nDistancef = c.GetDistance(*c1f, *c2f);
	int nMaxDistance = c.GetMaxDistance();
	
	double dDefensiveFactor = (double)nDistance/nMaxDistance;
	double dAttackFactor = 1 - dDefensiveFactor;
	double dDefensiveFactorf = (double)nDistancef/nMaxDistance;
	double dAttackFactorf = 1 - dDefensiveFactorf;
	
	std::vector<IssueOrder> defensiveOrders;
	std::vector<IssueOrder> attackOrders;
	
	//GenerateDefensiveOrders(pw);
	//GenerateAttackOrders(pw);
	
	
}


void DoTurn(const PlanetWars& pw)
{
	// (1) Decision to attack depends on number of ships of my own compared
	// against the enemy, and on the total fleets attacking now
	// comparison is done by using growth factor to forecast forces
	int nMyShips = 0;
	int nEnemyShips = 0;
	std::vector<Planet> lPlanets = pw.Planets();
	for (int i = 0; i < lPlanets.size(); i++) 
	{
		const Planet& p = lPlanets[i];
		if ( p.Owner() == 0)
			nEnemyShips += p.NumShips();
		if ( p.Owner() == 1)
			nMyShips += p.NumShips() + (p.GrowthRate()*5);
		else
			nEnemyShips += p.NumShips() + (p.GrowthRate()*5);
 	}

	std::vector<Fleet> myFleets = pw.MyFleets();
        for (int i = 0; i < myFleets.size(); i++)
        {
		const Fleet& f = myFleets[i];
		const Planet& p = pw.GetPlanet(f.DestinationPlanet());
		if (p.Owner() == 0 )
		{
			nMyShips += (f.NumShips() > p.NumShips()) ? f.NumShips() : 0;
	 	}
	        if (p.Owner() == 2 )
                {
                        nMyShips += (f.NumShips() > (p.NumShips() + f.TurnsRemaining()*p.GrowthRate())) ? f.NumShips() : 0;
                }
 	}

	std::vector<Fleet> enemyFleets = pw.EnemyFleets();
        for (int i = 0; i < enemyFleets.size(); i++)
        {
                const Fleet& f = enemyFleets[i];
                const Planet& p = pw.GetPlanet(f.DestinationPlanet());
                if (p.Owner() == 0 )
                {
                        nEnemyShips += (f.NumShips() > p.NumShips()) ? f.NumShips() : 0;
                }
                if (p.Owner() == 1 )
                {
                        nEnemyShips += (f.NumShips() > (p.NumShips() + f.TurnsRemaining()*p.GrowthRate())) ? f.NumShips() : 0;
                }
        }

        int nFleetsForAttack = 1 + (int)((1.0 + nEnemyShips*nEnemyShips)/(1.0 + nMyShips*5));
	if (pw.MyFleets().size() >= nFleetsForAttack)
	    return;

	srand ( time(NULL) );
	bool bContinueSrc = true;
	std::vector<int> fromPlanets;
	std::vector<int> attackedPlanets;

	std::vector<Planet> my_planets = pw.MyPlanets();

	while(bContinueSrc)
	{
		// (2) Find the strongest planet of my own
		int source = -1;
		double source_score = -999999.0;
		int source_num_ships = 0;
		for (int i = 0; i < my_planets.size(); i++)
		{
			const Planet& p = my_planets[i];
			//just issue an order from this planet?
			bool bAttackingFromSrc = false;
			for ( int k = 0; k < fromPlanets.size(); k++)
			{
				if ( p.PlanetID() == fromPlanets[k])
					bAttackingFromSrc = true;
			}
			if ( bAttackingFromSrc)
				continue;
			double score = (double)p.NumShips();
			if (score > source_score) 
			{
			      source_score = score;
			      source = p.PlanetID();
			      source_num_ships = p.NumShips();
			}
		}
			
		bool bContinue = true;
		while(source >= 0 && bContinue)
		{
			// (3) Find the weakest enemy or neutral planet.
			int dest = -1;
			double dest_score = -999999.0;
			std::vector<Planet> not_my_planets = pw.NotMyPlanets();
			for (int i = 0; i < not_my_planets.size(); i++) 
			{
				const Planet& p = not_my_planets[i];
				//just issue an order against this planet?
				bool bAttacking = false;
				for ( int k = 0; k < attackedPlanets.size(); k++)
				{
					if ( p.PlanetID() == attackedPlanets[k])
						bAttacking = true;
				}
				if ( bAttacking)
					continue;
				//have we got a fleet attacking this planet now?
			        for (int j = 0; j < myFleets.size() && !bAttacking; j++)
		        	{
			                const Fleet& f = myFleets[j];
		        	        const Planet& pf = pw.GetPlanet(f.DestinationPlanet());
					if (p.PlanetID() == pf.PlanetID())
					{
						bAttacking = true;
					} 
		 		}
				if ( bAttacking)
					continue;
	
				double score = 0.0;
				if ( p.Owner() == 2)
					score = 1.0 / (1 + p.NumShips() + (p.GrowthRate()* pw.Distance(source, dest)));
				else	
					score = 1.0 / (1 + p.NumShips());
		
				if (score > dest_score) 
				{
				      dest_score = score;
				      dest = p.PlanetID();
				}
			}	
		
			// (4) Send x ships from my strongest planet to the weakest
			// planet that I do not own. x is the minimum required to 
			// conquer the planet
			if (source >= 0 && dest >= 0)
			{
				const Planet &p = pw.GetPlanet(dest);
				int nShips = 0;
				if ( p.Owner() == 2)
					nShips = p.NumShips() + 1 + (p.GrowthRate()* pw.Distance(source, dest));
				else
					nShips = p.NumShips() + 1;
		
				if ( nShips < source_num_ships)
				{
					pw.IssueOrder(source, dest, nShips);
					Planet &ps = (Planet&)pw.GetPlanet(source);
					ps.RemoveShips(nShips);
					source_num_ships -= nShips;
					attackedPlanets.insert(attackedPlanets.end(), dest); 
				}
				else if ( fromPlanets.size() > 0)
				{
	 				//defensive, send fleets to greater planets		
					int nPlanet = (fromPlanets.size() == 1) ? 0: (rand() % (fromPlanets.size()-1));
					pw.IssueOrder(source, fromPlanets[nPlanet], source_num_ships);
					source_num_ships = 0;
					bContinue = false;
				}
				else
					bContinue = false;
			}
			else if (fromPlanets.size() > 0)
			{
				//defensive, send fleets to greater planets
				int nPlanet = (fromPlanets.size() == 1) ? 0: (rand() % (fromPlanets.size()-1));
				pw.IssueOrder(source, fromPlanets[nPlanet], source_num_ships);
				source_num_ships = 0;
				bContinue = false;
			}
			else
				bContinue = false;
		}
		
		if ( source >= 0)
			fromPlanets.insert(fromPlanets.end(), source);
		else
			bContinueSrc = false;
		
	}
}




// This is just the main game loop that takes care of communicating with the
// game engine for you. You don't have to understand or change the code below.
int main(int argc, char *argv[]) {
  std::string current_line;
  std::string map_data;
  while (true) {
    int c = std::cin.get();
    current_line += (char)c;
    if (c == '\n') {
      if (current_line.length() >= 2 && current_line.substr(0, 2) == "go") {
        PlanetWars pw(map_data);
        map_data = "";
        DoTurn(pw);
	pw.FinishTurn();
      } else {
        map_data += current_line;
      }
      current_line = "";
    }
  }
  return 0;
}
