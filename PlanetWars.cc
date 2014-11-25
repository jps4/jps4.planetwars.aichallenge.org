#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <time.h> 

#include "PlanetWars.h"

#if DEBUGFILE
#include <fstream>
#endif


//turn number of current game
extern int nTurn;
extern int nTimestamp;


void StringUtil::Tokenize(const std::string& s,
                          const std::string& delimiters,
                          std::vector<std::string>& tokens) {
  std::string::size_type lastPos = s.find_first_not_of(delimiters, 0);
  std::string::size_type pos = s.find_first_of(delimiters, lastPos);
  while (std::string::npos != pos || std::string::npos != lastPos) {
    tokens.push_back(s.substr(lastPos, pos - lastPos));
    lastPos = s.find_first_not_of(delimiters, pos);
    pos = s.find_first_of(delimiters, lastPos);
  }
}

std::vector<std::string> StringUtil::Tokenize(const std::string& s,
                                              const std::string& delimiters) {
  std::vector<std::string> tokens;
  Tokenize(s, delimiters, tokens);
  return tokens;
}

//*************************************************************************

Node::Node ( int nPlanetID, int nDistance) : m_nPlanetID(nPlanetID), m_nDistance(nDistance)
{
}

int Node::GetPlanetID() const
{
	return m_nPlanetID;	
}
int Node::GetDistance() const
{
	return m_nDistance;
}

DistanceNode::DistanceNode(int size) : m_nSize(0)
{
	m_nodes = new Node*[size];	
}
DistanceNode::~DistanceNode()
{
	Delete();
	delete [] m_nodes;
	m_nodes = NULL;		
}
void DistanceNode::Add(int nPlanetID, int nDistance)
{
	Node* node = new Node(nPlanetID, nDistance);
	Add(node);	
}
void DistanceNode::Add(Node* node)
{
	m_nodes[m_nSize++] = node;		
}
void DistanceNode::Delete()
{
	if (m_nodes)
	{
		for ( int n = 0; n <m_nSize; n++)
		{
			delete m_nodes[n];
			m_nodes[n] = NULL;
		}
		m_nSize = 0;
	}	
}

static int CompareNodes(const void *arg1, const void *arg2)
{
 if( (*((Node **)arg1))->GetDistance() < (*((Node **)arg2))->GetDistance() ) return -1;
   else if( (*((Node **)arg1))->GetDistance() > (*((Node **)arg2))->GetDistance()) return 1;
   else return 0;
}

void DistanceNode::Sort()
{
	qsort((void*)m_nodes, m_nSize, sizeof(Node*), *CompareNodes);
}

int DistanceNode::Find(int nPlanetID)
{
	for ( int n = 0; n <m_nSize; n++)
	{
		Node* node = m_nodes[n];
		if ( node->GetPlanetID() == nPlanetID)
			return n;
	}	
	
	return -1;
}
Node* DistanceNode::Get(int nId)
{
	if ( nId < 0 || nId >= m_nSize)
		return NULL;
		
	return m_nodes[nId];
}

//*************************************************************************

Fleet::Fleet(int owner,
             int num_ships,
             int source_planet,
             int destination_planet,
             int total_trip_length,
             int turns_remaining) {
  owner_ = owner;
  num_ships_ = num_ships;
  source_planet_ = source_planet;
  destination_planet_ = destination_planet;
  total_trip_length_ = total_trip_length;
  turns_remaining_ = turns_remaining;
}

int Fleet::Owner() const {
  return owner_;
}

int Fleet::NumShips() const {
  return num_ships_;
}

int Fleet::SourcePlanet() const {
  return source_planet_;
}

int Fleet::DestinationPlanet() const {
  return destination_planet_;
}

int Fleet::TotalTripLength() const {
  return total_trip_length_;
}

int Fleet::TurnsRemaining() const {
  return turns_remaining_;
}

//*************************************************************************

Planet::Planet(int planet_id,
               int owner,
               int num_ships,
               int growth_rate,
               double x,
               double y,
               int nTurn /* = 0*/) : planet_id_(planet_id), owner_(owner),
	                 growth_rate_(growth_rate), x_(x), y_(y), turn_(nTurn), 
	                 nMinimumState_(0), nMaxAid_(0), nMaxAidEnemy_(0), 
	                 nTerritoryScore_(0), bPlanetLost_(false)
	                
{
  for ( int i = 0; i< 3; i++)
  {
  	if ( i == owner)
  		num_ships_[i] = num_ships;
  	else
  		num_ships_[i] = 0;
  }  		
  nMinimumState_ = num_ships;
}

Planet::Planet(const Planet &p) : planet_id_(p.planet_id_), owner_(p.owner_),
	                 growth_rate_(p.growth_rate_), x_(p.x_), y_(p.y_), turn_(p.turn_), 
	                 nMinimumState_(p.nMinimumState_), nMaxAid_(p.nMaxAid_), 
	                 nMaxAidEnemy_(p.nMaxAidEnemy_), nTerritoryScore_(p.nTerritoryScore_), 
	                 bPlanetLost_(p.bPlanetLost_)
{
  for ( int i = 0; i< 3; i++)
  {
  	if ( i == owner_)
  		num_ships_[i] = p.NumShips();
  	else
  		num_ships_[i] = 0;
  }  		
}

Planet::~Planet()
{
	
}

int Planet::PlanetID() const {
  return planet_id_;
}

int Planet::Owner() const {
  return owner_;
}

int Planet::NumShips() const {
  return num_ships_[owner_];
}

int Planet::GrowthRate() const {
  return growth_rate_;
}

double Planet::X() const {
  return x_;
}

double Planet::Y() const {
  return y_;
}

void Planet::Owner(int new_owner) {
  owner_ = new_owner;
}

void Planet::NumShips(int new_num_ships) {
  num_ships_[owner_] = new_num_ships;
}

void Planet::AddShips(int amount) {
  num_ships_[owner_] += amount;
  nMinimumState_ += amount;
}

void Planet::RemoveShips(int amount) {
  num_ships_[owner_] -= amount;
  nMinimumState_ -= amount;
}

// Interface for future state of the planet
// return the number of turns in the future that corresponds to the state of the planet
int Planet::GetTurn() const
{
	return turn_;
}
// includes data of fleet as arraiving to the planet (in turn turn_, obviously)
bool Planet::FleetArrive(const Fleet& f)
{
	if ( f.DestinationPlanet() != planet_id_)
		//something weird is happened
		return false;

	num_ships_[f.Owner()] += f.NumShips();		
	return true;
}

