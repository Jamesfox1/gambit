//
// FILE: algfunc.cc -- Solution algorithm functions for GCL
//
// @(#)algfunc.cc	2.16 6/23/97
//

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "rational.h"

#include "gwatch.h"
#include "mixedsol.h"
#include "behavsol.h"
#include "nfg.h"
#include "nfplayer.h"
#include "efg.h"

#include "vertenum.h"


extern Portion *ArrayToList(const gArray<int> &A);
extern Portion *ArrayToList(const gArray<double> &A);
extern Portion *ArrayToList(const gArray<gRational> &A);
extern gVector<double>* ListToVector_Float(ListPortion* list);
extern gVector<gRational>* ListToVector_Rational(ListPortion* list);
extern gMatrix<double>* ListToMatrix_Float(ListPortion* list);
extern gMatrix<gRational>* ListToMatrix_Rational(ListPortion* list);


//
// Useful utilities for creation of lists of profiles
//
template <class T> class Mixed_ListPortion : public ListValPortion   {
  public:
    Mixed_ListPortion(const gList<MixedSolution<T> > &);
    virtual ~Mixed_ListPortion()   { }
};

Mixed_ListPortion<double>::Mixed_ListPortion(const gList<MixedSolution<double> > &list)
{
  _DataType = porMIXED_FLOAT;
  for (int i = 1; i <= list.Length(); i++)
    Append(new MixedPortion<double>(new MixedSolution<double>(list[i])));
}

Mixed_ListPortion<gRational>::Mixed_ListPortion(const gList<MixedSolution<gRational> > &list)
{
  _DataType = porMIXED_RATIONAL;
  for (int i = 1; i <= list.Length(); i++)
    Append(new MixedPortion<gRational>(new MixedSolution<gRational>(list[i])));
}


template <class T> class Behav_ListPortion : public ListValPortion   {
  public:
    Behav_ListPortion(const gList<BehavSolution<T> > &);
    virtual ~Behav_ListPortion()   { }
};

Behav_ListPortion<double>::Behav_ListPortion(
			   const gList<BehavSolution<double> > &list)
{
  _DataType = porBEHAV_FLOAT;
  for (int i = 1; i <= list.Length(); i++)
    Append(new BehavPortion<double>(new BehavSolution<double>(list[i])));
}

Behav_ListPortion<gRational>::Behav_ListPortion(
			      const gList<BehavSolution<gRational> > &list)
{
  _DataType = porBEHAV_RATIONAL;
  for (int i = 1; i <= list.Length(); i++)
    Append(new BehavPortion<gRational>(new BehavSolution<gRational>(list[i])));
}


//-------------
// AgentForm
//-------------

static Portion *GSM_AgentForm_Float(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();
  gWatch watch;

  Nfg<double> *N = MakeAfg(E);
  
  ((FloatPortion *) param[1])->Value() = watch.Elapsed();
  
  if (N)
    return new NfgValPortion<double>(N);
  else
    return new ErrorPortion("Conversion to agent form failed");
}

static Portion *GSM_AgentForm_Rational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();
  gWatch watch;

  Nfg<gRational> *N = MakeAfg(E);
  
  ((FloatPortion *) param[1])->Value() = watch.Elapsed();

  if (N)
    return new NfgValPortion<gRational>(N);
  else
    return new ErrorPortion("Conversion to agent form failed");
}

//------------
// Behav
//------------

static Portion *GSM_Behav_Float(Portion **param)
{
  MixedSolution<double> &mp = * (MixedSolution<double>*) ((MixedPortion<double>*) param[0])->Value();

  Nfg<double> &N = mp.BelongsTo(); 
  const Efg<double> &E = *(const Efg<double> *) N.AssociatedEfg();

  BehavSolution<double> *bp = new BehavSolution<double>(E);
  MixedToBehav(N, mp, E, *bp);

  return new BehavPortion<double>(bp);
}

static Portion *GSM_Behav_Rational(Portion **param)
{
  MixedSolution<gRational> &mp = 
    * (MixedSolution<gRational>*) ((MixedPortion<gRational>*) param[0])->Value();

  Nfg<gRational> &N = mp.BelongsTo(); 
  const Efg<gRational> &E = *(const Efg<gRational> *) N.AssociatedEfg();

  BehavSolution<gRational> *bp = new BehavSolution<gRational>(E);
  MixedToBehav(N, mp, E, *bp);

  return new BehavPortion<gRational>(bp);
}

//------------------
// EnumMixedSolve
//------------------

#include "enum.h"

static Portion *GSM_EnumMixed_Nfg(Portion **param)
{
  NFSupport* S = ((NfSupportPortion*) param[0])->Value();
  NFPayoffs *N = ((NfSupportPortion *) param[0])->PayoffTable();
  Portion* por = 0;

  EnumParams EP;

  EP.stopAfter = ((IntPortion *) param[1])->Value();

  EP.tracefile = &((OutputPortion *) param[4])->Value();
  EP.trace = ((IntPortion *) param[5])->Value();
  
  switch(N->Type())   {
  case gDOUBLE:
    {
      EnumModule<double> EM(* (Nfg<double>*) N, EP, *S);
      EM.Enum();
      ((IntPortion *) param[2])->Value() = EM.NumPivots();
      ((FloatPortion *) param[3])->Value() = EM.Time();
      por = new Mixed_ListPortion<double>(EM.GetSolutions());
    }
    break;
  case gRATIONAL:
    {
      EnumModule<gRational> EM(* (Nfg<gRational>*) N, EP, *S);
      EM.Enum();
      ((IntPortion *) param[2])->Value() = EM.NumPivots();
      ((FloatPortion *) param[3])->Value() = EM.Time();
      por = new Mixed_ListPortion<gRational>(EM.GetSolutions());
    }
    break;
  default:
    assert(0);
  }

  assert(por != 0);
  return por;
}

#include "enumsub.h"


static Portion *GSM_EnumMixed_Efg(Portion **param)
{
  EFSupport &support = *((EfSupportPortion *) param[0])->Value();
  const BaseEfg &E = support.BelongsTo();

  if (!((BoolPortion *) param[1])->Value())
    return new ErrorPortion("algorithm not implemented for extensive forms");

  EnumParams EP;
  EP.stopAfter = ((IntPortion *) param[2])->Value();

  EP.tracefile = &((OutputPortion *) param[5])->Value();
  EP.trace = ((IntPortion *) param[6])->Value();
  
  Portion *por;

  switch (E.Type())  {
    case gDOUBLE:
    {
      EnumBySubgame<double> EM((const Efg<double> &) E, support, EP);
  
      EM.Solve();
      gList<BehavSolution<double> > solns(EM.GetSolutions());

      ((IntPortion *) param[3])->Value() = EM.NumPivots();
      ((FloatPortion *) param[4])->Value() = EM.Time();

      por = new Behav_ListPortion<double>(solns);
      break;
    }
    case gRATIONAL:
    {
      EnumBySubgame<gRational> EM((const Efg<gRational> &) E, support, EP);
  
      EM.Solve();
      gList<BehavSolution<gRational> > solns(EM.GetSolutions());

      ((IntPortion *) param[3])->Value() = EM.NumPivots();
      ((FloatPortion *) param[4])->Value() = EM.Time();

      por = new Behav_ListPortion<gRational>(solns);
      break;
    }
    default:
      assert(0);
  }

  return por;
}


