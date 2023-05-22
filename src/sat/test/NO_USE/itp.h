#ifndef ITP_H
#define ITP_H

#include "Proof.h"
#include "Solver.h"
#include "strash.h"
//#include <cstdarg>

//==== ITP ====== [ Abstract class ] ==========================================
//      is    a ProofTraverser
//      has    setType to set the vartype label 
//=============================================================================

class ITP : virtual public ProofTraverser 
{

public:
   static const int VarA = -1 ;
   static const int VarB = -2 ; 
   static const int VarUnknow = -3 ; 

protected:

   vector<AigPtr  >     _subITP;
   bool                 _inv;

public:
   vector<int>          base;

   void set( bool i ){_inv = i; }
   ITP(){}
};

//==== Boolean AigPtr Calculator ==============================================
//    Boolean interface 
//=============================================================================

class AigCal
{
public:
    virtual ~AigCal(){};
    virtual AigPtr AND( const AigPtr & a , const AigPtr & b)= 0 ;
    virtual AigPtr OR( const AigPtr & a , const AigPtr & b){ return ~( AND ( (~a) , (~b)) );}
    virtual AigPtr ITE(const AigPtr & c , const AigPtr & t , const AigPtr & f){ return OR( AND(c,t),AND(~c,f)) ; }
};


//==== PITP ======== [ Abstract class ] =======================================

class PITP : virtual public ITP , virtual public AigCal , virtual public ProofTraverser 
{
public:
   /// constructor
   PITP():ITP(){}
   virtual ~PITP(){}

   /// interpolation usage
   virtual void root(const vec<Lit> &  c , bool A);
   virtual void chain(const vec<ClauseId> & cs , const vec<Lit> & xs );
private:
   AigPtr resolve(AigPtr a ,Lit v,  AigPtr b);

};

//==== MITP ===== [ Abstract class ] ==========================================

class MITP : virtual public ITP , virtual public AigCal , virtual public ProofTraverser 
{
public:
   MITP( ) :ITP(){}
   virtual ~MITP(){}

   virtual void root( const vec<Lit> & c , bool A);
   virtual void chain( const vec<ClauseId> & cs , const vec<Lit> & xs);
private:
   AigPtr resolve( AigPtr a, Lit v , AigPtr b );
};

//==== InterpolantITP =========================================================
//       a interface to wrapping SITP and MutiSITP
//=============================================================================
class InterpolantITP : virtual public ProofTraverser 
{
public :
   virtual AigPtr  interpolant() =0 ; 
};



//==== SITP ====== [] =========================================================
//         is a ITP , AigCal , InterpolantITP 
//         have StrashMgr reference , and a final construct interpolant 
//=============================================================================
class SITP : virtual public ITP , virtual public AigCal , virtual public InterpolantITP 
{
    StrashCkt & _ref;
    AigPtr      _interpolant;

public:

    //--- construct & destruct ---
    SITP( StrashCkt & k ,bool p);
    virtual ~SITP(){}

    //--- concrete func of AigCal ---
    virtual AigPtr AND( const AigPtr & a , const AigPtr & b){return _ref.createAND( a, b);}

    //--- concreate func of ITP ---
    virtual void done();
   
    //--- concrete func of InterpolantITP --- 
    virtual AigPtr   interpolant(){return _interpolant;}

    //--- utilization func ---
    virtual unsigned size();
};

//==== MultiITP ====== [] ======================================================
//         Wrap multi ITP container as to perform online interpolant construction
//=============================================================================
class MultiSITP : virtual public ProofTraverser 
{

    vector<SITP  *> _sitp;
    
public :
   //--- construct & destruct 
    MultiSITP( unsigned a );
    virtual ~MultiSITP();
    void set( unsigned i , SITP * s ){ _sitp[i] = s ;}

//    SITP * itp(unsigned i){ return _sitp[i] ;}
    SITP & operator[]( unsigned i ){ return * (_sitp[i] ); }

    //--- concrete func of ProofTraverser --- 
    virtual void root   (const vec<Lit>& c , bool A);
    virtual void chain  (const vec<ClauseId>& cs, const vec<Lit>& xs);
    virtual void deleted(ClauseId c);
    virtual void done   ();
};


//==== SMITP ====== [ Concrete class ] ========================================
//         is a McMillan construction structure hash circuit 
//       
//         applying the [McMillan] method to construct the [interpolation] to
//         the complete structure hash circuit 
//
//=============================================================================


class SMITP : virtual public SITP , virtual public MITP
{
public:
    virtual ~SMITP(){}
    SMITP( StrashCkt & k , bool p ):SITP(k,p){}
};

//==== SPITP ====== [ Concrete class ] ========================================
//         is a Pudlark construction structure hash circuit 
//       
//         applying the [Pudlark] method to construct the [interpolation] to
//         the complete structure hash circuit 
//
//=============================================================================

class SPITP: virtual public SITP , virtual public PITP
{
public:
    virtual ~SPITP(){}
    SPITP( StrashCkt & k ,  bool p):SITP(k,p){}
};

//==== Checker ================================================================
//
//=============================================================================
struct Checker : virtual public ProofTraverser 
{
    vec< vec<Lit> >  clauses;
    vec< vec< ClauseId > > vcs;
    vec< vec<Lit> > vxs;
    vector< bool > phase;

    int TRA_INT;
    Checker():TRA_INT(0){}
    void root   (const vec<Lit>& c ,bool A);
    void chain  (const vec<ClauseId>& cs, const vec<Lit>& xs) ;
    void deleted(ClauseId c) ;
    void resolve(vec<Lit>& main, vec<Lit>& other, Lit x);
    void traverse( ProofTraverser & ); 
    void done();
};

/******************************************************************************
 * [Synopsis]   help for saving the proof traverser information 
******************************************************************************/
class ITPMgr
{
   protected:
   ProofTraverser * _pft; // internal  
   SITP           * _itp; // construction 
   Proof          * _pf;  // proof 
   AigPtr           _interpolant; // constructed 

   public:
      
   enum CONST_MODE{ ON_THE_FLY , NONE , POST };

   int CC_MIN;// 
   int ITP_CONTAINER;
   int PFT_CONTAINER;
   int ITP_PHASE;
   int MODE;

   
   ITPMgr( ):
      _pft( NULL ), _itp( NULL ),_pf( NULL ),

      CC_MIN(1),    ITP_CONTAINER(3),   PFT_CONTAINER(4),
      ITP_PHASE(0), MODE( ON_THE_FLY )
      {}
   ~ITPMgr();

   ProofTraverser * pfGen( Solver & S, StrashCkt & ckt ); // parameter for _itp 
   AigPtr           itpGen();
};

#endif
