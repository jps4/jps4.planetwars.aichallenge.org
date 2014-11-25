// This file contains helper code that does all the boring stuff for you.
// The code in this file takes care of storing lists of planets and fleets, as
// well as communicating with the game engine. You can get along just fine
// without ever looking at this file. However, you are welcome to modify it
// if you want to.
#ifndef PLANET_WARS_H_
#define PLANET_WARS_H_

#include <string>
#include <vector>
#include <map>

#define DEBUGFILE 0


// This is a utility class that parses strings.
class StringUtil {
 public:
  // Tokenizes a string s into tokens. Tokens are delimited by any of the
  // characters in delimiters. Blank tokens are omitted.
  static void Tokenize(const std::string& s,
                       const std::string& delimiters,
                       std::vector<std::string>& tokens);

  // A more convenient way of calling the Tokenize() method.
  static std::vector<std::string> Tokenize(
                       const std::string& s,
                       const std::string& delimiters = std::string(" "));
};

//************************************************************************
//Path Finder

class Node
{
	int m_nPlanetID;
	int m_nDistance;
	
public:
	Node ( int nPlanetID, int nDistance);
	int GetPlanetID() const;
	int GetDistance() const;
};

class DistanceNode
{
	Node ** m_nodes;
	int m_nSize;

public:
	DistanceNode(int size = 256);
	~DistanceNode();
	void Add(int nPlanetID, int nDistance);
	void Add(Node* node);
	void Delete();

	int Find(int nPlanetID);
	Node* Get(int nId);
	
	void Sort();
};

//*************************************************************************
class PlanetWars;
class Order
{
	//puntuacion asociada a la orden
	double m_dScore;
	int m_nSourcePlanet;
	int m_nDestinationPlanet;
	int m_nPathDestinationPlanet;
	int m_nShips;
	
	public:
		
	Order(int source, int dest, int ships, double score);
	Order(int source, int dest, int path, int ships, double score);
	double GetScore() const;
	void SetScore(double dScore);
	void SetSourcePlanet(int nSourcePlanet);
	int GetSourcePlanet() const;
	void SetDestinationPlanet(int nDestinationPlanet);
	int GetDestinationPlanet() const;
	void SetPathDestinationPlanet(int nPathDestinationPlanet);
	int GetPathDestinationPlanet() const;
	void SetShips(int nShips);
	int GetShips() const;
	
};

class Orders
{
	Order ** m_orders;
	int m_nSize;

	void Sort();
	
public:
	Orders(int size = 4096);
	~Orders();
	void Add(int source, int dest, int ships, double score);
	void Add(int source, int dest, int nPathDest, int ships, double score);
	void Add(Order* order);
	void Execute(const PlanetWars& pw);
	//returns true if an order from source to dest with higher score exists
	bool Check(int source, int dest) const;
	void Delete();
	Order* GetFirst();
};



//*************************************************************************
// This class stores details about one fleet. There is one of these classes
// for each fleet that is in flight at any given time.
class Fleet {
 public:
  // Initializes a fleet.
  Fleet(int owner,
        int num_ships,
        int source_planet = -1,
        int destination_planet = -1,
        int total_trip_length = -1,
        int turns_remaining = -1);

  // Returns the playerID of the owner of the fleet. Your player ID is always
  // 1. So if the owner is 1, you own the fleet. If the owner is 2 or some
  // other number, then this fleet belongs to your enemy.
  int Owner() const;

  // Returns the number of ships that comprise this fleet.
  int NumShips() const;

  // Returns the ID of the planet where this fleet originated.
  int SourcePlanet() const;

  // Returns the ID of the planet where this fleet is headed.
  int DestinationPlanet() const;

  // Returns the total distance that is being traveled by this fleet. This
  // is the distance between the source planet and the destination planet,
  // rounded up to the nearest whole number.
  int TotalTripLength() const;

  // Returns the number of turns until this fleet reaches its destination. If
  // this value is 1, then the fleet will hit the destination planet next turn.
  int TurnsRemaining() const;

 private:
  int owner_;
  int num_ships_;
  int source_planet_;
  int destination_planet_;
  int total_trip_length_;
  int turns_remaining_;
};



//*************************************************************************
// Stores information about one planet. There is one instance of this class
// for each planet on the map.
class Planet {
 public:
  // Initializes a planet.
  Planet(int planet_id,
         int owner,
         int num_ships,
         int growth_rate,
         double x,
         double y,
         int nTurn = 0);

  Planet(const Planet &p);
  ~Planet();
  
  // Returns the ID of this planets. Planets are numbered starting at zero.
  int PlanetID() const;

  // Returns the ID of the player that owns this planet. Your playerID is
  // always 1. If the owner is 1, this is your planet. If the owner is 0, then
  // the planet is neutral. If the owner is 2 or some other number, then this
  // planet belongs to the enemy.
  int Owner() const;