//-----------------
// EnumPureSolve
//-----------------

#include "nfgpure.h"

static Portion *GSM_EnumPure_Nfg(Portion **param)
{
  NFSupport* S = ((NfSupportPortion*) param[0])->Value();
  NFPayoffs* N = ((NfSupportPortion *) param[0])->PayoffTable();
  Portion* por;

  gWatch watch;

  switch(N->Type())
  {
  case gDOUBLE:
    {
      gList<MixedSolution<double> > solns;
      FindPureNash(* (Nfg<double>*) N, *S, solns);
      por = new Mixed_ListPortion<double>(solns);
    }
    break;
  case gRATIONAL:
    {
      gList<MixedSolution<gRational> > solns;
      FindPureNash(* (Nfg<gRational>*) N, *S, solns);
      por = new Mixed_ListPortion<gRational>(solns);
    }
    break;
  default:
    assert(0);
  }

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  return por;
}

#include "efgpure.h"
#include "psnesub.h"

static Portion *GSM_EnumPure_Efg(Portion **param)
{
  EFSupport &support = *((EfSupportPortion *) param[0])->Value();
  const BaseEfg &E = support.BelongsTo();

  Portion *por;
  
  switch (E.Type())  {
    case gDOUBLE:
    {
      gList<BehavSolution<double> > solns;

      if (((BoolPortion *) param[1])->Value())   {
	PureNashBySubgame<double> M((const Efg<double> &) E, support);
	M.Solve();
	solns = M.GetSolutions();
	((FloatPortion *) param[3])->Value() = M.Time();
      }
      else  {
	EfgPSNEBySubgame<double> M((const Efg<double> &) E, support);
	M.Solve();
	solns = M.GetSolutions();
	((FloatPortion *) param[3])->Value() = M.Time();
      }

      por = new Behav_ListPortion<double>(solns);
      break;
    }
    case gRATIONAL:
    {
      gList<BehavSolution<gRational> > solns;

      if (((BoolPortion *) param[1])->Value())   {
	PureNashBySubgame<gRational> M((const Efg<gRational> &) E, support);
	M.Solve();
	solns = M.GetSolutions();
	((FloatPortion *) param[3])->Value() = M.Time();
      }
      else  {
	EfgPSNEBySubgame<gRational> M((const Efg<gRational> &) E, support);
	M.Solve();
	solns = M.GetSolutions();
	((FloatPortion *) param[3])->Value() = M.Time();
      }

      por = new Behav_ListPortion<gRational>(solns);
      break;
    }
    default:
      assert(0);
  }

  return por;
}

//------------------
// GobitGridSolve
//------------------

#include "grid.h"

static Portion *GSM_GobitGrid_Support(Portion **param)
{
  NFSupport& S = * ((NfSupportPortion*) param[0])->Value();
  NFPayoffs* N = ((NfSupportPortion *) param[0])->PayoffTable();
  Portion* por = 0;

  GridParams GP;
  
  if(((TextPortion*) param[1])->Value() != "")
    GP.pxifile = new gFileOutput(((TextPortion*) param[1])->Value());
  else
    GP.pxifile = &gnull;
  GP.minLam = ((FloatPortion *) param[2])->Value();
  GP.maxLam = ((FloatPortion *) param[3])->Value();
  GP.delLam = ((FloatPortion *) param[4])->Value();
  GP.powLam = ((IntPortion *) param[5])->Value();
  GP.delp1 = ((FloatPortion *) param[6])->Value();
  GP.tol1 = ((FloatPortion *) param[7])->Value();
  
  GP.delp2 = ((FloatPortion *) param[8])->Value();
  GP.tol2 = ((FloatPortion *) param[9])->Value();

  GP.multi_grid = 0;
  if(GP.delp2 > 0.0 && GP.tol2 > 0.0)GP.multi_grid = 1;
  
  switch(N->Type())  {
    case gDOUBLE:  {
	GridSolveModule GM(* (Nfg<double>*) N, GP, S);
	GM.GridSolve();
	// ((IntPortion *) param[10])->Value() = GM.NumEvals();
	// ((FloatPortion *) param[11])->Value() = GM.Time();
	gList<MixedSolution<double> > solns;
	por = new Mixed_ListPortion<double>(solns);
	if (GP.pxifile != &gnull)  delete GP.pxifile;
      }
      break;
    case gRATIONAL:
      return new ErrorPortion("The rational version of GobitGridSolve is not implemented");
      break;
    default:
      assert(0);
  }

  assert(por != 0);
  return por;
}

//---------------
// GobitSolve
//---------------

#include "ngobit.h"
#include "egobit.h"

