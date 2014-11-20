#include <cmath>
#include "Centroid.h"



Centroid::Centroid() : m_dX(0.0), m_dY(0.0), m_nWeight(0), m_nOwner(0), m_nPlanets(0)
{
}


// Use the following functions to set the properties of this planet. Note
// that these functions only affect your program's copy of the game state.
// You can't steal your opponent's planets just by changing the owner to 1
// using the Owner(int) function! :-)
void Centroid::SetX(double x)
{
	m_dX = x;	
}
void Centroid::SetY(double y)
{
	m_dY = y;	
}
void Centroid::SetWeight(int nWeight)
{
	m_nWeight = nWeight;
}
void Centroid::SetOwner(int nOwner)
{
	m_nOwner = nOwner;
}
void Centroid::SetPlanets(int nPlanets)
{
	m_nPlanets = nPlanets;
}
	
// The position of the planet in space.
double Centroid::X() const
{
	return m_dX;
}
double Centroid::Y() const
{
	return m_dY;
}

// Returns the ID of the player that owns this planet. Your playerID is
// always 1. If the owner is 1, this is your planet. If the owner is 0, then
// the planet is neutral. If the owner is 2 or some other number, then this
// planet belongs to the enemy.
int Centroid::Owner() const
{
	return m_nOwner;
}
int Centroid::Weight() const
{
	return m_nWeight;
}
int Centroid::Planets() const
{
	return m_nPlanets;
}
	
int Centroid::GetDistance(const Centroid& c) const
{
  double dx = X() - c.X();
  double dy = Y() - c.Y();
  return (int)ceil(sqrt(dx * dx + dy * dy));	
	
}
int Centroid::GetDistance(const Planet& p) const
{
  double dx = X() - p.X();
  double dy = Y() - p.Y();
  return (int)ceil(sqrt(dx * dx + dy * dy));	
}		

//*************************
//class Centroids
//*************************

Centroids::Centroids( const PlanetWars& planetWars) : m_nMaxDistance(0), m_planetWars(planetWars)
{
	//calculate greatest distance between planets
	std::vector<Planet> lPlanets = planetWars.Planets();
	for(int i = 0; i< lPlanets.size(); i++)
	{
		const Planet& p1 = lPlanets[i];
		for(int j = i+1; j< lPlanets.size(); j++)
		{
			const Planet& p2 = lPlanets[j];
			int nDistance = planetWars.Distance(p1.PlanetID(), p2.PlanetID());
			if ( nDistance > m_nMaxDistance)
				m_nMaxDistance = nDistance;
		}
	}
}

int Centroids::GetMaxDistance() const
{
	return 	m_nMaxDistance;
}