  // The number of ships on the planet. This is the "population" of the planet.
  int NumShips() const;

  // Returns the growth rate of the planet. Unless the planet is neutral, the
  // population of the planet grows by this amount each turn. The higher this
  // number is, the faster this planet produces ships.
  int GrowthRate() const;

  // The position of the planet in space.
  double X() const;
  double Y() const;

  // Use the following functions to set the properties of this planet. Note
  // that these functions only affect your program's copy of the game state.
  // You can't steal your opponent's planets just by changing the owner to 1
  // using the Owner(int) function! :-)
  void Owner(int new_owner);
  void NumShips(int new_num_ships);
  void AddShips(int amount);
  void RemoveShips(int amount);


  // Interface for future state of the planet
  // return the number of turns in the future that corresponds to the state of the planet
  int GetTurn() const;
  // includes data of fleet as arraiving to the planet (in turn turn_)
  bool FleetArrive(const Fleet& f);
  // does battle resolution and increments turn
  int BattleResolution();
  
  int GetMinimumState() const;
  int CalculateMinimumState();
  int GetMaxAid() const;
//  void SetMaxAid(std::vector<Planet> planets, std::vector<Fleet> fleets);
  void SetMaxAid(int nMaxAid);
  int GetMaxAidEnemy() const;
  void SetMaxAidEnemy(int nMaxAid);
//  void SetMaxAidEnemy(std::vector<Planet> planets, std::vector<Fleet> fleets);
  int GetLocationScore() const;
  void SetLocationScore(int nScore);
  bool IsFrontPlanet() const;
  //required supplies to not lose the planet
  int GetRequiredSupplyShips() const;

  //true if a T-turn planet can be defended in that turn
  bool IsStable() const;
  //are we going to loose this planet in this turn???
  bool IsGoingToBeLost() const;
  
  
  int Distance(const Planet&p1);
  
 private:
  int planet_id_;
  int owner_;
  int num_ships_[3];
  int growth_rate_;
  double x_, y_;
  
  //indicates the turn in the future of this planet
  int turn_;
  //indicates the free ships of this planet. That is, for turn turn_ , we (or the enemy, if he is the owner)
  //could safely send this ships away without having problems with the possesion of the
  //planet, at least until turn turn_
  int nMinimumState_;
  //maximum number of ships I can send to this planet, in turn turn_
  int nMaxAid_;
  //maximum number of ships enemy can send to this planet, in turn turn_
  int nMaxAidEnemy_;
  //location score. >1 implies "backland", <1 is "frontland".
  int nTerritoryScore_;

  
  //are we going to loose this planet in this turn???
  bool bPlanetLost_ ;
};





//*************************************************************************
class PlanetWars {
 public:
  PlanetWars();
  ~PlanetWars();
  
  // Initializes the game state given a string containing game state data.
  void NewTurn(const std::string& game_state);

  // Returns the number of planets on the map. Planets are numbered starting
  // with 0.
  int NumPlanets() const;

  // Returns the planet with the given planet_id. There are NumPlanets()
  // planets. They are numbered starting at 0.
  const Planet& GetPlanet(int planet_id) const ;

  // Returns the number of fleets.
  int NumFleets() const;

  // Returns the fleet with the given fleet_id. Fleets are numbered starting
  // with 0. There are NumFleets() fleets. fleet_id's are not consistent from
  // one turn to the next.
  const Fleet& GetFleet(int fleet_id) const;

  // Returns a list of all the planets.
  std::vector<Planet> Planets() const;

  // Return a list of all the planets owned by the current player. By
  // convention, the current player is always player number 1.
  std::vector<Planet> MyPlanets() const;

  // Return a list of all neutral planets.
  std::vector<Planet> NeutralPlanets() const;

  // Return a list of all the planets owned by rival players. This excludes
  // planets owned by the current player, as well as neutral planets.
  std::vector<Planet> EnemyPlanets() const;

  // Return a list of all the planets that are not owned by the current
  // player. This includes all enemy planets and neutral planets.
  std::vector<Planet> NotMyPlanets() const;

  // Return a list of all the fleets.
  std::vector<Fleet> Fleets() const;

  // Return a list of all the fleets owned by the current player.
  std::vector<Fleet> MyFleets() const;

  // Return a list of all the fleets owned by enemy players.
  std::vector<Fleet> EnemyFleets() const;

  // Writes a string which represents the current game state. This string
  // conforms to the Point-in-Time format from the project Wiki.
  std::string ToString() const;

  // Returns the distance between two planets, rounded up to the next highest
  // integer. This is the number of discrete time steps it takes to get between
  // the two planets.
  int Distance(int source_planet, int destination_planet) const;

  	//returns distance max from and to every planet in the map
	int GetMaxDistance() const;
	//returns the distance with the farest planet
	int CalculateMaxDistance(const Planet &p);
	//returns the distance with the farest planet of given owner 
	int CalculateMaxDistance(const Planet &p, int nOwner);
 	//returns the average distance with enemy planets
	int CalculateAvgEnemyDistance(const Planet &p);