static Portion *GSM_Gobit_Start(Portion **param)
{
  if (param[0]->Spec().Type == porMIXED_FLOAT)  {
    MixedSolution<double> &start = 
      * (MixedSolution<double> *) ((MixedPortion<double> *) param[0])->Value();
    Nfg<double> &N = start.BelongsTo();
  
    NFGobitParams NP;
    if (((TextPortion *) param[1])->Value() != "")
      NP.pxifile = new gFileOutput(((TextPortion *) param[1])->Value());
    else
      NP.pxifile = &gnull;
    NP.minLam = ((FloatPortion *) param[2])->Value();
    NP.maxLam = ((FloatPortion *) param[3])->Value();
    NP.delLam = ((FloatPortion *) param[4])->Value();
    NP.powLam = ((IntPortion *) param[5])->Value();
    NP.fullGraph = ((BoolPortion *) param[6])->Value();

    NP.maxitsN = ((IntPortion *) param[7])->Value();
    NP.tolN = ((FloatPortion *) param[8])->Value();
    NP.maxits1 = ((IntPortion *) param[9])->Value();
    NP.tol1 = ((FloatPortion *) param[10])->Value();
    
    NP.tracefile = &((OutputPortion *) param[14])->Value();
    NP.trace = ((IntPortion *) param[15])->Value();

    gWatch watch;
    gList<MixedSolution<double> > solutions;
    Gobit(N, NP, start, solutions, 
	  ((IntPortion *) param[12])->Value(),
	  ((IntPortion *) param[13])->Value());

    ((FloatPortion *) param[11])->Value() = watch.Elapsed();

    Portion *por = new Mixed_ListPortion<double>(solutions);

    if (NP.pxifile != &gnull)  delete NP.pxifile;
    return por;
  }
  else  {     // BEHAV_FLOAT  
    BehavSolution<double>& start = 
      * (BehavSolution<double> *) ((BehavPortion<double> *) param[0])->Value();
    Efg<double> &E = start.BelongsTo();
  
    EFGobitParams EP;
    if(((TextPortion*) param[1])->Value() != "")
      EP.pxifile = new gFileOutput(((TextPortion*) param[1])->Value());
    else
      EP.pxifile = &gnull;
    EP.minLam = ((FloatPortion *) param[2])->Value();
    EP.maxLam = ((FloatPortion *) param[3])->Value();
    EP.delLam = ((FloatPortion *) param[4])->Value();
    EP.powLam = ((IntPortion *) param[5])->Value();
    EP.fullGraph = ((BoolPortion *) param[6])->Value();
    
    EP.maxitsN = ((IntPortion *) param[7])->Value();
    EP.tolN = ((FloatPortion *) param[8])->Value();
    EP.maxits1 = ((IntPortion *) param[9])->Value();
    EP.tol1 = ((FloatPortion *) param[10])->Value();
  
    EP.tracefile = &((OutputPortion *) param[14])->Value();
    EP.trace = ((IntPortion *) param[15])->Value();
    
    gWatch watch;
    
    gList<BehavSolution<double> > solutions;
    Gobit(E, EP, start, solutions,
	  ((IntPortion *) param[12])->Value(),
	  ((IntPortion *) param[13])->Value());
    
    ((FloatPortion *) param[11])->Value() = watch.Elapsed();
    
    Portion * por = new Behav_ListPortion<double>(solutions);

    if (EP.pxifile != &gnull)   delete EP.pxifile;
    return por;
  }
}


//------------
// LcpSolve
//------------

#include "lemke.h"

static Portion *GSM_Lcp_Nfg(Portion **param)
{
  NFSupport& S = * ((NfSupportPortion*) param[0])->Value();
  NFPayoffs* N = ((NfSupportPortion *) param[0])->PayoffTable();
  Portion* por = 0;

  LemkeParams LP;
  LP.stopAfter = ((IntPortion *) param[1])->Value();

  LP.tracefile = &((OutputPortion *) param[4])->Value();
  LP.trace = ((IntPortion *) param[5])->Value();
  
  switch(N->Type())  {
  case gDOUBLE:
    {
      LemkeModule<double> LS(* (Nfg<double>*) N, LP, S);
      LS.Lemke();
      ((IntPortion *) param[2])->Value() = LS.NumPivots();
      ((FloatPortion *) param[3])->Value() = LS.Time();
      por = new Mixed_ListPortion<double>(LS.GetSolutions());
    }
    break;
  case gRATIONAL:
    {
      LemkeModule<gRational> LS(* (Nfg<gRational>*) N, LP, S);
      LS.Lemke();
      ((IntPortion *) param[2])->Value() = LS.NumPivots();
      ((FloatPortion *) param[3])->Value() = LS.Time();
      por = new Mixed_ListPortion<gRational>(LS.GetSolutions());
    }
    break;
  default:
    assert(0);
  }
  assert(por != 0);
  return por;
}


#include "lemketab.h"

Portion* GSM_Lcp_ListFloat(Portion** param)
{
  gMatrix<double>* a = ListToMatrix_Float((ListPortion*) param[0]);
  gVector<double>* b = ListToVector_Float((ListPortion*) param[1]);
  
  LTableau<double>* tab = new LTableau<double>(*a, *b);
  tab->LemkePath(0);
  gVector<double> vector;
  tab->BasisVector(vector);
  Portion* result = ArrayToList(vector);
  delete tab;
  delete a;
  delete b;
  
  return result;
}

Portion* GSM_Lcp_ListRational(Portion** param)
{
  gMatrix<gRational>* a = ListToMatrix_Rational((ListPortion*) param[0]);
  gVector<gRational>* b = ListToVector_Rational((ListPortion*) param[1]);
  
  LTableau<gRational>* tab = new LTableau<gRational>(*a, *b);
  tab->LemkePath(0);
  gVector<gRational> vector;
  tab->BasisVector(vector);
  Portion* result = ArrayToList(vector);
  delete tab;
  delete a;
  delete b;
  
  return result;
}



#include "seqform.h"
#include "lemkesub.h"

static Portion *GSM_Lcp_Efg(Portion **param)
{
  EFSupport& S = *((EfSupportPortion*) param[0])->Value();
  Portion* por;

  SeqFormParams SP;
  SP.stopAfter = ((IntPortion *) param[2])->Value();  
  SP.tracefile = &((OutputPortion *) param[5])->Value();
  SP.trace = ((IntPortion *) param[6])->Value();

  switch(S.BelongsTo().Type())
  {
  case gDOUBLE:
    {
      // getting E from S.BelongsTo() doesn't work for some reason...
      Efg<double>* E = (Efg<double>*) &S.BelongsTo();
      gList<BehavSolution<double> > solns;
      
      SeqFormModule<double> SM(*E, SP, S);
      SM.Lemke();
      
      solns = SM.GetSolutions();
      por = new Behav_ListPortion<double>(solns);
      
      ((IntPortion *) param[3])->Value() = SM.NumPivots();
      ((FloatPortion *) param[4])->Value() = SM.Time();
    }
    break;
  case gRATIONAL:
    {
      // getting E from S.BelongsTo() doesn't work for some reason...
      Efg<gRational>* E = (Efg<gRational>*) &S.BelongsTo();
      gList<BehavSolution<gRational> > solns;
      
      SeqFormModule<gRational> SM(*E, SP, S);
      SM.Lemke();
      
      solns = SM.GetSolutions();
      por = new Behav_ListPortion<gRational>(solns);
      
      ((IntPortion *) param[3])->Value() = SM.NumPivots();
      ((FloatPortion *) param[4])->Value() = SM.Time();
    }
    break;
  default:
    assert(0);
  }

  return por;
}


//-------------
// LiapSolve
//-------------

#include "liapsub.h"
#include "eliap.h"

