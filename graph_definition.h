#include <mmo/mmo_class.h>
#include <ast/ast_types.h>

#include <boost/graph/adjacency_list.hpp>


#ifndef GRAPH_DEFINITION_2
#define GRAPH_DEFINITION_2


/* E: equations, U: unknowns*/
enum VertexType{E, U};

/*we use Bundled properties*/
struct VertexProperties{
	VertexType type;
	MMO_Equation equation;
	EquationType eqType;
	AST_String variableName;
	AST_ExpressionList unknowns;
	int count; //size of the array or number of equations
};

/*
* genericIndex: if the edge represents an occurrence of the form 
* a[i-1] in some equation, then this list contains the expression: i-1.
* indexes: it contains the number of indexes of a variable (in case it is
* an array) that are used in the equation connected by the edge.
*/

struct EdgeProperties{
	AST_Expression genericIndex; 
	AST_IntegerSet indexes;
};

typedef boost::adjacency_list<boost::listS, boost::listS,
		boost::undirectedS, VertexProperties, EdgeProperties> CausalizationGraph;
typedef CausalizationGraph::vertex_descriptor Vertex;
typedef CausalizationGraph::edge_descriptor Edge;

#endif