static int CompareInt(const void *arg1, const void *arg2)
{
 if(*(int *)arg1 < *(int *)arg2) return 1;
   else if(*(int *)arg1 > *(int *)arg2) return -1;
   else return 0;
}
// does battle resolution and increments turn. Returns new owner
int Planet::BattleResolution()
{
	//Just make the growth of planet's ships
	if ( owner_ > 0)
		num_ships_[owner_] += growth_rate_;	


	int nForces = 0;
	int* arShipsOrdered = new int[3];
	int i = 0;
	for ( i = 0; i < 3; i++)
	{
		arShipsOrdered[i] = num_ships_[i] ;
		if ( num_ships_[i] > 0)
			nForces++;
	}

	
	int nOldOwner = owner_;
	
	switch(nForces)
	{

	case 0:
		//nothing to do
		break;
	case 1:
		//If there is only one force, that is the new occupation of the planet.
		for ( i = 0; i < 3; i++)
		{
			if ( num_ships_[i] > 0)
				owner_ = i;
		}
		break;
	case 2:
	case 3:
		//if there are two forces, the new owner of the planet is the owner of the largest 
		//force, and the losing player’s ship count is subtracted from the winning player’s 
		//ship count as the new population. However, if both forces are the same size, 
		//then the winner is the original owner of the planet, and the planet’s new ship 
		//count is zero.
		//If the original owner of the planet was neutral, then it is possible for there to be 
		//three forces fighting for one planet. In this case, the owner of the largest force is 
		//the new owner of the planet, and his ship count is reduced by the number of ships 
		//in the second largest force. If the top two forces are the same size, the original 
		//owner retains ownership of the planet but the forces are set to zero.
		qsort((void*)arShipsOrdered, 3, sizeof(int), *CompareInt);
		
		if ( arShipsOrdered[0] == arShipsOrdered[1])
		{
			for ( i = 0; i < 3; i++)
				 num_ships_[i] = 0;
		}
		else
		{
			for ( i = 0; i < 3; i++)
				 if ( num_ships_[i] == arShipsOrdered[0])
				 {
				 	owner_ = i;
				 	break;
				 }
				 
			num_ships_[0] = num_ships_[1] = num_ships_[2] = 0;
			num_ships_[owner_] = arShipsOrdered[0] - arShipsOrdered[1];
		}
		break;
	}
	turn_++;
	
	if ( arShipsOrdered)
	{
		delete [] arShipsOrdered;
		arShipsOrdered = NULL;
	}	
	
	//sets minimum state of this planet (turn turn_)
	nMinimumState_ = CalculateMinimumState();
	
	if ( nOldOwner == 1 && owner_ == 2)
		bPlanetLost_ = true;
		
	return owner_;	
}

int Planet::Distance(const Planet&p1)
{
  double dx = X() - p1.X();
  double dy = Y() - p1.Y();
  return (int)ceil(sqrt(dx * dx + dy * dy));
}

int Planet::GetMinimumState() const
{
	return nMinimumState_;
}	
int Planet::CalculateMinimumState()
{
	if ( Owner() != 1)
		return 0;
	else
		return NumShips();
}	
int Planet::GetMaxAid() const
{
	return nMaxAid_;
}	
void Planet::SetMaxAid(int nMaxAid)
{
	nMaxAid_ = nMaxAid;
}

int Planet::GetMaxAidEnemy() const
{
	return nMaxAidEnemy_;
}	
void Planet::SetMaxAidEnemy(int nMaxAid)
{
	nMaxAidEnemy_ = nMaxAid;
}

int Planet::GetLocationScore() const
{
	return nTerritoryScore_;	
}
void Planet::SetLocationScore(int nScore)
{
	nTerritoryScore_ = nScore;
}

bool Planet::IsFrontPlanet() const
{
	return nTerritoryScore_ > 0;
}		
		
int Planet::GetRequiredSupplyShips() const
{
	return (GetMaxAidEnemy() <= NumShips() ) ? 0 : GetMaxAidEnemy() - NumShips();
}
bool Planet::IsStable() const
{
	switch(owner_)
	{
	case 0:
	case 2:
		return GetMaxAidEnemy() + NumShips() <= GetMaxAid()  /*+ GrowthRate()*turn_*/;	
	case 1:
		return GetMaxAidEnemy() <= GetMaxAid() + NumShips() /*+ GrowthRate()*turn_*/;	
	}
}
bool Planet::IsGoingToBeLost() const
{
	return bPlanetLost_;	
}



//*************************************************************************

PlanetWars::PlanetWars() :  m_nMaxDistance(0), nMyGrowth_(0), nEnemyGrowth_(0), nMyGrowthFuture_(0), nEnemyGrowthFuture_(0)
{
}

void PlanetWars::NewTurn(const std::string& game_state)
{
	nTurn++;
	
	ParseGameState(game_state);

	//clears everything
	freeShips_.clear();
	freeShipsAidEnemy_.clear();
  	orders.Delete();
	definitiveOrders_.Delete();
	fps_.clear();
	paths_.clear();
	 
 	//basic calculations, same for every turn in the game
 	if (  m_nMaxDistance == 0)
 	{
		//calculate greatest distance between planets
		int i = 0;
		for(i = 0; i< planets_.size(); i++)
		{
			const Planet& p1 = planets_[i];
			for(int j = i+1; j< planets_.size(); j++)
			{
				const Planet& p2 = planets_[j];
				int nDistance = Distance(p1.PlanetID(), p2.PlanetID());
				if ( nDistance > m_nMaxDistance)
					m_nMaxDistance = nDistance;
			}
		}
		
		//calculate max distances from every planet
		CalculateMaxDistances();
		
		//calculate all distances, dictionary by planetID
		for ( i = 0; i < planets_.size(); i++)
		{
			DistanceNode* dNode = new DistanceNode();
			const Planet& p1 = planets_[i];
			for(int j = 0; j< planets_.size(); j++)
			{
				if ( i == j)
					continue;
					
				const Planet& p2 = planets_[j];
				int nDistance = Distance(p1.PlanetID(), p2.PlanetID());
				Node* node = new Node(p2.PlanetID(), nDistance);
				dNode->Add(node);
			}			
			dNode->Sort();
			planetDistances_[p1.PlanetID()] = dNode;
		}
	}

	//calculates paths from planet A to planet B
	CalculatePaths();
	
	CalculateAvgEnemyDistances();
	
	CalculateMyGrowth();
	CalculateEnemyGrowth();
	
}