static Portion *GSM_Liap_BehavFloat(Portion **param)
{
  BehavSolution<double> &start = 
    * (BehavSolution<double> *) ((BehavPortion<double> *) param[0])->Value();
  Efg<double> &E = start.BelongsTo();
  
  if (((BoolPortion *) param[1])->Value())   {
    NFLiapParams LP;

    LP.stopAfter = ((IntPortion *) param[2])->Value();
    LP.nTries = ((IntPortion *) param[3])->Value();

    LP.maxitsN = ((IntPortion *) param[4])->Value();
    LP.tolN = ((FloatPortion *) param[5])->Value();
    LP.maxits1 = ((IntPortion *) param[6])->Value();
    LP.tol1 = ((FloatPortion *) param[7])->Value();
    
    LP.tracefile = &((OutputPortion *) param[10])->Value();
    LP.trace = ((IntPortion *) param[11])->Value();

    gWatch watch;

    NFLiapBySubgame M(E, LP, start);
    M.Solve();

    ((FloatPortion *) param[7])->Value() = watch.Elapsed();
    ((IntPortion *) param[8])->Value() = M.NumEvals();

    Portion *por = new Behav_ListPortion<double>(M.GetSolutions());
    return por;
  }
  else  {
    EFLiapParams LP;

    LP.stopAfter = ((IntPortion *) param[2])->Value();
    LP.nTries = ((IntPortion *) param[3])->Value();

    LP.maxitsN = ((IntPortion *) param[4])->Value();
    LP.tolN = ((FloatPortion *) param[5])->Value();
    LP.maxits1 = ((IntPortion *) param[6])->Value();
    LP.tol1 = ((FloatPortion *) param[7])->Value();
    
    LP.tracefile = &((OutputPortion *) param[10])->Value();
    LP.trace = ((IntPortion *) param[11])->Value();

    gWatch watch;

    EFLiapBySubgame M(E, LP, start);
    M.Solve();

    ((FloatPortion *) param[7])->Value() = watch.Elapsed();
    ((IntPortion *) param[8])->Value() = M.NumEvals();

    Portion *por = new Behav_ListPortion<double>(M.GetSolutions());
    return por;
  }
}

#include "nliap.h"

static Portion *GSM_Liap_MixedFloat(Portion **param)
{
  MixedSolution<double> &start = 
    * (MixedSolution<double> *) ((MixedPortion<double> *) param[0])->Value();
  Nfg<double> &N = start.BelongsTo();

  NFLiapParams params;
  
  params.stopAfter = ((IntPortion *) param[1])->Value();
  params.nTries = ((IntPortion *) param[2])->Value();

  params.maxitsN = ((IntPortion *) param[3])->Value();
  params.tolN = ((FloatPortion *) param[4])->Value();
  params.maxits1 = ((IntPortion *) param[5])->Value();
  params.tol1 = ((FloatPortion *) param[6])->Value();
 
  params.tracefile = &((OutputPortion *) param[9])->Value();
  params.trace = ((IntPortion *) param[10])->Value();

  long niters;
  gWatch watch;
  gList<MixedSolution<double> > solutions;
  Liap(N, params, start, solutions,
       ((IntPortion *) param[8])->Value(),
       niters);

  ((FloatPortion *) param[7])->Value() = watch.Elapsed();

  Portion *por = new Mixed_ListPortion<double>(solutions);
  return por;
}

//------------
// LpSolve
//------------

#include "nfgcsum.h"

static Portion *GSM_Lp_Nfg(Portion **param)
{
  NFSupport& S = * ((NfSupportPortion*) param[0])->Value();
  NFPayoffs* N = ((NfSupportPortion *) param[0])->PayoffTable();
  Portion* por = 0;

  ZSumParams ZP;

  ZP.tracefile = &((OutputPortion *) param[3])->Value();
  ZP.trace = ((IntPortion *) param[4])->Value();

  switch(N->Type())  {
  case gDOUBLE:
    {
      if (((Nfg<double> *) N)->NumPlayers() > 2 ||
	  !IsConstSum((Nfg<double> &) *N))
	return new ErrorPortion("Only valid for two-person zero-sum games");

      ZSumModule<double> ZM(* (Nfg<double>*) N, ZP, S);
      ZM.ZSum();
      ((IntPortion *) param[1])->Value() = ZM.NumPivots();
      ((FloatPortion *) param[2])->Value() = ZM.Time();
      gList<MixedSolution<double> > solns;
      ZM.GetSolutions(solns);  por = new Mixed_ListPortion<double>(solns);
    }
    break;
  case gRATIONAL:
    {
      if (((Nfg<gRational> *) N)->NumPlayers() > 2 ||
	  !IsConstSum((Nfg<gRational> &) *N))
	return new ErrorPortion("Only valid for two-person zero-sum games");

      ZSumModule<gRational> ZM(*(Nfg<gRational>*) N, ZP, S);
      ZM.ZSum();
      ((IntPortion *) param[1])->Value() = ZM.NumPivots();
      ((FloatPortion *) param[2])->Value() = ZM.Time();
      gList<MixedSolution<gRational> > solns;
      ZM.GetSolutions(solns);  por = new Mixed_ListPortion<gRational>(solns);
    }
    break;
  default:
    assert(0);
  }

  assert(por != 0);
  return por;
}


#include "lpsolve.h"

Portion* GSM_Lp_ListFloat(Portion** param)
{
  gMatrix<double>* a = ListToMatrix_Float((ListPortion*) param[0]);
  gVector<double>* b = ListToVector_Float((ListPortion*) param[1]);
  gVector<double>* c = ListToVector_Float((ListPortion*) param[2]);
  if(!a || !b || !c)
    return 0;
  int nequals = ((IntPortion*) param[3])->Value();
  bool isFeasible;
  bool isBounded;
  
  LPSolve<double>* s = new LPSolve<double>(*a, *b, *c, nequals);
  Portion* result = ArrayToList(s->OptimumVector());
  isFeasible = s->IsFeasible();
  isBounded = s->IsBounded();
  delete s;
  delete a;
  delete b;
  delete c;
  
  ((BoolPortion*) param[4])->Value() = isFeasible;
  ((BoolPortion*) param[5])->Value() = isBounded;
  return result;
}

Portion* GSM_Lp_ListRational(Portion** param)
{
  gMatrix<gRational>* a = ListToMatrix_Rational((ListPortion*) param[0]);
  gVector<gRational>* b = ListToVector_Rational((ListPortion*) param[1]);
  gVector<gRational>* c = ListToVector_Rational((ListPortion*) param[2]);
  if(!a || !b || !c)
    return 0;
  int nequals = ((IntPortion*) param[3])->Value();
  bool isFeasible;
  bool isBounded;
  
  LPSolve<gRational>* s = new LPSolve<gRational>(*a, *b, *c, nequals);
  Portion* result = ArrayToList(s->OptimumVector());
  isFeasible = s->IsFeasible();
  isBounded = s->IsBounded();
  delete s;
  delete a;
  delete b;
  delete c;
  
  ((BoolPortion*) param[4])->Value() = isFeasible;
  ((BoolPortion*) param[5])->Value() = isBounded;
  return result;
}




#include "csumsub.h"
#include "efgcsum.h"

