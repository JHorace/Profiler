/**************************************************************************
filename Profiler.h
author James Sumihiro
Brief Description:
This program is an automatic instrumented profiler that tracks the cycles
a program spends in each function.
**************************************************************************/

#include <vector>
#include <intrin.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#pragma intrinsic(__rdtsc)
#pragma once

/**************************************************************************
 Class:       FunctionNode

 Description: FunctionNode represents a unique node of the function tree. 
              A new node is created every time a function is entered for the
              first time at a unique location on the tree. FunctionNode then  
              keeps track of how often the function is entered, and the total
              cycles spent in that function.
**************************************************************************/
class FunctionNode
{
public:
/**************************************************************************
 Function:    FunctionNode

 Description: The constructor for a function node. Assigns the return
              address of the function as its ID, and the previous node
              (if one exists) as its parent
 Inputs:      id - The return address of the function

              par - A pointer to the node representing the function that 
              called this one.
**************************************************************************/
  FunctionNode(void * id, FunctionNode* par) : ID(id), parent(par) {};

/**************************************************************************
 Function:    GetID

 Description: Get the return address of this function, which acts as a
              unique ID.

 Outputs:     The return address of the function, a void pointer.
**************************************************************************/
  void * GetID();

/**************************************************************************
 Function:    IncrementRecursion

 Description: Increments the recursion counter of this node. This is used
              so the node will be exited appropriately when recursion
              unwinds.
**************************************************************************/
  void IncrementRecursion();

/**************************************************************************
 Function:    DecrementsRecursion

 Description: Decrements the recursion counter of this node. This is used
              so the node will be exited appropriately when recursion
              unwinds.
**************************************************************************/
  void DecrementRecursion();

/**************************************************************************
 Function:    AddChild

 Description: Adds a child node to this node's list of children. This is
              called whenever the function this node represents calls
              another function.

 Inputs:      child - a pointer to the function node representing the called
              function.
**************************************************************************/
  void AddChild(FunctionNode* child);

/**************************************************************************
 Function:    GetParent

 Description: Returns the parent node of this node, representing the
              function that called this one.

 Outputs:     A pointer to the function node representing the function that 
              called this one.
**************************************************************************/
  FunctionNode* GetParent();

/**************************************************************************
 Function:    GetParent

 Description: Returns the parent node of this node, representing the
              function that called this one.

 Outputs:     A pointer to the function node representing the function that
              called this one.
**************************************************************************/
  unsigned GetRecursion();

/**************************************************************************
 Function:    GetChildren

 Description: Returns a vector of function node pointers representing the
              functions that this function called.

 Outputs:     A constant reference to a vector of function node pointers
              representing the functions called by this function.
**************************************************************************/
  const std::vector<FunctionNode*>& GetChildren();

/**************************************************************************
 Function:    SetStartTime

 Description: Sets the time (in cycles) that this function started. This
              will be used to determine the total time this function was
              took to complete.

 Inputs:      time - The current time (in cycles).
**************************************************************************/
  void SetStartTime(unsigned __int64 time);

/**************************************************************************
 Function:    AddTotalTime

 Description: Increments the total time this function has taken

 Inputs:      time = The additional time (in cycles).
**************************************************************************/
  void AddTotalTime(unsigned __int64 time);

/**************************************************************************
 Function:    PrintNode

 Description: Prints this function node's name. Its position in the
              function hierarchy is encoded in tabs.

 Inputs:      stream - an ofstream to print to
              level - the level of this function node in the function tree
**************************************************************************/
  void PrintNode(std::ofstream& stream, int level);

private:
  unsigned recursion = 0;
  unsigned __int64 start_time = 0;
  unsigned __int64 total_time = 0;
  void * ID;
  std::vector<FunctionNode*> children;
  std::vector<FunctionNode*> siblings;
  FunctionNode* parent;
};