Centroid* Centroids::GetCentroid(int nPlayer, int nTurn) const
{
	if ( nTurn < 0)
		return NULL;
		
	//weights and locations of my planets
	std::vector<Planet> planets;
	//weights and locations of my possible future planets
	std::vector<Planet> futurePlanets1;
	std::vector<Planet> futurePlanets2;
	
	//own fleets and enemy fleets (from the point of view of nPlayer)
	std::vector<Fleet> myFleets;
	std::vector<Fleet> enemyFleets;
	switch(nPlayer)
	{
	case 0:
		planets = m_planetWars.NeutralPlanets();
		enemyFleets = m_planetWars.Fleets();
		break;
	case 1:
		planets = m_planetWars.MyPlanets();
		futurePlanets1 = m_planetWars.NeutralPlanets();
		futurePlanets2 = m_planetWars.EnemyPlanets();
		myFleets = m_planetWars.MyFleets();
		enemyFleets = m_planetWars.EnemyFleets();
		break;	
	default: //2
		planets = m_planetWars.EnemyPlanets();
		futurePlanets1 = m_planetWars.NeutralPlanets();
		futurePlanets2 = m_planetWars.MyPlanets();
		myFleets = m_planetWars.EnemyFleets();
		enemyFleets = m_planetWars.MyFleets();
	}
	
	double dXW = 0.0;
	double dYW = 0.0;
	int nW = 0;
	int nPlanets = 0;
	// search into my planets
	for(int i = 0; i < planets.size(); i++) 
	{
		const Planet& p = planets[i];
		//if turn > 0, we have to see "in the future"  (fleets and planet growth)
		int nWi = p.NumShips() + ((p.Owner()!=0) ? p.GrowthRate() : 0)* nTurn;
		
		// friend fleets going to this planet
		for ( int j = 0; j < myFleets.size(); j++)
		{
			const Fleet& f = myFleets[j];
			if ( f.DestinationPlanet() != p.PlanetID())
				continue;
			//add fleet to the planet in turn (other option: consider location annd weight of fleet: TODO)
			if ( f.TurnsRemaining() <= nTurn)
				nWi += f.NumShips() + ((nTurn - f.TurnsRemaining())*((p.Owner()!=0) ? p.GrowthRate() : 0));		
		}
		
		//enemy fleets going to this planet
		for ( int k = 0; k < enemyFleets.size(); k++)
		{
			const Fleet& f = enemyFleets[k];
			if ( f.DestinationPlanet() != p.PlanetID())
				continue;
			//substract fleet to the planet in turn (other option: consider location annd weight of fleet: TODO)
			if ( f.TurnsRemaining() <= nTurn)
				nWi -= f.NumShips() + ((nTurn - f.TurnsRemaining())*((p.Owner()!=0) ? p.GrowthRate() : 0));
		}
		
		//TODO: if nWi < 0? seems that planet is not mine... maybe to include it in enemy's centroid?
		if (nWi > 0)
		{
			dXW += p.X()*nWi;
			dYW += p.Y()*nWi;
			nW += nWi;
			nPlanets++;
		}
	}

	if ( nTurn > 0)
	{
		//search into possible future planets
		for(int i = 0; i < futurePlanets1.size(); i++) 
		{
			const Planet& p = futurePlanets1[i];
			//if turn > 0, we have to see "in the future"  (fleets and planet growth)
			int nWi = -(p.NumShips() + ((p.Owner()!=0) ? p.GrowthRate() : 0)* nTurn);
			
			// friend fleets going to this planet
			for ( int j = 0; j < myFleets.size(); j++)
			{
				const Fleet& f = myFleets[j];
				if ( f.DestinationPlanet() != p.PlanetID())
					continue;
				//add fleet to the planet in turn (other option: consider location annd weight of fleet: TODO)
				if ( f.TurnsRemaining() <= nTurn)
					nWi += f.NumShips() + ((nTurn - f.TurnsRemaining())*((p.Owner()!=0) ? p.GrowthRate() : 0));		
			}
			
			//enemy fleets going to this planet
			for ( int k = 0; k < enemyFleets.size(); k++)
			{
				const Fleet& f = enemyFleets[k];
				if ( f.DestinationPlanet() != p.PlanetID())
					continue;
				//substract fleet to the planet in turn (other option: consider location annd weight of fleet: TODO)
				if ( f.TurnsRemaining() <= nTurn)
					nWi -= f.NumShips() + ((nTurn - f.TurnsRemaining())*((p.Owner()!=0) ? p.GrowthRate() : 0));
			}
			
			//if nWi < 0? seems that planet "would" be mine in nTurn turns 
			if (nWi > 0)
			{
				dXW += p.X()*nWi;
				dYW += p.Y()*nWi;
				nW += nWi;
				nPlanets++;
			}
		}
		for(int i = 0; i < futurePlanets2.size(); i++) 
		{
			const Planet& p = futurePlanets2[i];
			//if turn > 0, we have to see "in the future"  (fleets and planet growth)
			int nWi = -(p.NumShips() + ((p.Owner()!=0) ? p.GrowthRate() : 0)* nTurn);
			
			// friend fleets going to this planet
			for ( int j = 0; j < myFleets.size(); j++)
			{
				const Fleet& f = myFleets[j];
				if ( f.DestinationPlanet() != p.PlanetID())
					continue;
				//add fleet to the planet in turn (other option: consider location annd weight of fleet: TODO)
				if ( f.TurnsRemaining() <= nTurn)
					nWi += f.NumShips() + ((nTurn - f.TurnsRemaining())*((p.Owner()!=0) ? p.GrowthRate() : 0));		
			}
			
			//enemy fleets going to this planet
			for ( int k = 0; k < enemyFleets.size(); k++)
			{
				const Fleet& f = enemyFleets[k];
				if ( f.DestinationPlanet() != p.PlanetID())
					continue;
				//substract fleet to the planet in turn (other option: consider location annd weight of fleet: TODO)
				if ( f.TurnsRemaining() <= nTurn)
					nWi -= f.NumShips() + ((nTurn - f.TurnsRemaining())*((p.Owner()!=0) ? p.GrowthRate() : 0));
			}
			
			//if nWi < 0? seems that planet "would" be mine in nTurn turns 
			if (nWi > 0)
			{
				dXW += p.X()*nWi;
				dYW += p.Y()*nWi;
				nW += nWi;
				nPlanets++;
			}
		}
	}

	Centroid *c = new Centroid();
	//collect data
	c->SetX(dXW / nW);
	c->SetY(dYW / nW);
	c->SetOwner(nPlayer);
	c->SetWeight(nW);
	c->SetPlanets(nPlanets);
	
	return c;
}
	
int Centroids::GetDistance(const Centroid& c1, const Centroid& c2) const
{
	return c1.GetDistance(c2);
}

int Centroids::GetDistance(const Centroid& c1, const Planet& p1) const
{
	return c1.GetDistance(p1);	
}



//*********************
// IssueOrder
//*********************
IssueOrder::IssueOrder(int source, int dest, int nNum, double rating) : m_source_planet(source), m_destination_planet(dest),
		m_num_ships(nNum), m_rating(rating)
{
}

int IssueOrder::GetSource() const
{
	return 	m_source_planet;
}
int IssueOrder::GetDestination() const
{
	return m_destination_planet;
}
int IssueOrder::GetNum() const
{
	return m_num_ships;
}
double IssueOrder::GetRating() const
{
	return m_rating;
}

