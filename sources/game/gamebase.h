//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of base (full explicit representation) games
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef GAMEBASE_H
#define GAMEBASE_H

#include "nfgcont.h"
#include "mixed.h"

//
// Forward declarations
//
class gbtGame;
class gbtGameActionBase;
class gbtGameInfosetBase;
class gbtGameNodeBase;
class gbtGamePlayerBase;
class gbtGameBase;

class gbtGameOutcomeBase : public gbtGameOutcomeRep {
public:
  int m_id;
  gbtGameBase *m_efg;
  gbtText m_label;
  gbtBlock<gbtNumber> m_payoffs;

  gbtGameOutcomeBase(gbtGameBase *, int);

  int GetId(void) const { return m_id; }
  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }

  gbtArray<gbtNumber> GetPayoff(void) const { return m_payoffs; }
  gbtNumber GetPayoff(const gbtGamePlayer &p_player) const
  { return m_payoffs[p_player->GetId()]; }
  void SetPayoff(const gbtGamePlayer &p_player, const gbtNumber &p_value)
  { m_payoffs[p_player->GetId()] = p_value; } 

  void DeleteOutcome(void);
};

class gbtGameStrategyBase : public gbtGameStrategyRep {
public:
  int m_id;
  gbtText m_label;
  gbtGamePlayerBase *m_player;
  gbtArray<int> m_actions;
  long m_index;

  gbtGameStrategyBase(int p_id, gbtGamePlayerBase *p_player,
		      const gbtArray<int> &p_actions)
    : m_id(p_id), m_player(p_player), m_actions(p_actions) { }
  virtual ~gbtGameStrategyBase() { }

  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  int GetId(void) const { return m_id; }

  gbtGamePlayer GetPlayer(void) const;
  gbtGameAction GetAction(const gbtGameInfoset &p_infoset) const;

  const gbtArray<int> &GetBehavior(void) const { return m_actions; }
  long GetIndex(void) const;
};

class gbtGamePlayerBase : public gbtGamePlayerRep {
public:
  int m_id;
  gbtGameBase *m_efg;
  gbtText m_label;
  gbtBlock<gbtGameInfosetBase *> m_infosets;
  gbtBlock<gbtGameStrategyBase *> m_strategies;

  gbtGamePlayerBase(gbtGameBase *, int);
  virtual ~gbtGamePlayerBase();

  gbtGame GetGame(void) const;
  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  int GetId(void) const { return m_id; }

  bool IsChance(void) const { return (m_id == 0); }

  int NumInfosets(void) const { return m_infosets.Length(); }
  gbtGameInfoset NewInfoset(int p_actions);
  gbtGameInfoset GetInfoset(int p_index) const;

  int NumStrategies(void) const { return m_strategies.Length(); }
  gbtGameStrategy GetStrategy(int p_index) const 
  { return m_strategies[p_index]; }
};

class gbtGameActionBase : public gbtGameActionRep {
public:
  int m_id;
  gbtGameInfosetBase *m_infoset;
  bool m_deleted;
  gbtText m_label;

  gbtGameActionBase(gbtGameInfosetBase *, int);
  virtual ~gbtGameActionBase() { } 

  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  int GetId(void) const { return m_id; }
  
  gbtGameInfoset GetInfoset(void) const;
  gbtGamePlayer GetPlayer(void) const;

  gbtNumber GetChanceProb(void) const;
  bool Precedes(gbtGameNode) const;
  
  void DeleteAction(void);
};

class gbtGameInfosetBase : public gbtGameInfosetRep {
public:
  int m_id;
  gbtGamePlayerBase *m_player;
  bool m_deleted;
  gbtText m_label;
  int m_refCount;
  gbtBlock<gbtGameActionBase *> m_actions;
  gbtBlock<gbtNumber> m_chanceProbs;
  gbtBlock<gbtGameNodeBase *> m_members;
  int m_flag, m_whichbranch;

  gbtGameInfosetBase(gbtGamePlayerBase *, int id, int br);
  virtual ~gbtGameInfosetBase();

  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  int GetId(void) const { return m_id; }
  gbtGame GetGame(void) const;

  void DeleteInfoset(void);

  bool IsChanceInfoset(void) const;

  gbtGamePlayer GetPlayer(void) const;
  void SetPlayer(gbtGamePlayer);
  
  void SetChanceProb(int act, const gbtNumber &value); 
  gbtNumber GetChanceProb(int act) const;

  gbtGameAction InsertAction(int where);

