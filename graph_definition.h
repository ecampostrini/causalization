#include <mmo/mmo_class.h>
#include <ast/ast_types.h>

#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/icl/interval_set.hpp>
//#include <boost/icl/discrete_interval.hpp>


#ifndef GRAPH_DEFINITION_
#define GRAPH_DEFINITION_


/* E: equations, U: unknowns*/
enum VertexType{E, U};

/*we use Bundled properties*/
struct VertexProperties{
	VertexType type;
	MMO_Equation equation;
	EquationType eqType;
	string variableName;
	AST_Boolean isState;
	AST_Integer count; //size of the array or number of equations
	AST_Integer index; //for debug purposes
	//these fields are for compatibility with the previous
	//algorithm
	MMO_EquationList  eqs;
	AST_ExpressionList unknowns;
};

/*
* genericIndex: if the edge represents an occurrence of the form 
* a[i-1] in some equation, then this list contains the expression: i-1.
* indexes: it contains the number of indexes of a variable (in case it is
* an array) that are used in the equation connected by the edge.
*/
struct EdgeProperties{
	pair<AST_Integer, AST_Integer> genericIndex;
	boost::icl::interval_set<int> indexInterval;
};

typedef boost::adjacency_list<boost::listS, boost::listS,
		boost::undirectedS, VertexProperties, EdgeProperties> CausalizationGraph;
typedef CausalizationGraph::vertex_descriptor Vertex;
typedef CausalizationGraph::edge_descriptor Edge;

struct CausalizedVar{
	VertexProperties unknown;
	VertexProperties equation;
	EdgeProperties edge;
};

/*Classes for a potential redefinition of the Bundled properties*/

class GenericVertex{
	public:
		GenericVertex(VertexType t);
		VertexType type();
		void setCount(AST_Integer c);
		AST_Integer count();
		/*FALTA HACE LOS CASTEOS DINAMICOS*/
	protected:
		VertexType _type;
		AST_Integer _count;
};

class EquationVertex: public GenericVertex{
	public:
		EquationVertex(MMO_Equation equation);
		EquationType equationType();
	private:
		MMO_Equation equation;
};

class UnknownVertex : public GenericVertex{
	public:
		UnknownVertex(string variableName);
		void setState();
		AST_Boolean isState();
	private:
		AST_Boolean _isState;
};

#endif