static Portion *GSM_Lp_Efg(Portion **param)
{
  EFSupport &support = *((EfSupportPortion *) param[0])->Value();
  const BaseEfg &E = support.BelongsTo();
  
  if (E.NumPlayers() > 2 || !E.IsConstSum())
    return new ErrorPortion("Only valid for two-person zero-sum games");

  Portion *por;
  
  switch (E.Type())  {
    case gDOUBLE:
    {
      gList<BehavSolution<double> > solns;
  
      if (((BoolPortion *) param[1])->Value())   {
	ZSumParams ZP;

	ZP.tracefile = &((OutputPortion *) param[4])->Value();
	ZP.trace = ((IntPortion *) param[5])->Value();

	ZSumBySubgame<double> ZM((const Efg<double> &) E, support, ZP);

	ZM.Solve();

	solns = ZM.GetSolutions();

	((IntPortion *) param[2])->Value() = ZM.NumPivots();
	((FloatPortion *) param[3])->Value() = ZM.Time();
      }
      else  {
	CSSeqFormParams ZP;

	ZP.tracefile = &((OutputPortion *) param[4])->Value();
	ZP.trace = ((IntPortion *) param[5])->Value();

	CSSeqFormBySubgame<double> ZM((const Efg<double> &) E, support, ZP);

	ZM.Solve();

	solns = ZM.GetSolutions();

	((IntPortion *) param[2])->Value() = ZM.NumPivots();
	((FloatPortion *) param[3])->Value() = ZM.Time();
      }

      por = new Behav_ListPortion<double>(solns);
      break;
    }
    case gRATIONAL:
    {
      gList<BehavSolution<gRational> > solns;
  
      if (((BoolPortion *) param[1])->Value())   {
	ZSumParams ZP;

	ZP.tracefile = &((OutputPortion *) param[4])->Value();
	ZP.trace = ((IntPortion *) param[5])->Value();

	ZSumBySubgame<gRational> ZM((const Efg<gRational> &) E, support, ZP);

	ZM.Solve();

	solns = ZM.GetSolutions();

	((IntPortion *) param[2])->Value() = ZM.NumPivots();
	((FloatPortion *) param[3])->Value() = ZM.Time();
      }
      else  {
	CSSeqFormParams ZP;

	ZP.tracefile = &((OutputPortion *) param[4])->Value();
	ZP.trace = ((IntPortion *) param[5])->Value();

	CSSeqFormBySubgame<gRational> ZM((const Efg<gRational> &) E, support, ZP);

	ZM.Solve();

	solns = ZM.GetSolutions();

	((IntPortion *) param[2])->Value() = ZM.NumPivots();
	((FloatPortion *) param[3])->Value() = ZM.Time();
      }

      por = new Behav_ListPortion<gRational>(solns);
      break;
    } 
    default:
      assert(0); 
  }

  return por;
}


//---------
// Nfg
//---------

static Portion *GSM_Nfg_Float(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();
  gWatch watch;

  Nfg<double> *N = MakeReducedNfg(E, EFSupport(E));
  
  ((FloatPortion *) param[1])->Value() = watch.Elapsed();
  
  if (N)
    return new NfgValPortion<double>(N);
  else
    return new ErrorPortion("Conversion to reduced nfg failed");
}

static Portion *GSM_Nfg_Rational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();
  gWatch watch;

  Nfg<gRational> *N = MakeReducedNfg(E, EFSupport(E));
  
  ((FloatPortion *) param[1])->Value() = watch.Elapsed();

  if (N)
    return new NfgValPortion<gRational>(N);
  else
    return new ErrorPortion("Conversion to reduced nfg failed");
}


//----------
// Payoff
//----------

Portion* GSM_Payoff_BehavFloat(Portion** param)
{
  BehavSolution<double>* bp = 
    (BehavSolution<double>*) ((BehavPortion<double>*) param[0])->Value();
  EFPlayer *player = ((EfPlayerPortion *) param[1])->Value();

  return new FloatPortion(bp->Payoff(player->GetNumber()));
}

Portion* GSM_Payoff_BehavRational(Portion** param)
{
  BehavSolution<gRational>* bp = 
    (BehavSolution<gRational>*) ((BehavPortion<gRational>*) param[0])->Value();
  EFPlayer *player = ((EfPlayerPortion *) param[1])->Value();

  return new RationalPortion(bp->Payoff(player->GetNumber()));
}

Portion* GSM_Payoff_MixedFloat(Portion** param)
{
  MixedSolution<double>* mp = 
    (MixedSolution<double>*) ((MixedPortion<double>*) param[0])->Value();
  NFPlayer *player = ((NfPlayerPortion *) param[1])->Value();

  return new FloatPortion(mp->Payoff(player->GetNumber()));
}

Portion* GSM_Payoff_MixedRational(Portion** param)
{
  MixedSolution<gRational>* mp = 
    (MixedSolution<gRational>*) ((MixedPortion<gRational>*) param[0])->Value();
  NFPlayer *player = ((NfPlayerPortion *) param[1])->Value();

  return new RationalPortion(mp->Payoff(player->GetNumber()));
}

//----------------
// SimpDivSolve
//----------------

#include "simpdiv.h"

static Portion *GSM_Simpdiv_Nfg(Portion **param)
{
  NFSupport& S = * ((NfSupportPortion*) param[0])->Value();
  NFPayoffs* N = ((NfSupportPortion *) param[0])->PayoffTable();
  Portion* por = 0;

  SimpdivParams SP;
  SP.stopAfter = ((IntPortion *) param[1])->Value();
  SP.nRestarts = ((IntPortion *) param[2])->Value();
  SP.leashLength = ((IntPortion *) param[3])->Value();

  SP.tracefile = &((OutputPortion *) param[6])->Value();
  SP.trace = ((IntPortion *) param[7])->Value();

  switch(N->Type())
  {
  case gDOUBLE:
    {
      SimpdivModule<double> SM(* (Nfg<double>*) N, SP, S);
      SM.Simpdiv();
      ((IntPortion *) param[4])->Value() = SM.NumEvals();
      ((FloatPortion *) param[5])->Value() = SM.Time();
      por = new Mixed_ListPortion<double>(SM.GetSolutions());
    }
    break;
  case gRATIONAL:
    {
      SimpdivModule<gRational> SM(* (Nfg<gRational>*) N, SP, S);
      SM.Simpdiv();
      ((IntPortion *) param[4])->Value() = SM.NumEvals();
      ((FloatPortion *) param[5])->Value() = SM.Time();
      por = new Mixed_ListPortion<gRational>(SM.GetSolutions());
    }
    break;
  default:
    assert(0);
  }

  assert(por != 0);
  return por;
}

#include "simpsub.h"