PlanetWars::~PlanetWars()
{
	for ( int i = 0; i < planets_.size(); i++)
	{
		const Planet& p1 = planets_[i];
		DistanceNode* dNode = planetDistances_[p1.PlanetID()];
		if ( dNode)
		{
			delete dNode;
			dNode = NULL;
		}
	}	
}

void PlanetWars::CalculatePaths()
{
	for ( int i = 0; i < planets_.size(); i++)
	{
		const Planet & p = planets_[i];
		if (p.Owner() != 1)
			continue;
		paths_[p.PlanetID()] = CalculatePathsFromPlanet(p.PlanetID());
	}
}

std::map<int , int> PlanetWars::CalculatePathsFromPlanet(int nPlanetID)
{
	std::map<int , int> pathI;
	for ( int i = 0; i < planets_.size(); i++)
	{
		const Planet & p = planets_[i];
		if ( p.PlanetID() == nPlanetID)
			continue;
		
		pathI[p.PlanetID()] = CalculatePathFromTo(nPlanetID, p.PlanetID());
	}	
	return pathI;
}

int PlanetWars::CalculatePathFromTo(int source, int dest)
{
	if ( source == dest)
		return source;
	
	int nDest = planetDistances_[source]->Find(dest);
	if ( nDest <= 0)
		return dest;
		
	int nDistance = planetDistances_[source]->Get(nDest)->GetDistance();
	int nDistanceMax = (int)floor(2*nDistance/sqrt(2));
	
	int nNewHop = -1;
	for ( int i = nDest-1; i >=0 && nNewHop<0; i--)
	{
		Node* node = planetDistances_[source]->Get(i);
		const Planet& p = GetPlanet(node->GetPlanetID());
		if ( p.Owner() != 1)
			continue;
			
		int nDistanceSource = node->GetDistance();
		int nDistanceDest = Distance(node->GetPlanetID(), dest);
		
		if ( nDistanceDest >= nDistance)
			continue;
			
		if (nDistanceSource + nDistanceDest >= nDistanceMax)
			continue;
			
		nNewHop = node->GetPlanetID();
	}
	
	if (nNewHop < 0)
		return dest;
	else
		return CalculatePathFromTo(source, nNewHop); 
	
}



//makes all FPS calculation, needed to issue orders
void PlanetWars::CalculateFPS()
{
	//calculate all future planet states (FPS)
	GenerateFPS();
	
	//calculate location scoring for my planets in all turns
	for ( int t = 0; t <= m_nMaxDistance; t++)
	{
		GenerateLocationScoring(t);
		CalculateMaxAids(t);
	}

	//calculate free ships in max turns for each of my planets
	std::vector<Planet> myPlanets = MyPlanets();
	for ( int i =0; i< myPlanets.size(); i++)
	{
		const Planet& p = myPlanets[i];
		//we use the max distance of the own planet, just optimization...
		freeShips_[p.PlanetID()] = CalculateFreeShips(p, maxDistances_[p.PlanetID()]);
		freeShipsAidEnemy_[p.PlanetID()] = CalculateFreeShipsAidEnemy(p, avgEnemyDistances_[p.PlanetID()]);

		std::vector<Planet> pFPS = fps_[p.PlanetID()];
	}
	
	nMyGrowthFuture_ = CalculateMyGrowth(m_nMaxDistance);
	nEnemyGrowthFuture_ = CalculateEnemyGrowth(m_nMaxDistance);
	nMyShipsFuture_ = CalculateMyShips(m_nMaxDistance);
	nEnemyShipsFuture_ = CalculateEnemyShips(m_nMaxDistance);
	
}
void PlanetWars::ReCalculateFPS(Order* order)
{
  	orders.Delete();

	//recalculates minstate using new issued order
	ReGenerateFPS(order);	
	
	//calculate location scoring for my planets in all turns
	for ( int t = 0; t <= m_nMaxDistance; t++)
	{
		GenerateLocationScoring(t);
		CalculateMaxAids(t);
	}

	//recalculates free ships using removed ships of order's source planet
	int nPlanetID = order->GetSourcePlanet();
	const Planet& p = GetPlanet(nPlanetID);
	freeShips_[nPlanetID] = CalculateFreeShips(p, maxDistances_[nPlanetID]);
	freeShipsAidEnemy_[nPlanetID] = CalculateFreeShipsAidEnemy(p, avgEnemyDistances_[nPlanetID]);

	nMyGrowthFuture_ = CalculateMyGrowth(m_nMaxDistance);
	nEnemyGrowthFuture_ = CalculateEnemyGrowth(m_nMaxDistance);
}

int PlanetWars::NumPlanets() const {
  return planets_.size();
}

const Planet& PlanetWars::GetPlanet(int planet_id) const  {
  return planets_[planet_id];
}

int PlanetWars::NumFleets() const {
  return fleets_.size();
}

const Fleet& PlanetWars::GetFleet(int fleet_id) const {
  return fleets_[fleet_id];
}

std::vector<Planet> PlanetWars::Planets() const {
  std::vector<Planet> r;
  for (int i = 0; i < planets_.size(); ++i) {
    const Planet& p = planets_[i];
    r.push_back(p);
  }
  return r;
}

std::vector<Planet> PlanetWars::MyPlanets() const {
  std::vector<Planet> r;
  for (int i = 0; i < planets_.size(); ++i) {
    const Planet& p = planets_[i];
    if (p.Owner() == 1) {
      r.push_back(p);
    }
  }
  return r;
}

std::vector<Planet> PlanetWars::NeutralPlanets() const {
  std::vector<Planet> r;
  for (int i = 0; i < planets_.size(); ++i) {
    const Planet& p = planets_[i];
    if (p.Owner() == 0) {
      r.push_back(p);
    }
  }
  return r;
}

std::vector<Planet> PlanetWars::EnemyPlanets() const {
  std::vector<Planet> r;
  for (int i = 0; i < planets_.size(); ++i) {
    const Planet& p = planets_[i];
    if (p.Owner() > 1) {
      r.push_back(p);
    }
  }
  return r;
}

std::vector<Planet> PlanetWars::NotMyPlanets() const {
  std::vector<Planet> r;
  for (int i = 0; i < planets_.size(); ++i) {
    const Planet& p = planets_[i];
    if (p.Owner() != 1) {
      r.push_back(p);
    }
  }
  return r;
}

std::vector<Fleet> PlanetWars::Fleets() const {
  std::vector<Fleet> r;
  for (int i = 0; i < fleets_.size(); ++i) {
    const Fleet& f = fleets_[i];
    r.push_back(f);
  }
  return r;
}