  gbtGameAction GetAction(int act) const;
  int NumActions(void) const;

  gbtGameNode GetMember(int m) const;
  int NumMembers(void) const;

  bool Precedes(gbtGameNode) const;

  void MergeInfoset(gbtGameInfoset from);
  void Reveal(gbtGamePlayer);

  bool GetFlag(void) const;
  void SetFlag(bool);

  int GetWhichBranch(void) const;
  void SetWhichBranch(int);

  void PrintActions(gbtOutput &) const;
};

class gbtGameNodeBase : public gbtGameNodeRep {
public:
  int m_id;
  gbtGameBase *m_efg;
  bool m_deleted;
  gbtText m_label;
  int m_refCount;

  bool m_mark;
  gbtGameInfosetBase *m_infoset;
  gbtGameNodeBase *m_parent;
  gbtGameOutcomeBase *m_outcome;
  gbtBlock<gbtGameNodeBase *> m_children;
  mutable gbtGameNodeBase *m_whichbranch, *m_ptr, *m_gameroot;

  gbtGameNodeBase(gbtGameBase *, gbtGameNodeBase *);
  virtual ~gbtGameNodeBase();

  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  int GetId(void) const { return m_id; }
  gbtGame GetGame(void) const;

  int NumChildren(void) const;
  gbtGameNode GetChild(int i) const;
  gbtGameNode GetChild(const gbtGameAction &) const; 
  bool IsPredecessorOf(const gbtGameNode &) const;

  gbtGameNode GetParent(void) const;
  gbtGameAction GetPriorAction(void) const; // returns null if root node

  gbtGameInfoset GetInfoset(void) const;
  int GetMemberId(void) const;

  gbtGamePlayer GetPlayer(void) const;

  gbtGameNode GetSubgameRoot(void) const;
  bool IsSubgameRoot(void) const;

  gbtGameOutcome GetOutcome(void) const;
  void SetOutcome(const gbtGameOutcome &);

  gbtGameNode InsertMove(gbtGameInfoset);
  // Note: Starting in 0.97.1.1, this now deletes the *parent* move
  void DeleteMove(void);
  void DeleteTree(void);

  void JoinInfoset(gbtGameInfoset);
  gbtGameInfoset LeaveInfoset(void);

  void DeleteOutcome(gbtGameOutcomeBase *outc);

  gbtGameNode GetPriorSibling(void) const;
  gbtGameNode GetNextSibling(void) const;

  gbtGameNode GetPriorMember(void) const;
  gbtGameNode GetNextMember(void) const;

  void MarkSubtree(bool p_mark);
};

class gbtGameBase : public gbtGameRep {
public:
  bool sortisets;
  mutable long m_revision;
  mutable long m_outcomeRevision;
  gbtText m_label, comment;
  gbtBlock<gbtGamePlayerBase *> players;
  gbtBlock<gbtGameOutcomeBase *> outcomes;
  gbtBlock<gbtGameOutcomeBase *> m_results;
  gbtGameNodeBase *root;
  gbtGamePlayerBase *chance;

  // Create a trivial tree representation game
  gbtGameBase(void);
  // Create a matrix game
  gbtGameBase(const gbtArray<int> &);
  ~gbtGameBase();

  // this is for use with the copy constructor
  void CopySubtree(gbtGameBase *,
		   gbtGameNodeBase *, gbtGameNodeBase *);

  void CopySubtree(gbtGameNodeBase *, gbtGameNodeBase *,
		   gbtGameNodeBase *);

  gbtGameOutcome NewOutcome(int index);

  void WriteEfg(gbtOutput &, gbtGameNodeBase *) const;

  void Payoff(gbtGameNodeBase *n, gbtNumber,
	      const gbtPVector<int> &, gbtVector<gbtNumber> &) const;
  void Payoff(gbtGameNodeBase *n, gbtNumber,
	      const gbtArray<gbtArray<int> > &, gbtArray<gbtNumber> &) const;
  
  void InfosetProbs(gbtGameNodeBase *n, gbtNumber,
		    const gbtPVector<int> &, gbtPVector<gbtNumber> &) const;
    

  void IndexStrategies(void);
  void SortInfosets(void);
  void NumberNodes(gbtGameNodeBase *, int &);
  void ComputeReducedStrategies(void);

  void InsertMove(gbtGameNodeBase *, gbtGameInfosetBase *);
  void DeleteMove(gbtGameNodeBase *);
  void DeleteTree(gbtGameNodeBase *);

