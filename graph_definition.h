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
	string variableName;
	AST_Boolean isState;
	AST_Integer count; //size of the array or number of equations
};


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