std::vector<Fleet> PlanetWars::MyFleets() const {
  std::vector<Fleet> r;
  for (int i = 0; i < fleets_.size(); ++i) {
    const Fleet& f = fleets_[i];
    if (f.Owner() == 1) {
      r.push_back(f);
    }
  }
  return r;
}

std::vector<Fleet> PlanetWars::EnemyFleets() const {
  std::vector<Fleet> r;
  for (int i = 0; i < fleets_.size(); ++i) {
    const Fleet& f = fleets_[i];
    if (f.Owner() > 1) {
      r.push_back(f);
    }
  }
  return r;
}

std::string PlanetWars::ToString() const {
  std::stringstream s;
  unsigned int i = 0;
  for (i = 0; i < planets_.size(); ++i) {
    const Planet& p = planets_[i];
    s << "P " << p.X() << " " << p.Y() << " " << p.Owner()
      << " " << p.NumShips() << " " << p.GrowthRate() << std::endl;
  }
  for (i = 0; i < fleets_.size(); ++i) {
    const Fleet& f = fleets_[i];
    s << "F " << f.Owner() << " " << f.NumShips() << " "
      << f.SourcePlanet() << " " << f.DestinationPlanet() << " "
      << f.TotalTripLength() << " " << f.TurnsRemaining() << std::endl;
  }
  return s.str();
}

int PlanetWars::Distance(int source_planet, int destination_planet) const {
  const Planet& source = planets_[source_planet];
  const Planet& destination = planets_[destination_planet];
  double dx = source.X() - destination.X();
  double dy = source.Y() - destination.Y();
  return (int)ceil(sqrt(dx * dx + dy * dy));
}

void PlanetWars::IssueOrder(int source_planet,
                            int destination_planet,
                            int num_ships) const {
  std::cout << source_planet << " "
            << destination_planet << " "
            << num_ships << std::endl;
  std::cout.flush();

#if DEBUGFILE
	char szFileName[256];
	sprintf(szFileName, "%s_%d.log", "Xayide", nTimestamp);
  	std::ofstream  file_op(szFileName, std::fstream::app);
	file_op << source_planet << " "
            << destination_planet << " "
            << num_ships << "\r\n";
	file_op.close();
#endif
}

bool PlanetWars::IsAlive(int player_id) const {
	unsigned int i = 0;
	for (i = 0; i < planets_.size(); ++i) {
    if (planets_[i].Owner() == player_id) {
      return true;
    }
  }
  for (i = 0; i < fleets_.size(); ++i) {
    if (fleets_[i].Owner() == player_id) {
      return true;
    }
  }
  return false;
}

int PlanetWars::NumShips(int player_id) const {
  unsigned int i = 0;
  int num_ships = 0;
  for (i = 0; i < planets_.size(); ++i) {
    if (planets_[i].Owner() == player_id) {
      num_ships += planets_[i].NumShips();
    }
  }
  for (i = 0; i < fleets_.size(); ++i) {
    if (fleets_[i].Owner() == player_id) {
      num_ships += fleets_[i].NumShips();
    }
  }
  return num_ships;
}

int PlanetWars::ParseGameState(const std::string& s) {
  planets_.clear();
  fleets_.clear();
  std::vector<std::string> lines = StringUtil::Tokenize(s, "\n");
  int planet_id = 0;

#if DEBUGFILE
	char szFileName[256];
	sprintf(szFileName, "%s_%d.log", "Xayide", nTimestamp);
  	std::ofstream  file_op(szFileName, std::fstream::app);
	file_op << "\r\nTurn " << nTurn << "\r\n";

#endif

  for (unsigned int i = 0; i < lines.size(); ++i) {
    std::string& line = lines[i];

#if DEBUGFILE
	file_op << (char*)line.c_str() << "\r\n";
#endif

    size_t comment_begin = line.find_first_of('#');
    if (comment_begin != std::string::npos) {
      line = line.substr(0, comment_begin);
    }
    std::vector<std::string> tokens = StringUtil::Tokenize(line);
    if (tokens.size() == 0) {
      continue;
    }
    if (tokens[0] == "P") {
      if (tokens.size() != 6) {
        return 0;
      }
      Planet p(planet_id++,              // The ID of this planet
	       atoi(tokens[3].c_str()),  // Owner
               atoi(tokens[4].c_str()),  // Num ships
               atoi(tokens[5].c_str()),  // Growth rate
               atof(tokens[1].c_str()),  // X
               atof(tokens[2].c_str())); // Y
      planets_.push_back(p);
    } else if (tokens[0] == "F") {
      if (tokens.size() != 7) {
        return 0;
      }
      Fleet f(atoi(tokens[1].c_str()),  // Owner
              atoi(tokens[2].c_str()),  // Num ships
              atoi(tokens[3].c_str()),  // Source
              atoi(tokens[4].c_str()),  // Destination
              atoi(tokens[5].c_str()),  // Total trip length
              atoi(tokens[6].c_str())); // Turns remaining
      fleets_.push_back(f);
    } else {
      return 0;
    }
  }
#if DEBUGFILE
	file_op << "go\r\n\r\n";

  	file_op.close();
#endif
  
  return 1;
}

void PlanetWars::FinishTurn() const {
  std::cout << "go" << std::endl;
  std::cout.flush();
}

void PlanetWars::GenerateFPS()
{
	// each item contains information about a planet, then this vector contains 
	//info about a given planet in turn N
	for ( int i = 0; i < planets_.size(); i++)
	{
		std::vector<Planet> planetI;
		Planet& p = (Planet&) planets_[i];
		planetI.push_back(p);
		for ( int t = 0; t <= m_nMaxDistance; t++)
		{
			Planet p1(planetI.back());
			for ( int j = 0; j< fleets_.size(); j++)
			{
				const Fleet& f = fleets_[j];
				if ( f.DestinationPlanet() != p1.PlanetID())
					continue;
				if ( f.TurnsRemaining() != t+1)
					continue;
				
				p1.FleetArrive(f);	
			}
			p1.BattleResolution();	
		
			//updates maxAid and maxAidEnemy should be done with all FPS structure 
			//p1.SetMaxAid(MyPlanets(), MyFleets());
			//p1.SetMaxAidEnemy(EnemyPlanets(), EnemyFleets());
				
			planetI.push_back(p1);
		}

		fps_[p.PlanetID()] = planetI;	
	}
}