  gbtGameInfosetBase *NewInfoset(gbtGamePlayerBase *,
				  int p_id, int p_actions);
  void DeleteInfoset(gbtGameInfosetBase *);
  void JoinInfoset(gbtGameInfosetBase *, gbtGameNodeBase *); 
  gbtGameInfosetBase *LeaveInfoset(gbtGameNodeBase *);
  void MergeInfoset(gbtGameInfosetBase *, gbtGameInfosetBase *);
  void Reveal(gbtGameInfosetBase *, gbtGamePlayerBase *);
  void SetPlayer(gbtGameInfosetBase *, gbtGamePlayerBase *);

  void DeleteAction(gbtGameInfosetBase *, gbtGameActionBase *);

  void DeleteOutcome(gbtGameOutcomeBase *p_outcome);

  // Utility routines for subgames
  void MarkTree(const gbtGameNodeBase *, const gbtGameNodeBase *);
  bool CheckTree(const gbtGameNodeBase *, const gbtGameNodeBase *);
  void MarkSubgame(gbtGameNodeBase *, gbtGameNodeBase *);

  // Formerly the copy constructor
  gbtGame Copy(gbtGameNode = gbtGameNode(0)) const;
  
  // TITLE ACCESS AND MANIPULATION
  void SetLabel(const gbtText &s);
  gbtText GetLabel(void) const;
  
  void SetComment(const gbtText &);
  gbtText GetComment(void) const;

  // WRITING DATA FILES
  void WriteEfg(gbtOutput &p_file) const;
  void WriteNfg(gbtOutput &p_file) const;

  // DATA ACCESS -- GENERAL INFORMATION
  bool IsTree(void) const { return (root != 0); }
  bool IsMatrix(void) const { return (root == 0); }

  bool IsConstSum(void) const; 
  bool IsPerfectRecall(void) const;
  bool IsPerfectRecall(gbtGameInfoset &, gbtGameInfoset &) const;
  long RevisionNumber(void) const;
  gbtNumber MinPayoff(int pl) const;
  gbtNumber MaxPayoff(int pl) const;
 
  // DATA ACCESS -- NODES
  int NumNodes(void) const;
  gbtGameNode GetRoot(void) const;
  
  // DATA ACCESS -- PLAYERS
  int NumPlayers(void) const;
  gbtGamePlayer GetChance(void) const;
  gbtGamePlayer NewPlayer(void);
  gbtGamePlayer GetPlayer(int index) const;

  // DATA ACCESS -- OUTCOMES
  int NumOutcomes(void) const;
  gbtGameOutcome GetOutcome(int p_id) const;
  gbtGameOutcome NewOutcome(void);

  // DATA ACCESS -- SUPPORTS
  gbtEfgSupport NewEfgSupport(void) const;
  gbtNfgSupport NewNfgSupport(void) const;

  gbtNfgContingency NewContingency(void) const;

  // DATA ACCESS -- PROFILES
  gbtMixedProfile<double> NewMixedProfile(double) const;
  gbtMixedProfile<gbtRational> NewMixedProfile(const gbtRational &) const;
  gbtMixedProfile<gbtNumber> NewMixedProfile(const gbtNumber &) const;

  // EDITING OPERATIONS
  void DeleteEmptyInfosets(void);

  gbtGameNode CopyTree(gbtGameNode src, gbtGameNode dest);
  gbtGameNode MoveTree(gbtGameNode src, gbtGameNode dest);

  gbtGameAction InsertAction(gbtGameInfoset);
  gbtGameAction InsertAction(gbtGameInfoset, const gbtGameAction &at);

  void SetChanceProb(gbtGameInfoset, int, const gbtNumber &);

  void MarkSubgames(void);
  bool MarkSubgame(gbtGameNode);
  void UnmarkSubgame(gbtGameNode);
  void UnmarkSubgames(gbtGameNode);

  int BehavProfileLength(void) const;
  int MixedProfileLength(void) const;
  int TotalNumInfosets(void) const;

  gbtArray<int> NumInfosets(void) const;  // Does not include chance infosets
  int NumPlayerInfosets(void) const;
  gbtPVector<int> NumActions(void) const;
  int NumPlayerActions(void) const;
  gbtPVector<int> NumMembers(void) const;
  gbtArray<int> NumStrategies(void) const;
  int NumStrats(int) const;
  
  // COMPUTING VALUES OF PROFILES
  void Payoff(const gbtPVector<int> &profile,
		      gbtVector<gbtNumber> &payoff) const;
  void Payoff(const gbtArray<gbtArray<int> > &profile,
		      gbtArray<gbtNumber> &payoff) const;

