#include <causalize/causalize2/graph_definition.h>
#include <mmo/mmo_class.h>

class CausalizationStrategy{
	public:
		CausalizationStrategy(CausalizationGraph g, AST_String name, AST_ExpressionList unknowns);			
		void causalize();
		MMO_EquationList getEquations();
	private:
		void remove_edge_from_array(Vertex, Edge);
		void remove_edge_from_array(Edge, map<Edge, Vertex>);
		CausalizationGraph graph;
		MMO_EquationList equations1toN;
		MMO_EquationList equationsNto1;
		AST_Integer equationNumber;
		AST_Integer unknownNumber;
		list<Vertex> *equationDescriptors, *unknownDescriptors;
		AST_ClassList cl;
		AST_String _name;
		AST_ExpressionList _unknowns;
};