void PlanetWars::ReGenerateFPS(Order* order)
{
	int t = 0;
	//recalculates min state based on new order, destination planet
	//new fleet is already on fleets enumerate
	std::vector<Planet> planetS;
	Planet& p1 = (Planet&) planets_[order->GetSourcePlanet()];
	planetS.push_back(p1);
	for ( t = 0; t <= m_nMaxDistance; t++)
	{
		Planet p0(planetS.back());
		for ( int j = 0; j< fleets_.size(); j++)
		{
			const Fleet& f = fleets_[j];
			if ( f.DestinationPlanet() != p0.PlanetID())
				continue;
			if ( f.TurnsRemaining() != t+1)
				continue;
			
			p0.FleetArrive(f);	
		}
		p0.BattleResolution();	
	
		//updates maxAid and maxAidEnemy should be done with all FPS structure 
		//p0.SetMaxAid(MyPlanets(), MyFleets());
		//p0.SetMaxAidEnemy(EnemyPlanets(), EnemyFleets());
		
		planetS.push_back(p0);
	}

	fps_[p1.PlanetID()] = planetS;		

	
	std::vector<Planet> planetI;
	Planet& p2 = (Planet&) planets_[order->GetDestinationPlanet()];
	planetI.push_back(p2);
	for ( t = 0; t < m_nMaxDistance; t++)
	{
		Planet p0(planetI.back());
		for ( int j = 0; j< fleets_.size(); j++)
		{
			const Fleet& f = fleets_[j];
			if ( f.DestinationPlanet() != p0.PlanetID())
				continue;
			if ( f.TurnsRemaining() != t+1)
				continue;
				
			p0.FleetArrive(f);
		}
		p0.BattleResolution();	
		 
		//updates maxAid and maxAidEnemy should be done with all FPS structure 
		//p0.SetMaxAid(MyPlanets(), MyFleets());
		//p0.SetMaxAidEnemy(EnemyPlanets(), EnemyFleets());
		
		planetI.push_back(p0);
	}
	
	fps_[p2.PlanetID()] = planetI;

}
//calculates minimum state for a planet. That is, the maximum number of ships
//we can send from the planet by means of FPS until given turn 
int PlanetWars::CalculateFreeShips(const Planet& p, int nUntilTurn)
{
	int nMinShips = 999999;
	std::vector<Planet> &planetI = (std::vector<Planet> &)fps_[p.PlanetID()];

	for ( int i = 0; i < planetI.size() && i < nUntilTurn +1; i++)
	{
		const Planet& pt = planetI[i];
		if (nMinShips > pt.GetMinimumState())
			nMinShips = pt.GetMinimumState();
	}
	
	return nMinShips;
}
//calculates minimum state for a planet, including possible Enemy Aid and mine
int PlanetWars::CalculateFreeShipsAidEnemy(const Planet& p, int nUntilTurn)
{
	int nMinShips = 999999;
	std::vector<Planet> &planetI = (std::vector<Planet> &)fps_[p.PlanetID()];

	for ( int i = 0; i < planetI.size() && i < nUntilTurn +1; i++)
	{
		const Planet& pt = planetI[i];
		int nTotalAmountForces = pt.GetMinimumState() - pt.GetMaxAidEnemy() + pt.GetMaxAid();
		if (nTotalAmountForces < 0)
			nTotalAmountForces = 0;
		if (nMinShips > nTotalAmountForces)
			nMinShips = nTotalAmountForces;
	}
	
	return nMinShips;
}

//returns the distance with the farest planet
int PlanetWars::CalculateMaxDistance(const Planet &p)
{
	int nMaxDistance = 0;
	for ( int i = 0; i < planets_.size(); i++)
	{
		const Planet& p1 = planets_[i];
		int nDistance = Distance(p.PlanetID(), p1.PlanetID());
		if ( nDistance > nMaxDistance)
			nMaxDistance = nDistance;
	}	
	return nMaxDistance;
}

int PlanetWars::CalculateMaxDistance(const Planet &p, int nOwner)
{
	int nMaxDistance = 0;
	for ( int i = 0; i < planets_.size(); i++)
	{
		const Planet& p1 = planets_[i];
		if ( p1.Owner() != nOwner)
			continue;
		int nDistance = Distance(p.PlanetID(), p1.PlanetID());
		if ( nDistance > nMaxDistance)
			nMaxDistance = nDistance;
	}	
	return nMaxDistance;		
}


void PlanetWars::CalculateMaxDistances()
{
	for ( int i = 0; i < planets_.size(); i++)
	{
		const Planet& p1 = planets_[i];
		maxDistances_[p1.PlanetID()] = CalculateMaxDistance(p1);	
	}	
}
void PlanetWars::CalculateAvgEnemyDistances()
{
	for ( int i = 0; i < planets_.size(); i++)
	{
		const Planet& p1 = planets_[i];
		avgEnemyDistances_[p1.PlanetID()] = CalculateAvgEnemyDistance(p1);	
	}	
}
int PlanetWars::CalculateAvgEnemyDistance(const Planet &p)
{
	int nAvgDistance = 0;
	for ( int i = 0; i < planets_.size(); i++)
	{
		const Planet& p1 = planets_[i];
		if ( p1.Owner() != 2)
			continue;
		int nDistance = Distance(p.PlanetID(), p1.PlanetID());
		nAvgDistance += nDistance;
	}	
	if ( EnemyPlanets().size() > 0)
		nAvgDistance /=EnemyPlanets().size();
	return nAvgDistance;
}


void PlanetWars::GenerateLocationScoring(int nTurn)
{
	// "front" planets are near enemy ones. Find the nearest planet of mine for each enemy's planet
	for ( int i = 0; i < planets_.size(); i++)
	{
		std::vector<Planet> planet1FPS = fps_[planets_[i].PlanetID()];
		const Planet& p1Future = planet1FPS[nTurn];
		
		if(p1Future.Owner() != 2)
			continue;
			
		int nMinDistance = 999999;
		int nPlanet = -1;
		for ( int j = 0; j < planets_.size(); j++)
		{
			std::vector<Planet> planet2FPS = fps_[planets_[j].PlanetID()];
			const Planet& p2Future = planet2FPS[nTurn];

			if ( p2Future.Owner() != 1)
				continue;
			int nDistance = Distance(p1Future.PlanetID(), p2Future.PlanetID());
			if ( nDistance < nMinDistance)
			{
				nMinDistance = nDistance;
				nPlanet = p2Future.PlanetID();	
			}
		}
		
		if ( nPlanet != -1)
		{
			std::vector<Planet> planetFPS = fps_[nPlanet];
			Planet& p = (Planet&)planetFPS[nTurn];
			p.SetLocationScore(p.GetLocationScore() +1 );
			planetFPS[nTurn] = p;
			fps_[nPlanet] = planetFPS;
		}
	}
}

