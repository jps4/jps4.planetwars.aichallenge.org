#ifndef CENTROID_H_
#define CENTROID_H_

#include <string>
#include <vector>
#include "PlanetWars.h"


class Centroid
{
	double m_dX;
	double m_dY;
	
	int m_nWeight;
	int m_nOwner;
	int m_nPlanets;
public:
	Centroid();
	
	// Use the following functions to set the properties of this planet. Note
	// that these functions only affect your program's copy of the game state.
	// You can't steal your opponent's planets just by changing the owner to 1
	// using the Owner(int) function! :-)
	void SetX(double x);
	void SetY(double y);
	void SetWeight(int nWeight);
	void SetOwner(int nOwner);
	void SetPlanets(int nPlanets);		
	
	// The position of the planet in space.
	double X() const;
	double Y() const;

	// Returns the ID of the player that owns this planet. Your playerID is
	// always 1. If the owner is 1, this is your planet. If the owner is 0, then
	// the planet is neutral. If the owner is 2 or some other number, then this
	// planet belongs to the enemy.
	int Owner() const;
	int Weight() const;
	int Planets() const;
	
	int GetDistance(const Centroid& c) const;
	int GetDistance(const Planet& p) const;
		
};


class Centroids
{
	PlanetWars m_planetWars;
	int m_nMaxDistance;
public:
	Centroids( const PlanetWars& planetWars);
	Centroid* GetCentroid(int nPlayer, int nTurn) const;
	
	int GetMaxDistance() const;
	int GetDistance(const Centroid& c1, const Centroid& c2) const;
	int GetDistance(const Centroid& c1, const Planet& p1) const;
};


class IssueOrder
{
	int m_source_planet;
	int m_destination_planet;
	int m_num_ships;
	double m_rating;
	
public:
	IssueOrder(int source, int dest, int nNum, double rating);
	
	int GetSource() const;
	int GetDestination() const;
	int GetNum() const;
	double GetRating() const;
};

#endif