  void InfosetProbs(const gbtPVector<int> &profile,
			    gbtPVector<gbtNumber> &prob) const;
};

class gbtNfgContingencyTree : public gbtNfgContingencyRep {
private:
  gbtGameBase *m_nfg;
  gbtArray<gbtGameStrategy> m_profile;
  
public:
  gbtNfgContingencyTree(gbtGameBase *);
  virtual ~gbtNfgContingencyTree() { }
  
  virtual gbtNfgContingencyRep *Copy(void) const;

  gbtGameStrategy GetStrategy(const gbtGamePlayer &p_player) const
    { return m_profile[p_player->GetId()]; }
  void SetStrategy(gbtGameStrategy);

  void SetOutcome(const gbtGameOutcome &) const;
  gbtGameOutcome GetOutcome(void) const;

  gbtNumber GetPayoff(const gbtGamePlayer &) const;
};

class gbtNfgContingencyTable : public gbtNfgContingencyRep {
private:
  gbtGameBase *m_nfg; 
  long m_index;
  gbtArray<gbtGameStrategy> m_profile;
  
public:
  gbtNfgContingencyTable(gbtGameBase *);
  virtual ~gbtNfgContingencyTable() { }
  
  virtual gbtNfgContingencyRep *Copy(void) const;

  gbtGameStrategy GetStrategy(const gbtGamePlayer &p_player) const
    { return m_profile[p_player->GetId()]; }
  void SetStrategy(gbtGameStrategy);

  void SetOutcome(const gbtGameOutcome &) const;
  gbtGameOutcome GetOutcome(void) const;

  gbtNumber GetPayoff(const gbtGamePlayer &) const;
};


class gbtNfgSupportBase : public gbtNfgSupportRep {
public:
  gbtGameBase *m_nfg;
  // This really could be a gbtPVector<bool> probably, but we'll keep
  // it this way for now to placate possibly older compilers.
  gbtPVector<int> m_strategies;
  gbtText m_label;
  
  bool Undominated(gbtNfgSupportBase &newS, int pl, bool strong,
		   gbtOutput &tracefile, gbtStatus &status) const;

  // LIFECYCLE
  gbtNfgSupportBase(gbtGameBase *);
  ~gbtNfgSupportBase() { }

  // OPERATORS
  // This acts as the assignment operator and copy constructor
  gbtNfgSupportRep *Copy(void) const;
  
  bool operator==(const gbtNfgSupportRep &) const;
  bool operator!=(const gbtNfgSupportRep &p_support) const
  { return !(*this == p_support); }

  // DATA ACCESS: GENERAL
  gbtGame GetGame(void) const { return m_nfg; }

  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  
  // DATA ACCESS: STRATEGIES
  int NumStrats(int pl) const;
  int NumStrats(const gbtGamePlayer &p_player) const 
    { return NumStrats(p_player->GetId()); }
  gbtArray<int> NumStrategies(void) const;
  int MixedProfileLength(void) const;

  int GetIndex(gbtGameStrategy) const;
  bool Contains(gbtGameStrategy) const;

  // DATA ACCESS -- PROFILES
  gbtMixedProfile<double> NewMixedProfile(double) const;
  gbtMixedProfile<gbtRational> NewMixedProfile(const gbtRational &) const;
  gbtMixedProfile<gbtNumber> NewMixedProfile(const gbtNumber &) const;

  // MANIPULATION
  void AddStrategy(gbtGameStrategy);
  void RemoveStrategy(gbtGameStrategy);
  
  // DOMINANCE AND ELIMINATION OF STRATEGIES
  bool Dominates(gbtGameStrategy, gbtGameStrategy, bool strong) const;
  bool IsDominated(gbtGameStrategy, bool strong) const; 

  gbtNfgSupport Undominated(bool strong, const gbtArray<int> &players,
			    gbtOutput &tracefile, gbtStatus &status) const;
  gbtNfgSupport MixedUndominated(bool strong, gbtPrecision precision,
				 const gbtArray<int> &players,
				 gbtOutput &, gbtStatus &status) const;

  // OUTPUT
  void Output(gbtOutput &) const;

  // IMPLEMENTATION OF gbtConstGameRep INTERFACE
  bool IsTree(void) const { return m_nfg->IsTree(); }
  bool IsMatrix(void) const { return m_nfg->IsMatrix(); }
  