void PlanetWars::CalculateMaxAids(int nTurn)
{
	if ( nTurn == 0 || nTurn > m_nMaxDistance)
		return;
		
	for ( int i = 0; i < planets_.size(); i++)
	{
		int nPlanet = planets_[i].PlanetID();
		std::vector<Planet> planetiFPS = fps_[nPlanet];
		Planet& pinTurn = (Planet&)planetiFPS[nTurn];
		pinTurn.SetMaxAid(0);
		pinTurn.SetMaxAidEnemy(0);
		
		for ( int k = 0; k < planets_.size(); k++)
		{
			if ( i == k)
				continue;

			const Planet& p = GetPlanet(planets_[k].PlanetID());					
			int nDistance = Distance(p.PlanetID(), pinTurn.PlanetID());
			
			if ( nDistance > nTurn)
				continue;

			std::vector<Planet> planetkFPS = fps_[p.PlanetID()];
			const Planet& pkd = planetkFPS[nTurn - nDistance];
			if ( pkd.Owner() == 1)
				pinTurn.SetMaxAid(pinTurn.GetMaxAid() + pkd.NumShips());
			if ( pkd.Owner() == 2)
				pinTurn.SetMaxAidEnemy(pinTurn.GetMaxAidEnemy() + pkd.NumShips());
		}

		planetiFPS[nTurn] = pinTurn;
		fps_[nPlanet] = planetiFPS;		
	}
}	
		

int PlanetWars::GetFreeShips(int planetID) 
{
	return freeShips_[planetID];
}
int PlanetWars::GetFreeShipsAidEnemy(int planetID) 
{
	return freeShipsAidEnemy_[planetID];
}

int PlanetWars::GetMaxDistance() const
{
	return m_nMaxDistance;
}
int PlanetWars::GetMaxDistance(int planetID) 
{
	return maxDistances_[planetID];
}

bool PlanetWars::IamGoingToLose() const
{
	if ( nMyGrowthFuture_ < nEnemyGrowthFuture_)
		return true;
	else if ( nMyGrowthFuture_ == nEnemyGrowthFuture_ && nMyShipsFuture_ < nEnemyShipsFuture_)
		return true;

	return false;			
}


void PlanetWars::GenerateSupplyMovements(const Planet& p)
{
	int nSource = p.PlanetID();
	int nShips = GetFreeShips(nSource);

	//if ( bOnlyOnePlanet)
	//{
		int nShipsEstimate = GetFreeShipsAidEnemy(nSource);
		if ( nShips > nShipsEstimate)
			nShips = nShipsEstimate;
	//}

	if ( nShips == 0)
		return;

	int i = 0;
	for ( i = 0; i < planets_.size() ; i++)
	{
		const Planet& p1 = planets_[i];
		if ( p1.PlanetID() == nSource)
			continue;
		if ( CheckOrder(nSource, p1.PlanetID()))
			continue;

		//second aproach: look "stable" planets before inestable ones
		int nDistance = Distance(p1.PlanetID(), p.PlanetID());
		std::vector<Planet> planet1FPS = fps_[p1.PlanetID()];

		const Planet& p1FutureAll = planet1FPS[GetMaxDistance(p.PlanetID())];
		if ( p1FutureAll.Owner() != 1)
			continue;
		if ( !p1FutureAll.IsFrontPlanet())
			continue;
		
		const Planet& p1Future = planet1FPS[nDistance];
		double nScore = (double)p1.GrowthRate()*(p1Future.GetLocationScore()+1) / (nDistance* nDistance);
		if ( !p1Future.IsStable())
			nScore /= 10;

		int dest = paths_[nSource][p1.PlanetID()];
		orders.Add(nSource, p1.PlanetID(), dest, nShips, nScore);	
	}
}

int PlanetWars::CalculateMyGrowth()
{
	nMyGrowth_ = 0;
	std::vector<Planet> myPlanets = MyPlanets();
	for ( int i =0; i< myPlanets.size(); i++)
	{
		const Planet& p = myPlanets[i];
		nMyGrowth_ += p.GrowthRate();
	}
	return nMyGrowth_;
}
int PlanetWars::CalculateEnemyGrowth()
{
	nEnemyGrowth_ = 0;
	std::vector<Planet> enemyPlanets = EnemyPlanets();
	for ( int i =0; i< enemyPlanets.size(); i++)
	{
		const Planet& p = enemyPlanets[i];
		nEnemyGrowth_ += p.GrowthRate();
	}
	return nEnemyGrowth_;
}

int PlanetWars::CalculateMyGrowth(int nTurn) 
{
	int nMyGrowthT_ = 0;
	for ( int i =0; i< planets_.size(); i++)
	{
		const std::vector<Planet> &  pFPS = fps_[planets_[i].PlanetID()];
		const Planet& p = pFPS[m_nMaxDistance];
		if ( p.Owner() == 1)
			nMyGrowthT_ += p.GrowthRate();
	}
	return nMyGrowthT_;	
}
int PlanetWars::CalculateEnemyGrowth(int nTurn) 
{
	int nEnemyGrowthT_ = 0;
	for ( int i =0; i< planets_.size(); i++)
	{
		const std::vector<Planet> &  pFPS = fps_[planets_[i].PlanetID()];
		const Planet& p = pFPS[m_nMaxDistance];
		if ( p.Owner() == 2)
			nEnemyGrowthT_ += p.GrowthRate();
	}
	return nEnemyGrowthT_;	
}
int PlanetWars::CalculateMyShips(int nTurn) 
{
	int nMyShipsT_ = 0;
	for ( int i =0; i< planets_.size(); i++)
	{
		const std::vector<Planet> &  pFPS = fps_[planets_[i].PlanetID()];
		const Planet& p = pFPS[m_nMaxDistance];
		if ( p.Owner() == 1)
			nMyShipsT_ += p.NumShips();
	}
	return nMyShipsT_;	
}
int PlanetWars::CalculateEnemyShips(int nTurn) 
{
	int nEnemyShipsT_ = 0;
	for ( int i =0; i< planets_.size(); i++)
	{
		const std::vector<Planet> &  pFPS = fps_[planets_[i].PlanetID()];
		const Planet& p = pFPS[m_nMaxDistance];
		if ( p.Owner() == 2)
			nEnemyShipsT_ += p.NumShips();
	}
	return nEnemyShipsT_;	
}