/**************************************************************************
 Class:       Profiler

 Description: Profiler holds a reference to the head of the function tree,
			        and contains the functionality to append nodes to the tree
			        upon entering a new function. Profiler is designed as a static
			        singleton that is automatically constructed the first time it
			        is accessed. It is automatically destroyed at the end of 
			        execution by ProfilerDestroyer, another singleton, and prints
			        the function tree prior to destruction.
**************************************************************************/
class Profiler
{
public:
/**************************************************************************
 Function:    GetInstance

 Description: Returns the instance of this singleton. Creates it if it
              doesn't already exist.

 Output:      A pointer to the instance of this singleton
**************************************************************************/
  static Profiler* GetInstance();

/**************************************************************************
 Function:    Enter

 Description: Called upon entrance to a function in the function's prolog.
              Creates a new function node if one doesn't already exist.
              Increments the recursion counter if the function entered is
              the same as the previous function. Otherwise appends the new
              function node to the current function node's list of child
              functions, then sets the current function node to that new
              function node.

 Input:       time - the current time in cycles
              signature - the function signature, used as an ID                    
**************************************************************************/
  void Enter(unsigned __int64 time, void * signature);
/**************************************************************************
 Function:    Leave

 Description: Called upon exit from a function in the function's epilog.
              Records the time the function node was exited so total time
              can be determined. Sets the current function node pointer to
              the function's parent.

 Input:       time - the current time in cycles
**************************************************************************/
  void Leave(unsigned __int64 time);

/**************************************************************************
 Function:    Profile

 Description: Prints the entire function tree, with each node represented
              by an unmangled function name, as well as the time spent in
              each function.
**************************************************************************/
  void Profile();
  static Profiler* instance;
  static ProfilerDestroyer destroyer;
protected:
/**************************************************************************
 Function:    Profiler

 Description: Profiler default constructer. Protected, because no one
              be able to construct a profiler except itself.
**************************************************************************/
  Profiler();

/**************************************************************************
 Function:    ~Profiler

 Description: Profiler destroyer. Has no side effects. Delete should only
              be called by ProfilerDestroyer so that ordered execution of
              printing is maintained, and so that ownership of the Profiler
              singleton is clear.
**************************************************************************/
  ~Profiler();
  friend class ProfilerDestroyer;
private:
/**************************************************************************
 Function:    Profiler

 Description: Profiler copy constructor. Private, because this should never
              be used.
**************************************************************************/
  Profiler(Profiler const&) {};

/**************************************************************************
 Function:    operator=

 Description: Profiler assignment operator. Private, because it should
              never be used.
**************************************************************************/
  Profiler& operator=(Profiler const&) {};
  FunctionNode* current = nullptr;
  std::ofstream file;
};

/**************************************************************************
 Class:       ProfilerDestroyer

 Description: A friend class of Profiler. ProfilerDestroyer exists to
              maintain responsibility for the Profiler singleton. This
              allows safe destruction of the Profiler singleton, by ensuring
              its destructor can only be called by ProfilerDestroyer.
              ProfilerDestroyer is itself destroyed at the end of program
              execution, at which point it destroys the Profiler.
**************************************************************************/
class ProfilerDestroyer
{
public:
/**************************************************************************
 Function:    ProfilerDestroyer

 Description: Constructs a ProfilerDestroyer.
**************************************************************************/
  ProfilerDestroyer(Profiler * singleton = nullptr) : profiler(singleton) {};

/**************************************************************************
 Function:    ~ProfilerDestroyer

 Description: Destroys a ProfilerDestroyer, which then destroys the attached
              Profiler, resulting in the output of the function tree.
**************************************************************************/
  ~ProfilerDestroyer();

/**************************************************************************
 Function:    SetSingleton

 Description: Sets the singleton Profiler instance this profiler is
              responsible for. Called upon singleton creation.

 Inputs:      singleton - a pointer to the singleton Profiler
**************************************************************************/
  void SetSingleton(Profiler* singleton) { profiler = singleton; };
private:
  Profiler* profiler = nullptr;
};

/**************************************************************************
 Function:    _penter

 Description: A custom function prolog that replaces the default generated
              function prologs when the /Gh compiler flag is set. This
              prolog calls the profiler's Enter function, resulting in the
              calling function's address and the time the function was
              entered being logged as a function node.
**************************************************************************/
extern "C" void _penter(void);

/**************************************************************************
 Function:    _pexit

 Description: A custom function epilog that replaces the default generated
              function epilogs when the /GH compiler flag is set. This
              epilog calls the profiler's Exit function, which logs the
              time this function was exited in the function node that
              represents the calling function.
**************************************************************************/
extern "C" void _pexit(void);
