#include <causalize/causalize2/graph_definition.h>
#include <mmo/mmo_class.h>

class CausalizationStrategy{
	public:
		CausalizationStrategy(CausalizationGraph g);			
		void causalize();
		MMO_EquationList getEquations();
	private:
		CausalizationGraph graph;
		MMO_EquationList equations1toN;
		MMO_EquationList equationsNto1;
		AST_Integer equationNumber;
		AST_Integer unknownNumber;
		list<Vertex> *equationDescriptors, *unknownDescriptors;
};