int PlanetWars::GetMyGrowthFuture()
{
	return nMyGrowthFuture_;	
}
int PlanetWars::GetEnemyGrowthFuture()
{
	return nEnemyGrowthFuture_;
}

void PlanetWars::GenerateAttackMovements(const Planet& p1, bool bParanoicMode)
{
	//free ships taking into account actual fleets
	int nMaxShipsToSend = GetFreeShips(p1.PlanetID());
	int nMyPlanets = MyPlanets().size();
	
	//if ( bOnlyOnePlanet)
	//{
		int nShipsEstimate = GetFreeShipsAidEnemy(p1.PlanetID());
		if ( nMaxShipsToSend > nShipsEstimate)
			nMaxShipsToSend = nShipsEstimate;
	//}
	
	if ( nMaxShipsToSend == 0)
		return;

	for ( int j = 0; j < planets_.size(); j++)
	{
		const Planet& p2 = planets_[j];	
		//don't stop this, maybe we are going to loose this planet in turn T
		//if ( p2.Owner() == 1)
		//	continue;
		if ( p1.PlanetID() == p2.PlanetID())
			continue;
		if ( CheckOrder(p1.PlanetID(), p2.PlanetID()))
			continue;

		//look into the future nDistance turns, depending on distance	
		int nDistance = Distance(p1.PlanetID(), p2.PlanetID());
		const std::vector<Planet> &  pOtherFPS = fps_[p2.PlanetID()];
		const Planet& pFuture = pOtherFPS[nDistance];

		int nProfitTurn = -1; //turn in which we make profit by conquering this planet
		//we use pFuture instead p2. This provides us with an attack which is defensive,
		//because the planet could be mine but in the future will be of the enemy
		const Planet& pAnterior = pOtherFPS[nDistance -1];
		bool bWasMine = false;

		switch(pFuture.Owner())
		{
		case 0:
			//neutral planet, cost = ships on the planet
			if ( pAnterior.NumShips() == pFuture.NumShips() && p2.GrowthRate() > 0 )
				nProfitTurn = (pFuture.NumShips() / p2.GrowthRate())  + 1;
			break;
		case 1:
			//will be mine!!! nothing to do (¿?)
			break;
		case 2:
			//enemy planet
			//important trick: avoid attack planet if in previous turn 
			//is neutral	
			if ( pAnterior.Owner() == 0)
				//avoid attack! we would lose
				nProfitTurn = -1;
			else
				nProfitTurn = 1;

			for ( int k = 0; k< nDistance; k++)
			{
				const Planet& pk = pOtherFPS[k];
				if ( pk.Owner() == 1)
					bWasMine = true;
			}

			break;
		}
		
		int nShips = -1;
		if ( nProfitTurn > 0 )
		{
			int nTurn = (nDistance + nProfitTurn < m_nMaxDistance) ? nDistance + nProfitTurn : m_nMaxDistance;
			bool bDefenseAttackedPlanet = false;
			if (bWasMine)
			{
				//if ( p2.Owner() == 1 && pFuture.Owner() == 2)
			
				//special case, we only need to support our planet!!!
				bDefenseAttackedPlanet = true;
				if (pAnterior.Owner() == 1)
					nShips = pFuture.NumShips();
				else
					nShips = pFuture.NumShips() + 1;
			}
			else
			{
				const Planet& pProfit = pOtherFPS[nTurn];
				// how many ships the enemy can send to this planet before make profit?
				if ( bParanoicMode)
				{
					const Planet& pProfitAnt = pOtherFPS[nTurn-1];
					nShips = (pProfit.GetMaxAidEnemy() > pProfitAnt.GetMaxAid() ? pProfit.GetMaxAidEnemy() - pProfitAnt.GetMaxAid() : 0)
												 + pFuture.NumShips() + 1;
				}
				else
				{
					nShips = pProfit.GetMaxAidEnemy() + pFuture.NumShips() + 1;
				}
			}
			
			//be sure p1 is not in problems by sending these ships
			if ( nShips > 0 && nShips <= nMaxShipsToSend)
			{
				//calculate a score for this move
				double dScore = 0.0;
				if ( bParanoicMode && !bDefenseAttackedPlanet)
					dScore = (double)avgEnemyDistances_[p2.PlanetID()] / (nDistance*nDistance );
				else
					dScore = (double)p2.GrowthRate() / ((nDistance + nProfitTurn)*(nDistance + nProfitTurn));
				
				int dest = p2.PlanetID();
				if (!bDefenseAttackedPlanet)
					int dest = paths_[p1.PlanetID()][p2.PlanetID()];				
				orders.Add(p1.PlanetID(), p2.PlanetID(), dest, nShips, (bDefenseAttackedPlanet ? 5*dScore : dScore));
			}
		}
	}		
}

void PlanetWars::ClearAttackOrders()
{
	m_nAttackOrders = 0;
}
int PlanetWars::GetAttackOrders() const
{
	return m_nAttackOrders;
}
		
void PlanetWars::DefendPossibleLostPlanets(const Planet& p1)
{
	const std::vector<Planet> &  pFPS = fps_[p1.PlanetID()];

	bool bContinueSearch = true;
	for ( int i = 1; i < maxDistances_[p1.PlanetID()] && bContinueSearch; i++)
	{
		const Planet& pFuture = (Planet&)pFPS[i];
		if ( pFuture.IsGoingToBeLost())
		{
			int nShipsToSend = pFuture.NumShips();
			if ( nShipsToSend > 0)
			{
				//just send NumShips()+1 to avoid this
				for ( int j = 0; j < planets_.size(); j++)
				{
					const Planet& p2 = planets_[j];	
					if ( p2.Owner() != 1)
						continue;
					if ( p1.PlanetID() == p2.PlanetID())
						continue;
					if ( CheckOrder(p2.PlanetID(), p1.PlanetID()))
						continue;

					int nDistance = Distance(p1.PlanetID(), p2.PlanetID());
					bool bTarde = false;
					if ( nDistance > i)
					{
						//está demasiado lejos!
						bTarde = true;
						const Planet& pFuture2 = (Planet&)pFPS[nDistance];
						if ( pFuture2.Owner() == 2)
							nShipsToSend = pFuture2.NumShips()+1;
						else
							continue;
					}	
					
					int nShips = GetFreeShips(p2.PlanetID());
					int nShipsEstimate = GetFreeShipsAidEnemy(p2.PlanetID());

					if ( nShips == 0)
						continue;

					if ( nShips > nShipsToSend)
						nShips = nShipsToSend;
	
					int nFactor = (nShipsEstimate < nShips) ? 5 : 10;
					if ( !pFuture.IsStable())
						nFactor /= 2;
					if ( bTarde)
						nFactor /= 5;
					
					double dScore = (double)p1.GrowthRate() / (nDistance*nDistance);
					orders.Add(p2.PlanetID(), p1.PlanetID(), nShips, nFactor*dScore);
				}
				bContinueSearch = false;
			}
		}
	}
	
}

