#include <causalize/causalize2/graph_definition.h>
#include <mmo/mmo_class.h>

class CausalizationStrategy{
	public:
		CausalizationStrategy(CausalizationGraph g);			
		void causalize();
		MMO_EquationList getEquations();
	private:
		int remove_edge_from_array(Vertex, Edge);
		CausalizationGraph graph;
		AST_Integer equationNumber;
		AST_Integer unknownNumber;
		list<Vertex> *equationDescriptors, *unknownDescriptors;
		vector<CausalizedVar> equations1toN;
		vector<CausalizedVar> equationsNto1;
		void causalize1toN(Vertex unknown, Vertex equation, Edge e);
		void causalizeNto1(Vertex unknown, Vertex equation, Edge e);
};