static Portion *GSM_Simpdiv_Efg(Portion **param)
{
  EFSupport &support = *((EfSupportPortion *) param[0])->Value();
  const BaseEfg &E = support.BelongsTo();
  
  if (!((BoolPortion *) param[1])->Value())  
    return new ErrorPortion("algorithm not implemented for extensive forms");

  SimpdivParams SP;
  SP.stopAfter = ((IntPortion *) param[2])->Value();
  SP.nRestarts = ((IntPortion *) param[3])->Value();
  SP.leashLength = ((IntPortion *) param[4])->Value();

  SP.tracefile = &((OutputPortion *) param[7])->Value();
  SP.trace = ((IntPortion *) param[8])->Value();

  Portion *por;
  
  switch (E.Type())  {
    case gDOUBLE:
    {
      SimpdivBySubgame<double> SM((const Efg<double> &) E, support, SP);
      SM.Solve();

      ((IntPortion *) param[5])->Value() = SM.NumEvals();
      ((FloatPortion *) param[6])->Value() = SM.Time();
  
      por = new Behav_ListPortion<double>(SM.GetSolutions());
      break;
    }
    case gRATIONAL:
    {
      SimpdivBySubgame<gRational> SM((const Efg<gRational> &) E, support, SP);
      SM.Solve();

      ((IntPortion *) param[5])->Value() = SM.NumEvals();
      ((FloatPortion *) param[6])->Value() = SM.Time();
  
      por = new Behav_ListPortion<gRational>(SM.GetSolutions());
      break;
    }
    default:
      assert(0);
  }

  return por;
}




Portion* GSM_VertEnum_Float( Portion** param )
{
  gMatrix<double>* A = ListToMatrix_Float((ListPortion*) param[0]);
  gVector<double>* b = ListToVector_Float((ListPortion*) param[1]);
  gVector<double>* start = ListToVector_Float((ListPortion*) param[2]);

  VertEnum< double >* vertenum = NULL;
  if( start->Length() == 0 )
    vertenum = new VertEnum< double >( *A, *b );
  else 
    /*vertenum = new VertEnum< double >( *A, *b, *start );*/
    gout << "Please use NewVertEnum for this purpose.\n\n";
  gList< gVector< double > > verts;
  vertenum->Vertices( verts );

  delete A;
  delete b;
  delete start;
  delete vertenum;

  ListPortion* list = new ListValPortion();
  int i = 0;
  for( i = 1; i <= verts.Length(); ++i )
  {
    list->Append( ArrayToList( verts[i] ) );
  }

  return list;
}


Portion* GSM_VertEnum_Rational( Portion** param )
{
  gMatrix<gRational>* A = ListToMatrix_Rational((ListPortion*) param[0]);
  gVector<gRational>* b = ListToVector_Rational((ListPortion*) param[1]);
  gVector<gRational>* start = ListToVector_Rational((ListPortion*) param[2]);

  VertEnum< gRational >* vertenum = NULL;
  if( start->Length() == 0 )
    vertenum = new VertEnum< gRational >( *A, *b );
  else 
    /*vertenum = new VertEnum< gRational >( *A, *b, *start );*/
    gout << "Please use NewVertEnum for this purpose.\n\n";

  gList< gVector< gRational > > verts;
  vertenum->Vertices( verts );

  delete A;
  delete b;
  delete start;
  delete vertenum;

  ListPortion* list = new ListValPortion();
  int i = 0;
  for( i = 1; i <= verts.Length(); ++i )
  {
    list->Append( ArrayToList( verts[i] ) );
  }

  return list;
}




Portion* GSM_NewVertEnum_Float( Portion** param )
{
  gMatrix<double>* A = ListToMatrix_Float((ListPortion*) param[0]);
  gVector<double>* b = ListToVector_Float((ListPortion*) param[1]);
  gVector<double>* start = ListToVector_Float((ListPortion*) param[2]);

  NewVertEnum< double >* vertenum = NULL;
  if( start->Length() == 0 )
    /*vertenum = new NewVertEnum< double >( *A, *b );*/
    gout << "Please use VertEnum for this.\n\n";
  else 
    vertenum = new NewVertEnum< double >( *A, *b, *start );

  gList< gVector< double > > verts;
  vertenum->Vertices( verts );

  delete A;
  delete b;
  delete start;
  delete vertenum;

  ListPortion* list = new ListValPortion();
  int i = 0;
  for( i = 1; i <= verts.Length(); ++i )
  {
    list->Append( ArrayToList( verts[i] ) );
  }

  return list;
}


Portion* GSM_NewVertEnum_Rational( Portion** param )
{
  gMatrix<gRational>* A = ListToMatrix_Rational((ListPortion*) param[0]);
  gVector<gRational>* b = ListToVector_Rational((ListPortion*) param[1]);
  gVector<gRational>* start = ListToVector_Rational((ListPortion*) param[2]);

  NewVertEnum< gRational >* vertenum = NULL;
  if( start->Length() == 0 )
    /*vertenum = new NewVertEnum< gRational >( *A, *b );*/
    gout << "Please use VertEnum for this.\n\n";
  else 
    vertenum = new NewVertEnum< gRational >( *A, *b, *start );

  gList< gVector< gRational > > verts;
  vertenum->Vertices( verts );

  delete A;
  delete b;
  delete start;
  delete vertenum;

  ListPortion* list = new ListValPortion();
  int i = 0;
  for( i = 1; i <= verts.Length(); ++i )
  {
    list->Append( ArrayToList( verts[i] ) );
  }

  return list;
}