void PlanetWars::ExecuteOrders()
{
	definitiveOrders_.Execute((const PlanetWars&)*this);
}

Order* PlanetWars::GetFirstVirtualOrder()
{
	return orders.GetFirst();
}
void PlanetWars::AddDefinitiveOrder(Order* currentOrder)
{
	definitiveOrders_.Add(currentOrder);
	//create a virtual fleet with this order
	int nDistance = Distance(currentOrder->GetSourcePlanet(), currentOrder->GetDestinationPlanet());
	Fleet f(1, currentOrder->GetShips(), currentOrder->GetSourcePlanet(), currentOrder->GetDestinationPlanet(), nDistance, nDistance);
	fleets_.push_back(f);	
	
	//substract fleet number from source planet
	Planet& p = (Planet&)planets_[currentOrder->GetSourcePlanet()];
	p.RemoveShips(currentOrder->GetShips());
}
bool PlanetWars::CheckOrder(int source, int dest) const
{
	return definitiveOrders_.Check(source, dest);
}

bool PlanetWars::IsFrontPlanet(int nPlanetID, int nTurn)
{
	std::vector<Planet> pFPS = fps_[nPlanetID];
	const Planet& p = pFPS[nTurn];
	return p.IsFrontPlanet();
}
bool PlanetWars::IsMine(int nPlanetID, int nTurn)
{
	std::vector<Planet> pFPS = fps_[nPlanetID];
	const Planet& p = pFPS[nTurn];
	return p.Owner() == 1;	
}


//****************************************************

Order::Order(int source, int dest, int ships, double score) : m_dScore(score), m_nSourcePlanet(source),
					m_nDestinationPlanet(dest), m_nPathDestinationPlanet( dest), m_nShips(ships)			
{		
}
Order::Order(int source, int dest, int path, int ships, double score) : m_dScore(score), m_nSourcePlanet(source),
					m_nDestinationPlanet(dest), m_nPathDestinationPlanet(path), m_nShips(ships)			
{		
}

double Order::GetScore() const
{
	return m_dScore;	
}
void Order::SetScore(double dScore)
{
	m_dScore = dScore;
}
void Order::SetSourcePlanet(int nSourcePlanet)
{
	m_nSourcePlanet = nSourcePlanet;
}
int Order::GetSourcePlanet() const
{
	return m_nSourcePlanet;
}
void Order::SetDestinationPlanet(int nDestinationPlanet)
{
	m_nDestinationPlanet = nDestinationPlanet;
}
int Order::GetDestinationPlanet() const
{
	return m_nDestinationPlanet;
}
void Order::SetPathDestinationPlanet(int nPathDestinationPlanet)
{
	m_nPathDestinationPlanet = nPathDestinationPlanet;
}
int Order::GetPathDestinationPlanet() const
{
	return m_nPathDestinationPlanet;
}
void Order::SetShips(int nShips)
{
	m_nShips = nShips;
}
int Order::GetShips() const
{
	return m_nShips;
}

//****************************************************
Orders::Orders(int size /*= 4096*/) : m_nSize(0)
{
	m_orders = new Order*[size];
}
Orders::~Orders()
{
	if (m_orders)
	{
		for ( int n = 0; n <m_nSize; n++)
		{
			delete m_orders[n];
		}
		delete [] m_orders;
		m_orders = NULL;		
	}
}

void Orders::Delete()
{
	if (m_orders)
	{
		for ( int n = 0; n <m_nSize; n++)
		{
			delete m_orders[n];
			m_orders[n] = NULL;
		}
		m_nSize = 0;
	}	
}
void Orders::Add(int source, int dest, int ships, double score)
{
	Order * order = new Order(source, dest, ships, score);
	Add(order);
}
void Orders::Add(int source, int dest, int nPathDest, int ships, double score)
{
	Order * order = new Order(source, dest, nPathDest, ships, score);
	Add(order);
}
void Orders::Add(Order* order)
{
	m_orders[m_nSize++] = order;	
}

static int CompareOrders(const void *arg1, const void *arg2)
{
 if( (*((Order **)arg1))->GetScore() < (*((Order **)arg2))->GetScore() ) return 1;
   else if( (*((Order **)arg1))->GetScore() > (*((Order **)arg2))->GetScore()) return -1;
   else return 0;
}

void Orders::Sort()
{
	qsort((void*)m_orders, m_nSize, sizeof(Order*), *CompareOrders);
}

void Orders::Execute(const PlanetWars& pw)
{
	Sort();
	for ( int i = 0; i < m_nSize; i++)
	{
		Order* order = m_orders[i];
		if ( order->GetSourcePlanet() == order->GetPathDestinationPlanet())
			continue;
		if ( order->GetShips() < 1)
			continue;
		
		Planet& p = (Planet&)pw.GetPlanet(order->GetSourcePlanet());
		//as we are modifying planet states for FPS calculation with our own orders, we can't test this
		//int nShips = (p.NumShips() < order->GetShips()) ? p.NumShips() : order->GetShips();
			
		if ( order->GetShips() > 0)
			pw.IssueOrder(order->GetSourcePlanet(), order->GetPathDestinationPlanet(), order->GetShips());
	}
}

//returns true if an order from source to dest with higher score exists
bool Orders::Check(int source, int dest) const
{
	for ( int i = 0; i < m_nSize; i++)
	{
		Order* order = m_orders[i];
		if ( order->GetSourcePlanet() == source &&
			order->GetDestinationPlanet() == dest)
			return true;
	}
	
	return false;		
}

Order* Orders::GetFirst()
{
	if ( m_nSize == 0)
		return NULL;
	
	Sort();
	Order* firstOrder = m_orders[0];
	Order * order = new Order(firstOrder->GetSourcePlanet(), firstOrder->GetDestinationPlanet(), firstOrder->GetPathDestinationPlanet(),
						 firstOrder->GetShips(), firstOrder->GetScore());
	return order;						 
}