  int NumPlayers(void) const { return m_nfg->NumPlayers(); }
  gbtGamePlayer GetPlayer(int index) const; 
  
  int NumOutcomes(void) const { return m_nfg->NumOutcomes(); }
  gbtGameOutcome GetOutcome(int index) const 
  { return m_nfg->GetOutcome(index); }

  bool IsConstSum(void) const { return m_nfg->IsConstSum(); }
  gbtNumber MaxPayoff(int pl = 0) const { return m_nfg->MaxPayoff(pl); }
  gbtNumber MinPayoff(int pl = 0) const { return m_nfg->MinPayoff(pl); }

  // The following are just echoed from the base game.  In the future,
  // derivation from gbtGame will handle these.
  gbtText GetComment(void) const { return m_nfg->GetComment(); }
  void SetComment(const gbtText &p_comment) { m_nfg->SetComment(p_comment); }

  gbtNfgSupport NewNfgSupport(void) const { return Copy(); }
  gbtNfgContingency NewContingency(void) const;

};

template <class T>
class gbtMixedProfileTable : public gbtMixedProfileRep<T> {
private:
  gbtPVector<T> m_profile;
  gbtNfgSupportBase *m_support;

  // Private Payoff functions

  T PPayoff(int pl, int index, int i) const;
  void PPayoff(int pl, int const_pl, int const_st, int cur_pl, long index,
	       T prob, T&value) const;
  void PPayoff(int pl, int const_pl1, int const_st1, int const_pl2, 
	       int const_st2, int cur_pl, long index, T prob, T &value) const;
  void PPayoff(int pl, int const_pl, int cur_pl, long index, T prob,
	       gbtVector<T> &value) const;
  T Payoff(const gbtGameOutcome &o, int pl) const;

public:
  gbtMixedProfileTable(const gbtNfgSupportBase &p_support);
  gbtMixedProfileTable(const gbtMixedProfileTable<T> &);
  gbtMixedProfileTable(const gbtBehavProfile<T> &);
  virtual ~gbtMixedProfileTable() { delete m_support; }

  gbtMixedProfileTable<T> *Copy(void) const;
  
  gbtGame GetGame(void) const  { return m_support->GetGame(); }
  
  T Payoff(int pl) const;
  T Payoff(int pl, gbtGameStrategy) const;
  T Payoff(int pl, int player1, int strat1, int player2, int strat2) const;
  void Payoff(int pl, int const_pl, gbtVector<T> &payoff) const;

  bool operator==(const gbtMixedProfileRep<T> &) const;

  gbtNfgSupport GetSupport(void) const   { return m_support; }

  const T &operator()(int pl, int st) const { return m_profile(pl, st); } 
  T &operator()(int pl, int st) { return m_profile(pl, st); }

  const T &operator[](int i) const { return m_profile[i]; }
  T &operator[](int i) { return m_profile[i]; }

  gbtVector<T> GetRow(int row) const { return m_profile.GetRow(row); }
  void SetRow(int row, const gbtVector<T> &p_vector) 
  { m_profile.SetRow(row, p_vector); }
  const gbtArray<int> &Lengths(void) const { return m_profile.Lengths(); }

  void CopyRow(int row, const gbtPVector<T> &p_vector) 
  { m_profile.CopyRow(row, p_vector); }
  void CopyRow(int row, const gbtMixedProfile<T> &p_profile) 
  { m_profile.CopyRow(row, dynamic_cast<gbtMixedProfileTable<T> *>(p_profile.Get())->m_profile); }

  // IMPLEMENTATION OF gbtGameObject INTERFACE
  gbtText GetLabel(void) const { return ""; }
  void SetLabel(const gbtText &) { }

  gbtText GetComment(void) const { return ""; }

  // IMPLEMENTATION OF gbtConstGameRep INTERFACE
  bool IsTree(void) const { return m_support->IsTree(); }
  bool IsMatrix(void) const { return m_support->IsMatrix(); }
  
  int NumPlayers(void) const { return m_support->NumPlayers(); }
  gbtGamePlayer GetPlayer(int index) const { return m_support->GetPlayer(index); }
  
  int NumOutcomes(void) const { return m_support->NumOutcomes(); }
  gbtGameOutcome GetOutcome(int index) const 
  { return m_support->GetOutcome(index); }

  bool IsConstSum(void) const { return m_support->IsConstSum(); }
  gbtNumber MaxPayoff(int pl = 0) const { return m_support->MaxPayoff(pl); }
  gbtNumber MinPayoff(int pl = 0) const { return m_support->MinPayoff(pl); }