  // Sends an order to the game engine. The order is to send num_ships ships
  // from source_planet to destination_planet. The order must be valid, or
  // else your bot will get kicked and lose the game. For example, you must own
  // source_planet, and you can't send more ships than you actually have on
  // that planet.
  void IssueOrder(int source_planet,
		  int destination_planet,
		  int num_ships) const;

  // Returns true if the named player owns at least one planet or fleet.
  // Otherwise, the player is deemed to be dead and false is returned.
  bool IsAlive(int player_id) const;

  // Returns the number of ships that the given player has, either located
  // on planets or in flight.
  int NumShips(int player_id) const;

  // Sends a message to the game engine letting it know that you're done
  // issuing orders for now.
  void FinishTurn() const;

	int GetFreeShips(int planetID) ;
	int GetFreeShipsAidEnemy(int planetID) ;
	int GetMaxDistance(int planetID) ;

	// supply movements FROM p
	void GenerateSupplyMovements(const Planet& p);
	//attack movements FROM p
	void GenerateAttackMovements(const Planet& p, bool bParanoicMode);
	// defense movements (to avoid lost planets) TO p
	void DefendPossibleLostPlanets(const Planet& p);
	void ExecuteOrders();

	//makes all FPS calculation, needed to issue orders
	void CalculateFPS();
	void ReCalculateFPS(Order* order);

	Order* GetFirstVirtualOrder();
	void AddDefinitiveOrder(Order* currentOrder);
	bool CheckOrder(int source, int dest) const;
	
	bool IsFrontPlanet(int nPlanetID, int nTurn);
	bool IsMine(int nPlanetID, int nTurn);

  int GetMyGrowthFuture();
  int GetEnemyGrowthFuture();

  bool IamGoingToLose() const;
  void ClearAttackOrders();
  int GetAttackOrders() const;

 private:
  // Parses a game state from a string. On success, returns 1. On failure,
  // returns 0.
  int ParseGameState(const std::string& s);

  //generates de fuyture planet states structure
  void GenerateFPS();
  void ReGenerateFPS(Order* order);
  
  //calculates minimum state for a planet. That is, the maximum number of ships
  //we can send from the planet by means of FPS until given turn 
  int CalculateFreeShips(const Planet& p, int nUntilTurn);
 
  //calculates minimum state for a planet, including possible Enemy Aid and mine
  int CalculateFreeShipsAidEnemy(const Planet& p, int nUntilTurn);

  //fills maxDistances_ map, useful for avoid snipping 
  void CalculateMaxDistances();
  void CalculateAvgEnemyDistances();

  //generates location score on myplanets, for every turn
  void GenerateLocationScoring(int nTurn);

  // calculates MaxAid y Max AidEnemy para el turno nTurn. Actualiza FPS.
  void CalculateMaxAids(int nTurn);

  //calculates growths in current turn 
  int CalculateMyGrowth();
  int CalculateEnemyGrowth();

  //calculates growths in turn T based on FPS
  int CalculateMyGrowth(int nTurn) ;
  int CalculateEnemyGrowth(int nTurn) ;
  int CalculateMyShips(int nTurn) ;
  int CalculateEnemyShips(int nTurn) ;

  
  // Store all the planets and fleets. OMG we wouldn't wanna lose all the
  // planets and fleets, would we!?
  std::vector<Planet> planets_;
  std::vector<Fleet> fleets_;
  
  void CalculatePaths();
  std::map<int , int> CalculatePathsFromPlanet(int nPlanetID);
  int CalculatePathFromTo(int source, int dest);
  
  int m_nMaxDistance;
  //list with all sets of planets, future state in turn n (map by PlanetID)
  std::map<int, std::vector<Planet> > fps_;
  //hash by planetID of minimum state por a planet in MaxDistance turns
  std::map<int, int> freeShips_;
  //hash by PlanetID with maximum distances to each planet
  std::map<int, int> maxDistances_;
  //hash by PlanetID with average distances to enemyplanets
  std::map<int, int> avgEnemyDistances_;
  //hash by planetID of minimum state for a planet in MaxDistance turns, including MaxAidEnemy
  std::map<int, int> freeShipsAidEnemy_;
  
  //hash by planetID of distances with the rest of universe, ordered by distance
  std::map<int, DistanceNode*> planetDistances_;
  
  //hash of hashes of paths to supply ships
  std::map<int, std::map<int, int> > paths_;
  
  int nMyGrowth_;
  int nEnemyGrowth_;
  int nMyGrowthFuture_;
  int nEnemyGrowthFuture_;
  int nMyShipsFuture_;
  int nEnemyShipsFuture_;
  
  Orders orders;
  Orders definitiveOrders_;

  int m_nAttackOrders;
};



#endif