void Init_algfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("AgentForm", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_AgentForm_Float, porNFG_FLOAT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0), BYREF));
  
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_AgentForm_Rational, porNFG_RATIONAL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("efg", porEFG_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0), BYREF));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Behav", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Behav_Float, porBEHAV_FLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("mixed", porMIXED_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Behav_Rational, 
				       porBEHAV_RATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("mixed", porMIXED_RATIONAL));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("EnumMixedSolve", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_EnumMixed_Nfg, 
				       PortionSpec(porMIXED, 1), 6));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("stopAfter", porINTEGER,
					    new IntPortion(0)));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("nPivots", porINTEGER,
					    new IntPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 4, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(0, 5, ParamInfoType("traceLevel", porINTEGER,
					    new IntPortion(0)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_EnumMixed_Efg, 
				       PortionSpec(porBEHAV, 1), 7));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("support", porEFSUPPORT));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("asNfg", porBOOL,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("stopAfter", porINTEGER,
					    new IntPortion(0)));
  FuncObj->SetParamInfo(1, 3, ParamInfoType("nPivots", porINTEGER,
					    new IntPortion(0), BYREF));
  FuncObj->SetParamInfo(1, 4, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0.0), BYREF));
  FuncObj->SetParamInfo(1, 5, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(1, 6, ParamInfoType("traceLevel", porINTEGER,
					    new IntPortion(0)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("EnumPureSolve", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_EnumPure_Nfg, 
				       PortionSpec(porMIXED, 1), 5));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("stopAfter", porINTEGER,
					    new IntPortion(0)));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(0, 4, ParamInfoType("traceLevel", porINTEGER,
					    new IntPortion(0)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_EnumPure_Efg, 
				       PortionSpec(porBEHAV, 1), 6));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("support", porEFSUPPORT));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("asNfg", porBOOL,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("stopAfter", porINTEGER,
					    new IntPortion(0)));
  FuncObj->SetParamInfo(1, 3, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0.0), BYREF));
  FuncObj->SetParamInfo(1, 4, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(1, 5, ParamInfoType("traceLevel", porINTEGER,
					    new IntPortion(0)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("GobitGridSolve", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_GobitGrid_Support, 
				       PortionSpec(porMIXED, 1), 14));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("pxifile", porTEXT,
					    new TextPortion("")));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("minLam", porFLOAT,
					    new FloatPortion(0.001)));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("maxLam", porFLOAT,
					    new FloatPortion(500.0)));
  FuncObj->SetParamInfo(0, 4, ParamInfoType("delLam", porFLOAT,
					    new FloatPortion(0.02)));
  FuncObj->SetParamInfo(0, 5, ParamInfoType("powLam", porINTEGER,
					    new IntPortion(1)));
  FuncObj->SetParamInfo(0, 6, ParamInfoType("delp1", porFLOAT,
					    new FloatPortion(.1)));
  FuncObj->SetParamInfo(0, 7, ParamInfoType("tol1", porFLOAT,
					    new FloatPortion(.1)));
  FuncObj->SetParamInfo(0, 8, ParamInfoType("delp2", porFLOAT,
					    new FloatPortion(.01)));
  FuncObj->SetParamInfo(0, 9, ParamInfoType("tol2", porFLOAT,
					    new FloatPortion(.01)));
  FuncObj->SetParamInfo(0, 10, ParamInfoType("nEvals", porINTEGER,
					    new IntPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 11, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 12, ParamInfoType("traceFile", porOUTPUT,
					     new OutputRefPortion(gnull),
					     BYREF));
  FuncObj->SetParamInfo(0, 13, ParamInfoType("traceLevel", porINTEGER,
					     new IntPortion(0)));

  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("GobitSolve", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Gobit_Start, 
				       PortionSpec(porMIXED_FLOAT |
						   porBEHAV_FLOAT, 1), 16));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("start",
					    porMIXED_FLOAT | porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("pxifile", porTEXT,
					    new TextPortion("")));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("minLam", porFLOAT,
					    new FloatPortion(0.001)));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("maxLam", porFLOAT,
					    new FloatPortion(500.0)));
  FuncObj->SetParamInfo(0, 4, ParamInfoType("delLam", porFLOAT,
					    new FloatPortion(0.02)));
  FuncObj->SetParamInfo(0, 5, ParamInfoType("powLam", porINTEGER,
					    new IntPortion(1)));
  FuncObj->SetParamInfo(0, 6, ParamInfoType("fullGraph", porBOOL,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 7, ParamInfoType("maxitsN", porINTEGER,
					    new IntPortion(20)));
  FuncObj->SetParamInfo(0, 8, ParamInfoType("tolN", porFLOAT,
					    new FloatPortion(1.0e-10)));
  FuncObj->SetParamInfo(0, 9, ParamInfoType("maxits1", porINTEGER,
					    new IntPortion(100)));
  FuncObj->SetParamInfo(0, 10, ParamInfoType("tol1", porFLOAT,
					     new FloatPortion(2.0e-10)));
  FuncObj->SetParamInfo(0, 11, ParamInfoType("time", porFLOAT,
					     new FloatPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 12, ParamInfoType("nEvals", porINTEGER,
					     new IntPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 13, ParamInfoType("nIters", porINTEGER,
					     new IntPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 14, ParamInfoType("traceFile", porOUTPUT,
					     new OutputRefPortion(gnull), 
					     BYREF));
  FuncObj->SetParamInfo(0, 15, ParamInfoType("traceLevel", porINTEGER,
					     new IntPortion(0)));

  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("LcpSolve", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Lcp_Nfg, 
				       PortionSpec(porMIXED, 1), 6));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("stopAfter", porINTEGER, 
					    new IntPortion(0)));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("nPivots", porINTEGER, 
					    new IntPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("time", porFLOAT, 
					    new FloatPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 4, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(0, 5, ParamInfoType("traceLevel", porINTEGER,
					    new IntPortion(0)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Lcp_ListFloat, 
				       PortionSpec(porFLOAT, 1), 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("a", PortionSpec(porFLOAT,2),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("b", PortionSpec(porFLOAT,1),
					    REQUIRED, BYVAL));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Lcp_ListRational, 
				       PortionSpec(porRATIONAL, 1), 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("a", PortionSpec(porRATIONAL,2),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("b", PortionSpec(porRATIONAL,1),
					    REQUIRED, BYVAL));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Lcp_Efg, 
				       PortionSpec(porBEHAV, 1), 7));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("support", porEFSUPPORT));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("asNfg", porBOOL,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(3, 2, ParamInfoType("stopAfter", porINTEGER,
					    new IntPortion(0)));
  FuncObj->SetParamInfo(3, 3, ParamInfoType("nPivots", porINTEGER,
					    new IntPortion(0), BYREF));
  FuncObj->SetParamInfo(3, 4, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0.0), BYREF));
  FuncObj->SetParamInfo(3, 5, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(3, 6, ParamInfoType("traceLevel", porINTEGER,
					    new IntPortion(0)));
  
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("LiapSolve", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Liap_BehavFloat, 
				       PortionSpec(porBEHAV_FLOAT, 1), 12));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("start", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("asNfg", porBOOL,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("stopAfter", porINTEGER,
					    new IntPortion(1)));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("nTries", porINTEGER,
					    new IntPortion(10)));
  FuncObj->SetParamInfo(0, 4, ParamInfoType("maxitsN", porINTEGER,
					    new IntPortion(20)));
  FuncObj->SetParamInfo(0, 5, ParamInfoType("tolN", porFLOAT,
					    new FloatPortion(1.0e-10)));
  FuncObj->SetParamInfo(0, 6, ParamInfoType("maxits1", porINTEGER,
					    new IntPortion(100)));
  FuncObj->SetParamInfo(0, 7, ParamInfoType("tol1", porFLOAT,
					    new FloatPortion(2.0e-10)));
  FuncObj->SetParamInfo(0, 8, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 9, ParamInfoType("nEvals", porINTEGER,
					    new IntPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 10, ParamInfoType("traceFile", porOUTPUT,
					     new OutputRefPortion(gnull), 
					     BYREF));
  FuncObj->SetParamInfo(0, 11, ParamInfoType("traceLevel", porINTEGER,
					     new IntPortion(0)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Liap_MixedFloat, 
				       PortionSpec(porMIXED_FLOAT, 1), 11));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("start", porMIXED_FLOAT));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("stopAfter", porINTEGER,
					    new IntPortion(1)));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("nTries", porINTEGER,
					    new IntPortion(10)));
  FuncObj->SetParamInfo(1, 3, ParamInfoType("maxitsN", porINTEGER,
					    new IntPortion(20)));
  FuncObj->SetParamInfo(1, 4, ParamInfoType("tolN", porFLOAT,
					    new FloatPortion(1.0e-10)));
  FuncObj->SetParamInfo(1, 5, ParamInfoType("maxits1", porINTEGER,
					    new IntPortion(100)));
  FuncObj->SetParamInfo(1, 6, ParamInfoType("tol1", porFLOAT,
					    new FloatPortion(2.0e-10)));
  FuncObj->SetParamInfo(1, 7, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0), BYREF));
  FuncObj->SetParamInfo(1, 8, ParamInfoType("nEvals", porINTEGER,
					    new IntPortion(0), BYREF));
  FuncObj->SetParamInfo(1, 9, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(1, 10, ParamInfoType("traceLevel", porINTEGER,
					     new IntPortion(0)));

  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("LpSolve", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Lp_Nfg, 
				       PortionSpec(porMIXED, 1), 5));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("nPivots", porINTEGER,
					    new IntPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull),
					    BYREF));
  FuncObj->SetParamInfo(0, 4, ParamInfoType("traceLevel", porINTEGER,
					    new IntPortion(0)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Lp_Efg, 
				       PortionSpec(porBEHAV, 1), 6));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("support", porEFSUPPORT));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("asNfg", porBOOL,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("nPivots", porINTEGER,
					    new IntPortion(0), BYREF));
  FuncObj->SetParamInfo(1, 3, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0.0), BYREF));
  FuncObj->SetParamInfo(1, 4, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(1, 5, ParamInfoType("traceLevel", porINTEGER,
					    new IntPortion(0)));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Lp_ListFloat, 
				       PortionSpec(porFLOAT, 1), 6));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("a", PortionSpec(porFLOAT,2),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("b", PortionSpec(porFLOAT,1),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(2, 2, ParamInfoType("c", PortionSpec(porFLOAT,1),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(2, 3, ParamInfoType("nEqualities", porINTEGER));
  FuncObj->SetParamInfo(2, 4, ParamInfoType("isFeasible", porBOOL,
					    new BoolPortion(false), BYREF));
  FuncObj->SetParamInfo(2, 5, ParamInfoType("isBounded", porBOOL,
					    new BoolPortion(false), BYREF));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Lp_ListRational, 
				       PortionSpec(porRATIONAL, 1), 6));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("a", PortionSpec(porRATIONAL,2),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("b", PortionSpec(porRATIONAL,1),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(3, 2, ParamInfoType("c", PortionSpec(porRATIONAL,1),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(3, 3, ParamInfoType("nEqualities", porINTEGER));
  FuncObj->SetParamInfo(3, 4, ParamInfoType("isFeasible", porBOOL,
					    new BoolPortion(false), BYREF));
  FuncObj->SetParamInfo(3, 5, ParamInfoType("isBounded", porBOOL,
					    new BoolPortion(false), BYREF));

  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Nfg", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Nfg_Float, porNFG_FLOAT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0), BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Nfg_Rational, porNFG_RATIONAL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("efg", porEFG_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0), BYREF));
  gsm->AddFunction(FuncObj);




  FuncObj = new FuncDescObj("Payoff", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Payoff_BehavFloat, porFLOAT, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porEFPLAYER));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Payoff_BehavRational,
				       porRATIONAL, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("player", porEFPLAYER));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Payoff_MixedFloat, porFLOAT, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("profile", porMIXED_FLOAT));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("player", porNFPLAYER));
  
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Payoff_MixedRational, 
				       porRATIONAL, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("profile", porMIXED_RATIONAL));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("player", porNFPLAYER));

  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("SimpDivSolve", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Simpdiv_Nfg, 
				       PortionSpec(porMIXED, 1), 8));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("stopAfter", porINTEGER,
					    new IntPortion(1)));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("nRestarts", porINTEGER,
					    new IntPortion(1)));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("leashLength", porINTEGER,
					    new IntPortion(0)));
  FuncObj->SetParamInfo(0, 4, ParamInfoType("nEvals", porINTEGER,
					    new IntPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 5, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 6, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(0, 7, ParamInfoType("traceLevel", porINTEGER,
					    new IntPortion(0)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Simpdiv_Efg, 
				       PortionSpec(porBEHAV, 1), 9));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("support", porEFSUPPORT));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("asNfg", porBOOL,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("stopAfter", porINTEGER,
					    new IntPortion(1)));
  FuncObj->SetParamInfo(1, 3, ParamInfoType("nRestarts", porINTEGER,
					    new IntPortion(1)));
  FuncObj->SetParamInfo(1, 4, ParamInfoType("leashLength", porINTEGER,
					    new IntPortion(0)));
  FuncObj->SetParamInfo(1, 5, ParamInfoType("nEvals", porINTEGER,
					    new IntPortion(0), BYREF));
  FuncObj->SetParamInfo(1, 6, ParamInfoType("time", porFLOAT,
					    new FloatPortion(0.0), BYREF));
  FuncObj->SetParamInfo(1, 7, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(1, 8, ParamInfoType("traceLevel", porINTEGER,
					    new IntPortion(0)));
  gsm->AddFunction(FuncObj);




  FuncObj = new FuncDescObj("VertEnum", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_VertEnum_Float, 
				       PortionSpec(porFLOAT, 2), 3));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("A", PortionSpec(porFLOAT,2),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("b", PortionSpec(porFLOAT,1),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("start", PortionSpec(porFLOAT,1),
					    new ListValPortion(), BYVAL));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_VertEnum_Rational, 
				       PortionSpec(porRATIONAL, 2), 3));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("A", PortionSpec(porRATIONAL,2),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("b", PortionSpec(porRATIONAL,1),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("start", PortionSpec(porRATIONAL,1),
					    new ListValPortion(), BYVAL));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("NewVertEnum", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewVertEnum_Float, 
				       PortionSpec(porFLOAT, 2), 3));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("A", PortionSpec(porFLOAT,2),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("b", PortionSpec(porFLOAT,1),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("start", PortionSpec(porFLOAT,1),
					    new ListValPortion(), BYVAL));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_NewVertEnum_Rational, 
				       PortionSpec(porRATIONAL, 2), 3));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("A", PortionSpec(porRATIONAL,2),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("b", PortionSpec(porRATIONAL,1),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("start", PortionSpec(porRATIONAL,1),
					    new ListValPortion(), BYVAL));
  gsm->AddFunction(FuncObj);

}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class Mixed_ListPortion<double>;
TEMPLATE class Mixed_ListPortion<gRational>;

TEMPLATE class Behav_ListPortion<double>;
TEMPLATE class Behav_ListPortion<gRational>;