  gbtMixedProfile<double> NewMixedProfile(double) const;
  gbtMixedProfile<gbtRational> NewMixedProfile(const gbtRational &) const;
  gbtMixedProfile<gbtNumber> NewMixedProfile(const gbtNumber &) const;

  // IMPLEMENTATION OF gbtConstNfgRep INTERFACE
  virtual gbtArray<int> NumStrategies(void) const 
  { return m_support->NumStrategies(); }
  virtual int MixedProfileLength(void) const
  { return m_support->MixedProfileLength(); }

  gbtNfgContingency NewContingency(void) const;

  gbtNfgSupport NewNfgSupport(void) const { return m_support->NewNfgSupport(); }
};

template <class T>
class gbtMixedProfileTree : public gbtMixedProfileRep<T> {
private:
  gbtPVector<T> m_profile;
  gbtNfgSupport m_support;

public:
  gbtMixedProfileTree(const gbtNfgSupport &);
  gbtMixedProfileTree(const gbtMixedProfileTree<T> &);
  gbtMixedProfileTree(const gbtBehavProfile<T> &);
  virtual ~gbtMixedProfileTree() { }

  gbtMixedProfileTree<T> *Copy(void) const;
  
  gbtGame GetGame(void) const  { return m_support->GetGame(); }
  
  T Payoff(int pl) const;
  T Payoff(int pl, gbtGameStrategy) const;
  T Payoff(int pl, int player1, int strat1, int player2, int strat2) const;
  void Payoff(int pl, int const_pl, gbtVector<T> &payoff) const;

  bool operator==(const gbtMixedProfileRep<T> &) const;

  gbtNfgSupport GetSupport(void) const   { return m_support; }

  const T &operator()(int pl, int st) const { return m_profile(pl, st); } 
  T &operator()(int pl, int st) { return m_profile(pl, st); }

  const T &operator[](int i) const { return m_profile[i]; }
  T &operator[](int i) { return m_profile[i]; }

  gbtVector<T> GetRow(int row) const { return m_profile.GetRow(row); }
  void SetRow(int row, const gbtVector<T> &p_vector) 
  { m_profile.SetRow(row, p_vector); }
  const gbtArray<int> &Lengths(void) const { return m_profile.Lengths(); }

  void CopyRow(int row, const gbtPVector<T> &p_vector) 
  { m_profile.CopyRow(row, p_vector); }
  void CopyRow(int row, const gbtMixedProfile<T> &p_profile) 
  { m_profile.CopyRow(row, dynamic_cast<gbtMixedProfileTree<T> *>(p_profile.Get())->m_profile); }

  // IMPLEMENTATION OF gbtGameObject INTERFACE
  gbtText GetLabel(void) const { return ""; }
  void SetLabel(const gbtText &) { }

  gbtText GetComment(void) const { return ""; }

  // IMPLEMENTATION OF gbtConstGameRep INTERFACE
  bool IsTree(void) const { return m_support->IsTree(); }
  bool IsMatrix(void) const { return m_support->IsMatrix(); }
  
  int NumPlayers(void) const { return m_support->NumPlayers(); }
  gbtGamePlayer GetPlayer(int index) const { return m_support->GetPlayer(index); }
  
  int NumOutcomes(void) const { return m_support->NumOutcomes(); }
  gbtGameOutcome GetOutcome(int index) const 
  { return m_support->GetOutcome(index); }

  bool IsConstSum(void) const { return m_support->IsConstSum(); }
  gbtNumber MaxPayoff(int pl = 0) const { return m_support->MaxPayoff(pl); }
  gbtNumber MinPayoff(int pl = 0) const { return m_support->MinPayoff(pl); }

  gbtMixedProfile<double> NewMixedProfile(double) const;
  gbtMixedProfile<gbtRational> NewMixedProfile(const gbtRational &) const;
  gbtMixedProfile<gbtNumber> NewMixedProfile(const gbtNumber &) const;

  // IMPLEMENTATION OF gbtConstNfgRep INTERFACE
  virtual gbtArray<int> NumStrategies(void) const 
  { return m_support->NumStrategies(); }
  virtual int MixedProfileLength(void) const
  { return m_support->MixedProfileLength(); }

  gbtNfgContingency NewContingency(void) const;

  gbtNfgSupport NewNfgSupport(void) const { return m_support->NewNfgSupport(); }
};


#endif  // GAMEBASE_H
